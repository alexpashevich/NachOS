/* syscalls.h 
 * 	Nachos system call interface.  These are Nachos kernel operations
 * 	that can be invoked from user programs, by trapping to the kernel
 *	via the "syscall" instruction.
 *
 *	This file is included by user programs and by the Nachos kernel. 
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation 
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt			0
#define SC_Exit			1
#define SC_Exec			2
#define SC_Join			3
#define SC_Create		4
#define SC_Open			5
#define SC_Read			6
#define SC_Write		7
#define SC_Close		8
#define SC_Fork			9
#define SC_Yield		10

#ifdef CHANGED
#define SC_PutChar			11
#define SC_PutString		12
#define SC_GetChar			13
#define SC_GetString		14
#define SC_PutInt			15
#define SC_GetInt			16
#define SC_UserThreadCreate 17
#define SC_UserThreadExit	18
#define SC_UserThreadJoin	19
#define SC_ForkExec			20
#define SC_CreateFile		21 
#define SC_OpenFile			22
#define SC_CloseFile		23
#define SC_ReadFile			24
#define SC_WriteFile		25 
#endif

#ifdef NETWORK
#define SC_CreateConnection 26
#define SC_SendData         27
#define SC_ReceiveData      28
#endif // NETWORK

#define MAX_FILE_SIZE 500

#ifdef IN_USER_MODE
#ifdef CHANGED
typedef struct {
	int slot;
	int id;
} pthread;

#endif
// LB: This part is read only on compiling the test/*.c files.
// It is *not* read on compiling test/start.S


/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the 
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking, 
 * from the system call entry point in exception.cc.
 */

/* Stop Nachos, and print out performance stats */
void Halt () __attribute__((noreturn));


/* Address space control operations: Exit, Exec, and Join */

/* This user program is done (status = 0 means exited normally). */
void Exit (int status) __attribute__((noreturn));

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;

/* Run the executable, stored in the Nachos file "name", and return the 
 * address space identifier
 */
SpaceId Exec (char *name);

/* Only return once the the user program "id" has finished.  
 * Return the exit status.
 */
int Join (SpaceId id);


/* File system operations: Create, Open, Read, Write, Close
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 */

/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;

/* when an address space starts up, it has two open files, representing 
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */

#define ConsoleInput	0
#define ConsoleOutput	1

/* Create a Nachos file, with "name" */
void Create (char *name);

/* Open the Nachos file "name", and return an "OpenFileId" that can 
 * be used to read and write to the file.
 */
OpenFileId Open (char *name);

/* Write "size" bytes from "buffer" to the open file. */
void Write (char *buffer, int size, OpenFileId id);

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int Read (char *buffer, int size, OpenFileId id);

/* Close the file, we're done reading and writing to it. */
void Close (OpenFileId id);



/* User-level thread operations: Fork and Yield.  To allow multiple
 * threads to run within a user program. 
 */

/* Fork a thread to run a procedure ("func") in the *same* address space 
 * as the current thread.
 */
void Fork (void (*func) ());

/* Yield the CPU to another runnable thread, whether in this address space 
 * or not. 
 */
void Yield ();

#ifdef CHANGED
/* Put one char to an output stream */
void PutChar (char c);

/* Put a string to an output stream */
void PutString (char *c);

/* Put one int to an output stream */
void PutInt (int n);

/* Get one char from an input stream */
char GetChar ();

/* Get a string with length n from an input stream */
void GetString (char *s, int n);

/* Get one int from an input stream */
void GetInt (int *n);

/* Create a new thread executing the function with arguments */
int UserThreadCreate(void (*f)(void *), void *arg, void* tid);

/* Destroy the thread */
void UserThreadExit();

/* Wait for other thread */
void UserThreadJoin(void *thread);

/*  Create new process */
int ForkExec(char *exec);

/*  Open a file */
int UserCreateFile(char *filePath);

/*  Open a file */
int UserOpenFile(char *path);

/*  Close a file */
int UserCloseFile(int id);

/*  Read from a file */
int UserReadFile(int id, char* into, int numBytes);

/*  Write to a file */
int UserWriteFile(int id, char* from, int numBytes);

/* Create connection with another machine */
int CreateConnection(int addr, int port, int receivingPort);

/* Send data to another machine */
int SendData(int addr, int port, const char *data, int size);

/*  Receive data from another machine */
void ReceiveData(int port, char *data, int *size);

#endif


#endif // IN_USER_MODE

#endif /* SYSCALL_H */
