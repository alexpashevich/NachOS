#include "syscall.h"

#define THIS "bbb"
#define THAT "bbb"

const int N = 1; // Choose it large enough!

void puts(char *s)
{
	char *p; for (p = s; *p != '\0'; p++) PutChar(*p);
}

void f(void *s)
{
	int i; for (i = 0; i < N; i++) puts((char *)s);
	UserThreadExit();
}

void b(void *s)
{
	int i; for (i = 0; i < N; i++) puts((char *)s);
}

int
main()
{
	UserThreadCreate(f, (void *) THIS);
	b((void*) THAT);

	PutString("UserPages0 has terminated...\n");
	return 1;
}