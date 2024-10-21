#include "common.h"
#include "bootloader_def.h"
#include "bootloader_opt.h"
#include "uart.h"

//#define _BSD_SOURCE

#define SEND_CMD(c)             if ( ( retval = send_cmd ( c ) ) != OK ) break;
#define CMD_ANSWER(c,s,d)       if ( ( retval = cmd_answer ( c,s,d ) ) < OK ) break;
#define CHECK_ANSWER(rv,rm) {   retval = get_ack ( BOOT_RX_DELAY ); \
                                if ( retval != BOOT_CMD_ACK ){      \
                                    if ( retval == BOOT_CMD_NACK ){ \
                                        retval = rv;                \
                                        dbg(rm);                    \
                                    }                               \
                                    break;                          \
                                    }                                   \
                                }

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
static uint8_t get_ack ( useconds_t wait_us )
{
    uint8_t retval = ERROR;

    delay ( wait_us ); 
    do{
        if ( !rx_byte ( &retval ) ){
            retval = ERR_RX_FAIL;
            break;
        }
        switch ( retval ){
            case BOOT_CMD_ACK:
                retval = OK;
                break;
            case BOOT_CMD_NACK:
                retval = ERR_NACK;
                break;
            default:
                break;
        }
    }while ( 0 );
    
    return retval; 
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
    int retval;
    
    retval = get_ack ( wait_us );
    if ( retval == OK )
    {
        do{
            delay ( wait_us );
            if ( !rx_byte ( buff ) ){
                retval = ERR_RX_FAIL;
                break;
            }   
            if ( *buff == BOOT_CMD_ACK )
                break;
            retval++;
            buff++;

            if ( retval >= buff_size ){
                retval = ERR_OVERFLOW;
                break;
            }
        }
        while ( 1 );    
    }
    return retval;
}
//-----------------------------------------------
static int send_n ( uint8_t * data, int size, uint8_t crc )
{
    int retval = ERROR;
    
    assert ( data );

    while ( size-- ){
        if ( ! tx_byte ( *data ) ){
            retval = ERR_TX_FAIL;
            break;
        }            
        crc ^= *data;
        data++;
        retval++;
    }
    if ( retval > 0 )
        if ( !tx_byte ( crc ) )
                retval = ERR_TX_FAIL; 

    return retval;
}
//-----------------------------------------------
static int send_cmd ( uint8_t cmd )
{
    assert ( cmd < BOOT_CMD_IDX_MAX );
    int retval;

    if ( cmd_set[cmd].allow != CMD_ALLOWED )
        retval = ERR_CMD_NOT_ALLOW;
    else
        retval = send_n ( &cmd_set[cmd].cmd, 1, BOOT_CMD_CHKSUMM );

    return  retval;
}
//-----------------------------------------------
static int send_addr ( uint32_t addr )
{
    uint8_t * ptr = &tx_buff[3];
    
    *( uint32_t *)tx_buff = 0;
    
    while ( addr ){
        *ptr = addr & 0xFF;
        addr >>= 8;
        ptr--;
    }
    return send_n ( tx_buff, 4, 0 );
}
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
static int boot_get ( )
{
    int retval, cmd_set_idx, ans_cmd_idx;
    BOOT_CMD_GET_ANSWER_T * ans = ( BOOT_CMD_GET_ANSWER_T * )rx_buff;

    do{
        SEND_CMD ( BOOT_CMD_GET_IDX );
        CMD_ANSWER ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY );
#ifdef DEBUG        
        hex ( rx_buff, retval );
#endif
        if ( retval < 3 || 
             !ans->ver || 
             ans->n != ( retval - 2 ) ){
            retval =  ERR_UNKNOWN_ANSWER;
            break;
        }
    
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
    int retval;

    do{
        SEND_CMD ( BOOT_CMD_GET_VERSION_IDX );
        CMD_ANSWER ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY );
#ifdef DEBUG        
        hex ( rx_buff, retval );
#endif
        retval = rx_buff[0];

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_get_id ( )
{
    int retval;

    do{
        SEND_CMD ( BOOT_CMD_GET_ID_IDX );
        CMD_ANSWER ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY ) ;
#ifdef DEBUG        
        hex ( rx_buff, retval );
#endif
        if ( retval != 3 || 
             rx_buff[0] != 1 ||
             rx_buff[1] != 4 ){
            retval =  ERR_UNKNOWN_ANSWER;
            break;
        }

        retval = rx_buff[2];

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_read ( uint32_t addr, uint8_t * data, uint8_t size )
{
    int retval = ERR_WRONG_SIZE;
    uint8_t ans;

    assert ( data );

    do{
        if ( !size )
            break;
        SEND_CMD ( BOOT_CMD_READ_MEMORY_IDX );
        CHECK_ANSWER ( ERR_RDP_ACTIVE, "BOOT_READ_CMD::Fail: RDP is active.\n" );

        if ( ( retval = send_addr ( addr ) ) != 4 )
            break;
        CHECK_ANSWER ( ERR_WRONG_ADDR, "BOOT_READ_CMD::Fail: wrong address.\n" );

        if ( ( retval = send_n ( &size, 1, BOOT_CMD_CHKSUMM ) ) != 1 )
            break;
        CHECK_ANSWER ( ERR_WRONG_SIZE, "BOOT_READ_CMD::Fail: wrong data size.\n" );

        retval = rx_uart ( data, size );
        hex ( data, retval );
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
    uint8_t chip_id = 0, test_read[4];
    int ans;
 
    if ( !open_uart (port , BOOT_UART_SPEED ) )
    {
        close_uart();
        return 0;
    }
    msg("\tWaiting for connection...\n");
    while ( 1 )
    {
        tx_byte ( BOOT_CMD_HELLO );
        if ( get_ack ( 1 ) == BOOT_CMD_ACK ){
            chip_id = boot_connecting ( );
            if ( chip_id )
                break;
        }
        delay ( BOOT_HELLO_TICK ); 

    }

    msg ( "\tDetected STM32 chip with ID 0x%.2X.\n", chip_id );
    msg ( "\tBootloader v%d.%d\n", boot_version >> 4, boot_version & 0xf );
    
    ans = boot_read ( BOOT_TEST_READ_ADDR, test_read, 4 );
    close_uart();
    return 1;
}