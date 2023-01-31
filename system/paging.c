/* paging.c - functions to enable paging */

#include <xinu.h>

void init_paging(void) {
    uint32 i, j, index;
    phy_addr_t xinu_addr;

    /* Initialize arrays to store memory information */

	for (i = 0; i < MAX_FFS_SIZE; i++) {
		frame_list[i].addr.fm_num = XINU_PAGES + i;
		frame_list[i].valid = FALSE;
	}

	for (i = 0; i < MAX_PT_SIZE; i++) {
		page_list[i].addr.fm_num = XINU_PAGES + MAX_FFS_SIZE + i;
		page_list[i].valid = FALSE;
	}

	for (i = 0; i < MAX_SWAP_SIZE; i++) {
		swap_list[i].addr.fm_num = XINU_PAGES + MAX_FFS_SIZE + MAX_PT_SIZE + i;
		swap_list[i].valid = FALSE;
	}

	/* Initialize Xinu Pages, FFS Pages, PT Pages, and Swap Pages */	

	for (i = 0; i < ((XINU_PAGES + MAX_FFS_SIZE + MAX_PT_SIZE + MAX_SWAP_SIZE) >> 10); i++) {
		index = new_PD_PT();
		for (j = 0; j < 1024; j++) {
			xinu_addr.fm_num = i * 1024 + j;
			new_PTE(index, xinu_addr);
		}
	}

	/* Initialize System Page Directory */

	index = new_PD_PT();	//create a new page directory for system

	proctab[currpid].page_dir = page_list[index].addr;

	//initialize the new page directory with xinu pages, FFS pages, PT pages, and Swap pages
	for (i = 0; i < ((XINU_PAGES + MAX_FFS_SIZE + MAX_PT_SIZE + MAX_SWAP_SIZE) >> 10); i++) {
		xinu_addr.fm_num = page_list[i].addr.fm_num;
		new_PDE(index, xinu_addr);
	}

	set_evec(14, (uint32)pagefault_handler_disp);

	set_PDBR(proctab[currpid].page_dir);
	debug_print("About to start paging\n");
	enable_paging();
	debug_print("We paging now\n");
}

void init_PD(pid32 pid) {
	uint32 i;
	phy_addr_t xinu_addr;
	uint32 index = new_PD_PT();	//create a new page directory for process

	proctab[pid].page_dir = page_list[index].addr;

	//initialize the new page directory with xinu pages
	for (i = 0; i < (XINU_PAGES >> 10); i++) {
		xinu_addr.fm_num = page_list[i].addr.fm_num;
		new_PDE(index, xinu_addr);
	}
}

uint32 new_PD_PT(void) {
    uint32 i;

    for (i = 0; i < MAX_PT_SIZE; i++) {
        if (page_list[i].valid == FALSE) {
            page_list[i].valid = TRUE;
            return i;
        }
    }

    return MAX_PT_SIZE;	//return max size of the page table area upon failure
}

uint32 new_frame(void) {
    uint32 i;

    for (i = 0; i < MAX_FFS_SIZE; i++) {
        if (frame_list[i].valid == FALSE) {
            frame_list[i].valid = TRUE;
            return i;
        }
    }

    return MAX_FFS_SIZE;	//return max size of the page table area upon failure
}

pd_t make_PDE(bool8 pres, bool8 valid, uint32 base) {
	pd_t PDE;

	PDE.pd_pres 	= pres;		/* page table present?		*/
	PDE.pd_write 	= 1;		/* page is writable?		*/
	PDE.pd_user 	= 0;		/* is use level protection?	*/
	PDE.pd_pwt 	    = 1;		/* write through cachine for pt?*/
	PDE.pd_pcd	    = 1;		/* cache disable for this pt?	*/
	PDE.pd_acc	    = 1;		/* page table was accessed?	*/
	PDE.pd_mbz	    = 0;		/* must be zero			*/
	PDE.pd_fmb	    = 0;		/* four MB pages?		*/
	PDE.pd_global	= 0;		/* global (ignored)		*/
    PDE.pd_valid    = valid;
	PDE.pd_avail 	= 0;		/* for programmer's use		*/
	PDE.pd_base	    = base;		/* location of page table?	*/

	return PDE;
}

pt_t make_PTE(bool8 pres, bool8 valid, uint32 base) {
	pt_t PTE;

	PTE.pt_pres		= pres;		    /* page is present?		*/
	PTE.pt_write 	= 1;		    /* page is writable?		*/
	PTE.pt_user		= 0;		    /* is use level protection?	*/
	PTE.pt_pwt		= 1;		    /* write through for this page? */
	PTE.pt_pcd		= 1;		    /* cache disable for this page? */
	PTE.pt_acc		= 1;		    /* page was accessed?		*/
	PTE.pt_dirty 	= 0;		    /* page was written?		*/
	PTE.pt_mbz		= 0;		    /* must be zero			*/
	PTE.pt_global	= 0;		    /* should be zero in 586	*/
    PTE.pt_valid    = valid;
	PTE.pt_avail 	= 0;			/* for programmer's use		*/
	PTE.pt_base		= base;			/* location of page?		*/

	return PTE;
}

