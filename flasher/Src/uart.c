#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "uart.h"

/*
*   based on:   https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
*   SPEED:  B115200, B230400, B9600, B19200, B38400, B57600, B1200, B2400, B4800
*   PARITY: 0                       no parity 
*           PARENB|PARODD           enable parity and use odd
*           PARENB                  enable parity and use even
*           PARENB|PARODD|CMSPAR    mark parity
*           PARENB|CMSPAR           space parity
*/

static int uart = -1;

//-----------------------------------------------
static int get_port_attribs ( struct termios * tty )
{
    assert ( tty );
    int retval = 0;
    
    do{
        memset ( tty, 0, sizeof *tty );
        
        if ( uart < 0 )
            break;
        
        if ( tcgetattr ( uart, tty ) != 0 ){
            printf ( "error %d from tcgetattr", errno );
            break;
        } 

        retval = 1;
    }while ( 0 );

    return retval;
}
//-----------------------------------------------
static int set_port_attribs ( int speed, int parity )
{
    struct termios tty;
    int retval = 0;

    do{
        if ( !get_port_attribs ( &tty ) )
            break;
        
        cfsetospeed ( &tty, speed );
        cfsetispeed ( &tty, speed );

        tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~( IXON | IXOFF | IXANY );   // shut off xon/xoff ctrl

        tty.c_cflag |= ( CLOCAL | CREAD );          // ignore modem controls,
                                                    // enable reading
        tty.c_cflag &= ~( PARENB | PARODD );        // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;    
    
        if ( tcsetattr ( uart, TCSANOW, &tty ) != 0 ){
            printf ( "error %d from tcsetattr", errno );
            break;
        }

        retval = 1;
    }while ( 0 );
    
    return retval;
}
//-----------------------------------------------
void set_port_blocking ( int should_block )
{
    struct termios tty;
    
    do{
        if ( !get_port_attribs ( &tty ) )
            break;

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if ( tcsetattr (uart, TCSANOW, &tty ) != 0 )
            printf ( "error %d setting term attributes", errno );
    }
    while ( 0 );
}
//-----------------------------------------------
void close_port ( )
{
    if ( uart >= 0 )
    {
        close ( uart );
        uart = -1;
    }
}
//-----------------------------------------------
size_t tx_port ( unsigned char * data, size_t size )
{
    assert ( data );
    size_t retval = 0;

    if ( uart >= 0 && size )
        return write ( uart, data, size );

    return 0;
}
//-----------------------------------------------
int tx_byte ( unsigned char data )
{
    return ( int ) tx_port ( &data, 1 );
}
//-----------------------------------------------
size_t rx_port ( unsigned char * data, size_t size )
{
    assert ( data );
    size_t retval = 0;

    if ( uart >= 0 && size )
        return read ( uart, data, size );
    return 0;
}
//-----------------------------------------------
int rx_byte ( unsigned char * data )
{
    assert ( data );
    return ( int ) rx_port ( data, 1 );
}
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
int open_port ( char * port, int speed )
{
    assert ( port );
    int retval = 0;

    do
    {
        close_port ();
        
        uart = open ( port, O_RDWR | O_NOCTTY | O_SYNC );
        if ( uart < 0 ){
            printf ("error %d opening %s: %s", errno, port, strerror (errno));
            break;
        }

        if ( !set_port_attribs ( speed, PARENB ) )
            break;
        
        set_port_blocking ( 0 );

        retval = 1;

    }while ( 0 );

    if ( !retval )
        close_port ();
    
    return retval;
}
