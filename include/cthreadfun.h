#ifndef __cthread_fun__
#define __cthread_fun__

int  firstTime();
int  getTicket();
int  module(int num);
void dispatcher(ucontext_t thread);
int  scheduler();
int  wakeup(csem_t *sem);
int  block(csem_t *sem);
int searchTID_struct(PFILA2 fila, int TID);
int searchTID_int(PFILA2 fila, int TID);
TCB_t* searchTCB(PFILA2 fila, int TID);
int  deleteFila(PFILA2 fila);
int  deletTCBFila(PFILA2 fila, TCB_t *tcb);


#endif
