#ifndef __cthread_fun__
#define __cthread_fun__

int  firstTime();
int  getTicket();
int  module(int num);
void dispatcher(ucontext_t *thread);
void scheduler();
int  wakeup(sem_t *sem);
int block(csem_t *sem);


#endif