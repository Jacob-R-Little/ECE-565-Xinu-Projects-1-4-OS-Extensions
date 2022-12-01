/* paging_debug.c - functions to debug paging */

#include <xinu.h>

void debug_print(char *fmt, ...) {       
    #ifdef DEBUG
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
    #endif
}
