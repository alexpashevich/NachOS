#include "syscall.h"

void func(void *arg) {
	int myarg = ((int*)arg)[0];
	PutString("The argument = ");
	PutInt(myarg);
	PutChar('\n');
	char ch = GetChar();
	PutString("second thread get char ");
	PutChar(ch);
	PutChar('\n');
	UserThreadExit();
}

int
main()
{
	PutString("main started to execute\n");
	int a[1];
	*a = 228;

	if(UserThreadCreate(func, (void*)a) == -1)
	{
		PutString("Thread was not created!\n");
	}

	char ch = GetChar();
	PutString("first thread get char ");
	PutChar(ch);
	PutChar('\n');
	return 0;
}