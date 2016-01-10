#ifdef CHANGED
#include "thread.h"
#include "system.h"

class funcAndArg {
public:
	int f;
	int arg;
	funcAndArg(int _f, int _arg) { f = _f; arg = _arg; }
};

static void StartUserThread(int myfuncandarg) {
// getting the real arguments at first
	int f = ((funcAndArg*) myfuncandarg)->f;
	int arg = ((funcAndArg*) myfuncandarg)->arg;
	delete (funcAndArg*)myfuncandarg;
// set to 0 all registers except SP
	currentThread->space->InitRegisters();
// initialize backups of registers
	currentThread->RestoreUserState();
	int oldSP = machine->ReadRegister(StackReg);
// initialize the stack pointer 3 pages below the SP of the main program
	machine->WriteRegister(StackReg, oldSP - 3 * PageSize);
// pass the arguments
	machine->WriteRegister(4, arg);
// and starts the interpreter Machine::Run
	machine->WriteRegister(PCReg, f);
	machine->WriteRegister(NextPCReg, f + 4);
	machine->Run();
// function never reaches this point
}

int do_UserThreadCreate(int f, int arg) {
// finding out if we have enough resources to create a thread
	int SP = machine->ReadRegister(StackReg);
	if (SP <= 4 * PageSize)
		return -1;
// creating a new thread if there are enough resources
    Thread *newthread = new Thread ("thread created by user");
    currentThread->space->IncrementCounter();
// initializing it
    newthread->SaveUserState();
// putting it in the thread queue to execute
    funcAndArg *myfuncandarg = new funcAndArg(f, arg);
    newthread->Fork (StartUserThread, (int)myfuncandarg);
	return 0;
}

void do_UserThreadExit() {
	currentThread->space->DecrementCounter();
	currentThread->space = NULL;
	currentThread->Finish();
}
#endif