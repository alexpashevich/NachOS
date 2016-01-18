#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

void bar(void *arg)
{
	PutString("I am in bar thread!\n");
	int tmp = 1;
	int i;
	
	for (i = 1; i < 100; ++i)
	{
		tmp *= 123*i;
	}	

	PutString("Bar thread: Done waiting, now I terminate!\n");
	UserThreadExit();	
}

void foo (void *arg)
{
	PutString("I am in foo thread!\n");
	
	int t_id;
	if ( (t_id = UserThreadCreate(bar, 0)) == -1 )
	{
		PutString("Could not create a new user thread.\n");
	}	

	PutString("Foo thread: done my job and now waiting for bar thread.\n");
	UserThreadJoin(t_id); // comment this line to see difference
	PutString("Foo thread: Done waiting, now I terminate!\n");
	UserThreadExit();	
}

int main () {
	
	int f;
	if ( (f = UserThreadCreate(foo, 0)) == -1)
	{
		PutString("Could not create a new user thread.\n");
	}
	// UserThreadJoin(f);

	PutString("Main has finished its job and waiting for threads to finish...\n");
	return 0;
}
