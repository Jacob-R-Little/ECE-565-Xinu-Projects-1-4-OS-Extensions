/* burst.c - burst_execution */

#include <xinu.h>

void burst_sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
}

void burst_execution(
    uint32 number_bursts,
    uint32 burst_duration,
    uint32 sleep_duration
    )
{
    uint32 i;
    struct procent *prptr = &proctab[currpid];
    
    for (i=1; i<number_bursts+1; i++) {
        while (prptr->runtime < burst_duration * i);
        sleepms(sleep_duration);
    }

}