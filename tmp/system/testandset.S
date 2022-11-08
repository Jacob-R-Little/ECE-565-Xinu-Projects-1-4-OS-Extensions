/* testandset.S - test and set (for x86) */

		.text
		.globl	test_and_set

/*------------------------------------------------------------------------
 * test_and_set -  X86 test and set; the call is test_and_set(uint32 *ptr, uint32 new_value)
 *------------------------------------------------------------------------
 *
 *  This x86 assembly function replicates the below function in C utilizing
 *  the atomic XCHG instruction:
 *
 *  uint32 test_and_set(uint32 *ptr, uint32 new_value) {
 *      uint32 old_value = *ptr;
 *      *ptr = new_value;
 *      return old_value;
 *  }
 */
test_and_set:
    pushl	%ebp		    /* Push ebp onto stack */
	movl	%esp,%ebp	    /* Record current SP in ebp	*/
    pushl   %ebx            /* Save register ebx */
	
    movl    8(%ebp),%ebx    /* ebx = ptr */
                            /* ebx is acting similarly to the variable old_value */
                            /* However, it holds ptr instead of ptr dereferenced */

    movl    12(%ebp),%eax   /* eax = new_value */

    xchg    (%ebx),%eax      /* Perform atomic exchange */
                            /* ebx dereferenced <--> eax */
                            /* *ptr <--> new_value */
                            /* eax now holds old_value so it is ready for return */

    popl    %ebx            /* Restore register ebx */
    popl    %ebp            /* Pop ebp from the stack */
    ret                     /* Return from function */