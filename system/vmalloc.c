/* vmalloc.c - vmalloc, vfree */

#include <xinu.h>

char* vmalloc(uint32 nbytes) {
    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    return NULL;
}

syscall vfree(char* ptr, uint32 nbytes) {
    set_PDBR(proctab[NULLPROC].page_dir);   // change to system virtual address space

    set_PDBR(proctab[currpid].page_dir);    // return to current process virtual address space
    return SYSERR;
}