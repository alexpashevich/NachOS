#include "syscall.h"

void func (void *arg) {
	PutString("I am here\n");
	UserThreadExit();
}

int main () {
	UserThreadCreate(func, 0);
	UserThreadCreate(func, 0);
	UserThreadCreate(func, 0);
	UserThreadCreate(func, 0);
	UserThreadCreate(func, 0);
	// GetChar();
	PutString("Main is finishing executing\n");
	return 0;
}
