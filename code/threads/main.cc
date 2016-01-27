// main.cc 
//      Bootstrap code to initialize the operating system kernel.
//
//      Allows direct calls into internal operating system functions,
//      to simplify debugging and testing.  In practice, the
//      bootstrap code would just initialize data structures,
//      and start a user program to print the login prompt.
//
//      Most of this file is not needed until later assignments.
//
// Usage: nachos -d <debugflags> -rs <random seed #>
//              -s -x <nachos file> -c <consoleIn> <consoleOut>
//              -f -cp <unix file> <nachos file>
//              -p <nachos file> -r <nachos file> -l -D -t
//              -n <network reliability> -m <machine id>
//              -o <other machine id>
//              -z
//
//    -d causes certain debugging messages to be printed (cf. utility.h)
//    -rs causes Yield to occur at random (but repeatable) spots
//    -z prints the copyright message
//
//  USER_PROGRAM
//    -s causes user programs to be executed in single-step mode
//    -x runs a user program
//    -c tests the console
//    -sc tests the synchconsole
//
//  FILESYS
//    -f causes the physical disk to be formatted
//    -cp copies a file from UNIX to Nachos
//    -p prints a Nachos file to stdout
//    -r removes a Nachos file from the file system
//    -l lists the contents of the Nachos directory
//    -D prints the contents of the entire file system 
//    -t tests the performance of the Nachos file system
//
//  NETWORK
//    -n sets the network reliability
//    -m sets this machine's host id (needed for the network)
//    -o runs a simple test of the Nachos network software
//	  -oc runs a circle test of the Nachos network software
//	  -or runs a test of reliable networks
//	  -ov runs a test of variable size transfer
//
//  NOTE -- flags are ignored until the relevant assignment.
//  Some of the flags are interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"


// External functions used by this file

extern void ThreadTest (void), Copy (const char *unixFile, const char *nachosFile);
extern void Print (char *file), PerformanceTest (void);
extern void StartProcess (char *file), ConsoleTest (char *in, char *out);
extern void SynchConsoleTest (char *in, char *out);
#ifdef NETWORK
extern void MailTest (int networkID);
#endif
extern void shell (char** cmd);

void intro(void); // local function to print out shell introduction

#ifdef CHANGED
extern void MailCircleTest (int n);
extern void ReliableMailTest (int networkID);
extern void VariableMailTest (int networkID);
#endif

//----------------------------------------------------------------------
// main
//      Bootstrap the operating system kernel.  
//      
//      Check command line arguments
//      Initialize data structures
//      (optionally) Call test procedure
//
//      "argc" is the number of command line arguments (including the name
//              of the command) -- ex: "nachos -d +" -> argc = 3 
//      "argv" is an array of strings, one for each command line argument
//              ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------

