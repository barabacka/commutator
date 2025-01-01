#include "common.h"
#include "ver.h"
#include "stm32.h"
#include <errno.h>

typedef enum{
    FSM_IDLE = 0,
    FSM_CHECK_PARAM,
    FSM_OPEN_PORT,
    FSM_GPIO_INIT,
    FSM_GPIO_RESET,
    FSM_CONNECT,
    FSM_TEST_RDP,
    FSM_UNLOCK_RDP,
    FSM_UNLOCK_WRP,
    FSM_ERASE_FULL,    
    FSM_ERASE_EX,
    FSM_GET_DATA,
    FSM_WRITE,
    FSM_FAIL_WRITE,
    FSM_FAIL,
    FSM_EXIT,
    FSM_CLOSE_PORT,
    FSM_DONE,
    FSM_IDX_MAX
}FSM_STATES_T;

typedef struct{
    BL_MCU_INFO_T            bl;
    const STM32_MCU_INFO_T * core;
    int rd_allow;
    int wr_allow;                      
}MCU_INFO_T;

typedef enum{
    FFL_CHECK = 0,
    FFL_READ,
}T_OPEN_FILE_MODE;

typedef enum{
    STM_BOOT_DISABLE = 0,
    STM_BOOT_ENABLE
}T_STM_BOOT_MODE;

