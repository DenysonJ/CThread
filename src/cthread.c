#include <ucontext.h>
#include <string.h>
#include <stdlib.h>
#include "support.h"
#include "cdata.h"
#include "cthread.h"
#include "cthreadfun.h"


int currentTid = 1;    // id da próxima thread a ser criada
int IsFirst = TRUE;    // se é a primeira vez que estamos executando

int ReturnContext = 0; // setcontext vai executar a instrução depois de getcontext, 
					   //quando entramos no escalonador precisamos saber se está voltando pelo setcontext

PFILA2 filaAptos; // Fila de aptos
PFILA2 filaBlock; // Fila de bloqueados
PFILA2 filaEsperados; //Fila com tid de todas as threads esperadas por algum join
PFILA2 filaTerm;  //Fila com o tid das threads que terminaram

TCB_t *Exec;  //Ponteiro para thread que está executando

ucontext_t scheduleContext; //Contexto do escalonador para ser usado no uc_link


int cidentify (char *name, int size)
{
	char* group = "Daniel Maia - 243672\nDenyson Grellert - 243676";

	if(firstTime())
		return ERROR;

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
	error = firstTime();
	if(error)
		return error;

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
	retornaremos o contexto para o escalonador (uc_link) */
	threadContext.uc_link          = &scheduleContext;
	threadContext.uc_stack.ss_sp   = threadStack;
	threadContext.uc_stack.ss_size = SIGSTKSZ*sizeof(char);

	// cria um novo fluxo para executar a função passada como parâmetro
	makecontext(&threadContext, (void*)start, 1, arg);

	threadTCB->context = threadContext;

	//coloca na fila de apto
	error = AppendFila2(filaAptos, threadTCB);

	threadTCB->tid = PROCST_APTO;
	
	if (error != FALSE)
		return ERROR;
	else 	
		return threadTCB->tid;
}

int cyield(void)
{
	int error = 0;

	error = firstTime();
	if(error)
		return error;

	ReturnContext = 0;

	error = scheduler(PROCST_APTO);

	return error;
}

int cjoin(int tid)
{
	int error = 0;

	error = firstTime();
	if(error)
		return error;

	if(tid >= currentTid)        //thread com esse tid ainda não foi criada, logo tid inválido
		return ERROR_INVALID_TID;

	if(searchTID(filaEsperados, tid) == TRUE) //só pode ter um cjoin para cada thread(tid)
		return ERROR_TID_USED;

	if(searchTID(filaTerm, tid) == TRUE) //thread já terminou
		return ERROR_INVALID_TID;

	error = scheduler(PROCST_BLOQ);

	return error;
}

int csem_init(csem_t *sem, int count)
{
	int error = 0;

	error = firstTime();
	if(error)
		return error;

	if (sem != NULL)
	{
		sem->count = count;	
		if (!CreateFila2(sem->fila))	
			return 0;	
		else 
			return ERROR_CREATE_FILA; // erro
	}
	else 
		return ERROR_NULL_POINTER; // erro
}

int cwait(csem_t *sem)
{
	int error = 0;	

	error = firstTime();
	if(error)
		return error;

	sem->count --;

	//se recurso não está disponível, bloqueamos a thread que está executando
	if (sem->count < 0)
		error = block(sem);

	return error;
}

int csignal(csem_t *sem)
{
	int error = 0;
	
	error = firstTime();
	if(error)
		return error;		

	sem->count ++;	
	
	if (sem->count >= 0)
		error = wakeup(sem);

	return error;
}


void dispatcher(ucontext_t context)
{
	setcontext(&context);
}

int scheduler(int fila)
{
	int ticket;
	TCB_t *winner;
	TCB_t *threadAux, threadExec;
	FILA2 winnerAux;
	int error;

	if (ReturnContext)
	{
		ReturnContext = 0;
		return 0;
	}

	getcontext(&(threadExec.context));
	Exec->context = threadExec.context;

	switch(fila)
	{
		case PROCST_APTO:
			//colocar thread executando no apto
			error = AppendFila2(filaAptos, Exec);
			break;

		//case PROCST_EXEC:
		// 	break;

		case PROCST_BLOQ:
			error = AppendFila2(filaBlock, Exec);
			break;

		case PROCST_TERMINO:
			error = AppendFila2(filaTerm, Exec->tid);

			if(searchTID(filaEsperados, Exec->tid) == TRUE) //tinha um cjoin para esta thread
			{
				//liberar thread q está esperando
				//como saber qual estava esperando está?
			}

			free(Exec->context.uc_stack.ss_sp); //libera Stack
			free(Exec);							//libera TCB

			break;
	}

	//Sorteia um ticket
	ticket = getTicket();	
	
	// Seta iterador no primeiro da fila
	if(FirstFila2(filaAptos))
	{
		free(filaAptos);
		if(GetAtIteratorFila2(filaBlock)!=NULL)
		{
			printf("Ainda possuem threads bloqueadas, mas nenhuma thread apta a executar\n");
		}
		deleteFila(filaBlock);
		deleteFila(filaEsperados);
		deleteFila(filaTerm);
		free(filaBlock);
		free(filaEsperados);
		free(filaTerm);
		//desalocar filas
		exit(0); //fila deve estar vazia logo posso sair do programa (não há threads para executar)
	}
	
	// Inicializa o vencedor com o primeiro da fila 
	winner = (TCB_t*)GetAtIteratorFila2(filaAptos);
	winnerAux = *filaAptos;

	//Enquanto não chegamos no final da fila
	while(!NextFila2(filaAptos))
	{
		// Percorre a fila 		
		threadAux = (TCB_t*)GetAtIteratorFila2(filaAptos);	

		// Se a thread atual está mais próxima que o atual vencedor
		if (module(threadAux->ticket - ticket) < module(winner->ticket - ticket))
		{
			winner = threadAux;
			winnerAux = *filaAptos;
		}
		// senão, se tiverem o mesmo ticket pegamos o menor id
		else if (module(threadAux->ticket - ticket) == module(winner->ticket - ticket))
		{
			if (threadAux->tid < winner->tid)
			{
				winner = threadAux;
				winnerAux = *filaAptos;
			}
		}
	}	
	Exec = winner;
	DeleteAtIteratorFila2(&winnerAux); //ele está apontando para o ganhador do processador, deletando da fila de aptos

	ReturnContext = 1;  //o contexto da thread pode ter sido salva pelo escalonador
	dispatcher(winner->context);

	return error;
}


