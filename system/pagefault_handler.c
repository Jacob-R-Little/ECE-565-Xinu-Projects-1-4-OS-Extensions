/* pagefault_handler.c - pagefault_handler */

#include <xinu.h>

void pagefault_handler(void) {
    virt_addr_t pgfault_addr;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    uint32 cr2 = read_cr2();
    pgfault_addr.pd_offset = cr2 >> 22;
    pgfault_addr.pt_offset = (cr2 & 0x003FF000) >> 12;

    phy_addr_t PD_addr = proctab[currpid].page_dir, PT_addr;

    pd_t PDE = get_PDE_virt(PD_addr.fm_num, pgfault_addr.pd_offset);
    pt_t PTE = get_PTE_virt(PDE.pd_base, pgfault_addr.pt_offset);

    if ((PDE.pd_valid == FALSE) || (PTE.pt_valid == FALSE)) {   // Segmentation Fault
        kprintf("P%d:: SEGMENTATION_FAULT\n", currpid);
        kill(currpid);
        set_PDBR(proctab[NULLPROC].page_dir);    // change to system virtual address space
        return;
    }

    // Lazy Allocation
    uint32 index = new_frame();
    PT_addr.fm_num = PDE.pd_base;
    set_PTE(fm_index(PT_addr), pgfault_addr.pt_offset, make_PTE(1, 1, frame_list[index].addr.fm_num));

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    return;
}