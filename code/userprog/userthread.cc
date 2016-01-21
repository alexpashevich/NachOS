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

// // getting the real arguments at first
	int f = ((funcAndArg*) myFuncAndArg)->f;
	int arg = ((funcAndArg*) myFuncAndArg)->arg;
	delete (funcAndArg*)myFuncAndArg;

// initialize backups of registers
	currentThread->space->InitRegisters();
	currentThread->RestoreUserState();
// initialize the stack pointer of the thread program
    machine->WriteRegister(StackReg, 
    			currentThread->space->mainStackTop - currentThread->stackSlotNb * threadStackSize);

// pass the arguments
	machine->WriteRegister(4, arg);

// set PC (program counter) registers
	machine->WriteRegister(PCReg, f);
	machine->WriteRegister(NextPCReg, f + 4);

	// machine->WriteRegister(LastPCReg, 0);
// and start the interpreter Machine::Run
	machine->Run();
// function never reaches this point
}

//----------------------------------------------------------------------
// do_UserThreadCreate
//		Creates a new thread newThread, initialise it and place it in
//		the threads queue (in the kernel). Returns 0 on success and
//		-1 on failure.
//----------------------------------------------------------------------

int
do_UserThreadCreate(int f, int arg)
{

	DEBUG('t', "Creating new User Thread.\n");

// finding out if we have enough resources to create a thread
	int slotNb;
	currentThread->space->lock->P();	
	if( (slotNb = currentThread->space->stackMap->Find()) == -1 )
	{
		/* Cannot create new thread because of lack of free memory */
		currentThread->space->lock->V();
		return -1;
	}
	currentThread->space->lock->V();

// creating a new thread if there are enough resources
    Thread *newThread = new Thread ("Thread created by user");
    currentThread->space->IncrementCounter();

// finding newThread's stack pointer initial address (no additional 3*PageSize space between stacks)
    newThread->stackSlotNb = slotNb; // need to set it to know which slot to free in do_UserThreadExit()
    currentThread->space->threadArray[slotNb] = newThread;

// putting new thread in the thread queue to execute
    newThread->space = currentThread->space;
    funcAndArg *myfuncandarg = new funcAndArg(f, arg);
    newThread->Fork (StartUserThread, (int)myfuncandarg);
    return slotNb;	
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
	DEBUG('t', "Terminating User Thread.\n");
	// printf("Kasuje freda kurwulus!\n");
	currentThread->space->DecrementCounter();
	currentThread->space->lock->P();
	currentThread->space->stackMap->Clear(currentThread->stackSlotNb);
	currentThread->space->lock->V();
	while(!currentThread->waitingList->IsEmpty())
	{
		IntStatus oldLevel = interrupt->SetLevel (IntOff);
		scheduler->ReadyToRun ((Thread*) currentThread->waitingList->Remove());
		(void) interrupt->SetLevel (oldLevel);
	}
	currentThread->space->threadArray[currentThread->stackSlotNb] = NULL;
	currentThread->Finish();
}

//----------------------------------------------------------------------
// UserThreadJoin
// 		Wait for other thread to terminate.
//----------------------------------------------------------------------

void
do_UserThreadJoin(int threadId)
{
	DEBUG('t', "Waiting for thread %d.\n", threadId);	
	currentThread->space->lock->P();
	if (0 < threadId && threadId < currentThread->space->threadsNb)
	{
		if(currentThread->space->stackMap->Test(threadId))
		{

			Thread *tmp = (Thread*) currentThread->space->threadArray[threadId];
			if(tmp != NULL)
			{
				tmp->waitingList->Prepend(currentThread);	
			}
			IntStatus oldLevel = interrupt->SetLevel (IntOff);
			currentThread->space->lock->V();
			currentThread->Sleep();
			(void) interrupt->SetLevel (oldLevel);
		}
	}
	currentThread->space->lock->V();
}

int 
do_UserForkExec(char* exec)
{
	DEBUG('t', "Create new process %s.\n", exec);

	OpenFile *executable = fileSystem->Open (exec);

    if (executable == NULL)
    {
		printf ("Unable to open file %s\n", exec);
	  	return -1;
    }
    
    Thread* newThread = new Thread("process");
    newThread->space = new AddrSpace (executable);
    delete executable;

    funcAndArg *myfuncandarg = new funcAndArg(0, 0);
    newThread->Fork (StartUserThread, (int)myfuncandarg);
 
    return 0;
}
#endif