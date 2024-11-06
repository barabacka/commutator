#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
    ERR_MIN = -16,
    ERR_OVERFLOW,
    ERR_TIMEOUT,
    ERR_RDP_ACTIVE,
    ERR_WRONG_SIZE,
    ERR_WRONG_ADDR,
    ERR_WRONG_ARGUMENTS_NUMBER,
    ERR_WRONG_ARGUMENT,
    ERR_OPEN_UART,
    ERR_WRONG_FLASH_FILE,
    ERR_USUPPORTED_VERSION,
    ERR_UNKNOWN_ANSWER,
    ERR_CMD_NOT_ALLOW,
    ERR_NACK,
    ERR_TX_FAIL,
    ERR_RX_FAIL,
    ERROR,
    OK    
}ERRORS_T;
static_assert(ERROR == 0, "Please check ERR_MIN value!");

typedef struct{
  uint8_t ver;
  uint8_t core_id;
  uint8_t cmdset_num;
}BL_MCU_INFO_T;

//- from common.c
void hex ( uint8_t * dt, int size );

//- from bootloader.c
int bl_open_uart ( char * port );
void bl_close_uart ( void );
int bl_connect ( BL_MCU_INFO_T * mcu );
int bl_rdp_unblock ( void );
int bl_read ( uint32_t addr, uint8_t * data, uint8_t size );
int bl_erase_full ( void );
int bl_erase_ext ( uint16_t pg_num, uint16_t * pages );