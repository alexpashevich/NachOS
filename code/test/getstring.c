#include "syscall.h"

int
main()
{
	char s[10];
	PutString("Please type string with maximum 10 characters...\n");
	GetString(s, 10);
	PutString("You have just typed: ");
	PutString(s);
	PutChar('\n');
	
	return 0;
}