#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void bye(char* string)
{
	printf("Before a cyield\n");
	cyield();
	printf("%s \n", string);
}

int main()
{
	int tid, error;
	
	tid = ccreate((void*) bye, (void*)"I am the thread created by thread main");
	
	cyield();
	
	error = cjoin(tid);
	
	printf("Main \tcjoin returned: %d\n", error);
	
	return 0;
}
