#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

//#define DEBUG 1

#define msg(...)    {printf(__VA_ARGS__); fflush(stdout);}
#ifdef DEBUG
  #define dbg(...)    printf(__VA_ARGS__)
#else
  #define dbg
#endif

typedef enum{
    ERR_MIN = -19,
    ERR_WRITE,
    ERR_OVERFLOW,
    ERR_TIMEOUT,
    ERR_RDP_ACTIVE,
    ERR_WRONG_SIZE,
    ERR_WRONG_ADDR,
    ERR_WRONG_ARGUMENTS_NUMBER,
    ERR_WRONG_ARGUMENT,
    ERR_OPEN_UART,
    ERR_OPEN_MEM,
    ERR_MMAP,
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

typedef struct{
    int    use;
    uint8_t rst;
    uint8_t boot0;
}GPIO_PARAM_T;

//- from common.c
void hex ( uint8_t * dt, int size );

//- from bootloader.c
int bl_open_uart ( char * port );
void bl_close_uart ( void );
int bl_connect ( BL_MCU_INFO_T * mcu );
int bl_rdp_unblock ( void );
int bl_read ( uint32_t addr, uint8_t * data, uint8_t size );
int bl_write ( uint32_t addr, uint8_t * data, uint16_t size );
int bl_er_wr_unblock ( void );
int bl_erase_full ( void );
int bl_erase_ext ( uint16_t pg_num, uint16_t * pages );

//- from gpio.c
void gpio_init ( GPIO_PARAM_T * gpio );
void gpio_reset ( GPIO_PARAM_T * gpio, int boot ); 
void gpio_finish ( GPIO_PARAM_T * gpio );
