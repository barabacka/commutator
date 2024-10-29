
#include "common.h"

typedef enum{
    FSM_IDLE = 0,
    FSM_CHECK_PARAM,
    FSM_OPEN_PORT,
    FSM_CONNECT,
    FSM_EXIT,
    FSM_CLOSE_PORT,
    FSM_DONE,
    FSM_IDX_MAX
}FSM_STATES_T;

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
//-----------------------------------------------
int fsm ( int argc, char** argv )
{
#define FSM_INC    state++;
#define FSM_SET(s) state=s;
#define FSM_NEXT   {state++; break;}
#define FSM_GO(s)  {state=s; break;}

    FSM_STATES_T    state = FSM_IDLE;
    int             res;

    while ( 1 ){
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
                msg ( "\tWaiting for connection...\n" );
                FSM_NEXT

            case FSM_CONNECT:
                if ( bl_connect ( ) == OK )
                    FSM_INC
                break;
                

            case FSM_EXIT:
                FSM_NEXT

            case FSM_CLOSE_PORT:
                bl_close_uart ( );
                FSM_NEXT

            case FSM_DONE:
                return res;
            
            default:
                return ERROR;
       } 
    }   
}
