/* paging.h */

#ifndef _PAGING_H_
#define _PAGING_H_

// #define MAIN
#define SMALL_TEST
// #define LARGE_TEST

#define DEBUG

/* Macros */
#define XINU_PAGES      8192    /* number of pages used by default by Xinu               */
#define PAGE_SIZE       4096    /* number of bytes per page                              */
#define MAX_FFS_SIZE    16*1024 /* size of FFS space  (in frames)                        */
#define MAX_SWAP_SIZE   32*1024 /* size of swap space  (in frames)                       */
#define MAX_PT_SIZE     1024    /* size of space used for page tables (in frames)        */

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_valid : 1;
  unsigned int pd_avail : 2;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_valid : 1;  
  unsigned int pt_avail : 2;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

/* Structure for a virtual address */

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

/* Structure for a physical address */

typedef struct{
  unsigned int fm_offset : 12;		/* frame offset			*/
  unsigned int fm_num : 20;		/* frame number			*/
} phy_addr_t;

typedef struct {
  phy_addr_t addr;
  bool8 valid;
} mem_info_t;

mem_info_t swap_list [MAX_SWAP_SIZE];
mem_info_t page_list [MAX_PT_SIZE];
mem_info_t frame_list [MAX_FFS_SIZE];

/* Functions to manipulate control registers and enable paging (see control_reg.c)	 */

unsigned long read_cr0(void);
unsigned long read_cr2(void);
unsigned long read_cr3(void);
unsigned long read_cr4(void);
void write_cr0(unsigned long n);
void write_cr3(unsigned long n);
void write_cr4(unsigned long n);
void enable_paging();
void pagefault_handler();
void set_PDBR(phy_addr_t addr);

/* functions for paging operation */

void init_paging(void);
void init_PD(pid32 pid);
uint32 new_PD_PT(void);
uint32 new_frame(void);
pd_t make_PDE(bool8 pres, bool8 valid, uint32 base);
pt_t make_PTE(bool8 pres, bool8 valid, uint32 base);
uint32 new_PDE(uint32 pg_dir, phy_addr_t addr);
uint32 new_PTE(uint32 pg_tab, phy_addr_t addr);
void set_PDE(uint32 pg_dir, uint32 entry, pd_t pde);
void set_PTE(uint32 pg_tab, uint32 entry, pt_t pte);
pd_t get_PDE(uint32 pg_dir, uint32 entry);
pt_t get_PTE(uint32 pg_tab, uint32 entry);
pd_t get_PDE_virt(uint32 frame, uint32 entry);
pt_t get_PTE_virt(uint32 frame, uint32 entry);
uint32 fm_index(phy_addr_t addr);
uint32 PID_list_index(pid32 pid);
void kill_user(pid32 pid);

pid32	vcreate(void *, uint32, pri16, char *, uint32, ...);

char* vmalloc(uint32);
char* vfound(virt_addr_t addr, uint32 nbytes);
syscall vfree(char*, uint32);
syscall vdealloc(virt_addr_t addr, uint32 nbytes);

uint32 free_ffs_pages();
uint32 free_swap_pages();
uint32 allocated_virtual_pages(pid32);
uint32 used_ffs_frames(pid32);


/* functions for paging debug */

void debug_print(char *fmt, ...);
void debug_print_PD(pid32 pid);
void debug_verify_PD_system_pages(void);
void debug_verify_PD_xinu_pages(pid32 pid);
syscall print_ready_list();

#endif
