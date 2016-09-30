#include <ucontext.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

	if(threadStack == NULL)
		return ERROR_ALLOCATION;

	// aloca espaço para estrutura da thread
	threadTCB = malloc(sizeof(TCB_t));

	if(threadTCB == NULL)
		return ERROR_ALLOCATION;
	
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
	threadTCB->state   = PROCST_APTO;

	//coloca na fila de apto
	error = AppendFila2(filaAptos, threadTCB);
	
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
	int error = 0, exist;
	TID_t* node;

	error = firstTime();
	if(error)
		return error;
		
	//printf("1\n");

	if(tid >= currentTid)        //thread com esse tid ainda não foi criada, logo tid inválido
		return ERROR_INVALID_TID;
		
	exist = searchTID_struct(filaEsperados, tid);
	
	//printf("2\n");
	
	if(exist > 0) //só pode ter um cjoin para cada thread(tid)
		return ERROR_TID_USED;
		
	//printf("3\n");

	if(searchTID_int(filaTerm, tid) == TRUE) //thread já terminou
		return ERROR_INVALID_TID;
		
	//printf("4\n");

	node = malloc(sizeof(TID_t));
	
	//printf("5\n");

	node->tid_esperado = tid;
	node->tid_cjoin = Exec->tid;
	
	//printf("6\n");

	AppendFila2(filaEsperados, (void*)node);
	
	//printFilaTID(filaEsperados);
	printf("\a\n");//testamos de tudo, debugamos com o gdb, o erro vem do setcontext(), mas o contexto de quando funciona (com o printf)  e quando não funciona são exatamente iguais
	
	ReturnContext = 0;
	error = scheduler(PROCST_BLOQ);
	
	//printf("8\n");

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
		sem->fila = malloc(sizeof(FILA2));
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
	
	//printf("cwait 1\n");

	//se recurso não está disponível, bloqueamos a thread que está executando
	if (sem->count < 0)
	{
		//printf("cwait 2\n");
		error = block(sem);
	}

	return error;
}

int csignal(csem_t *sem)
{
	int error = 0;
	
	error = firstTime();
	if(error)
		return error;
		
	//printf("csignal 1\n");		

	sem->count ++;	
	
	if (sem->count >= 0)
	{
		//printf("csignal 2\n");
		error = wakeup(sem);
	}
	
	return error;
}


void dispatcher(ucontext_t context)
{
	setcontext(&context);
}

int scheduler(int fila)
{
	int ticket, tid;
	TCB_t *winner;
	TCB_t *threadAux;
	ucontext_t context;
	int error, apto = 0;
	int *tid_termino;

	getcontext(&context);   

	if (ReturnContext)
	{
		//printf("ReturnContext\n");
		ReturnContext = 0;
		if(fila != PROCST_TERMINO)
			return 0;
	}

	Exec->context = context;

	switch(fila)
	{
		case PROCST_APTO:
			//colocar thread executando no apto
			//printf("escalonador apto\n");
			apto = 1;
			break;

		case PROCST_BLOQ:
			//printf("escalonador block\n");
			error = AppendFila2(filaBlock, (void*)Exec);
			break;

		case PROCST_TERMINO:
			//printf("escalonador termino\n");
			tid_termino = (int*)malloc(sizeof(int));
			
			*tid_termino = Exec->tid;
		
			error = AppendFila2(filaTerm, (void*)tid_termino);
			
			//printf("escalonador deu append\n");

			tid = searchTID_struct(filaEsperados, Exec->tid);
			
			//printf("searc retornou %d\n", tid);
			
			if(tid >= 0) //tinha um cjoin para esta thread
			{
				//liberar thread q está esperando
				//printf("escalonador tinha cjoin\n");
				threadAux = searchTCB(filaBlock, tid);
				if(threadAux != NULL)
				{
					error += AppendFila2(filaAptos, threadAux);
					error += deletTCBFila(filaBlock, threadAux);
					//printf("escalonador desbloqueou thread\n");
				}
			}

			free(Exec->context.uc_stack.ss_sp); //libera Stack
			free(Exec);							//libera TCB

			break;
	}
	
	//printf("escalonador ticket\n");
	
	//Sorteia um ticket
	ticket = getTicket();	
	
	//printf("escalonador \n");
	
	// Seta iterador no primeiro da fila
	if(FirstFila2(filaAptos))
	{
		if(apto)
		{
			//Nesse caso, só há a thread que chamou o cyield para executar e ela não está na fila de aptos, mas executando
			ReturnContext = 1; 
			dispatcher(Exec->context);
		}
		else
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
	}
	
	// Inicializa o vencedor com o primeiro da fila 
	winner = (TCB_t*)GetAtIteratorFila2(filaAptos);

	//Enquanto não chegamos no final da fila
	while(!NextFila2(filaAptos))
	{

		// Percorre a fila 		
		threadAux = (TCB_t*)GetAtIteratorFila2(filaAptos);

		if(threadAux == NULL)
			continue;	

		// Se a thread atual está mais próxima que o atual vencedor
		if (module(threadAux->ticket - ticket) < module(winner->ticket - ticket))
		{
			winner = threadAux;
		}
		// senão, se tiverem o mesmo ticket pegamos o menor id
		else if (module(threadAux->ticket - ticket) == module(winner->ticket - ticket))
		{
			if (threadAux->tid < winner->tid)
			{
				//printf("winner\n");
				winner = threadAux;
			}
		}
	}
	

	if(apto)
	{
		//printf("escalonador apto 2\n");
		error = AppendFila2(filaAptos, (void*)Exec);
	}

	Exec = winner;
	
	deletTCBFila(filaAptos, winner); //ele está apontando para o ganhador do processador, deletando da fila de aptos
	
	//printf(" \n");
	
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
	
	Exec = mainTCB;

	//contexto da main será setado no escalonador, quando ela "perder o controle" do processador

	return 0;
}

