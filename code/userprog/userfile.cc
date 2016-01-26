#ifdef CHANGED
#ifdef FILESYS
#include "thread.h"
#include "system.h"
#include "filesys.h"

//----------------------------------------------------------------------
// do_UserOpenFile
// 
//----------------------------------------------------------------------

int do_UserCreateFile(char* path, char* name)
{
	
// Create a Nachos file of the same length
    DEBUG('f', "Creating new file %s!\n", name);
    
	if ( !fileSystem->MoveToDirectory(path) ){
		printf("Couldn't get to directory!\n");
		return -1;
	}
    if ( !fileSystem->Create(name, 0) ) {	 // Create Nachos file
		
		printf("Create: couldn't create new file %s\n", name);
		return -1;
    }
	return 0;
}

//----------------------------------------------------------------------
// do_UserOpenFile
// 
//----------------------------------------------------------------------

int do_UserOpenFile(char* path, char* name)
{

	if ( !fileSystem->MoveToFile(path) ){
		printf("Couldnt go to directory!\n");
		return -1;
	}

	OpenFile *file = fileSystem->Open(name);
	currentThread->addFile(file);

	return 0;
}

//----------------------------------------------------------------------
// do_UserCloseFile
// 
//----------------------------------------------------------------------

int do_UserCloseFile(int pos)
{
	delete currentThread->removeFile(pos);
	
	return 0;
}

//----------------------------------------------------------------------
// do_UserReadFile
// 
//----------------------------------------------------------------------

int do_UserReadFile(int id, int into, int numBytes)
{
	OpenFile *file = currentThread->getFile(id);
	char buff[numBytes];
	int nbByte = file->Read(buff, numBytes);
    
    int i;
    for (i = 0; i < numBytes; ++i) {
       ASSERT(machine->WriteMem(into + i, 1, (int)(buff[i])));
   	}
	
	return nbByte;
}

//----------------------------------------------------------------------
// do_UserWriteFile
// 
//----------------------------------------------------------------------

int do_UserWriteFile(int id, char* from, int numBytes)
{
	OpenFile *file = currentThread->getFile(id);
	
	// int buff[numBytes / sizeof(int)];
 //    int i;
 //    for (i = 0; i < numBytes; ++i) {
 //       ASSERT(machine->ReadMem(from + i, 1, (buff[i])));
 //   	}	
   	int nbByte = file->Write(from, numBytes);
	return nbByte;
}
#endif
#endif