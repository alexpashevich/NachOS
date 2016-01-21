#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

int 
main () {
		
	PutString("\nMain program is executing...\n");

	ForkExec("userpages0");
	ForkExec("userpages1");

	PutString("\nMain has finished its job and waiting for threads to finish...\n");
	// GetChar();
	PutString("Main has terminated...\n");
	return 0;
}
