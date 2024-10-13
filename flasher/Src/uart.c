#include <unistd.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include "common.h"
#include "uart.h"
#include <string.h>



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
static int get_uart_attribs ( struct termios * tty )
{
    assert ( tty );
    int retval = 0;
    
    do{
        memset ( tty, 0, sizeof *tty );
        
        if ( uart < 0 )
            break;
        
        if ( tcgetattr ( uart, tty ) != 0 ){
            msg ( "error %d from tcgetattr", errno );
            break;
        } 

        retval = 1;
    }while ( 0 );

    return retval;
}
//-----------------------------------------------
static int set_uart_attribs ( int speed, int parity )
{
    struct  termios tty;
    int     retval = 0;
    speed_t bspeed;
    do{
        if ( !get_uart_attribs ( &tty ) )
            break;

        switch ( speed ){
            case 1200:  
                bspeed = B1200; 
                break;
            case 2400:  
                bspeed = B2400; 
                break;
            case 4800:  
                bspeed = B4800; 
                break;
            case 9600:  
                bspeed = B9600; 
                break;
            case 19200:  
                bspeed = B19200; 
                break;
            case 38400:  
                bspeed = B38400; 
                break;
            case 57600:  
                bspeed = B57600; 
                break;
            case 115200:  
                bspeed = B115200; 
                break;
            case 230400:  
                bspeed = B230400; 
                break;
            default:
                assert ( 0 );
                break;
        }
        
        cfsetospeed ( &tty, bspeed );
        cfsetispeed ( &tty, bspeed );

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
            msg ( "error %d from tcsetattr", errno );
            break;
        }

        retval = 1;
    }while ( 0 );
    
    return retval;
}
//-----------------------------------------------
void set_uart_blocking ( int should_block )
{
    struct termios tty;
    
    do{
        if ( !get_uart_attribs ( &tty ) )
            break;

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if ( tcsetattr (uart, TCSANOW, &tty ) != 0 )
            msg ( "error %d setting term attributes", errno );
    }
    while ( 0 );
}
//-----------------------------------------------
void close_uart ( )
{
    if ( uart >= 0 )
    {
        close ( uart );
        uart = -1;
    }
}
//-----------------------------------------------
size_t tx_uart ( uint8_t * data, size_t size )
{
    assert ( data );
    size_t retval = 0;

    if ( uart >= 0 && size )
        return write ( uart, data, size );

    return 0;
}
//-----------------------------------------------
int tx_byte ( uint8_t data )
{
    return ( int ) tx_uart ( &data, 1 );
}
//-----------------------------------------------
size_t rx_uart ( uint8_t * data, size_t size )
{
    assert ( data );
    size_t retval = 0;

    if ( uart >= 0 && size )
        return read ( uart, data, size );
    return 0;
}
//-----------------------------------------------
int rx_byte ( uint8_t * data )
{
    assert ( data );
    return ( int ) rx_uart ( data, 1 );
}
//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
int open_uart ( char * port, int speed )
{
    assert ( uart );
    int retval = 0;

    do
    {
        close_uart ();
        
        uart = open ( port, O_RDWR | O_NOCTTY | O_SYNC );
        if ( uart < 0 ){
            msg ("error %d opening %s: %s", errno, port, strerror (errno));
            break;
        }

        if ( !set_uart_attribs ( speed, PARENB ) )
            break;
        
        set_uart_blocking ( 0 );

        retval = 1;

    }while ( 0 );

    if ( !retval )
        close_uart ();
    
    return retval;
}
