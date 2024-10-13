#include "common.h"

static int check_arguments ( int argc, char** argv )
{
#define CHECK_ARG(m)     if ( ( m ) ) break; retval ++; 
    int retval = -5;
    FILE * file;
    do{
        CHECK_ARG ( argc != 3 )
        
        file = fopen( argv[1], "r" );
        CHECK_ARG ( file == NULL )
        fclose ( file );
        
        file = fopen( argv[2], "r" );
        CHECK_ARG ( file == NULL )
        fseek ( file, 0, SEEK_END);
         
        if ( ftell ( file ) > 0 )
            retval = 1;
        fclose ( file );

    }while ( 0 );
    
    if ( retval <= 0 )
    {
        msg ("Wrong argument! (%d)\n", retval );
        msg ("Usage: \n");
        msg ("\t ./flasher \"/dev/uart\" \"FILE_TO_FLASH.bin\"\n");
        retval = 0;
    }

    return retval;
}
//-----------------------------------------------
int main ( int argc, char** argv )
{
    msg ( "Commutator: Raspberry PI onboard STM32 flasher by Barabaka. 2024\n" );
    if ( !check_arguments ( argc, argv ) )
        return 1;
    boot_start ( argv[1] );
    return 0;
}



