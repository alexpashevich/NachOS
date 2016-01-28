/* sort.c 
 *
 */

#include "syscall.h"

int
main ()
{
	PutString("Testing filesystem syscalls!\n");
	
	int id = UserCreateFile("/test/file/simple");
	id = UserOpenFile("/test/file/simple");
	
	char *buf = "abracadabra";
	UserWriteFile(id, buf, 12);
	UserCloseFile(id);

	id = UserOpenFile("/test/file/simple");
	
	char bu[12];
	UserReadFile(id, bu, 12);
	UserCloseFile(id);

	PutString(bu);

	return 0;
}
