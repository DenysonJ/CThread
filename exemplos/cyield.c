#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void hello()
{
	printf("Hello\n");
}

int main()
{
	int tid, error;

	tid = ccreate((void*)hello, (void*)NULL);

	printf("TID: %d \n", tid);
	
	error = cyield();

	printf("cyield returned: %d \n", error);

	return 0;
}
