// exception.cc 
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.  
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "userthread.h"
#include "userfile.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}


//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions 
//      are in machine.h.
//----------------------------------------------------------------------


#ifdef CHANGED
void copyStringFromMachine(int from, char *to, unsigned size)
{
  unsigned i;
  int temp[1];
  for (i = 0; i < size; ++i)
  {
    ASSERT(machine->ReadMem(from + i, 1, temp));
    to[i] = ((char*)temp)[0];
    if (((char*)temp)[0] == '\0')
      break;
  }
  if (i == size)
    to[size - 1] = '\0';
}
#endif

void
ExceptionHandler (ExceptionType which)
{
  int type = machine->ReadRegister(2);

#ifndef CHANGED
  if ((which == SyscallException) && (type == SC_Halt))
      {
   DEBUG ('a', "Shutdown, initiated by user program.\n");
   interrupt->Halt ();
      }
    else
      {
   printf ("Unexpected user mode exception %d %d\n", which, type);
   ASSERT (FALSE);
      }

    // LB: Do not forget to increment the pc before returning!
    UpdatePC ();
    // End of addition
}
#else
  if (which == SyscallException) {
    switch (type) {
      case SC_Halt: {
        int res = machine->ReadRegister(4);
        while (currentThread->space->GetCounterValue() > 1) {
          currentThread->space->mainthreadwait->P();
        }
        DEBUG('a', "Shutdown, initiated by user program.\n");
        printf("\nMain program has finished with value %d\n", res);
        interrupt->Halt();
        break;
      }
      case SC_Exit: {
        int res = machine->ReadRegister(4);
        while (currentThread->space->GetCounterValue() > 1) {
          currentThread->space->mainthreadwait->P();
        }
        DEBUG('a', "Shutdown, end of main function.\n");
        printf("\nMain program has finished with value %d\n", res);
        
        
        if(currentThread->stackSlotNb == 0)
        {
            machine->lock->P();
            --machine->processCnt;
            machine->lock->V();  

            if( machine->processCnt == 0)
            {
              interrupt->Halt();  
            }
            else
            {
              currentThread->Finish();
            }
        }
        break;
      }
      case SC_PutChar: {
        char ch = (char)machine->ReadRegister(4);
        synchconsole->SynchPutChar(ch);
        break;
      }
      case SC_PutString: {
        int from = machine->ReadRegister(4);
        bufferlock->P();
        copyStringFromMachine(from, stringbuffer, MAX_STRING_SIZE);
        synchconsole->SynchPutString(stringbuffer);
        bufferlock->V();
        break;
      }
      case SC_GetChar: {
        char ch = synchconsole->SynchGetChar();
        machine->WriteRegister(2, (int)ch);
        break;
      }
      case SC_GetString: {
        int to = machine->ReadRegister(4);
        int n = machine->ReadRegister(5);
        char buf[n];
        ++n; // make space for '\0' character added at the end of string
        synchconsole->SynchGetString(buf, n);
        int i;
        for (i = 0; i < n; ++i) {
          ASSERT(machine->WriteMem(to + i, 1, (int)(buf[i])));
          if (buf[i] == '\0')
            break;
        }
        break;
      }
      case SC_PutInt: {
        int val = machine->ReadRegister(4);
        char buf[12];
        snprintf(buf, 12, "%d", val);
        synchconsole->SynchPutString(buf);
        break;
      }
      case SC_GetInt: {
        int addr = machine->ReadRegister(4);
        char buf[12];
        synchconsole->SynchGetString(buf, 12);
        int res = 0;
        sscanf(buf, "%d", &res);
        ASSERT(machine->WriteMem(addr, 4, res));
        break;
      }
      case SC_UserThreadCreate: {
        int f = machine->ReadRegister(4);
        int arg = machine->ReadRegister(5);
        int tid = machine->ReadRegister(6);
        int res = do_UserThreadCreate(f, arg, tid);
        machine->WriteRegister(2, res);
        break;
      }
      case SC_UserThreadExit: {
        do_UserThreadExit();
        break;
      }
      case SC_UserThreadJoin: {
        do_UserThreadJoin(machine->ReadRegister(4));
        break;
      }
      case SC_ForkExec: {
        int from = machine->ReadRegister(4);
        bufferlock->P();
        copyStringFromMachine(from, stringbuffer, MAX_STRING_SIZE);
        int res = do_UserForkExec(stringbuffer);
        bufferlock->V();
        machine->WriteRegister(2, res);
        break;
      }
#ifdef FILESYS      
      case SC_CreateFile: {
        char filePath[MAX_STRING_SIZE];
        int path = machine->ReadRegister(4);
        bufferlock->P();
        copyStringFromMachine(path, stringbuffer, MAX_STRING_SIZE);
        strcpy(filePath, stringbuffer);
        bufferlock->V();
        int res = do_UserCreateFile(filePath);
        machine->WriteRegister(2, res);
        break;
      }
      case SC_OpenFile: {
        char filePath[MAX_STRING_SIZE];
        int path = machine->ReadRegister(4);
        bufferlock->P();
        copyStringFromMachine(path, stringbuffer, MAX_STRING_SIZE);
        strcpy(filePath, stringbuffer);
        bufferlock->V();
        int res = do_UserOpenFile(filePath);
        machine->WriteRegister(2, res);
        break;
      }
      case SC_CloseFile: {
        int id = machine->ReadRegister(4);
        int res = do_UserCloseFile(id);
        machine->WriteRegister(2, res);
        break;
      }
      case SC_ReadFile: {
        int id = machine->ReadRegister(4);
        int into = machine->ReadRegister(5);
        int numBytes = machine->ReadRegister(6);      
        int res = do_UserReadFile(id, into, numBytes);
        machine->WriteRegister(2, res);
        break;
      }
      case SC_WriteFile: {
        char filePath[MAX_STRING_SIZE];
        int id = machine->ReadRegister(4);
        int from = machine->ReadRegister(5);
        int numBytes = machine->ReadRegister(6); 
        bufferlock->P();
        copyStringFromMachine(from, stringbuffer, numBytes);
        strcpy(filePath, stringbuffer);
        bufferlock->V();
        int res = do_UserWriteFile(id, filePath, numBytes);
        machine->WriteRegister(2, res);
        break;
      }       
#endif                       
      default: {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
      }
    }
    UpdatePC();
  }
}
#endif
