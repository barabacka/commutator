#pragma once

void close_uart ( );
int open_uart ( char * port, int speed );
size_t tx_uart ( uint8_t * data, size_t size );
int tx_byte ( uint8_t data );
size_t rx_uart ( uint8_t * data, size_t size );
int rx_byte ( uint8_t * data );
