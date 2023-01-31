/* vmalloc.c - vmalloc, vfree */

#include <xinu.h>

char* vmalloc(uint32 nbytes) {
    uint32 i,j;
    pd_t PDE;
    pt_t PTE;
    virt_addr_t start;
    uint32 counter = 0;
    char * ptr = NULL;

    intmask mask = disable();
    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    phy_addr_t PD_addr = proctab[currpid].page_dir;

    for (i = (XINU_PAGES >> 10); i < 1024; i++) {
        PDE = get_PDE_virt(PD_addr.fm_num, i);
        if (PDE.pd_valid == FALSE) {
            // debug_print("~~~vmalloc: PDE INVALID~~~\n");
            if (counter == 0) {
                start.pd_offset = i;
                start.pt_offset = 0;
            }
            counter += 1024;
            if ((counter << 12) >= nbytes) {
                ptr = vfound(start, nbytes);
                set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
                restore(mask);
                return ptr;
            }
        }
        else {
            // debug_print("~~~vmalloc: PDE VALID~~~\n");
            for (j = 0; j < 1024; j++) {
                PTE = get_PTE_virt(PDE.pd_base, j);
                if (PTE.pt_valid == FALSE) {
                    if (counter == 0) {
                        start.pd_offset = i;
                        start.pt_offset = j;
                    }
                    counter++;
                    if ((counter << 12) >= nbytes) {
                        ptr = vfound(start, nbytes);
                        set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
                        restore(mask);
                        return ptr;
                    }
                }
                else {
                    counter = 0;
                }
            }
        }
    }

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    restore(mask);
    return (char *)SYSERR;
}

char* vfound(virt_addr_t addr, uint32 nbytes) {
    uint32 i,j;
    phy_addr_t PD_addr = proctab[currpid].page_dir, PT_addr;
    pd_t PDE;
    uint32 index, j_start = addr.pt_offset, empty_page_list_indices = 0;

    uint32 numPages = (nbytes >> 12) + (uint32)((nbytes % (1 << 12)) != 0);
    uint32 numPageTables = (numPages >> 10) + (uint32)((numPages % (1 << 10)) != 0);

    for (i = 0; i < MAX_PT_SIZE; i++) {
        if (page_list[i].valid == FALSE) empty_page_list_indices++;
    }

    if (numPageTables > empty_page_list_indices) return (char *)SYSERR;

    // debug_print("~~~vfound~~~\n");

    for (i = addr.pd_offset; i < 1024; i++) {
        PDE = get_PDE_virt(PD_addr.fm_num, i);
        if (PDE.pd_valid == FALSE) {
            // debug_print("~~~NEW PDE~~~\n");
            index = new_PD_PT();
            set_PDE(fm_index(PD_addr), i, make_PDE(1, 1, page_list[index].addr.fm_num));
            PDE = get_PDE_virt(PD_addr.fm_num, i);
        }
        for (j = j_start; j < 1024; j++) {
            // debug_print("~~~NEW PTE~~~\n");
            numPages--;
            PT_addr.fm_num = PDE.pd_base;
            set_PTE(fm_index(PT_addr), j, make_PTE(0, 1, 0));
            if (numPages == 0) return (char *)((addr.pd_offset << 22) + (addr.pt_offset << 12));
        }
        j_start = 0;
    }

    return (char *)SYSERR;
}

syscall vfree(char* ptr, uint32 nbytes) {
    uint32 i,j;
    pd_t PDE;
    pt_t PTE;
    virt_addr_t addr;
    uint32 counter = 0;
    syscall signal;

    intmask mask = disable();
    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    phy_addr_t PD_addr = proctab[currpid].page_dir;
    addr.pd_offset = ((uint32)ptr) >> 22;
    addr.pt_offset = (((uint32)ptr) & 0x003FF000) >> 12;

    uint32 j_start = addr.pt_offset;

    for (i = addr.pd_offset; i < 1024; i++) {
        PDE = get_PDE_virt(PD_addr.fm_num, i);
        if (PDE.pd_valid == FALSE) {
            set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
            restore(mask);
            return SYSERR;
        }
        else {
            for (j = j_start; j < 1024; j++) {
                PTE = get_PTE_virt(PDE.pd_base, j);
                if (PTE.pt_valid == FALSE) {
                    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
                    restore(mask);
                    return SYSERR;
                }
                counter++;
                if ((counter << 12) >= nbytes) {
                    signal = vdealloc(addr, nbytes);
                    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
                    restore(mask);
                    return signal;
                }
            }
        }
        j_start = 0;
    }

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    restore(mask);
    return SYSERR;
}

syscall vdealloc(virt_addr_t addr, uint32 nbytes) {
    uint32 i,j;
    pd_t PDE;
    pt_t PTE;
    phy_addr_t frame;

    phy_addr_t PD_addr = proctab[currpid].page_dir, PT_addr;
    uint32 numPages = (nbytes >> 12) + (uint32)((nbytes % (1 << 12)) != 0);

    uint32 j_start = addr.pt_offset;

    for (i = addr.pd_offset; i < 1024; i++) {
        PDE = get_PDE_virt(PD_addr.fm_num, i);
        for (j = j_start; j < 1024; j++) {
            PTE = get_PTE_virt(PDE.pd_base, j);
            frame.fm_num = PTE.pt_base;
            frame_list[fm_index(frame)].valid = FALSE;
            numPages--;
            PT_addr.fm_num = PDE.pd_base;
            set_PTE(fm_index(PT_addr), j, make_PTE(0, 0, 0));
            if (numPages == 0) return OK;
        }
        j_start = 0;
    }

    return SYSERR;
}
