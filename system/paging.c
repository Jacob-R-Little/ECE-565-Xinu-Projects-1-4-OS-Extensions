/* paging.c - functions to enable paging */

#include <xinu.h>

void new_PD(phy_addr_t addr) {

}

void new_PT(phy_addr_t addr) {
    unsigned int i, j;
    unsigned int index = MAX_PT_SIZE;
    pt_t xinu_pt;

	xinu_pt.pt_pres		= 1;		/* page is present?		*/
	xinu_pt.pt_write 	= 1;		/* page is writable?		*/
	xinu_pt.pt_user		= 0;		/* is use level protection?	*/
	xinu_pt.pt_pwt		= 1;		/* write through for this page? */
	xinu_pt.pt_pcd		= 1;		/* cache disable for this page? */
	xinu_pt.pt_acc		= 1;		/* page was accessed?		*/
	xinu_pt.pt_dirty 	= 0;		/* page was written?		*/
	xinu_pt.pt_mbz		= 0;		/* must be zero			*/
	xinu_pt.pt_global	= 0;		/* should be zero in 586	*/
	xinu_pt.pt_avail 	= 0;		/* for programmer's use		*/
	xinu_pt.pt_base		= addr.fm_num;		/* location of page?		*/

    for (i = 0; i < MAX_PT_SIZE; i++) {
        if (page_list[i].valid == FALSE) {
            index = i;
            break;
        }
    }

    if (index == MAX_PT_SIZE) { /* page list overflow */
        return;
    }

    *(pt_t *)((page_list[index].addr.fm_num << 12) + page_list[index].addr.fm_offset + (j << 2)) = xinu_pt;

}
