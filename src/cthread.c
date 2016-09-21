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
PFILA2 filaBlock; // Fila de bloqueados

TCB_t *Exec;  //Ponteiro para thread que está executando

ucontext_t scheduleContext; //Contexto do escalonador para ser usado no uc_link


int cidentify (char *name, int size)
{
	char* group = "Daniel Maia - 243672\nDenyson Grellert - 243676\nFelipe Tormes - 243686";

	firstTime();

	if (size < 0)
		return ERROR;

	if (name == NULL)
		return ERROR_NULL_POINTER;

	else
		strncpy(name, group, size);

	return 0;
} 

void dispatcher(ucontext_t *thread)
{
	setcontext(&thread);
}

void shceduler()
{

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

	if (threadStack == NULL)
		return ERROR_ALLOCATION;

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
	threadContext.uc_link          = &scheduleContext;
	threadContext.uc_stack.ss_sp   = threadStack;
	threadContext.uc_stack.ss_size = SIGSTKSZ*sizeof(char);

	threadTCB->context = threadContext;

	// cria um novo fluxo para executar a função passada como parâmetro
	makecontext(&threadContext, (void*) start, 1, &threadContext);

	//coloca na fila de apto
	error = AppendFila2(filaAptos, threadTCB);

	threadTCB->tid = PROCST_APTO;
	
	if (error != FALSE)
		return ERROR;
	else 	
		return threadTCB->tid;
}

// Função auxiliar que retorna um ticket entre 0 e 255
int getTicket ()
{
	return Random2() % 255;
}

int firstTime ()
{
	char *threadStack;
	TCB_t *mainTCB;


	if (IsFirst == FALSE)
		return NOTFIRST; 

	IsFirst = FALSE;

	CreateFila2(filaAptos);

	// cria contexto do escalonador
	getcontext(&scheduleContext);
	threadStack = malloc(SIGSTKSZ*sizeof(char));

	scheduleContext.uc_stack.ss_sp 	 = threadStack;
	scheduleContext.uc_stack.ss_size = SIGSTKSZ*sizeof(char);

	
	// cria estrutura pra threadmain
	mainTCB = malloc(sizeof(TCB_t));

	mainTCB->tid    = 0; //main deve ter tid zero (especificação)
	mainTCB->state  = PROCST_EXEC;
	mainTCB->ticket = getTicket();

	//contexto da main será setado no escalonador, quando ela "perder" o controle do processador

	return 0;
}


int csem_init(csem_t *sem, int count)
{
	if (sem != NULL)
	{
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