int
main (int argc, char **argv)
{
    int argCount;		// the number of arguments 
    // for a particular command

    DEBUG ('t', "Entering main");
    (void) Initialize (argc, argv);

#ifdef THREADS
    ThreadTest ();
#endif

    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount)
      {

	  argCount = 1;
	  if (!strcmp (*argv, "-z"))	// print copyright
	      printf ("%s", copyright);
#ifdef USER_PROGRAM
	  if (!strcmp (*argv, "-x"))
	    {			// run a user program
		ASSERT (argc > 1);
		StartProcess (*(argv + 1));
		argCount = 2;
	    }
	  else if (!strcmp (*argv, "-c"))
	    {			// test the console
		if (argc == 1)
		    ConsoleTest (NULL, NULL);
		else
		  {
		      ASSERT (argc > 2);
		      ConsoleTest (*(argv + 1), *(argv + 2));
		      argCount = 3;
		  }
		// interrupt->Halt ();	// once we start the console, then 
		// Nachos will loop forever waiting 
		// for console input
	    }
#ifdef CHANGED
	   else if (!strcmp (*argv, "-sc"))
	   	{
	   	if (argc == 1)
	   		SynchConsoleTest (NULL, NULL);
	   	else
	   	  {
	   	  	  ASSERT(argc > 2);
	   	  	  SynchConsoleTest(*(argv + 1), *(argv + 2));
	   	  	  argCount = 3;
	   	  }
	   	 // interrupt->Halt (); // once we start the console, then 
		// Nachos will loop forever waiting 
		// for console input
	   	}
#endif
#endif // USER_PROGRAM
#ifdef FILESYS
#ifndef CHANGED	   	
	  if (!strcmp (*argv, "-cp"))
	    {			// copy from UNIX to Nachos
		ASSERT (argc > 2);
		Copy (*(argv + 1), *(argv + 2));
		argCount = 3;
	    }
	  else if (!strcmp (*argv, "-p"))
	    {			// print a Nachos file
		ASSERT (argc > 1);
		Print (*(argv + 1));
		argCount = 2;
	    }
	  else if (!strcmp (*argv, "-r"))
	    {			// remove Nachos file
		ASSERT (argc > 1);
		fileSystem->Remove (*(argv + 1));
		argCount = 2;
	    }
	  else if (!strcmp (*argv, "-l"))
	    {			// list Nachos directory
		fileSystem->List ();
	    }
	  else if (!strcmp (*argv, "-D"))
	    {			// print entire filesystem
		fileSystem->Print ();
	    }
	  else if (!strcmp (*argv, "-t"))
	    {			// performance test
		PerformanceTest ();
	    }
        interrupt->Halt();	   	

#else
	
  else if ( !strcmp (*argv, "-shell") )
  {

	// shell takes max only 3 params - if have time, improve that!
	char arg1[100];
	char arg2[100];
	char arg3[100];
	
	char* cmd[3];
	cmd[0] = arg1;
	cmd[1] = arg2;
	cmd[2] = arg3;

	// intro();
	while(TRUE)
	{	   	
	  shell(cmd);
	  
	  // printf("First arg1: %s\n", cmd[0]);
	  // printf("Second arg1: %s\n", cmd[1]);
	  // printf("Third arg1: %s\n", cmd[2]);

	  if (!strcmp (cmd[0], "cp"))
	  {			// copy from UNIX to Nachos
		// ASSERT (argc > 2);
		Copy (cmd[1], cmd[2]);			
		// argCount = 3;					
	  }
	  else if (!strcmp (cmd[0], "p"))
	  {			// print a Nachos file
		// ASSERT (argc > 1);
		Print (cmd[1]);
		// argCount = 2;
	  }
	  else if (!strcmp (cmd[0], "r"))
	  {			// remove Nachos file
		// ASSERT (argc > 1);
		fileSystem->Remove (cmd[1]);
		// argCount = 2;
	  }
	  else if (!strcmp (cmd[0], "l"))
	  {			// list Nachos directory
		fileSystem->List ();
	  }
	  else if (!strcmp (cmd[0], "D"))
	  {			// print entire filesystem
	    	// printf("I am inside if lala %s\n", cmd[0]);
		fileSystem->Print ();
	  }
	  else if (!strcmp (cmd[0], "t"))
	  {			// performance test
		PerformanceTest ();
	  }
      else if (!strcmp (cmd[0], "mkdir"))
	  {			// Create a directory with argument as name
	    	//ASSERT (argc > 1);
		if(cmd[1] != NULL)
			fileSystem->CreateDirectory(cmd[1]);
		// argCount = 2;
      }     
      else if (!strcmp (cmd[0], "rm"))
      {			// delete directory with argument name
       	// ASSERT (argc > 1);	
       	fileSystem->RemoveDirectory(cmd[1]);
       	// argCount = 2;
      }
      else if (!strcmp (cmd[0], "cd"))
      {			// move to directory with argument name
       	// ASSERT (argc > 1);
       	fileSystem->MoveToDirectory(cmd[1]);
       	// argCount = 2;
      }
    }
  }
  // interrupt->Halt();
#endif//CHANGED
#endif // FILESYS
#ifdef NETWORK
	  if (!strcmp (*argv, "-o"))
	    {
		ASSERT (argc > 1);
		Delay (2);	// delay for 2 seconds
		// to give the user time to 
		// start up another nachos
		MailTest (atoi (*(argv + 1)));
		argCount = 2;
	  }
#ifdef CHANGED
      else if (!strcmp (*argv, "-oc")) {
		ASSERT (argc > 1);
		Delay (7);	// delay for 2 seconds
		// to give the user time to 
		// start up another nachos
		MailCircleTest (atoi (*(argv + 1)));
		argCount = 2;
      } else if (!strcmp (*argv, "-or")) {
		ASSERT (argc > 1);
		Delay (2);	// delay for 2 seconds
		// to give the user time to 
		// start up another nachos
		ReliableMailTest (atoi (*(argv + 1)));
		argCount = 2;
      } else if (!strcmp (*argv, "-ov")) {
		ASSERT (argc > 1);
		Delay (2);	// delay for 2 seconds
		// to give the user time to 
		// start up another nachos
		VariableMailTest (atoi (*(argv + 1)));
		argCount = 2;
      }
#endif // CHANGED
#endif // NETWORK
      }
      // interrupt->Halt();
    currentThread->Finish ();	// NOTE: if the procedure "main" 
    // returns, then the program "nachos"
    // will exit (as any other normal program
    // would).  But there may be other
    // threads on the ready list.  We switch
    // to those threads by saying that the
    // "main" thread is finished, preventing
    // it from returning.
    return (0);			// Not reached...
}

void intro(void)
{
	printf("###########################################################\n");
	printf("### 	                                             	###\n");
	printf("### 	Welcome in my super shell you sneaky bastard! 	###\n");
	printf("### 	                                             	###\n");
	printf("###########################################################\n");
}