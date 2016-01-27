#ifdef CHANGED
#ifdef FILESYS
#include "thread.h"
#include "system.h"
#include "filesys.h"



void getNameFromPath(char* path, char* name)
{
    char name1[30];
    strcpy(name1, path);

    char *token;
    char last[20];

    const char s[2] = "/";

   	token = strtok(name1, s);
   	
   	while( token != NULL )
   	{ 
   		strcpy(last, token);
    	token = strtok(NULL, s);
   	}

   	strcpy(name, last);
}
//----------------------------------------------------------------------
// do_UserOpenFile
// 
//----------------------------------------------------------------------

int do_UserCreateFile(char* path)
{
	char buff[20];
	char *fileName = buff;
	getNameFromPath(path, fileName);
	printf("Last dir name: %s\n", fileName);
// Create a Nachos file of the same length
    DEBUG('f', "Creating new file %s!\n", path);
    
	if ( !fileSystem->MoveToFile(path, fileName) ){
		printf("Couldn't get to directory!\n");
		return -1;
	}
    if ( !fileSystem->Create(fileName, 0) ) {	 // Create Nachos file
		
		printf("Create: couldn't create new file %s\n", fileName);
		fileSystem->setCurrentSector();
		return -1;
    }
    fileSystem->setCurrentSector();
	return 0;
}

//----------------------------------------------------------------------
// do_UserOpenFile
// 
//----------------------------------------------------------------------

int do_UserOpenFile(char* path)
{
	char buff[20];
	char *fileName = buff;
	getNameFromPath(path, fileName);
	if ( !fileSystem->MoveToFile(path, fileName) ){
		fileSystem->setCurrentSector();
		printf("Couldnt go to directory!\n");
		return -1;
	}

	OpenFile *file = fileSystem->Open(fileName);
	currentThread->addFile(file);
	fileSystem->setCurrentSector();
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
   	int nbByte = file->Write(from, numBytes);
	return nbByte;
}

#endif
#endif