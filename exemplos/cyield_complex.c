#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void hello()
{
	printf("Hello\n");
}

void nonono(int n)
{
	int i;
	for(i = 0; i < n; i++)
	{
		printf("no\n");
		cyield();
	}
}

void bye(char* string)
{
	printf("Before a cyield\n");
	cyield();
	printf("%s \n", string);
}

int main()
{
	int tid1, tid2, tid3, error, i;
	
	tid1 = ccreate((void*)hello, (void*)NULL);
	tid2 = ccreate((void*)nonono, (void*)5);
	printf("Main \tTID1: %d \n", tid1);
	printf("Main \tTID2: %d \n", tid2);
	
	error = cyield();
	
	printf("Main \tcyield returned: %d\n", error);
	
	tid3 = ccreate((void*)bye, (void*)"Thread finishing and saying goodbye!");
	printf("Main \tTID: %d \n", tid3);
	
	for(i = 0; i < 5; i++)
	{
		error = cyield();
	
		printf("Main \tcyield returned: %d\n", error);
	}

	return 0;
}
