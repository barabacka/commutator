#include "common.h"

void hex ( uint8_t * dt, int size )
{
#define HEX_PRINT_STR   { sprintf ( pstr, "\n" ); msg ( str ); }
    char    str[3 * 0x11];
    char  * pstr = str;
    int     cnt = 0;

    while (size--)
    {
        sprintf ( pstr, "%.2X ", *dt++ );
        pstr += 3;
        cnt++;
        if ( cnt == 0x10 ){
            HEX_PRINT_STR
            pstr = str;
            cnt = 0;
        }
    }

    if ( cnt )
        HEX_PRINT_STR
}
