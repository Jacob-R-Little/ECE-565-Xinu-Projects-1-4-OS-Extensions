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
    uint32 time_capture;
    struct procent *prptr = &proctab[currpid];
    for (i=0; i<number_bursts; i++) {
        time_capture = prptr->runtime;
        //burst_sync_printf("%d EXECUTE from %d to %d\n", currpid, time_capture, time_capture + burst_duration);
        while (prptr->runtime < time_capture + burst_duration);
        //burst_sync_printf("%d SLEEP from %d to %d\n", currpid, ctr1000, ctr1000 + sleep_duration);
        sleepms(sleep_duration);
    }

}