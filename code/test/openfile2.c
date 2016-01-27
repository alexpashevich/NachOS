/* sort.c 
 *
 */

#include "syscall.h"

int
main ()
{
	PutString("Testing filesystem syscalls!\n");
	
	int id = UserOpenFile("test");
	int id2 = UserOpenFile("test2");

	char *buf = "First write to file!";
	UserWriteFile(id, buf, 20);
	UserCloseFile(id);

	char *bu = "Write to second file!";
	UserWriteFile(id2, bu, 21);
	UserCloseFile(id2);

	// PutString(bu);

	return 0;
}
