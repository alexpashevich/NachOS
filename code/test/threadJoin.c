#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

void bar(void *arg)
{
	PutString("I am in second level thread!\n");
	UserThreadExit();	
}

void foo (void *arg)
{
	PutString("I aam in first level thread!\n");
	
	// int t_id;
	// if ( (t_id = UserThreadCreate(bar, 0)) == -1 )
	// {
	// 	PutString("Could not create a new user thread.\n");
	// }	
	// PutInt(t_id);
	// PutString("First level thread: done my job and now waiting for other thread.\n");
	// UserThreadJoin(t_id);
	// PutString("First level thread: Done waiting, now I terminate!\n");
	UserThreadExit();	
}

int main () {
	
	int thread;
	if ( (thread = UserThreadCreate(foo, 0)) == -1)
	{
		PutString("Could not create a new user thread.\n");
	}
	// UserThreadJoin(thread);
	UserThreadCreate(foo, 0);
	PutInt(thread);
	PutString("Main has finished its job and waiting for threads to finish...\n");
	return 0;
}
