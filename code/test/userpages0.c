#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

void bar(void *arg)
{
// 	PutString("I am in bar thread!\n");
// 	PutString("Bar thread: Done waiting, now I terminate!\n");
	// PutChar('\n');
	PutString("b\n");
	PutString("a\n");
	PutString("r\n");
	
	PutString("b\n");
	PutString("a\n");
	PutString("r\n");	
	UserThreadExit();	
}

void foo (void *arg)
{
	// PutString("I am in foo threads!\n");
	// PutString("Foo thread: Done waiting, now I terminate!\n");
	// PutChar('\n');
	PutString("f\n");
	PutString("o\n");
	PutString("o\n");

	PutString("f\n");
	PutString("o\n");
	PutString("o\n");	
	UserThreadExit();	
}

int main () {
	
	int f;
	if ( (f = UserThreadCreate(foo, 0)) == -1)
	{
		PutString("Could not create a new user thread.\n");
	}

	int b;
	if ( (b = UserThreadCreate(bar, 0)) == -1)
	{
		PutString("Could not create a new user thread.\n");
	}

	// UserThreadJoin(f);
	// UserThreadJoin(b);

	PutString("\nMain has finished its job and waiting for threads to finish...\n");
	return 0;
}
