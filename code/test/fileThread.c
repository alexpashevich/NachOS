/* openfile.c 
 *
 */

#include "syscall.h"


void foo(void *arg)
{
	PutString("Foo  thread: Started working!\n");
	
	int id =  UserOpenFile("test2/file2");
	char *buf = "Second thread wrote something here ";
	UserWriteFile(id, buf, 36);

	char *bu = "and here!";
	UserWriteFile(id, bu, 10);	
		
	UserCloseFile(id);
	PutString("Foo  thread: Done working, now I terminate!\n");
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
	
	int id = UserOpenFile("test2/file1");
	
	UserThreadJoin(&tid);

	char *buf = "First thread changed that file!";
	UserWriteFile(id, buf, 32);
	
	char *bu = " and again!";
	UserWriteFile(id, bu, 12);	
	
	UserCloseFile(id);

	PutString("Main thread: Done waiting, now I terminate!\n");
	return 0;
}
