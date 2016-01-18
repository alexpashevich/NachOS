// addrspace.h 
//      Data structures to keep track of executing user programs 
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#ifdef CHANGED
#include "synch.h"
#include "bitmap.h"
#endif

#define UserStackSize		24 * PageSize //1024	// increase this as necessary!
#ifdef CHANGED
#define threadStackSize 4 * PageSize  // 512
#endif

class AddrSpace
{
  public:
    AddrSpace (OpenFile * executable);	// Create an address space,
    // initializing it with the program
    // stored in the file "executable"
    ~AddrSpace ();		// De-allocate an address space

    void InitRegisters ();	// Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState ();		// Save/restore address space-specific
    void RestoreState ();	// info on a context switch 
    
#ifdef CHANGED
  public:
    void IncrementCounter(); // in case one more thread started to use this addrspace
    void DecrementCounter(); // in case one thread using this addrspace was destroyed
    int GetCounterValue();
    static void ReadAtVirtual(OpenFile *executable, int virtualaddr,
                int numBytes, int position, TranslationEntry *pageTable,unsigned numPages)
    Semaphore *mainthreadwait;
    Semaphore *lock;

    BitMap *stackMap; // bitMap for userThread's stack allocation
    int mainStackTop; // userThread's stacks are allocated below main stack, so we need
                      // to know where main stack ends
    int threadsNb;    // make those private?
    void** threadArray;
    int threadId;     // unique thread iD
  private:
    int counter;
#endif

  private:
    TranslationEntry * pageTable;	// Assume linear page table translation
    // for now!
    unsigned int numPages;	// Number of pages in the virtual 
    // address space
};

#endif // ADDRSPACE_H
