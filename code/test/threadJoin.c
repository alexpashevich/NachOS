#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

// uncomment UserThreadJoin and recompile

void bar(void *arg)
{
	PutString("Bar  thread: Started working!\n");
	int tmp = 1;
	int i;
	
	for (i = 1; i < 100; ++i)
	{
		tmp *= 123*i;
	}	

	PutString("Bar  thread: Done working, now I terminate!\n");
	UserThreadExit();	
}

void foo (void *arg)
{
	PutString("Foo  thread: Started working!\n");
	
	pthread tid;
	int b;
	if ( (b = UserThreadCreate(bar, 0, &tid)) == -1 )
	{
		PutString("Could not create a new user thread.\n");
	}
	else 
	{
		PutString("Foo  thread: Created bar thread and now waiting for it to finish...\n");
		UserThreadJoin(&tid); // comment this line to see difference
		PutString("\nFoo  thread: Done waiting, now I terminate!\n");	
	}	
	UserThreadExit();	
}

int main () {

	pthread tid;
	int f;	
	if ( (f = UserThreadCreate(foo, 0, &tid)) == -1 )
	{
		PutString("Could not create a new user thread.\n");	
	}
	else 
	{
		PutString("Main thread: Created foo thread and now waiting for it to finish...\n");
		UserThreadJoin(&tid);
		PutString("Main thread: Done waiting, now I terminate!\n");
	}	
	return 0;
}
