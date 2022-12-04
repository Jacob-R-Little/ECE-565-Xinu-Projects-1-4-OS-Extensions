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
        PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
        if (PDE.pd_valid == FALSE) {
            counter += 1024;
        }
        else {
            for (j = 0; j < 1024; j++) {
                PTE = *(pt_t *)((PDE.pd_base << 12) + (j << 2));
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
    uint32 index, j_start = addr.pt_offset;

    uint32 numPages = (nbytes >> 12) + (uint32)((nbytes % (1 << 12)) != 0);

    for (i = addr.pd_offset; i < 1024; i++) {
        PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
        if (PDE.pd_valid == FALSE) {
            index = new_PD_PT();
            set_PDE(index, i, make_PDE(1, 1, page_list[index].addr.fm_num));
        }
        for (j = j_start; j < 1024; j++) {
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
        PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
        if (PDE.pd_valid == FALSE) {
            set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
            restore(mask);
            return SYSERR;
        }
        else {
            for (j = j_start; j < 1024; j++) {
                PTE = *(pt_t *)((PDE.pd_base << 12) + (j << 2));
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

    intmask mask = disable();
    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    phy_addr_t PD_addr = proctab[currpid].page_dir, PT_addr;
    uint32 numPages = (nbytes >> 12) + (uint32)((nbytes % (1 << 12)) != 0);

    uint32 j_start = addr.pt_offset;

    for (i = addr.pd_offset; i < 1024; i++) {
        PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
        for (j = j_start; j < 1024; j++) {
            PTE = *(pt_t *)((PDE.pd_base << 12) + (j << 2));
            frame.fm_num = PTE.pt_base;
            frame_list[fm_index(frame)].valid = FALSE;
            numPages--;
            PT_addr.fm_num = PDE.pd_base;
            set_PTE(fm_index(PT_addr), j, make_PTE(0, 0, 0));
            if (numPages == 0) return OK;
        }
        j_start = 0;
    }

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    restore(mask);
    return SYSERR;
}
