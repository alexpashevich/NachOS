/* openfile.c 
 *
 */

#include "syscall.h"


void foo(void *arg)
{
	PutString("Bar  thread: Started working!\n");

	
	int id2 =  UserOpenFile("test2");
	char *buf = "Siema to ja Marcin";
	UserWriteFile(id2, buf, 18);
	// PutInt(id2);
	UserCloseFile(id2);
	id2 = UserOpenFile("test2");
	char *bu = " a teraz powiem ty!";
	UserWriteFile(id2, bu, 19);

		PutInt(id2);	
		
	UserCloseFile(id2);
		// UserCloseFile(id2);	
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

	// PutString("Main thread: Created foo thread and now waiting for it to finish...\n");

		UserThreadJoin(&tid);
	// UserCreateFile("testt");
	
	// int id = UserOpenFile("testt");
	
// write to newly created file
	
	
	// int id =  UserOpenFile("test", mode);
	int id2 = UserOpenFile("test");
	
	char *buf = "Siema to ja Marcin";
	UserWriteFile(id2, buf, 18);
	// PutInt(id2);
	// UserCloseFile(id2);
	// id2 = UserOpenFile("test");
	char *bu = " a teraz powiem ty a teraz powiem ty";
	UserWriteFile(id2, bu, 40);
	
	PutInt(id2);	
	
	// UserCloseFile(id);
	UserCloseFile(id2);

// read from file
	// char buff[12];
	// id = UserOpenFile("test");
	// UserReadFile(id, buff, 12);
	// UserCloseFile(id);

	// id = UserOpenFile("test");
	// char *bu = "12345678987";
	// UserWriteFile(id, bu, 12);
	// UserCloseFile(id);

	// PutString("File content: ");
	// PutString(buff);
	// PutString("Testing filesystem syscalls!\n");
			PutString("Main thread: Done waiting, now I terminate!\n");
	return 0;
}
