#ifndef __UART_H__
#define __UART_H__

void close_port ( );
int open_port ( char * port, int speed );
size_t tx_port ( unsigned char * data, size_t size );
int tx_byte ( unsigned char data );
size_t rx_port ( unsigned char * data, size_t size );
int rx_byte ( unsigned char * data );

#endif //__UART_H__