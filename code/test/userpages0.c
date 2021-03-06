#include "syscall.h"

#define THIS "aaa"
#define THAT "bbb"

const int N = 10; // Choose it large enough!

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
	pthread tid;
	UserThreadCreate(f, (void *) THIS, &tid);
	UserThreadCreate(f, (void *) THIS, &tid);
	// UserThreadCreate(f, (void *) THIS, &tid);
	// UserThreadCreate(f, (void *) THIS, &tid);
	// UserThreadCreate(f, (void *) THIS, &tid);
	
	ForkExec("multiplethreads");

	b((void*) THAT);
	b((void*) THAT);
	b((void*) THAT);

	PutString("\nUserPages0 has terminated...\n");
	return 0;
}