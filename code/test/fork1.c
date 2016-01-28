#include "syscall.h"

#define THIS "aa"
#define THAT "bb"

const int N = 5; // Choose it large enough!

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
	UserThreadCreate(f, (void *) THIS, &tid);
	
	b((void*) THAT);	
	
	PutString("\nUserPages1 has terminated...\n");
	return 1;
}