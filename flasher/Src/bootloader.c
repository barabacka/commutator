#include <stdio.h>
#include <unistd.h>
#include "bootloader.h"
#include "bootloader_opt.h"
#include "uart.h"

//#define _BSD_SOURCE
static unsigned char rx_buff[BUFF_SIZE];
static unsigned char tx_buff[BUFF_SIZE];

static void hex ( unsigned char * buff, int len )
{
    while ( len -- )
        printf ( "%.2X ", *buff++ );
    printf ( "\n" );
}
//-----------------------------------------------
static unsigned char summ ( unsigned char * buff, int len )
{
    unsigned char sum = BOOT_CMD_CHKSUMM;

    while ( len-- )
        sum ^= *buff++;
        
    return sum;
}
//-----------------------------------------------
static int ack ( int wait_us )
{
    unsigned char c;

    if ( wait_us ) 
        usleep ( wait_us );
    
    return ( rx_byte ( &c ) == 1 && c == BOOT_CMD_ACK ) ? 1 : 0; 
}
//-----------------------------------------------
static void flush_rx ( int wait_us )
{
    unsigned char c;

    do{
        if ( wait_us ) 
            usleep ( wait_us );
    }
    while ( rx_byte ( &c ) );
}
//-----------------------------------------------
static int cmd_answer ( unsigned char * buff, int buff_size, int wait_us )
{
    int retval = 0, len = 0;
    
    if ( ack ( wait_us ) )
    {
        do{
            if ( wait_us ) 
                usleep ( wait_us );
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
static int send_cmd ( unsigned char cmd )
{
    tx_buff[0] = cmd;
    tx_buff[1] = summ ( tx_buff, 1 );
    return tx_port ( tx_buff, 2 ) == 2 ? 1 : 0; 
}
//-----------------------------------------------
static int boot_get_version ()
{
    int len, c;
    send_cmd ( BOOT_CMD_LIST );
    len = cmd_answer ( rx_buff, BUFF_SIZE, 0 );
    hex ( rx_buff, len );
    return len;
}
//-----------------------------------------------

int boot_start ( char * port )
{

    if ( !open_port (port , BOOT_UART_SPEED ) )
    {
        close_port();
        return 0;
    }
    printf("Start to connect...\n");
    while ( 1 )
    {
        tx_byte ( BOOT_CMD_HELLO );
        if ( ack ( 1 ))
        {
            flush_rx ( 1 );
            boot_get_version ();
            break;
        }
        usleep ( BOOT_HELLO_TICK ); 

    }
    printf ( "Exit!\n" );
    close_port();
    return 1;
}