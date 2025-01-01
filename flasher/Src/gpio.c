#include "common.h"
#include <fcntl.h>
#include <sys/mman.h>

//Based on https://elinux.org/RPi_GPIO_Code_Samples#Direct_register_access

#define RASPI_ZERO              1

#ifdef RASPI_ZERO
#define BCM2708_PERI_BASE       0x20000000
#else
#define BCM2708_PERI_BASE       0x3F000000
#endif
#define GPIO_BASE               ( BCM2708_PERI_BASE + 0x200000 ) /* GPIO controller */

#define INP_GPIO(g)             *(gpio_map+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)             *(gpio_map+((g)/10)) |=  (1<<(((g)%10)*3))

#define GPIO_SET                *(gpio_map+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR                *(gpio_map+10) // clears bits which are 1 ignores bits which are 0

#define PAGE_SIZE               (4*1024)
#define BLOCK_SIZE              (4*1024)

#define BIT_POS(b1)             (1<<(b1))
#define TWO_BITS_POS(b1,b2)     ((1<<(b1))|(1<<(b2)))


// I/O access
volatile unsigned * gpio_map;

//-----------------------------------------------
// Set up a memory regions to access GPIO
//-----------------------------------------------
static int setup_io ( void )
{
    int retval = OK;
    int  mem_fd;
    void * gpio_map_t;

    do{
        /* open /dev/mem */
        if ( ( mem_fd = open("/dev/mem", O_RDWR | O_SYNC ) ) < 0 ) {
            retval = ERR_OPEN_MEM;
            break;
        }
        
        /* mmap GPIO */
        gpio_map_t = mmap(
            NULL,             //Any adddress in our space will do
            BLOCK_SIZE,       //Map length
            PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
            MAP_SHARED,       //Shared with other processes
            mem_fd,           //File to map
            GPIO_BASE         //Offset to GPIO peripheral
        );
        
        close ( mem_fd ); //No need to keep mem_fd open after mmap   
        
        if ( gpio_map_t == MAP_FAILED ) {
            retval = ERR_MMAP;
            break;
        }

        // Always use volatile pointer!
        gpio_map = ( volatile unsigned * ) gpio_map_t;
    }while ( 0 );

    return retval;
}

//-----------------------------------------------
void gpio_init ( GPIO_PARAM_T * gpio )
{
    do{
        if ( !gpio->use || !gpio->rst || !gpio->boot0 )
            break;
        if ( setup_io ( ) != OK ){
            gpio->use = 0;
            break;
        }
        //clear out GPIO state
        GPIO_CLR = TWO_BITS_POS( gpio->rst, gpio->boot0 );
        //before set GPIO mode as output set them as input
        INP_GPIO ( gpio->rst ); 
        INP_GPIO ( gpio->boot0 );
        OUT_GPIO ( gpio->rst ); 
        OUT_GPIO ( gpio->boot0 );
    }while ( 0 );  
}
//-----------------------------------------------
void gpio_reset ( GPIO_PARAM_T * gpio, int boot )
{
    if ( gpio->use ){
        //set RST
        GPIO_SET = BIT_POS( gpio->rst );
        //manage boot0
        if ( boot ){
            GPIO_SET = BIT_POS( gpio->boot0 );
        }else{
            GPIO_CLR = BIT_POS( gpio->boot0 );
        }
        usleep ( 100000 );
        //clear RST
        GPIO_CLR = BIT_POS ( gpio->rst );
        usleep ( 100000 );
    } 
}
//-----------------------------------------------
void gpio_finish ( GPIO_PARAM_T * gpio )
{
    if ( gpio->use ){
        //clear out GPIO state
        GPIO_CLR = TWO_BITS_POS( gpio->rst, gpio->boot0 );
        INP_GPIO ( gpio->rst ); 
        INP_GPIO ( gpio->boot0 );
        munmap ( ( void * ) gpio_map, BLOCK_SIZE );
    }
}
