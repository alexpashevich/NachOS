#include "syscall.h"

int
main()
{
	PutString("Please enter an integer\n");
	int res[1];
	GetInt(res);
	PutString("You have just entered: ");
	PutInt(*res);
	PutChar('\n');
	Halt();
}