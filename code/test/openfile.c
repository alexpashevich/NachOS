/* sort.c 
 *
 */

#include "syscall.h"

int
main ()
{
	PutString("Testing filesystem syscalls!\n");
	
	int id = UserOpenFile("test");
	
	char *buf = "abracadabra";
	UserWriteFile(id, buf, 12);
	// UserCloseFile(id);

	char bu[11];
	UserReadFile(id, bu, 12);
	UserCloseFile(id);

	PutString(bu);

	return 0;
}
