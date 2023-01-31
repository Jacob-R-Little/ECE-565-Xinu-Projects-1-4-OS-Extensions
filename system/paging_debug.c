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
        set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

        uint32 i,j;
        phy_addr_t PD_addr = proctab[pid].page_dir;
        pd_t PDE;
        pt_t PTE;

        uint32 i_range = 10; // 1024
        uint32 j_range = 1024; // 1024

        debug_print("\n~~~~~ P%d Page Directory (%05x) ~~~~~\n\n", pid, PD_addr.fm_num);

        for (i = 0; i < i_range; i++) {
            PDE = get_PDE_virt(PD_addr.fm_num, i);
            if (PDE.pd_valid == TRUE) {
                for (j = 0; j < j_range; j++) {
                    PTE = get_PTE_virt(PDE.pd_base, j);
                    if (PTE.pt_valid == TRUE) {
                        if (!(j % 8)) debug_print("\n%05x | ", PDE.pd_base);
                        debug_print("%05x %05x | ", (i << 10) + j, PTE.pt_base);
                    }
                }
                debug_print("\n");
            }
        }

        debug_print("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", pid);

        set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
        restore(mask);
    #endif
}

void debug_print_PD_Heap(pid32 pid) {
    #ifdef DEBUG
        intmask mask = disable();
        set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

        uint32 i,j;
        phy_addr_t PD_addr = proctab[pid].page_dir;
        pd_t PDE;
        pt_t PTE;

        uint32 i_range = 1024; // 1024
        uint32 j_range = 32; // 1024

        debug_print("\n~~~~~ P%d Page Directory (%05x) ~~~~~\n\n", pid, PD_addr.fm_num);

        for (i = (XINU_PAGES >> 10); i < i_range; i++) {
            PDE = get_PDE_virt(PD_addr.fm_num, i);
            if (PDE.pd_valid == TRUE) {
                for (j = 0; j < j_range; j++) {
                    PTE = get_PTE_virt(PDE.pd_base, j);
                    if (PTE.pt_valid == TRUE) {
                        if (!(j % 8)) debug_print("\n%05x | ", PDE.pd_base);
                        debug_print("%05x %05x | ", (i << 10) + j, PTE.pt_base);
                    }
                }
                debug_print("\n");
            }
        }

        debug_print("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", pid);

        set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
        restore(mask);
    #endif
}

void debug_verify_PD_system_pages(void) {
    #ifdef DEBUG
        intmask mask = disable();
        set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

        uint32 i,j;
        phy_addr_t PD_addr = proctab[NULLPROC].page_dir;
        pd_t PDE;
        pt_t PTE;
        bool8 verified = TRUE;

        debug_print("\n~~~~~ P%d Page Directory Verify System Pages (%05x) ~~~~~\n\n", NULLPROC, PD_addr.fm_num);

        for (i = 0; i < ((XINU_PAGES + MAX_FFS_SIZE + MAX_PT_SIZE + MAX_SWAP_SIZE) >> 10); i++) {
            PDE = get_PDE_virt(PD_addr.fm_num, i);
            for (j = 0; j < 1024; j++) {
                PTE = get_PTE_virt(PDE.pd_base, j);
                if (((i << 10) + j) != (PTE.pt_base)) {
                    verified = FALSE;
                    if (j < 4) debug_print("%05x | %05x %05x\n", PDE.pd_base, (i << 10) + j, PTE.pt_base);
                }
            }
        }
        if (verified) debug_print("\n~~~~~ P%d System Page Directory VERIFIED!!! (%05x) ~~~~~\n\n", NULLPROC, PD_addr.fm_num);
        else debug_print("\n~~~~~ P%d System Page Directory INCORRECT (%05x) ~~~~~\n\n", NULLPROC, PD_addr.fm_num);

        set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
        restore(mask);
    #endif
}

void debug_verify_PD_xinu_pages(pid32 pid) {
    #ifdef DEBUG
        intmask mask = disable();
        set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

        uint32 i,j;
        phy_addr_t PD_addr = proctab[pid].page_dir;
        pd_t PDE;
        pt_t PTE;
        bool8 verified = TRUE;

        debug_print("\n~~~~~ P%d Page Directory Verify Xinu Pages (%05x) ~~~~~\n\n", pid, PD_addr.fm_num);

        for (i = 0; i < (XINU_PAGES >> 10); i++) {
            PDE = get_PDE_virt(PD_addr.fm_num, i);
            for (j = 0; j < 1024; j++) {
                PTE = get_PTE_virt(PDE.pd_base, j);
                if (((i << 10) + j) != (PTE.pt_base)) {
                    verified = FALSE;
                    if (j < 4) debug_print("%05x | %05x %05x\n", PDE.pd_base, (i << 10) + j, PTE.pt_base);
                }
            }
        }
        if (verified) debug_print("\n~~~~~ P%d Page Directory VERIFIED!!! (%05x) ~~~~~\n\n", pid, PD_addr.fm_num);
        else debug_print("\n~~~~~ P%d Page Directory INCORRECT (%05x) ~~~~~\n\n", pid, PD_addr.fm_num);

        set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
        restore(mask);
    #endif
}

syscall print_ready_list() {
    #ifdef DEBUG
        intmask mask = disable();	/* Interrupt mask		*/
        qid16	next = firstid(readylist);
        qid16	tail = queuetail(readylist);
        

        kprintf("\n\n---READYLIST---\n");

        while(next != tail) {
            kprintf("%d\n", (uint32)next);
            next = queuetab[next].qnext;
        }

        kprintf("---------------\n\n");

        restore(mask);
    #endif
	return OK;
}

void debug_print_page_list_valid_bits() {
    #ifdef DEBUG
        uint32 i;
        for (i = 0; i < MAX_PT_SIZE; i++) {
            if (!(i % 8)) debug_print("\n%3x | ", i);
            debug_print("%d", page_list[i].valid);
        }
        debug_print("\n");
    #endif
}
