#ifndef USERTHREAD_H
#define USERTHREAD_H


#ifdef CHANGED
extern int do_UserThreadCreate(int f, int arg);
extern void do_UserThreadExit();
extern void do_UserThreadJoin(int threadId);
extern int do_UserForkExec(char* exec);
#endif

#endif