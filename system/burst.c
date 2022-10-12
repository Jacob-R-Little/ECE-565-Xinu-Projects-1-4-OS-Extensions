/* burst.c - burst_execution */

#include <xinu.h>

// #define DEBUG_BURST

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
        #ifdef DEBUG_BURST
            if (currpid == 11) burst_sync_printf("%d | LOOP: %d | EXEC: %d to %d\n", currpid, i, time_capture, time_capture + burst_duration);
        #endif
        while (prptr->runtime < time_capture + burst_duration);
        #ifdef DEBUG_BURST
            if (currpid == 11) burst_sync_printf("Runtime before Sleep: %d ", prptr->runtime);
        #endif
        sleepms(sleep_duration);
        #ifdef DEBUG_BURST
            if (currpid == 11) burst_sync_printf("| Runtime after Wake Up: %d\n", prptr->runtime);
        #endif
    }

}