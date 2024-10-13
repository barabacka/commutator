#ifndef __BOOTLOADER_OPT_H__
#define __BOOTLOADER_OPT_H__

#include <termios.h>

#define BUFF_SIZE           0x100

#define BOOT_UART_SPEED     B115200

#define BOOT_HELLO_TICK     1000

#define BOOT_CMD_CHKSUMM    0xFF

#define BOOT_CMD_HELLO      0x7F
#define BOOT_CMD_ACK        0x79
#define BOOT_CMD_LIST       0x00    
#define BOOT_CMD_01         0x00    /* 79 31 00 00 79 */
#define BOOT_CMD_02         0x00    /* 79 01 04 17 79 */




#endif //__BOOTLOADER_OPT_H__
