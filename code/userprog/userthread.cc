#ifdef CHANGED
#include "thread.h"
#include "system.h"
#include "bitmap.h"

//----------------------------------------------------------------------
// Structure needed to pass to Thread::Fork function as it takes only
// one argument for function arguments.
//----------------------------------------------------------------------

class funcAndArg
{
public:
	int f;
	int arg;
	funcAndArg(int _f, int _arg) { f = _f; arg = _arg; }
};

//----------------------------------------------------------------------
// StartUserThread
//      Function called after user context-switched.
//      When new thread is finally activated by the sequencer, it 
//		(new thread) executes the function StartUserThread().
//----------------------------------------------------------------------

static void 
StartUserThread(int myFuncAndArg)
{

// getting the real arguments at first
	int f = ((funcAndArg*) myFuncAndArg)->f;
	int arg = ((funcAndArg*) myFuncAndArg)->arg;
	delete (funcAndArg*)myFuncAndArg;

// initialize backups of registers
	currentThread->RestoreUserState();

// initialize the stack pointer 3 pages below the SP of the main program
	
	machine->WriteRegister(StackReg, currentThread->SP);

// pass the arguments
	machine->WriteRegister(4, arg);

// set PC (program counter) registers
	machine->WriteRegister(PCReg, f);
	machine->WriteRegister(NextPCReg, f + 4);

// and start the interpreter Machine::Run
	machine->Run();
// function never reaches this point
}

//----------------------------------------------------------------------
// do_UserThreadCreate
//		Creates a new thread newThread, initialise it and place it in
//		the threads queue (in the kernel). 
//----------------------------------------------------------------------

int
do_UserThreadCreate(int f, int arg)
{


// finding out if we have enough resources to create a thread
	if(currentThread->space->stackMap->NumClear() == 0)
	{
		/* Cannot create new thread because of lack of free memory */
		return -1;
	}

// creating a new thread if there are enough resources
    Thread *newThread = new Thread ("Thread created by user");
    currentThread->space->IncrementCounter();

    printf("counter value after thread creation %d\n", currentThread->space->GetCounterValue());

// initializing it
    newThread->stackSlot = currentThread->space->stackMap->Find();
    newThread->SP = currentThread->space->mainStackTop - newThread->stackSlot * threadStackSize;
// putting it in the thread queue to execute
    funcAndArg *myfuncandarg = new funcAndArg(f, arg);
    newThread->Fork (StartUserThread, (int)myfuncandarg);
    return 0;	
}

//----------------------------------------------------------------------
// do_UserThreadExit
// 		To finish, a user thread must destroy itself by a UserThreadExit 
//		system call, which invokes a function called do_UserThreadExit().
//		This function activates Thread::Finish at NachOS level.
//----------------------------------------------------------------------

void
do_UserThreadExit()
{
	currentThread->space->DecrementCounter();
	currentThread->space->stackMap->Clear(currentThread->stackSlot);
	// currentThread->space = NULL; // why this NULL?
	currentThread->Finish();
}

#endif