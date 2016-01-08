#include "syscall.h"

void print_int(int n) {
	PutInt(n);
	PutChar('\n');
}

int
main()
{
	print_int(5);
	print_int(423423);
	print_int(111111111);

	// Halt();
	return 7;
}