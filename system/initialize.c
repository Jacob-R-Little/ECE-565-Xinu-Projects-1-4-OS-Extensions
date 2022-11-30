/* initialize.c - nulluser, sysinit */

/* Handle system initialization and become the null process */

#include <xinu.h>
#include <string.h>

extern	void	start(void);	/* Start of Xinu code			*/
extern	void	*_end;		/* End of Xinu code			*/

/* Function prototypes */

extern	void main(void);	/* Main is the first process created	*/
static	void sysinit(); 	/* Internal system initialization	*/
extern	void meminit(void);	/* Initializes the free memory list	*/
local	process startup(void);	/* Process to finish startup tasks	*/

/* Declarations of major kernel variables */

struct	procent	proctab[NPROC];	/* Process table			*/
struct	sentry	semtab[NSEM];	/* Semaphore table			*/
struct	memblk	memlist;	/* List of free memory blocks		*/

/* Active system status */

int	prcount;		/* Total number of live processes	*/
pid32	currpid;		/* ID of currently executing process	*/

/* Control sequence to reset the console colors and cusor positiion	*/

#define	CONSOLE_RESET	" \033[0m\033[2J\033[;H"

/*------------------------------------------------------------------------
 * nulluser - initialize the system and become the null process
 *
 * Note: execution begins here after the C run-time environment has been
 * established.  Interrupts are initially DISABLED, and must eventually
 * be enabled explicitly.  The code turns itself into the null process
 * after initialization.  Because it must always remain ready to execute,
 * the null process cannot execute code that might cause it to be
 * suspended, wait for a semaphore, put to sleep, or exit.  In
 * particular, the code must not perform I/O except for polled versions
 * such as kprintf.
 *------------------------------------------------------------------------
 */

void	nulluser()
{	
	struct	memblk	*memptr;	/* Ptr to memory block		*/
	uint32	free_mem;		/* Total amount of free memory	*/
	
	/* Initialize the system */

	sysinit();

	/* Output Xinu memory layout */
	free_mem = 0;
	for (memptr = memlist.mnext; memptr != NULL;
						memptr = memptr->mnext) {
		free_mem += memptr->mlength;
	}
	kprintf("%10d bytes of free memory.  Free list:\n", free_mem);
	for (memptr=memlist.mnext; memptr!=NULL;memptr = memptr->mnext) {
	    kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)memptr, ((uint32)memptr) + memptr->mlength - 1);
	}

	kprintf("%10d bytes of Xinu code.\n",
		(uint32)&etext - (uint32)&text);
	kprintf("           [0x%08X to 0x%08X]\n",
		(uint32)&text, (uint32)&etext - 1);
	kprintf("%10d bytes of data.\n",
		(uint32)&ebss - (uint32)&data);
	kprintf("           [0x%08X to 0x%08X]\n\n",
		(uint32)&data, (uint32)&ebss - 1);

	/* Enable interrupts */

	enable();

	/* Initialize the network stack and start processes */

	net_init();

	/* Create a process to finish startup and start main */

	resume(create((void *)startup, INITSTK, INITPRIO,
					"Startup process", 0, NULL));

	/* Become the Null process (i.e., guarantee that the CPU has	*/
	/*  something to run when no other process is ready to execute)	*/

	while (TRUE) {
		;		/* Do nothing */
	}

}


/*------------------------------------------------------------------------
 *
 * startup  -  Finish startup takss that cannot be run from the Null
 *		  process and then create and resume the main process
 *
 *------------------------------------------------------------------------
 */
local process	startup(void)
{
	uint32	ipaddr;			/* Computer's IP address	*/
	char	str[128];		/* String used to format output	*/


	/* Use DHCP to obtain an IP address and format it */

	ipaddr = getlocalip();
	if ((int32)ipaddr == SYSERR) {
		kprintf("Cannot obtain an IP address\n");
	} else {
		/* Print the IP in dotted decimal and hex */
		ipaddr = NetData.ipucast;
		sprintf(str, "%d.%d.%d.%d",
			(ipaddr>>24)&0xff, (ipaddr>>16)&0xff,
			(ipaddr>>8)&0xff,        ipaddr&0xff);
	
		kprintf("Obtained IP address  %s   (0x%08x)\n", str,
								ipaddr);
	}

	/* Create a process to execute function main() */

	resume(create((void *)main, INITSTK, INITPRIO,
					"Main process", 0, NULL));

	/* Startup process exits at this point */

	return OK;
}