uint32 new_PDE(uint32 pg_dir, phy_addr_t addr) {
    pd_t xinu_pd;
    uint32 entry = 1024, i;

    for (i = 0; i < 1024; i++) {
        xinu_pd = get_PDE(pg_dir, i);
        if (xinu_pd.pd_valid == 0) {
            entry = i;
            break;
        }
    }
    
    if (entry == 1024) return entry;    // Page Table Full

    set_PDE(pg_dir, entry, make_PDE(1, 1, addr.fm_num));

    return entry;
}

uint32 new_PTE(uint32 pg_tab, phy_addr_t addr) {
    pt_t xinu_pt;
    uint32 entry = 1024, i;

    for (i = 0; i < 1024; i++) {
        xinu_pt = get_PTE(pg_tab, i);
        if (xinu_pt.pt_valid == 0) {
            entry = i;
            break;
        }
    }
    
    if (entry == 1024) return entry;    // Page Directory Full

    set_PTE(pg_tab, entry, make_PTE(1, 1, addr.fm_num));

    return entry;
}

void set_PDE(uint32 pg_dir, uint32 entry, pd_t pde) {
    *(pd_t *)((page_list[pg_dir].addr.fm_num << 12) + (entry << 2)) = pde;
}

void set_PTE(uint32 pg_tab, uint32 entry, pt_t pte) {
    *(pt_t *)((page_list[pg_tab].addr.fm_num << 12) + (entry << 2)) = pte;
}

pd_t get_PDE(uint32 pg_dir, uint32 entry) {
    return *(pd_t *)((page_list[pg_dir].addr.fm_num << 12) + (entry << 2));
}

pt_t get_PTE(uint32 pg_tab, uint32 entry) {
    return *(pt_t *)((page_list[pg_tab].addr.fm_num << 12) + (entry << 2));
}

pd_t get_PDE_virt(uint32 frame, uint32 entry) {
	return *(pd_t *)((frame << 12) + (entry << 2));
}

pt_t get_PTE_virt(uint32 frame, uint32 entry) {
	return *(pt_t *)((frame << 12) + (entry << 2));
}

uint32 fm_index(phy_addr_t addr) {
	if (addr.fm_num >= XINU_PAGES + MAX_FFS_SIZE + MAX_PT_SIZE)	// swap area address
		return addr.fm_num - XINU_PAGES - MAX_FFS_SIZE - MAX_PT_SIZE;
	if (addr.fm_num >= XINU_PAGES + MAX_FFS_SIZE)	// page table area address
		return addr.fm_num - XINU_PAGES - MAX_FFS_SIZE;
	if (addr.fm_num >= XINU_PAGES)	// FFS area address
		return addr.fm_num - XINU_PAGES;
	return addr.fm_num;	// XINU area page number
}

uint32 PID_list_index(pid32 pid) {
	return fm_index(proctab[pid].page_dir);
}

void kill_user(pid32 pid) {
	uint32 i,j;
	phy_addr_t PD_addr = proctab[pid].page_dir;
	phy_addr_t PT_addr;
	phy_addr_t FR_addr;
	pd_t PDE;
	pt_t PTE;

	for (i = 0; i < (XINU_PAGES >> 10); i++)
		set_PDE(fm_index(PD_addr), i, make_PDE(0, 0, 0));

	for (i = (XINU_PAGES >> 10); i < 1024; i++) {
		PDE = get_PDE_virt(PD_addr.fm_num, i);
		PT_addr.fm_num = PDE.pd_base;
		if (PDE.pd_valid == TRUE) {
			for (j = 0; j < 1024; j++) {
				PTE = get_PTE_virt(PDE.pd_base, j);
				FR_addr.fm_num = PTE.pt_base;
				if (PTE.pt_valid == TRUE) {
					set_PTE(fm_index(PT_addr), j, make_PTE(0, 0, 0));
					frame_list[fm_index(FR_addr)].valid = FALSE;	// deallocate frame
				}
			}
			set_PDE(fm_index(PD_addr), i, make_PDE(0, 0, 0));
			page_list[fm_index(PT_addr)].valid = FALSE;	// deallocate page table
		}
	}

	page_list[PID_list_index(pid)].valid = FALSE;	// deallocate page directory

}