#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

// change NumPhysPAges to 160 to show that we release memory

int 
main () {
		
	PutString("\nMain Fork program is executing...\n");

	ForkExec("fork1");
	ForkExec("fork2");
	
	ForkExec("fork1");

	char buf[20];
	GetString(buf, 20);

	ForkExec("fork1");

	PutString("Main Fork has terminated...\n");
	return 0;
}
