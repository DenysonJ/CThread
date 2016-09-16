#include <ucontext.h>
#include <string.h>
#include <stdlib.h>
#include "support.h"
#include "cdata.h"
#include "cthread.h"
#include "cthreadfun.h"


int currentTid = 1;   // id da próxima thread a ser criada
int IsFirst = TRUE; // se é a primeira vez que estamos executando

PFILA2 filaAptos; // Fila de aptos

ucontext_t scheduleContext;


int cidentify (char *name, int size)
{
	char* group = " Daniel Maia - 243672\n Denyson Grellert - 243676\n Felipe Tormes - 243686";

	if (size < 0)
		return ERROR;

	if (name == NULL)
		return ERROR_NULL_POINTER;

	else
		strncpy(name, group, size);

	return 0;
} 


int ccreate (void* (*start)(void*), void *arg)
{
	int error = FALSE;
	ucontext_t threadContext;
	char *threadStack;
	TCB_t *threadTCB;

	// é a primeira vez que executamos?
	firstTime();

	threadStack = malloc(SIGSTKSZ*sizeof(char));

	// aloca espaço para estrutura da thread
	threadTCB = malloc(sizeof(TCB_t));
	
	// inicializa a o TCB da thread
	threadTCB->tid    = currentTid;
	threadTCB->state  = PROCST_CRIACAO;
	threadTCB->ticket = getTicket();

	// incrementa o atual tid para a próxima thread criada
	currentTid ++;

	// cria uma estrutura de contexto
	getcontext(&threadContext);

	/* modifica a estrutura para o novo fluxo criado, ao fim da execução deste fluxo,
	retornaremos o contexto para o escalonador */
	threadContext.uc_link         = &scheduleContext;
	threadContext.uc_stack.ss_sp  = threadStack;
	threadContext.uc_stack.ss_size = sizeof(threadStack);

	threadTCB->context = threadContext;

	// cria um novo fluxo para executar a função passada como parâmetro
	makecontext(&threadContext, (void*) start, 1, &threadContext);

	//coloca na fila de apto
	error = AppendFila2(filaAptos, threadTCB);
	
	if (error == TRUE)
		return -1;
	else 	
		return 1;
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

	CreateFila2(filaAptos);
	// to do
	// criar contexto do escalonador
	// criar estrutura pra threadmain

	return 0;
}


int csem_init(csem_t *sem, int count)
{
	if (sem != NULL){
		sem->count = count;	
		if (!CreateFila2(sem->fila))	
			return 0;	
		else 
			return 1; // erro
	}
	else 
		return 1; // erro
}

int cwait(csem_t *sem)
{
	sem->count --;

	return 0;
}


