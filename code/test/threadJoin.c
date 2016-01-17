#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

// void bar(void *arg)
// {
// 	PutString("I am in second level thread!\n");
// 	int tmp = 1;
// 	int i;
// 	for (i = 1; i < 100; ++i)
// 	{
// 		tmp *= 123*i;
// 	}	
// 	PutString("Second level thread: Done waiting, now I terminate!\n");
// 	UserThreadExit();	
// }

// void foo (void *arg)
// {
// 	PutString("I am in first level thread!\n");
	
// 	int t_id;
// 	if ( (t_id = UserThreadCreate(bar, 0)) == -1 )
// 	{
// 		PutString("Could not create a new user thread.\n");
// 	}	
// 	// PutInt(t_id);
// 	PutString("First level thread: done my job and now waiting for other thread.\n");
// 	// UserThreadJoin(t_id);
// 	PutString("First level thread: Done waiting, now I terminate!\n");
// 	UserThreadExit();	
// }

void foo(void *arg)
{
	PutString("I am in foo thread!\n");
	int tmp = 1;
	int i;
	for (i = 1; i < 100; ++i)
	{
		tmp *= 123*i;
	}	
	PutString("Foo thread: Done waiting, now I terminate!\n");
	UserThreadExit();
}

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

int main () {
	
	// int thread;
	// if ( (thread = UserThreadCreate(foo, 0)) == -1)
	// {
	// 	PutString("Could not create a new user thread.\n");
	// }
	// UserThreadJoin(1);
	// UserThreadCreate(foo, 0);
	// PutInt(thread);
	
	int thread;	
	// if ( (thread = UserThreadCreate(foo, 0)) == -1)
	// {
	// 	PutString("Could not create a foo thread.\n");
	// }
	// UserThreadJoin(thread);
	if ( (thread = UserThreadCreate(bar, 0)) == -1)
	{
		PutString("Could not create a bar thread.\n");
	}
	UserThreadJoin(thread);

	PutString("Main has finished its job and waiting for threads to finish...\n");
	return 0;
}
