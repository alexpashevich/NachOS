#include "syscall.h"

void func (void *arg) {
	PutString("I am created\n");
	UserThreadExit();
}

int main () {
	UserThreadCreate(func, 0);
	UserThreadCreate(func, 0);
	UserThreadCreate(func, 0);
	PutString("I ve created the threads and waiting...\n");
	return 0;
}