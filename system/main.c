/*  main.c  - main */

#include <xinu.h>

process test1(){
	
	kprintf("HELLO! I am process %d\n", getpid());

	return OK;
}

process	main(void)
{

	printf("\n\n");

	resume(create((void *)test1, 8192, 8, "test1", 0));
	resume(create((void *)test1, 8192, 9, "test2", 0));
	resume(create((void *)test1, 8192, 10, "test3", 0));

	xsh_ps(1);
	print_ready_list();

	return OK;
}
