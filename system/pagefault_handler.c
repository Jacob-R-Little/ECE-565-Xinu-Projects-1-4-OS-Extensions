/* pagefault_handler.c - pagefault_handler */

#include <xinu.h>

void pagefault_handler(void) {
    virt_addr_t pgfault_addr;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    uint32 cr2 = read_cr2();
    pgfault_addr.pd_offset = cr2 >> 22;
    pgfault_addr.pt_offset = (cr2 & 0x003FF000) >> 12;

    phy_addr_t PD_addr = proctab[currpid].page_dir;

    pd_t PDE = *(pd_t *)((PD_addr.fm_num << 12) + (pgfault_addr.pd_offset << 2));
    pt_t PTE = *(pt_t *)((PD_addr.fm_num << 12) + (pgfault_addr.pt_offset << 2));

    if ((PDE.pd_valid == FALSE) || (PTE.pt_valid == FALSE)) {   // Segmentation Fault
        kprintf("P%d:: SEGMENTATION_FAULT\n", currpid);
        kill(currpid);
        set_PDBR(proctab[NULLPROC].page_dir);    // change to system virtual address space
        return;
    }

    // Lazy Allocation
    uint32 index = new_frame();
    set_PTE(index, pgfault_addr.pt_offset, make_PTE(1, 1, page_list[index].addr.fm_num));

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    return;
}