#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void bye(char* string)
{
	printf("Before a cyield\n");
	cyield();
	printf("%s \n", string);
}

void print_nice()
{
	printf("NICEE!\n");
	cyield();
	printf("NICEE AGAIN!\n");
}

void nice()
{
	int tid_filho, error;
	
	tid_filho = ccreate((void*)print_nice, (void*)NULL);
	
	printf("Thread nice tid %d\n", tid_filho);
	
	error = cjoin(tid_filho);
	
	printf("Thread nice retornando cjoin: %d\n", error);
}

int main()
{
	int tid1, tid2, error;
	
	tid1 = ccreate((void*) bye, (void*)"I am the thread created by thread main");
	tid2 = ccreate((void*) nice, (void*)NULL);
	
	printf("tid1 %d\n", tid1);
	printf("tid2 %d\n", tid2);
	
	error = cjoin(tid1);
	
	printf("Main \tcjoin 1 returned: %d\n", error);
	
	error = cjoin(tid2);
	
	printf("Main \tcjoin 2 returned: %d\n", error);
	
	return 0;
}
