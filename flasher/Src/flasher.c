#include "common.h"
#include "ver.h"
#include "stm32.h"

typedef enum{
    FSM_IDLE = 0,
    FSM_CHECK_PARAM,
    FSM_OPEN_PORT,
    FSM_CONNECT,
    FSM_TEST_RDP,
    FSM_UNLOCK_RDP,
    FSM_FAIL,
    FSM_EXIT,
    FSM_CLOSE_PORT,
    FSM_DONE,
    FSM_IDX_MAX
}FSM_STATES_T;

typedef struct{
    BL_MCU_INFO_T            bl;
    const STM32_MCU_INFO_T * core;
    //int                      
}MCU_INFO_T;

//-----------------------------------------------
static int check_arguments ( int argc, char** argv )
{
    int retval = OK;
    FILE * file;
    do{
        if ( argc != 3 ){
            retval = ERR_WRONG_ARGUMENTS_NUMBER;
            break; 
        }
        
        file = fopen( argv[1], "r" );
        if ( file == NULL ) {
            retval = ERR_OPEN_UART;
            break; 
        }
        fclose ( file );
        
        file = fopen( argv[2], "r" );
        if ( file == NULL ) {
            retval = ERR_WRONG_ARGUMENT;
            break; 
        }
        fseek ( file, 0, SEEK_END);
         
        if ( ftell ( file ) == 0 )
            retval = ERR_WRONG_FLASH_FILE;
        fclose ( file );

    }while ( 0 );
    
    if ( retval <= 0 )
    {
        msg ("Wrong argument! (%d)\n", retval );
        msg ("Usage: \n");
        msg ("\t ./flasher \"/dev/uart\" \"FILE_TO_FLASH.bin\"\n");
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
    int             res, i, cnt;
    int             working = 1;
    MCU_INFO_T      mcu;
    void *          data = NULL;
    
    msg ( "Commutator: Raspberry PI onboard STM32 flasher by Barabaka %s v%s\n", __DATE__, ver );

    while ( working ){
        switch ( state ){
            case FSM_IDLE:
                FSM_NEXT
            
            case FSM_CHECK_PARAM:
                res = check_arguments ( argc, argv );
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
                            FSM_INC
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
                    if ( res == ERR_RDP_ACTIVE )
                        FSM_GO ( FSM_UNLOCK_RDP );
                    FSM_GO ( answer_error ( res, FSM_FAIL ) );
                }
                msg("reading OK\n");
                FSM_GO ( FSM_EXIT );
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
                
            case FSM_FAIL:
                msg ( "Fail (%d)\n", res );
                FSM_NEXT
            case FSM_EXIT:
                FSM_NEXT

            case FSM_CLOSE_PORT:
                bl_close_uart ( );
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
