/*
 * cdata.h: arquivo de inclusão de uso apenas na geração da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida
 *
 */
#ifndef __cdata__
#define __cdata__

#define	PROCST_CRIACAO	0
#define	PROCST_APTO		1
#define	PROCST_EXEC		2
#define	PROCST_BLOQ		3
#define	PROCST_TERMINO	4

#define TRUE 	1
#define FALSE 	0

#define ERROR  	 -1
#define NOTFIRST -2
#define ERROR_NULL_POINTER -3
#define ERROR_ALLOCATION -4
#define ERROR_CREATE_FILA -5

/* NÃO ALTERAR ESSA struct */
typedef struct s_TCB { 
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
						// 0: Criação; 1: Apto; 2: Execução; 3: Bloqueado e 4: Término
	int 		ticket;		// "bilhete" de loteria da thread, para uso do escalonador
	ucontext_t 	context;	// contexto de execução da thread (SP, PC, GPRs e recursos) 
} TCB_t; 

#endif
