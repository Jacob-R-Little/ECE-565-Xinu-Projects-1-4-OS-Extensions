/* pagefault_handler.c - pagefault_handler */

#include <xinu.h>

void pagefault_handler(void) {
    virt_addr_t pgfault_addr;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    // debug_print("P%d:: PAGE_FAULT_HANDLER\n", currpid);

    uint32 cr2 = read_cr2();
    // debug_print("P%d:: CR2 = %x\n", currpid, cr2);
    pgfault_addr.pd_offset = cr2 >> 22;
    pgfault_addr.pt_offset = (cr2 & 0x003FF000) >> 12;

    phy_addr_t PD_addr = proctab[currpid].page_dir, PT_addr;

    pd_t PDE = get_PDE_virt(PD_addr.fm_num, pgfault_addr.pd_offset);
    pt_t PTE = get_PTE_virt(PDE.pd_base, pgfault_addr.pt_offset);

    if ((PDE.pd_valid == FALSE) || (PTE.pt_valid == FALSE)) {   // Segmentation Fault
        kprintf("P%d:: SEGMENTATION_FAULT\n", currpid);
        // if (PDE.pd_valid == FALSE) debug_print("P%d:: PDE INVALID\n", currpid);
        // if (PTE.pt_valid == FALSE) debug_print("P%d:: PTE INVALID\n", currpid);
        kill(currpid);
        set_PDBR(proctab[NULLPROC].page_dir);    // change to system virtual address space
        return;
    }

    // debug_print("P%d:: PG_FAULT_LAZY_ALLOC\n", currpid);

    // Lazy Allocation
    uint32 index = new_frame();
    PT_addr.fm_num = PDE.pd_base;
    set_PTE(fm_index(PT_addr), pgfault_addr.pt_offset, make_PTE(1, 1, frame_list[index].addr.fm_num));

    // debug_print("P%d:: PG_FAULT_LAZY_ALLOC_DONE\n", currpid);

    // debug_print_PD_Heap(currpid);

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    return;
}