//-----------------------------------------------
static unsigned char * ffl_open ( char * file_name, int * fsize, T_OPEN_FILE_MODE mode )
{
    unsigned char * dt = NULL;
    int retval = ERROR;
    FILE * file;
    
    assert ( file_name );
    assert ( fsize );

    do{
        file = fopen( file_name, "r" );
        if ( file == NULL ) {
            retval = ERR_WRONG_ARGUMENT;
            break; 
        }
        
        fseek ( file, 0, SEEK_END);
        retval = (int) ftell ( file ); 
        
        if ( retval > 0 ){
            if ( mode == FFL_READ ){

                dt = malloc ( retval );
                assert ( dt );

                rewind ( file );

                fread ( dt, 1, retval, file );
            }  
        }else{
            retval = ERR_WRONG_FLASH_FILE;
        }

        fclose ( file );

    }while ( 0 );

    *fsize = retval;

    return dt;
}
//-----------------------------------------------
static uint8_t get_gpio_num ( char * str_pin )
{
    uint8_t retval = 0;
    char  * endptr;
    long    value;

    do{
        errno = 0;
        value = strtol ( str_pin, &endptr, 10 );
        if (errno == ERANGE)
            break;
        if (*endptr != '\0')
            break;
        if ( value < 1 || value > 27 )
            break;
        retval = ( uint8_t )value; 

    }while ( 0 );
    return retval;
}
//-----------------------------------------------
static int check_arguments ( int argc, char** argv, GPIO_PARAM_T * gpio )
{
    int retval;
    FILE * file;

    do{
        if ( gpio )
            gpio->use = 0;
        if ( argc != 3 && argc != 5 ){
            retval = ERR_WRONG_ARGUMENTS_NUMBER;
            break; 
        }
        
        file = fopen( argv[1], "r" );
        if ( file == NULL ) {
            retval = ERR_OPEN_UART;
            break; 
        }
        fclose ( file );

        ffl_open ( argv[2], &retval, FFL_CHECK );

        if ( retval <= ERROR || argc == 3 || gpio == NULL)
            break;
        
        gpio->rst = get_gpio_num ( argv[3] );
        gpio->boot0 = get_gpio_num ( argv[4] );

        gpio->use = ( gpio->rst != 0 && gpio->boot0 != 0 ) ? 1 : 0;

    }while ( 0 );
    
    if ( retval > ERROR )
            retval = OK;
    else{
        msg ("Wrong argument! (%d)\n", retval );
        msg ("Usage: \n");
        msg ("\t ./flasher \"/dev/uart\" \"FILE_TO_FLASH.bin\" <Opt: Reset PIN> <Opt: BOOT0 PIN>\n");
    }

    return retval;
}
//-----------------------------------------------
//-----------------------------------------------
static int get_mem_index ( MCU_INFO_T * mcu, MEM_TYPE_T req )
{
    int retval = ERR_WRONG_ARGUMENT, i;

    do{
        if ( mcu == NULL ||
             mcu->core == NULL ||
             mcu->core->mem == NULL ||
             mcu->core->mem->num < 1 )
            break;
        for ( i = 0; i < mcu->core->mem->num; i++ )
            if ( mcu->core->mem->regs[i].type == req ){
                retval = i;
                break;
            }
    }while ( 0 );

    return retval;
} 
//-----------------------------------------------
static FSM_STATES_T answer_error ( int res, FSM_STATES_T def )
{
    FSM_STATES_T st;
    switch ( res )
    {
            break;
        case ERR_RDP_ACTIVE:
            st = FSM_CONNECT;
            break;
        case ERR_USUPPORTED_VERSION:
        case ERR_CMD_NOT_ALLOW:
            st = FSM_FAIL;
            break;
        default:
            st = def;
            break;
    }
    return st;
}
//-----------------------------------------------
int main ( int argc, char** argv )
{
#define FSM_INC    state++;
#define FSM_SET(s) state=s;
#define FSM_NEXT   {state++; break;}
#define FSM_GO(s)  {state=s; break;}

    FSM_STATES_T    state = FSM_IDLE;
    int             res, i, cnt, num, dt_size;
    int             working = 1;
    MCU_INFO_T      mcu;
    void *          data = NULL;
    GPIO_PARAM_T    gpio = {0};

    mcu.rd_allow = 0;
    mcu.wr_allow = 0;
    
    msg ( "Commutator: Raspberry PI onboard STM32 flasher by Barabaka %s v%s\n", __DATE__, ver );

    while ( working ){
        switch ( state ){
            case FSM_IDLE:
                FSM_NEXT
            
            case FSM_CHECK_PARAM:
                res = check_arguments ( argc, argv, &gpio );
                if ( res < OK )
                    FSM_GO ( FSM_DONE );
                FSM_NEXT
            case FSM_OPEN_PORT:
                res = bl_open_uart ( argv[1] );
                if ( res < OK )
                    FSM_GO ( FSM_DONE );
                cnt = 0;
                msg ( "\tWaiting for connection...\n" );
                FSM_NEXT

            case FSM_GPIO_INIT:
                gpio_init ( &gpio );
                FSM_NEXT

            case FSM_GPIO_RESET:
                if ( gpio.use ){
                    msg ( "\tHW reset" );
                    gpio_reset ( &gpio, STM_BOOT_ENABLE ); 
                }
                FSM_NEXT

            case FSM_CONNECT:
                mcu.core = NULL;
                res = bl_connect ( &mcu.bl );
                if ( res == OK ){
                    msg ( "\tDetected bootloader v%d.%d\n", mcu.bl.ver >> 4, mcu.bl.ver & 0xf );
                    i = 0;
                    while ( STM32[i].id ){
                        if ( STM32[i].id == mcu.bl.core_id ){
                            mcu.core = &STM32[i];
                            msg ( "\tDetected chip %s (0x%.2X).\n", mcu.core->name, mcu.bl.core_id );
                            if ( !mcu.rd_allow ){
                                FSM_INC
                            }else{ 
                                if ( !mcu.wr_allow ){
                                    FSM_SET ( FSM_UNLOCK_WRP )
                                }else{ 
                                    FSM_SET ( FSM_ERASE_FULL )
                                }
                            }
                            break;
                        }
                        i++;
                    }
                    if ( mcu.core == NULL ){
                        msg ( "\tDetected STM32 chip with unsupported ID 0x%.2X.\n", mcu.bl.core_id );
                        res = ERR_USUPPORTED_VERSION;
                    }
                }

                if ( res < OK )
                    FSM_SET ( answer_error ( res, FSM_CONNECT ) );

                break;
            
            case FSM_TEST_RDP:
                //try to read 4 bytes from flash
                i = get_mem_index ( &mcu, MEM_FLASH );
                if ( i < 0 ){
                    res = ERROR;
                    FSM_GO ( FSM_FAIL );
                }
                data = malloc ( 4 );
                assert ( data );
                res = bl_read ( mcu.core->mem->regs[i].start, (uint8_t*)data, 4 );
                free ( data );
                if ( res < OK ){
                    FSM_SET ( res == ERR_RDP_ACTIVE ? FSM_UNLOCK_RDP : answer_error ( res, FSM_FAIL ) );
                }else{
                    mcu.rd_allow = 1;
                    FSM_SET ( FSM_UNLOCK_WRP );
                }
                break;
            
            case FSM_UNLOCK_RDP:
                if ( cnt < 2 ){
                    res = bl_rdp_unblock ();
                    if ( res == OK ){
                        msg ( "\tWaiting for reconnection...\n" );
                        cnt ++;
                        FSM_GO ( FSM_CONNECT );
                    }
                }else
                    res = ERROR;
                FSM_GO ( answer_error ( res, FSM_FAIL ) );
                break;
            
            case FSM_UNLOCK_WRP:
                res = bl_er_wr_unblock ();
                mcu.wr_allow = 1;
                if ( res == ERR_RDP_ACTIVE )    
                    FSM_SET ( FSM_UNLOCK_RDP )
                else{
                    msg ( "\tWaiting for reconnection...\n" );
                    FSM_SET ( FSM_CONNECT )
                }
                break;           
                
            case FSM_ERASE_FULL:
                msg ( "Erasing whole flash..");
                res = bl_erase_full ( );
                if ( res >= OK ){
                    msg ( "OK\n" );
                    FSM_SET ( FSM_EXIT );
                }else{
                    if ( res == ERR_CMD_NOT_ALLOW || res == ERR_WRONG_SIZE ){
                        msg ( "." );
                        FSM_SET ( FSM_ERASE_EX );
                    }else{
                        FSM_SET ( FSM_FAIL );
                    }
                }
                break;

            case FSM_ERASE_EX:
                i = get_mem_index ( &mcu, MEM_PAGES );
                if ( i >= 0 && 
                     i < mcu.core->mem->num &&
                     mcu.core->mem->regs[i].pg_num > 0 ){
                    
                    num = mcu.core->mem->regs[i].pg_num;
                    data = malloc ( 2 * num );
                    assert ( data );
                    
                    for ( i = 0; i < num; i++ )
                        (( uint16_t * )data )[i] = i;
                                            
                    res = bl_erase_ext ( num, ( uint16_t * )data );
                    free ( data );
                    
                    if ( res >= OK ){
                        msg ( "OK\n" );
                        FSM_GO ( FSM_GET_DATA );
                    }
                }else
                    res = ERR_USUPPORTED_VERSION;
                FSM_SET ( FSM_FAIL );
                break;
            
            case FSM_GET_DATA:
                msg ( "Writing flash " );
                data = ( void * ) ffl_open ( argv[2], &dt_size, FFL_READ );
                if ( data == NULL ||
                    dt_size < OK ){
                        res = ERR_WRONG_FLASH_FILE;
                        FSM_SET ( FSM_FAIL_WRITE )
                    }else
                        FSM_INC
                break;

            case FSM_WRITE:
                i = get_mem_index ( &mcu, MEM_FLASH );
                if ( i < 0 ){
                    res = ERROR;
                    FSM_GO ( FSM_FAIL_WRITE );
                }
                
                cnt = 0;
                num = 0x100;

                while ( cnt < dt_size )
                {
                    if( ( cnt + num ) > dt_size )
                        num = dt_size - cnt;
                    res = bl_write ( mcu.core->mem->regs[i].start + cnt, ( ( uint8_t * ) data ) + cnt, num );
                    cnt += num;
                    if ( res < OK )
                        break;
                    msg ( "." );
                }
                
                if ( res < OK ){
                    FSM_GO ( FSM_FAIL_WRITE );
                }
                
                msg ( "OK\n" );
               
                free ( data );

                gpio_reset ( &gpio, STM_BOOT_DISABLE ); 
                
                FSM_GO (FSM_EXIT)

            case FSM_FAIL_WRITE:
                if ( data )
                    free ( data );
                FSM_NEXT

            case FSM_FAIL:
                msg ( "Fail (%d)\n", res );
                FSM_NEXT
            
            case FSM_EXIT:
                FSM_NEXT

            case FSM_CLOSE_PORT:
                bl_close_uart ( );
                gpio_finish ( &gpio );
                FSM_NEXT

            case FSM_DONE:
                working = 0;
                break;
            
            default:
                res = ERROR;
                working = 0;
                break;
        } 
    }

    return res;   
}
