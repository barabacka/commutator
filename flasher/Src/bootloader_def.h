#pragma once

#define BUFF_SIZE                   0x200
#define BOOT_UART_SPEED             115200
#define BOOT_HELLO_TICK             10000
#define BOOT_RX_DELAY               0
#define BOOT_ERASE_PAGE_TO          10

#define BOOT_TEST_READ_ADDR         0x8000000

typedef enum{
    CMD_RESTRICTED = 0,
    CMD_ALLOWED,
    CMD_PERMISSION_MAX
}BOOT_CMD_PERMISSION_T;

typedef struct{
    BOOT_CMD_PERMISSION_T   allow;
    uint8_t                 cmd;
}BOOT_CMD_ALLOW_T;
