#pragma once

#define BUFF_SIZE                   0x100
#define BOOT_UART_SPEED             115200
#define BOOT_HELLO_TICK             10000
#define BOOT_RX_DELAY               0
#define BOOT_ERASE_PAGE_TO          10

#define BOOT_TEST_READ_ADDR         0x8000000

typedef enum{
    ERR_MIN = -11,
    ERR_OVERFLOW,
    ERR_TIMEOUT,
    ERR_RDP_ACTIVE,
    ERR_WRONG_SIZE,
    ERR_WRONG_ADDR,
    ERR_UNKNOWN_ANSWER,
    ERR_CMD_NOT_ALLOW,
    ERR_NACK,
    ERR_TX_FAIL,
    ERR_RX_FAIL,
    ERROR,
    OK    
}BL_ERRORS_T;
static_assert(ERROR == 0, "Please Correct ERR_MIN value!");

typedef enum{
    CMD_RESTRICTED = 0,
    CMD_ALLOWED,
    CMD_PERMISSION_MAX
}BOOT_CMD_PERMISSION_T;

typedef struct{
    BOOT_CMD_PERMISSION_T   allow;
    uint8_t                 cmd;
}BOOT_CMD_ALLOW_T;