int wakeup(csem_t *sem) 
{
	int error = 0;
	TCB_t* aux;

	//printf("wakeup 1\n");
	// Primeiro da fila de bloqueados
	FirstFila2(sem->fila);
	//printf("wakeup 2\n");
	
	// Adiciona esse elemento na fila de aptos e depois o remove da fila de bloqueados do semáforo
	if (sem->fila != NULL)
	{
		//printf("wakeup 3\n");
		aux = (TCB_t*)GetAtIteratorFila2(sem->fila);
		if(aux == NULL)
		{
			return ERROR_INVALID_FILA;
		}
		//printf("wakeup depois delete\n");
		error = deletTCBFila(filaBlock, aux);
		//printf("wakeup depois delete\n");
		error = AppendFila2(filaAptos, GetAtIteratorFila2(sem->fila)) + error;	//getatiterator
		//printf("wakeup depois append\n");
		error = DeleteAtIteratorFila2(sem->fila) + error;
		//printf("wakeup 4\n");		
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

int searchTID_struct(PFILA2 fila, int TID)
{
	TID_t *pTID;
	
	//printf("search tid %d\n", TID);

	if(FirstFila2(fila)) //se não tem ninguem da erro
	{
		//printf("search 1\n");
		if(fila!=NULL)
			return ERROR;
		else
			return ERROR_INVALID_FILA;
	}
	//printf("search 2\n");
	do
	{
		pTID = (TID_t*)GetAtIteratorFila2(fila);

		if(pTID == NULL)
			continue;

		if(pTID->tid_esperado == TID)
			return pTID->tid_cjoin;

	}while(!NextFila2(fila));
	//printf("search 3\n");

	return ERROR;
}

int searchTID_int(PFILA2 fila, int TID)
{
	int *pTID;

	if(FirstFila2(fila))
		return ERROR_INVALID_FILA;

	do
	{
		pTID = (int*)GetAtIteratorFila2(fila);

		if(pTID == NULL)
			continue;

		if((*pTID) == TID)
			return TRUE;

	}while(!NextFila2(fila));

	return FALSE;
}


TCB_t* searchTCB(PFILA2 fila, int TID)
{
	TCB_t *aux;

	if(FirstFila2(fila))
		return NULL;

	do
	{
		aux = (TCB_t*)GetAtIteratorFila2(fila);

		if(aux->tid == TID)
			return aux;

	}while(!NextFila2(fila));

	return NULL;
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

int deletTCBFila(PFILA2 fila, TCB_t *tcb)
{
	TCB_t *aux;
	
	//printf("deletetcb\n");

	if(fila)
	{
		FirstFila2(fila);
		//printf("deletetcb\n");

		do
		{

			aux = (TCB_t*)GetAtIteratorFila2(fila);
			//printf("deletetcb depois do get\n");

			if(aux == NULL)
			{
				//printf("deletetcb aux null\n");
				continue;
			}
			
			//printf("deletetcb antes do acesso aux->tid\n");

			if(aux->tid == tcb->tid)
			{
				//printf("deletetcb achou tcb\n");
				DeleteAtIteratorFila2(fila);
				return 0;
			}
			
			//printf("deletetcb antes do next\n");


		}while(!NextFila2(fila));

		return ERROR_NOT_FOUND;
	}
	else
		return ERROR_NULL_POINTER;
}

int printFilaTID(PFILA2 fila)
{
	TID_t* aux;
	
	printf("printfila \n");

	if(FirstFila2(fila))
	{
		return ERROR;
	}
	else
	{
		do
		{
			aux = (TID_t*)GetAtIteratorFila2(fila);
			
			if(aux == NULL)
				continue;
			
			printf("\nTID esperado: %d\n", aux->tid_esperado);
			printf("TID cjoin   : %d\n", aux->tid_cjoin);
		}while(!NextFila2(fila));
		
		return 0;
	}
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
