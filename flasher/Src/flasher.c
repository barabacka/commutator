#include <stdio.h>
#include "bootloader.h"

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
        printf ("Wrong argument! (%d)\n", retval );
        printf ("Usage: \n");
        printf ("\t ./flasher \"/dev/uart\" \"FILE_TO_FLASH.bin\"\n");
        retval = 0;
    }

    return retval;
}
//-----------------------------------------------
int main ( int argc, char** argv )
{
    if ( !check_arguments ( argc, argv ) )
        return 1;
    boot_start ( argv[1] );
    return 0;
}



