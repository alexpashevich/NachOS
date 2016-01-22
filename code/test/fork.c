#include "syscall.h"

//--------------------------------------------------------------------------
// 	Check threadJoin function.
//--------------------------------------------------------------------------

int 
main () {
		
	PutString("\nMain Fork program is executing...\n");

	ForkExec("userpages0");
	ForkExec("userpages1");

	// PutString("\nMain Fork has finished its job and waiting for threads to finish...\n");
	// GetChar();
	
	ForkExec("userpages0");
	ForkExec("userpages1");

	ForkExec("userpages0");
	ForkExec("userpages1");

	ForkExec("userpages0");
	ForkExec("userpages1");

	ForkExec("userpages0");
	ForkExec("userpages1");

	ForkExec("userpages0");
	ForkExec("userpages1");

	PutString("Main Fork has terminated...\n");
	return 5;
}
