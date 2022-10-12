/* burst.c - burst_execution */

#include <xinu.h>

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