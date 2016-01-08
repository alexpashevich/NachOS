#include "syscall.h"

int
main()
{
	int i;
	char s[5];
	PutString("Please type 5 characters...\n");
	for (i = 0; i < 5; ++i)
	{
		s[i] = GetChar();
	}
	PutString("You have just typed: ");
	PutString(s);
	PutChar('\n');
	Halt();
}