#include "..\Include\spinmaster.h"

/*
 *@ brief    : Run the key-controlled SPU playback example.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : Does not normally return.
 *@ theme    : Run button scanning through software timers and preserve watchdog service.
 */
int main(void)
{
    bsp_init_global();
    bsp_run_app();
    return 0;
}
