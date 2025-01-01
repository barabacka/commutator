#include "common.h"
#include "bootloader_def.h"
#include "bootloader_opt.h"
#include "uart.h"

//#define _BSD_SOURCE

#define SEND_CMD(c)             if ( ( retval = send_cmd ( c ) ) != OK ) break;
#define CMD_ANSWER(c,s,d)       if ( ( retval = cmd_answer ( c,s,d ) ) < OK ) break;
#define CHECK_ANSWER(rv,rm) {   retval = get_ack ( BOOT_RX_DELAY ); \
                                if ( retval != OK ){                \
                                    if ( retval == ERR_NACK ){      \
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

static uint8_t rx_buff[BUFF_SIZE];
static uint8_t tx_buff[BUFF_SIZE];

//-----------------------------------------------
static inline void delay ( useconds_t usec )
{
    if ( usec ) 
        usleep ( usec );
}
//-----------------------------------------------
static int get_ack ( useconds_t wait_us )
{
    uint8_t ans;
    int retval;

    delay ( wait_us ); 
    if ( !rx_byte ( &ans ) )
        retval = ERR_RX_FAIL;
    else
        switch ( ans ){
            case BOOT_CMD_ACK:
                retval = OK;
                break;
            case BOOT_CMD_NACK:
                retval = ERR_NACK;
                break;
            default:
                dbg ( "get_ack::unknown answer: 0x%.2X\n", ans );
                retval = ERR_UNKNOWN_ANSWER;
                break;
        }
    
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
        retval = 0;
        do{
            delay ( wait_us );
            if ( !rx_byte ( buff ) ){
                retval = ERR_RX_FAIL;
                break;
            }   
            if ( *buff == BOOT_CMD_ACK )
                break;

            buff++;
            retval++;
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
        dbg ( "%.2X ", *data );
        if ( ! tx_byte ( *data ) ){
            retval = ERR_TX_FAIL;
            break;
        }            
        crc ^= *data;
        data++;
        retval++;
    }
    if ( retval > 0 ){
        dbg ( "%.2X\n", crc );
        if ( !tx_byte ( crc ) )
                retval = ERR_TX_FAIL;
    } 

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
static int boot_get ( BL_MCU_INFO_T * mcu )
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
             ans->n != ( retval - 2 ) ) {
            retval = ERR_UNKNOWN_ANSWER;
            break;
        }
    
        mcu->ver = ans->ver;
        //clear cmd set
        for ( cmd_set_idx = ( BOOT_CMD_GET_IDX + 1); cmd_set_idx < BOOT_CMD_IDX_MAX; cmd_set_idx++ )
            cmd_set[cmd_set_idx].allow = CMD_RESTRICTED;
        
        mcu->cmdset_num = 0;

        //regitrate commands set
        for ( ans_cmd_idx = 0; ans_cmd_idx < ans->n; ans_cmd_idx++ )
            for ( cmd_set_idx = ( BOOT_CMD_GET_IDX + 1); cmd_set_idx < BOOT_CMD_IDX_MAX; cmd_set_idx++ )
                if ( cmd_set[cmd_set_idx].cmd == ans->cmd[ans_cmd_idx] )
                {
                    cmd_set[cmd_set_idx].allow = CMD_ALLOWED;
                    mcu->cmdset_num ++;
                    break;
                }

        retval = mcu->cmdset_num >= BOOT_CMD_SET_MIN ? OK : ERR_USUPPORTED_VERSION;
    
    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_get_version ( BL_MCU_INFO_T * mcu )
{
    int retval;

    do{
        SEND_CMD ( BOOT_CMD_GET_VERSION_IDX );
        CMD_ANSWER ( rx_buff, BUFF_SIZE, BOOT_RX_DELAY );
#ifdef DEBUG        
        hex ( rx_buff, retval );
#endif
        if ( !mcu->ver )
            mcu->ver = rx_buff[0];
        
        retval = mcu->ver == rx_buff[0] ? OK : ERR_USUPPORTED_VERSION;

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_get_id ( BL_MCU_INFO_T * mcu )
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
            retval =  ERR_USUPPORTED_VERSION;
            break;
        }

        mcu->core_id = rx_buff[2];
        retval = OK;

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_readout_unprotect ( void )
{
    int retval;
    uint8_t ans;

    do{
        SEND_CMD ( BOOT_CMD_READOUT_UNPROTECT_IDX );
        CHECK_ANSWER ( ERR_UNKNOWN_ANSWER, "BOOT_RDP_UNPROTECT_CMD::Fail.\n" );
        //TODO: add timeout 
        do{
            retval = get_ack ( BOOT_RX_DELAY );
            if ( retval != ERR_RX_FAIL )
                break;
        }
        while ( 1 );
    }while ( 0 );

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

        size--;
        if ( ( retval = send_n ( &size, 1, BOOT_CMD_CHKSUMM ) ) != 1 )
            break;
        CHECK_ANSWER ( ERR_WRONG_SIZE, "BOOT_READ_CMD::Fail: wrong data size.\n" );

        retval = rx_uart ( data, size + 1 );

#ifdef DEBUG        
        hex ( data, retval );
#endif

    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_write ( uint32_t addr, uint8_t * data, uint16_t size )
{
    int retval = ERR_WRONG_SIZE, cnt;
    uint8_t ans, crc;

    assert ( data );

    do{
        if ( !size )
            break;
        SEND_CMD ( BOOT_CMD_WRITE_MEMORY_IDX );
        CHECK_ANSWER ( ERR_RDP_ACTIVE, "BOOT_WRITE_CMD::Fail: RDP is active.\n" );

        if ( send_addr ( addr ) != 4 ){
            retval = ERR_TX_FAIL;
            break;
        }
        CHECK_ANSWER ( ERR_WRONG_ADDR, "BOOT_WRITE_CMD::Fail: wrong address.\n" );

        cnt = 0; 
        tx_buff[cnt] = size - 1;
        crc = tx_buff[cnt++];
        while ( size ){
            tx_buff[cnt++] = *data;
            crc ^= *data;
            data++;
            size--;
            if ( cnt >= BUFF_SIZE ){
#ifdef DEBUG        
                hex ( tx_buff, cnt );
#endif
                if ( tx_uart ( tx_buff, cnt ) != cnt ){
                    retval = ERR_TX_FAIL;
                    break;
                }
                cnt = 0;
            }
        }
        if ( retval < OK )
            break;
        
        tx_buff[cnt++] = crc;
#ifdef DEBUG        
        hex ( tx_buff, cnt );
#endif
        if ( tx_uart ( tx_buff, cnt ) != cnt ){
            retval = ERR_TX_FAIL;
            break;
        }

        CHECK_ANSWER ( ERR_WRITE, "BOOT_WRITE_CMD::Fail.\n" );
    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_write_unprotect ( void )
{
    int retval;
    uint8_t ans;

    do{
        SEND_CMD ( BOOT_CMD_WRITE_UNPROTECT_IDX );
        CHECK_ANSWER ( ERR_RDP_ACTIVE, "BOOT_WR_UNPROTECT_CMD::Fail.\n" );
        //TODO: add timeout 
        do{
            retval = get_ack ( BOOT_RX_DELAY );
            if ( retval != ERR_RX_FAIL )
                break;
        }
        while ( 1 );
    }while ( 0 );

    return retval;
}
//-----------------------------------------------
static int boot_erase_ext ( uint16_t pages_num, uint16_t * pages )
{
    int retval = OK, cnt = 0, num;
    uint8_t ans, crc;
    do{
        SEND_CMD ( BOOT_CMD_EXTENDED_ERASE_IDX );   
        CHECK_ANSWER ( ERR_RDP_ACTIVE, "BOOT_EXT_ERASE_CMD::Fail: RDP is active.\n" ); 
        num = pages_num - 1;
        tx_buff[cnt++] = num >> 8;
        tx_buff[cnt++] = num & 0xFF;
        crc = tx_buff[0] ^ tx_buff[1];
        if ( ( pages_num >> 4 ) != 0xFFF ){
            //2 bytes for each page, so x2 
            pages_num <<= 1;
            while ( pages_num ){
                if ( ( pages_num & 1 ) == 0 )
                    tx_buff[cnt] = *pages >> 8;
                else{
                    tx_buff[cnt] = *pages & 0xFF;
                    pages++;
                }
                crc ^= tx_buff[cnt++];
                pages_num--;
                //check for buffer overflow
                if ( cnt >= BUFF_SIZE ){
#ifdef DEBUG        
                    hex ( tx_buff, cnt );
#endif
                    if ( tx_uart ( tx_buff, cnt ) != cnt ){
                        retval = ERR_TX_FAIL;
                        break;
                    }
                    cnt = 0;
                }

            }
        }

        if ( retval < OK )
            break;
        
        tx_buff[cnt++] = crc;
#ifdef DEBUG        
        hex ( tx_buff, cnt );
#endif
        if ( tx_uart ( tx_buff, cnt ) != cnt ){
            retval = ERR_TX_FAIL;
            break;
        }

        do{
            if ( ( retval = get_ack ( BOOT_ERASE_PAGE_TO ) ) == OK )
                break;
            if ( retval == ERR_NACK ){
                retval = ERR_WRONG_SIZE;
                break;
            }
            retval = ERR_TIMEOUT;
        } while ( num-- );
        
    } while ( 0 );
    
    return retval;
}
//-----------------------------------------------
static int boot_read_info ( BL_MCU_INFO_T * mcu  )
{
    int retval = 0;
    do{
        memset ( mcu, 0, sizeof(BL_MCU_INFO_T) );
        flush_rx ( 1 );
        
        if ( ( retval = boot_get ( mcu ) ) < OK )
            break;

        if ( ( retval = boot_get_version ( mcu ) ) < OK )
            break;

        retval = boot_get_id ( mcu );

    } while ( 0 );

    return retval;
}
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
int bl_open_uart ( char * port )
{
    if ( !open_uart ( port, BOOT_UART_SPEED ) )
        return ERR_OPEN_UART;
    return OK;
}
//-----------------------------------------------
void bl_close_uart ( void )
{
    close_uart ( );
}
//-----------------------------------------------
int bl_connect ( BL_MCU_INFO_T * mcu )
{
    int ans;
 
    tx_byte ( BOOT_CMD_HELLO );
    
    if ( get_ack ( 1 ) == OK )
        return boot_read_info ( mcu );

    delay ( BOOT_HELLO_TICK );

    return ERR_UNKNOWN_ANSWER;
}
//-----------------------------------------------
int bl_rdp_unblock ( void )
{
    return boot_readout_unprotect ();
}
//-----------------------------------------------
int bl_read ( uint32_t addr, uint8_t * data, uint8_t size )
{
    return boot_read ( addr, data, size );
}
//-----------------------------------------------
int bl_write ( uint32_t addr, uint8_t * data, uint16_t size )
{
    return boot_write ( addr, data, size );
}
//-----------------------------------------------
int bl_er_wr_unblock ( void )
{
    return boot_write_unprotect ( );
}
//-----------------------------------------------
int bl_erase_full ( void )
{
    return boot_erase_ext ( BOOT_ERASE_WHOLE, NULL );
}
//-----------------------------------------------
int bl_erase_ext ( uint16_t pg_num, uint16_t * pages )
{
    return boot_erase_ext ( pg_num, pages );
}
