#pragma once

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

//#define DEBUG 1

#define msg(...)    printf(__VA_ARGS__)
#ifdef DEBUG
  #define dbg(...)    printf(__VA_ARGS__)
#else
  #define dbg
#endif

typedef enum{
    ERR_MIN = -15,
    ERR_OVERFLOW,
    ERR_TIMEOUT,
    ERR_RDP_ACTIVE,
    ERR_WRONG_SIZE,
    ERR_WRONG_ADDR,
    ERR_WRONG_ARGUMENTS_NUMBER,
    ERR_WRONG_ARGUMENT,
    ERR_OPEN_UART,
    ERR_WRONG_FLASH_FILE,
    ERR_UNKNOWN_ANSWER,
    ERR_CMD_NOT_ALLOW,
    ERR_NACK,
    ERR_TX_FAIL,
    ERR_RX_FAIL,
    ERROR,
    OK    
}ERRORS_T;
static_assert(ERROR == 0, "Please check ERR_MIN value!");

//- from common.c
void hex ( uint8_t * dt, int size );

//- from bootloader.c
int bl_open_uart ( char * port );
void bl_close_uart ( void );
int bl_connect ( void );

//- from fsm.c
int fsm ( int argc, char** argv );



