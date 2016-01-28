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

	pthread tid;
	if(UserThreadCreate(func, (void*)a, &tid) == -1)
	{
		PutString("Thread was not created!\n");
	}

	return 0;
}