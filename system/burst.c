/* burst.c - burst_execution */

#include <xinu.h>

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
        //kprintf("EXECUTE from %d to %d\n", time_capture, time_capture + burst_duration);
        while (prptr->runtime < time_capture + burst_duration);
        //kprintf("Sleep from %d to %d\n", ctr1000, ctr1000 + sleep_duration);
        sleepms(sleep_duration);
    }

}