#include "syscall.h"

//--------------------------------------------------------------------------
// 	To check if right number of threads are created, change UserStackSize
//	and threadStackSize in addrspace.h. Main program's stack is as big as 
//	threads' stack. Main memory can be chanched in machine.h.
//--------------------------------------------------------------------------

void func (void *arg) {

	PutString("I am in user thread!\n");
	UserThreadExit();
}

int main () {

	pthread tid;
	int threadsNb = 16;
	int i;
	for (i = 0; i < threadsNb; ++i)
	{
		if(UserThreadCreate(func, 0, &tid) == -1)
		{
			PutString("Could not create a new user thread.\n");
		}
	}
	
	PutString("I ve created threads and waiting...\n");
	return 0;
}