#pragma once

#include "common.h"

typedef uint8_t core_idx;

typedef enum {
    MEM_FLASH = 2,
    MEM_SRAM,
    MEM_USER_OPT,
    MEM_SYS,
    MEM_EEPROM,
    MEM_PAGES,
    MEM_MAX
} MEM_TYPE_T;

typedef struct {
    MEM_TYPE_T  type;
    uint32_t    start;
        union{
            uint32_t    size;
            struct{
                uint16_t    pg_num;
                uint16_t    pg_size;
            };
        };
}MEM_REGION_T;

typedef struct {
    uint8_t      num;
    MEM_REGION_T regs[];
}MEM_REGIONS_T;

typedef struct {
    uint8_t             id;
    const char          * name;
    const MEM_REGIONS_T * mem;
} STM32_MCU_INFO_T;

//according to https://github.com/stlink-org/stlink/blob/testing/inc/stm32.h

const char mcu_unk[] = "X";
const char mcu_17[] = "STM32_L0_CAT3";
const MEM_REGIONS_T mem_17= {
    6, {
        { MEM_FLASH,    0x08000000, .size = 0x10000 },
        { MEM_PAGES,    0x08000000, .pg_num = 0x200, .pg_size = 0x80 },
        { MEM_SRAM,     0x20001000, .size = 0x1000  },
        { MEM_USER_OPT, 0x1FF80000, .size = 0x20    },
        { MEM_SYS,      0x1FF00000, .size = 0x1000  },
        { MEM_EEPROM,   0x08080000, .size = 0x800   },
    }
};

const STM32_MCU_INFO_T STM32[]={
    { 0x17, mcu_17, &mem_17 },
    {0}
};
