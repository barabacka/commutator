#pragma once

#define BUFF_SIZE                   0x100
#define BOOT_UART_SPEED             115200
#define BOOT_RX_DELAY               0

typedef enum{
    ERR_MIN = -8,
    ERR_OVERFLOW,
    ERR_RDP_ACTIVE,
    ERR_WRONG_SIZE,
    ERR_WRONG_ADDR,
    ERR_UNKNOWN_ANSWER,
    ERR_CMD_NOT_ALLOW,
    ERR_NACK,
    ERR_TX_FAIL,
    ERR_RX_FAIL,
    ERROR = 0,
    OK    
}BL_ERRORS_T;

typedef enum{
    CMD_RESTRICTED = 0,
    CMD_ALLOWED,
    CMD_PERMISSION_MAX
}BOOT_CMD_PERMISSION_T;

typedef struct{
    BOOT_CMD_PERMISSION_T   allow;
    uint8_t                 cmd;
}BOOT_CMD_ALLOW_T;