int firstTime()
{
	char *threadStack = NULL;
	TCB_t *mainTCB = NULL;


	if (IsFirst == FALSE)
		return NOTFIRST; 

	IsFirst = FALSE;

	filaAptos = malloc(sizeof(FILA2));
	filaBlock = malloc(sizeof(FILA2));
	filaEsperados = malloc(sizeof(FILA2));
	filaTerm = malloc(sizeof(FILA2));

	if(CreateFila2(filaAptos) || CreateFila2(filaBlock) || CreateFila2(filaEsperados) || CreateFila2(filaTerm))
		return ERROR_CREATE_FILA;

	// cria contexto do escalonador
	getcontext(&scheduleContext);
	threadStack = malloc(SIGSTKSZ*sizeof(char));

	if(threadStack == NULL)
		return ERROR_ALLOCATION;

	scheduleContext.uc_stack.ss_sp 	 = threadStack;
	scheduleContext.uc_stack.ss_size = SIGSTKSZ*sizeof(char);

	//passa como parâmetro para o escalonador a indicação que é o término da thread
	makecontext(&scheduleContext, (void*)scheduler, 1, PROCST_TERMINO);
	
	// cria estrutura pra threadmain
	mainTCB = malloc(sizeof(TCB_t));

	if(mainTCB == NULL)
		return ERROR_ALLOCATION;

	mainTCB->tid    = 0; //main deve ter tid zero (especificação)
	mainTCB->state  = PROCST_EXEC;
	mainTCB->ticket = getTicket();
	// mainTCB->context setar uc_link?

	Exec = mainTCB;

	//contexto da main será setado no escalonador, quando ela "perder o controle" do processador

	return 0;
}

int wakeup(csem_t *sem) 
{
	int error = 0;

	// Primeiro da fila de bloqueados
	FirstFila2(sem->fila);
	
	// Adiciona esse elemento na fila de aptos e depois o remove da fila de bloqueados do semáforo
	if (sem->fila != NULL)
	{
		error = AppendFila2(filaAptos, GetAtIteratorFila2(sem->fila));	//getatiterator
		error = DeleteAtIteratorFila2(sem->fila) + error;		
		//deletar da fila de bloqueados
	}

	return error;	
}

int block(csem_t *sem)
{
	int error;	

	//coloca a thread que está está executando na lista de bloqueados
	error = AppendFila2(sem->fila, Exec);

	ReturnContext = 0;

	scheduler(PROCST_BLOQ);	
	
	return error;
}

int searchTID(PFILA2 fila, int TID)
{
	int *pTID;

	if(FirstFila2(fila))
		return ERROR_INVALID_FILA;

	do
	{
		pTID = (int*)GetAtIteratorFila2(fila);

		if((*pTID) == TID)
			return TRUE;

	}while(!NextFila2(fila) && (*pTID)!=TID);

	return FALSE;
}

TCB_t* searchTCB(PFILA2 fila, int TID)
{
	TCB_t *aux;

	if(FirstFila2(fila))
		return ERROR_INVALID_FILA;

	do
	{
		aux = (TCB_t*)GetAtIteratorFila2(fila);

		if(aux->tid == TID)
			return TRUE;

	}while(!NextFila2(fila) && (*pTID)!=TID);

	return FALSE;
}

int deleteFila(PFILA2 fila)
{
	if(fila)
	{
		do
		{
			FirstFila2(fila);
			DeleteAtIteratorFila2(fila);

		} while(!FirstFila2(fila));

		return 0;
	}
	else
		return ERROR_NULL_POINTER;
}

// Função auxiliar que retorna um ticket entre 0 e 255
int getTicket()
{
	return Random2() % 255;
}

int module(int num)
{
	if (num < 0)
		return -num;
	else 
		return num;
}