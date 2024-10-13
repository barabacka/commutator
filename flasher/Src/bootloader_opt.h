#pragma once

#define BUFF_SIZE                   0x100

#define BOOT_UART_SPEED             115200
#define BOOT_RX_DELAY               0

#define BOOT_HELLO_TICK             10000


#define BOOT_CMD_CHKSUMM            0xFF
#define BOOT_CMD_ACK                0x79
#define BOOT_CMD_NACK               0x1F

//STM32 BOOTLOADER CMD
#define BOOT_CMD_HELLO              0x7F
#define BOOT_CMD_GET                0x00    
#define BOOT_CMD_GET_VERSION        0x01    
#define BOOT_CMD_GET_ID             0x02  
#define BOOT_CMD_READ_MEMORY        0x11 
#define BOOT_CMD_GO                 0x21 
#define BOOT_CMD_WRITE_MEMORY       0x31 
#define BOOT_CMD_ERASE              0x43 
#define BOOT_CMD_EXTENDED_ERASE     0x44 
#define BOOT_CMD_SPECIAL            0x50 
#define BOOT_CMD_EXTENDED_SPECIAL   0x51 
#define BOOT_CMD_WRITE_PROTECT      0x63 
#define BOOT_CMD_WRITE_UNPROTECT    0x73 
#define BOOT_CMD_READOUT_PROTECT    0x82 
#define BOOT_CMD_READOUT_UNPROTECT  0x92 

typedef enum{
    BOOT_CMD_GET_IDX= 0,
    BOOT_CMD_GET_VERSION_IDX,
    BOOT_CMD_GET_ID_IDX,
    BOOT_CMD_READ_MEMORY_IDX,
    BOOT_CMD_GO_IDX,
    BOOT_CMD_WRITE_MEMORY_IDX,
    BOOT_CMD_ERASE_IDX,
    BOOT_CMD_EXTENDED_ERASE_IDX,
    BOOT_CMD_SPECIAL_IDX,
    BOOT_CMD_EXTENDED_SPECIAL_IDX,
    BOOT_CMD_WRITE_PROTECT_IDX,
    BOOT_CMD_WRITE_UNPROTECT_IDX,
    BOOT_CMD_READOUT_PROTECT_IDX,
    BOOT_CMD_READOUT_UNPROTECT_IDX,
    BOOT_CMD_IDX_MAX
}BOOT_CMD_IDX_T;

typedef struct{
    uint8_t n;
    uint8_t ver;
    uint8_t cmd[0];                 
}BOOT_CMD_GET_ANSWER_T;

typedef enum{
    CMD_RESTRICTED = 0,
    CMD_ALLOWED,
    CMD_PERMISSION_MAX
}BOOT_CMD_PERMISSION_T;

typedef struct{
    BOOT_CMD_PERMISSION_T   allow;
    uint8_t                 cmd;
}BOOT_CMD_ALLOW_T;
