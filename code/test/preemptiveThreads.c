#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check if threads are restarted properly after context-switch.
//	Do some time consuming stuff in thread to make a context-switch.
//--------------------------------------------------------------------------

void func (void *arg) {
	PutString("I am in user thread!\n");
	int tmp = 1;
	int i;
	for (i = 1; i < 100; ++i)
	{
		tmp *= 123*i;
	}
	PutString("I am again in user thread!\n");
	UserThreadExit();
}

int main () {

	int threadsNb = 4;
	int i;
	
	for (i = 0; i < threadsNb; ++i)
	{
		if (UserThreadCreate(func, 0) == -1)
		{
			PutString("Could not create a new user thread.\n");
		}
	}

	PutString("Main has finished its job and waiting for threads to finish...\n");
	return 0;
}
