/* sort.c 
 *
 */

#include "syscall.h"

int
main ()
{
	PutString("Testing filesystem syscalls!\n");

	// UserCreateFile("..", "dupeczka");
	
	// int id = UserOpenFile("test");
	// char buff[12];
	// UserReadFile(id, buff, 12);
	// UserCloseFile(id);

	// PutString(buff);
	
	// char *buf = "abracadabra";
	// id = UserOpenFile("test");
	// UserWriteFile(id, buf, 12);
	// UserCloseFile(id);

	// char bu[12];
	// id = UserOpenFile("test");
	// UserReadFile(id, bu, 12);
	// UserCloseFile(id);

	// PutString(bu);
	
	// PutInt(id);

	UserCreateFile("../dupaaa/simple");
	// UserCloseFile(id);

	return 0;
}
