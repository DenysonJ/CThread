#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void hello(int id)
{
	int error;

	printf("Hello %d\n", id);
}

int main()
{
	int tid, i;

	for (i = 0; i<4; i++)
	{
		tid = ccreate((void*)hello, i);
	
		printf("TID: %d \n", tid);
		
		cyield();
	}	
	
	return 0;
}
