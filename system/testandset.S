

test_and_set:   # comment every line
    pushl	%ebp		/* Push ebp onto stack		*/
	movl	%esp,%ebp	/* Record current SP in ebp	*/
	pushal			    /* Push general regs. on stack	*/

        # XCHG A,B  exchange A with B

    popal			/* Restore general registers	*/
    ret
