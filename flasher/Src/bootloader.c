#include "common.h"
#include "bootloader_opt.h"
#include "uart.h"

//#define _BSD_SOURCE

static BOOT_CMD_ALLOW_T cmd_set[BOOT_CMD_IDX_MAX]={
    { CMD_ALLOWED,      BOOT_CMD_GET                },
    { CMD_RESTRICTED,   BOOT_CMD_GET_VERSION        },
    { CMD_RESTRICTED,   BOOT_CMD_GET_ID             },
    { CMD_RESTRICTED,   BOOT_CMD_READ_MEMORY        },
    { CMD_RESTRICTED,   BOOT_CMD_GO                 },
    { CMD_RESTRICTED,   BOOT_CMD_WRITE_MEMORY       },
    { CMD_RESTRICTED,   BOOT_CMD_ERASE              },
    { CMD_RESTRICTED,   BOOT_CMD_EXTENDED_ERASE     },
    { CMD_RESTRICTED,   BOOT_CMD_SPECIAL            },
    { CMD_RESTRICTED,   BOOT_CMD_EXTENDED_SPECIAL   },
    { CMD_RESTRICTED,   BOOT_CMD_WRITE_PROTECT      },
    { CMD_RESTRICTED,   BOOT_CMD_WRITE_UNPROTECT    },
    { CMD_RESTRICTED,   BOOT_CMD_READOUT_PROTECT    },
    { CMD_RESTRICTED,   BOOT_CMD_READOUT_UNPROTECT  }
};

static uint8_t boot_version = 0;

static uint8_t rx_buff[BUFF_SIZE];
static uint8_t tx_buff[BUFF_SIZE];

//-----------------------------------------------
__attribute__ ( ( __always_inline__) ) void delay ( useconds_t usec )
{
    if ( usec ) 
        usleep ( usec );
}
//-----------------------------------------------
 uint8_t summ ( uint8_t * buff, int len )
{
    uint8_t sum = BOOT_CMD_CHKSUMM;

    while ( len-- )
        sum ^= *buff++;
        
    return sum;
}
//-----------------------------------------------
static int ack ( useconds_t wait_us )
{
    uint8_t c;

    delay ( wait_us );     
    return ( rx_byte ( &c ) == 1 && c == BOOT_CMD_ACK ) ? 1 : 0; 
}
//-----------------------------------------------
static void flush_rx ( useconds_t wait_us )
{
    uint8_t c;

    do{
        delay ( wait_us );
    }
    while ( rx_byte ( &c ) );
}
//-----------------------------------------------
static int cmd_answer ( uint8_t * buff, int buff_size, useconds_t wait_us )
{
    int retval = 0, len = 0;
    
    if ( ack ( wait_us ) )
    {
        do{
            delay ( wait_us );
            if ( !rx_byte ( buff ) )
                break;
            if ( *buff == BOOT_CMD_ACK )
            {
                retval = len;
                break;
            }
            len++;
            buff++;
            if ( len >= buff_size )
                break;
        }
        while ( 1 );    
    }
    return retval;

}
//-----------------------------------------------
static int send_cmd ( uint8_t cmd )
{
    assert ( cmd < BOOT_CMD_IDX_MAX );
    int retval = 0;

    if ( cmd_set[cmd].allow == CMD_ALLOWED )
    { 
        tx_buff[0] = cmd_set[cmd].cmd;
        tx_buff[1] = summ ( tx_buff, 1 );
        retval = tx_uart ( tx_buff, 2 ) == 2 ? 1 : 0;
    }
    return  retval;
}
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
static int boot_get ( )
{
    int len, retval = 0, cmd_set_idx, ans_cmd_idx;
    BOOT_CMD_GET_ANSWER_T * ans = ( BOOT_CMD_GET_ANSWER_T * )rx_buff;

    do{
        if ( !send_cmd ( BOOT_CMD_GET_IDX ) )
            break;
        
        len = cmd_answer ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY );
#ifdef DEBUG        
        hex ( tx_buff, 2 );
        hex ( rx_buff, len );
#endif
        if ( len < 3 || 
             !ans->ver || 
             ans->n != ( len - 2 ) )
            break;

        boot_version = ans->ver;
        //clear cmd set
        for ( cmd_set_idx = ( BOOT_CMD_GET_IDX + 1); cmd_set_idx < BOOT_CMD_IDX_MAX; cmd_set_idx++ )
            cmd_set[cmd_set_idx].allow = CMD_RESTRICTED;
        
        //regitrate commands set
        for ( ans_cmd_idx = 0; ans_cmd_idx < ans->n; ans_cmd_idx++ )
            for ( cmd_set_idx = ( BOOT_CMD_GET_IDX + 1); cmd_set_idx < BOOT_CMD_IDX_MAX; cmd_set_idx++ )
                if ( cmd_set[cmd_set_idx].cmd == ans->cmd[ans_cmd_idx] )
                {
                    cmd_set[cmd_set_idx].allow = CMD_ALLOWED;
                    retval ++;
                    break;
                }
    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_get_version ( )
{
    int len, retval = 0;

    do{
        if ( !send_cmd ( BOOT_CMD_GET_VERSION_IDX ) )
            break;
        
        len = cmd_answer ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY );
#ifdef DEBUG        
        hex ( tx_buff, 2 );
        hex ( rx_buff, len );
#endif
        if ( len  )
            retval = rx_buff[0];

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_get_id ( )
{
    int len, retval = 0;

    do{
        if ( !send_cmd ( BOOT_CMD_GET_ID_IDX ) )
            break;
        
        len = cmd_answer ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY );
#ifdef DEBUG        
        hex ( tx_buff, 2 );
        hex ( rx_buff, len );
#endif
        if ( len != 3 || 
            rx_buff[0] != 1 ||
            rx_buff[1] != 4 )
            break;

        retval = rx_buff[2];

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
static int boot_connecting ( )
{
    int retval = 0;
    uint8_t chip_id = 0;
    do{
        flush_rx ( 1 );
        if ( !boot_get ( ) )
            break;
        if ( boot_version != boot_get_version ( ) )
            break;
        chip_id = boot_get_id ( );
        if ( !chip_id )
            break;

        retval = chip_id;

    } while ( 0 );
    return retval;
}
//-----------------------------------------------

int boot_start ( char * port )
{
    uint8_t chip_id = 0;
    if ( !open_uart (port , BOOT_UART_SPEED ) )
    {
        close_uart();
        return 0;
    }
    msg("\tWaiting for connection...\n");
    while ( 1 )
    {
        tx_byte ( BOOT_CMD_HELLO );
        if ( ack ( 1 ) ){
            chip_id = boot_connecting ( );
            if ( chip_id )
                break;
        }
        delay ( BOOT_HELLO_TICK ); 

    }

    msg ( "\tDetected STM32 chip with ID 0x%.2X.\n", chip_id );
    msg ( "\tBootloader v%d.%d\n", boot_version >> 4, boot_version & 0xf );
    
    close_uart();
    return 1;
}