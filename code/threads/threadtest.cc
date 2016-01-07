// threadtest.cc 
//      Simple test case for the threads assignment.
//
//      Create two threads, and have them context switch
//      back and forth between themselves by calling Thread::Yield, 
//      to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// SimpleThread
//      Loop 5 times, yielding the CPU to another ready thread 
//      each iteration.
//
//      "which" is simply a number identifying the thread, for debugging
//      purposes.
//----------------------------------------------------------------------

void
SimpleThread (int which)
{
    int num;

    for (num = 0; num < 5; num++)
      {
	  printf ("*** thread %d looped %d times\n", which, num);
	  //currentThread->Yield ();
      }
}

//----------------------------------------------------------------------
// ThreadTest
//      Set up a ping-pong between two threads, by forking a thread 
//      to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest ()
{
    DEBUG ('t', "Entering SimpleTest\n");

    Thread *t = new Thread ("forked thread");
	Thread *t1 = new Thread ("thread 2");
	Thread *t2 = new Thread ("thread 3");
Thread *t3 = new Thread ("thread 4");
Thread *t4 = new Thread ("thread 5");
    t->Fork (SimpleThread, 1);
	t1->Fork (SimpleThread, 2);
	t2->Fork (SimpleThread, 3);
t3->Fork (SimpleThread, 4);
t4->Fork (SimpleThread, 5);
    SimpleThread (0);
}
