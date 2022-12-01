/* pagefault_handler.c - pagefault_handler */

#include <xinu.h>

void pagefault_handler(void) {
    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    return;
}