/*------------------------------------------------------------------------
 *
 * sysinit  -  Initialize all Xinu data structures and devices
 *
 *------------------------------------------------------------------------
 */
static	void	sysinit()
{
	int32	i, j;
	struct	procent	*prptr;		/* Ptr to process table entry	*/
	struct	sentry	*semptr;	/* Ptr to semaphore table entry	*/

	/* Reset the console */

	kprintf(CONSOLE_RESET);
	kprintf("\n%s\n\n", VERSION);

	/* Initialize the interrupt vectors */

	initevec();
	
	/* Initialize free memory list */
	
	meminit();

	/* Initialize system variables */

	/* Count the Null process as the first process in the system */

	prcount = 1;

	/* Scheduling is not currently blocked */

	Defer.ndefers = 0;

	/* Initialize process table entries free */

	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		prptr->prstate = PR_FREE;
		prptr->prname[0] = NULLCH;
		prptr->prstkbase = NULL;
		prptr->prprio = 0;
	}

	/* Initialize the Null process entry */	

	prptr = &proctab[NULLPROC];
	prptr->prstate = PR_CURR;
	prptr->prprio = 0;
	strncpy(prptr->prname, "prnull", 7);
	prptr->prstkbase = getstk(NULLSTK);
	prptr->prstklen = NULLSTK;
	prptr->prstkptr = 0;
	prptr->page_dir.fm_num = 0;		// updated right before enabling paging
	prptr->page_dir.fm_offset = 0;
	currpid = NULLPROC;
	
	/* Initialize semaphores */

	for (i = 0; i < NSEM; i++) {
		semptr = &semtab[i];
		semptr->sstate = S_FREE;
		semptr->scount = 0;
		semptr->squeue = newqueue();
	}

	/* Initialize buffer pools */

	bufinit();

	/* Create a ready list for processes */

	readylist = newqueue();


	/* initialize the PCI bus */

	pci_init();

	/* Initialize the real time clock */

	clkinit();

	for (i = 0; i < NDEVS; i++) {
		init(i);
	}

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

	/* Initialize Xinu Pages */

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
	xinu_pt.pt_base		= 0;		/* location of page?		*/

	for (i = 1; i < 9; i++) {
		for (j = 0; j < 1024; j++) {
			xinu_pt.pt_base = (i - 1) * 1024 + j;
			*(pt_t *)((page_list[i].addr.fm_num << 12) + page_list[i].addr.fm_offset + (j << 2)) = xinu_pt;
		}
		page_list[i].valid = TRUE;
	}

	/* Initialize System Page Directory */
	pd_t system_pd;
	system_pd.pd_pres 	= 1;		/* page table present?		*/
	system_pd.pd_write 	= 1;		/* page is writable?		*/
	system_pd.pd_user 	= 0;		/* is use level protection?	*/
	system_pd.pd_pwt 	= 1;		/* write through cachine for pt?*/
	system_pd.pd_pcd	= 1;		/* cache disable for this pt?	*/
	system_pd.pd_acc	= 1;		/* page table was accessed?	*/
	system_pd.pd_mbz	= 0;		/* must be zero			*/
	system_pd.pd_fmb	= 0;		/* four MB pages?		*/
	system_pd.pd_global	= 0;		/* global (ignored)		*/
	system_pd.pd_avail 	= 0;		/* for programmer's use		*/
	system_pd.pd_base	= 0;		/* location of page table?	*/

	page_list[0].valid = TRUE;

	for (i = 1; i < 9; i++) {
		system_pd.pd_base = page_list[i].addr.fm_num;
		*(pd_t *)((page_list[0].addr.fm_num << 12) + page_list[0].addr.fm_offset + (i << 2)) = system_pd;
	}

	prptr->page_dir = page_list[0].addr;
	set_PDBR(prptr->page_dir);
	kprintf("About to start paging\n");
	enable_paging();
	kprintf("We paging now\n");

	return;
}

int32	stop(char *s)
{
	kprintf("%s\n", s);
	kprintf("looping... press reset\n");
	while(1)
		/* Empty */;
}

int32	delay(int n)
{
	DELAY(n);
	return OK;
}
