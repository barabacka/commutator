#include "common.h"
#include "ver.h"

//-----------------------------------------------
int main ( int argc, char** argv )
{
    msg ( "Commutator: Raspberry PI onboard STM32 flasher by Barabaka %s v%s\n", __DATE__, ver );
    return fsm ( argc, argv );
}



