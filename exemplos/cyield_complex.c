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
	int tid1, tid2, tid3, error;
	
	tid1 = ccreate((void*)hello, (void*)NULL);
	tid2 = ccreate((void*)nonono, (void*)5);
	printf("TID1: %d \n", tid1);
	printf("TID2: %d \n", tid2);
	
	error = cyield();
	
	printf("cyield returned: %d\n", error);
	
	tid3 = ccreate((void*)bye, (void*)"Thread finishing and saying goodbye!");
	printf("TID: %d \n", tid3);
	
	error = cyield();
	
	printf("cyield returned: %d\n", error);


}
