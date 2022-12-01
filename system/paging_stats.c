/* paging_stats.c - functions to return paging statistics */

#include <xinu.h>

uint32 free_ffs_pages() {
    uint32 free_pages = 0;
    int i;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    for (i=0; i < MAX_FFS_SIZE; i++) { //step through frame list
        if (frame_list[i].valid) continue; //if frame is valid, it is not free
		else ++free_pages; //invalid frames are counted
    }
    
    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
	return free_pages;
}

uint32 free_swap_pages() {
	uint32 free_pages = 0;
    int i;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    for (i=0; i < MAX_SWAP_SIZE; i++) { //step through frame list
        if (swap_list[i].valid) continue; //if frame is valid, it is not free
		else ++free_pages; //invalid frames are counted
    }

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
	return free_pages;
}

uint32 allocated_virtual_pages(pid32 pid) {
    uint32 i,j;
    phy_addr_t PD_addr = proctab[pid].page_dir;
    pd_t PDE;
    pt_t PTE;
    uint32 count = 0;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    for (i = 0; i < 1024; i++) {
        PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
        if (PDE.pd_valid == TRUE) {
            for (j = 0; j < 1024; j++) {
                PTE = *(pt_t *)((PDE.pd_base << 12) + (j << 2));
                if (PTE.pt_valid == TRUE) {
                    count++;
                }
            }
        }
    }

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
	return count;
}

uint32 used_ffs_frames(pid32 pid) {
    uint32 i,j;
    phy_addr_t PD_addr = proctab[pid].page_dir;
    pd_t PDE;
    pt_t PTE;
    uint32 count = 0;

    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    for (i = (XINU_PAGES >> 10); i < 1024; i++) {
        PDE = *(pd_t *)((PD_addr.fm_num << 12) + (i << 2));
        if ((PDE.pd_valid == TRUE) && (PDE.pd_pres == TRUE)) {
            for (j = 0; j < 1024; j++) {
                PTE = *(pt_t *)((PDE.pd_base << 12) + (j << 2));
                if ((PTE.pt_valid == TRUE) && (PTE.pt_pres == TRUE)) {
                    count++;
                }
            }
        }
    }

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
	return count;
}
