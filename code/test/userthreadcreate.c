#include "syscall.h"

void func(void *arg) {
	PutString("I am fucking created!!!\n");
	int myarg = ((int*)arg)[0];
	PutString("The argument = ");
	PutInt(myarg);
	PutChar('\n');
	UserThreadExit();
}

int
main()
{
	PutString("main started to execute\n");
	int a[1];
	*a = 228;
	UserThreadCreate(func, (void*)a);
	char ch = GetChar();
	Halt();
	return (int)ch; // just because if we don't use this char, the warning is arasen
}