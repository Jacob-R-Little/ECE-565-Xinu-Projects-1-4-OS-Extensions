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

	xinu_pd.pd_pres 	= 1;	/* page table present?		*/
	xinu_pd.pd_write 	= 1;	/* page is writable?		*/
	xinu_pd.pd_user 	= 0;	/* is use level protection?	*/
	xinu_pd.pd_pwt 	    = 1;	/* write through cachine for pt?*/
	xinu_pd.pd_pcd	    = 1;	/* cache disable for this pt?	*/
	xinu_pd.pd_acc	    = 1;	/* page table was accessed?	*/
	xinu_pd.pd_mbz	    = 0;	/* must be zero			*/
	xinu_pd.pd_fmb	    = 0;	/* four MB pages?		*/
	xinu_pd.pd_global	= 0;	/* global (ignored)		*/
    xinu_pd.pd_valid    = 1;
	xinu_pd.pd_avail 	= 0;	/* for programmer's use		*/
	xinu_pd.pd_base	    = addr.fm_num;	/* location of page table?	*/

    set_PDE(pg_dir, entry, xinu_pd);

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

	xinu_pt.pt_pres		= 1;		    /* page is present?		*/
	xinu_pt.pt_write 	= 1;		    /* page is writable?		*/
	xinu_pt.pt_user		= 0;		    /* is use level protection?	*/
	xinu_pt.pt_pwt		= 1;		    /* write through for this page? */
	xinu_pt.pt_pcd		= 1;		    /* cache disable for this page? */
	xinu_pt.pt_acc		= 1;		    /* page was accessed?		*/
	xinu_pt.pt_dirty 	= 0;		    /* page was written?		*/
	xinu_pt.pt_mbz		= 0;		    /* must be zero			*/
	xinu_pt.pt_global	= 0;		    /* should be zero in 586	*/
    xinu_pt.pt_valid    = 1;
	xinu_pt.pt_avail 	= 0;		    /* for programmer's use		*/
	xinu_pt.pt_base		= addr.fm_num;	/* location of page?		*/

    set_PTE(pg_tab, entry, xinu_pt);

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

