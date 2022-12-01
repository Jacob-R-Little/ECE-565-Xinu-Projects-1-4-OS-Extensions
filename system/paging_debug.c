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

void debug_print_PD(pid32 pid) {
    #ifdef DEBUG
        intmask mask = disable();
        uint32 i,j;
        phy_addr_t PD_addr = proctab[pid].page_dir;
        pd_t PDE;
        pt_t PTE;

        uint32 i_range = 10; // 1024
        uint32 j_range = 1024; // 1024

        debug_print("\n~~~~~ P%d Page Directory ~~~~~\n\n", pid);

        for (i = 0; i < i_range; i++) {
            PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
            if (PDE.pd_valid == TRUE) {
                for (j = 0; j < j_range; j++) {
                    PTE = *(pt_t *)((PDE.pd_base << 12) + (j << 2));
                    if (PTE.pt_valid == TRUE) {
                        if (!(j % 8)) debug_print("\n%8x | ", PDE.pd_base);
                        debug_print("%05x %05x | ", (i << 10) + j, PTE.pt_base);
                    }
                }
                debug_print("\n");
            }
        }

        debug_print("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", pid);

        restore(mask);
    #endif
}
