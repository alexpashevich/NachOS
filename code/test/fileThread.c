/* openfile.c 
 *
 */

#include "syscall.h"


void foo(void *arg)
{
	PutString("Bar  thread: Started working!\n");

	
	int id2 =  UserOpenFile("test2");
	char *buf = "Second thread wrote something here ";
	UserWriteFile(id2, buf, 36);

	// UserCloseFile(id2);
	// id2 = UserOpenFile("test2");
	char *bu = "and here!";
	UserWriteFile(id2, bu, 10);	
		
	UserCloseFile(id2);
	PutString("Bar  thread: Done working, now I terminate!\n");
	UserThreadExit();	
}

int
main ()
{
	PutString("Testing filesystem syscalls!\n");

	pthread tid;
	int f;
	if ( (f = UserThreadCreate(foo, 0, &tid)) == -1 )
	{
		PutString("Could not create a new user thread.\n");	
	}

	PutString("Main thread: Created foo thread and now waiting for it to finish...\n");

	UserThreadJoin(&tid);
	
	int id = UserOpenFile("test");
	
	char *buf = "First thread changed that file!";
	UserWriteFile(id, buf, 32);
	
	char *bu = " and again!";
	UserWriteFile(id, bu, 12);	
	
	UserCloseFile(id);

	PutString("Main thread: Done waiting, now I terminate!\n");
	return 0;
}
