#include <ucontext.h>
#include <string.h>
#include "support.h"
#include "cdata.h"
#include "cthread.h"
#include "cthreadfun.h"


int currentTid = 1;   // id da próxima thread a ser criada
int IsFirst = TRUE; // se é a primeira vez que estamos executando


int cidentify (char *name, int size)
{
	char* group = " Daniel Maia - \n Denyson Grellert - 243676\n Felipe Tormes - 243686";

	if (size < 0)
		return ERROR;

	if (name == NULL)
		return ERROR_NULL_POINTER;

	else
		strncpy(name, group, size);

	return 0;
} 


{
	ucontext_t threadContext;
	char threadStack[SIGSTKSZ];
	TCB_t threadTCB;

	// é a primeira vez que executamos?
	firstTime();

	// inicializa a o TCB da thread
	threadTCB.tid    = currentTid;
	threadTCB.state  = PROCST_CRIACAO;
	threadTCB.ticket = getTicket();

	// incrementa o atual tid para a próxima thread criada
	currentTid ++;

	// cria uma estrutura de contexto
	getContext(&threadContext);

	/* modifica a estrutura para o novo fluxo criado, ao fim da execução deste fluxo,
	retornaremos o contexto para o escalonador */
	threadContext.uc_link         = &scheduleContext;
	threadContext.uc_stack.ss_sp  = threadStack;
	threadContext.uc_stack.ss_size = sizeof(threadStack);

	// cria um novo fluxo para executar a função passada como parâmetro
	makecontext(&threadContext, void* (*start)(void*), 1, &threadContext);

	//to do
	//colocar na fila de apto
	return 0;
}

// Função auxiliar que retorna um ticket entre 0 e 255
int getTicket ()
{
	return Random2() % 255;
}

int firstTime ()
{
	if (IsFirst == FALSE)
		return NOTFIRST; 

	IsFirst = FALSE;

	//to do
	// criar contexto do escalonador
}
