#include <stdio.h>
#include "../include/support.h"
#include "../include/cthread.h"

void hello()
{
	printf("Hello\n");
}

int main()
{
	int tid;
	char nomes[120];	
	
	cidentify(nomes, 120);

	printf("%s \n", nomes);

	tid = ccreate((void*)hello, (void*)NULL);

	printf("TID: %d \n", tid);
	
	return 0;
}
