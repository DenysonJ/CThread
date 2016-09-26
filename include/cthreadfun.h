#ifndef __cthread_fun__
#define __cthread_fun__

int  firstTime();
int  getTicket();
int  module(int num);
void dispatcher(ucontext_t thread);
int  scheduler();
int  wakeup(csem_t *sem);
int  block(csem_t *sem);
int  searchTID(PFILA2 fila, int TID);


#endif
