// filesys.cc 
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk 
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them 
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   there is no synchronization for concurrent accesses
//	   files have a fixed size, set when the file is created
//	   files cannot be bigger than about 3KB in size
//	   there is no hierarchical directory structure, and only a limited
//	     number of files can be added to the system
//	   there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "disk.h"
#include "bitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"

#ifdef CHANGED
//#include <iostream>
//#include <string>
#include <stdio.h>
#include <string.h>
#include "openfile.h"
#endif //CHANGED

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known 
// sectors, so that they can be located on boot-up.
#define FreeMapSector 		    0
#define DirectorySector 	    1

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number 
// of files that can be loaded onto the disk.
#define FreeMapFileSize 	(NumSectors / BitsInByte)
#define NumDirEntries 		10 //remember to change numdirEntries in directory.cc
#define DirectoryFileSize 	(sizeof(DirectoryEntry) * NumDirEntries + sizeof(DirectoryInfo))

//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).  
//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{ 
#ifdef CHANGED    
    currentDirSector = 1;
#endif    
    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new BitMap(NumSectors);
        Directory *directory = new Directory(NumDirEntries);
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

#ifdef CHANGED

    directory->Initialize(DirectorySector, DirectorySector);  
#endif

        DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);

    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));
    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
#ifdef CHANGED
    dirHdr->is_Directory(TRUE); //Set as a directory
    // currDirHdr->is_Directory(TRUE);
    // parentDirHdr->is_Directory(TRUE);
    
    // currDirHdr->WriteBack(CurrentDirectorySector);
    // parentDirHdr->WriteBack(ParentDirectorySector);
#endif //CHANGED
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
     
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	directory->WriteBack(directoryFile);

	if (DebugIsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();

        delete freeMap; 
	    delete directory; 
	    delete mapHdr; 
	    delete dirHdr;
	}
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  Make sure the file doesn't already exist
//        Allocate a sector for the file header
// 	  Allocate space on disk for the data blocks for the file
//	  Add the name to the directory
//	  Store the new file header on disk 
//	  Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file 
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------

bool
FileSystem::Create(const char *name, int initialSize)
{
    Directory *directory;
    BitMap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG('f', "Creating file %s, size %d\n", name, initialSize);

    directory = new Directory(NumDirEntries);
#ifndef CHANGED    
    directory->FetchFrom(directoryFile);
#else
    OpenFile *file = new OpenFile(currentDirSector);
    directory->FetchFrom(file);
#endif
    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
        freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();	// find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
        else if (!directory->Add(name, sector))  
            success = FALSE;	// no space in directory
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, initialSize))
            	success = FALSE;	// no space on disk for data
	    else {	
	    	success = TRUE;
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
        #ifndef CHANGED        
    	    	directory->WriteBack(directoryFile);
    	#else 
               directory->WriteBack(file);  
        #endif    	
                freeMap->WriteBack(freeMapFile);
	    }
            delete hdr;
	}
        delete freeMap;
    }
    delete directory;
#ifdef CHANGED
    delete file;
#endif    
    return success;
}

//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.  
//	To open a file:
//	  Find the location of the file's header, using the directory 
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------

OpenFile *
FileSystem::Open(const char *name)
{  
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG('f', "Opening file %s\n", name);
#ifndef CHANGED    
    directory->FetchFrom(directoryFile);
#else 
    OpenFile *file = new OpenFile(currentDirSector);
    directory->FetchFrom(file);
#endif
    sector = directory->Find(name); 
    if (sector >= 0) 		
	   openFile = new OpenFile(sector);	// name was found in directory 
    delete directory;
#ifdef CHANGED
    delete file;
#endif    
    return openFile;				// return NULL if not found
}

//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool
FileSystem::Remove(const char *name)
{ 
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
        
    directory = new Directory(NumDirEntries);
#ifndef CHANGED    
    directory->FetchFrom(directoryFile);
#else
    OpenFile * file = new OpenFile(currentDirSector);
    directory->FetchFrom(file);
#endif    
    sector = directory->Find(name);
    if (sector == -1) {
       
       delete directory; 
#ifdef CHANGED
       delete file;
#endif       
       return FALSE;			 // file not found 
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
#ifndef CHANGED    
    directory->WriteBack(directoryFile);        // flush to disk
#else
    directory->WriteBack(file);        // flush to disk   
    delete file;
#endif    
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------
#ifndef CHANGED
void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);   
    directory->List();
    delete directory;
}
#else
void
FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *file = new OpenFile(currentDirSector);
    directory->FetchFrom(file); 

    directory->List();
    delete file;
    delete directory;    
}
#endif
//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void
FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    BitMap *freeMap = new BitMap(NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->FetchFrom(freeMapFile);
    freeMap->Print();
#ifndef CHANGED
    directory->FetchFrom(directoryFile);
#else
    OpenFile *file = new OpenFile(currentDirSector);
    directory->FetchFrom(file);
#endif    
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
#ifdef CHANGED
    delete file;
#endif
} 


#ifdef CHANGED
//----------------------------------------------------------------------
// FileSystem::Create Directory
//----------------------------------------------------------------------

bool
FileSystem::CreateDirectory(const char *name)
{  
    char keys[] = "ABCDEFGHIJKLMNOPRSTUWXYZabcdefghijklmnoprstuwxyx";
    if( (strcspn (name, keys)) != 0 || !strcmp(name, "") ){
        printf("Wrong name of folder!\n");
        return FALSE;
    }    
        
  Directory *directory;
  BitMap *freeMap;
  FileHeader *hdr;
  int sector;
  bool success;

  DEBUG('f', "Creating directory %s \n", name);

  directory = new Directory(NumDirEntries);
  OpenFile *file = new OpenFile(currentDirSector);
  directory->FetchFrom(file);

  if (directory->Find(name) != -1)
    success = FALSE;			// file is already in directory
  else
  {
    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);
    sector = freeMap->Find();	// find a sector to hold the file header
    if (sector == -1)
        {success = FALSE;}		// no free block for file header
    else if (!directory->AddDir(name, sector))   //set directory as directory TRUE means creating directory
        {success = FALSE;}	// no space in directory
    else
    {
        hdr = new FileHeader;

        if (!hdr->Allocate(freeMap, DirectoryFileSize))
          {success = FALSE;}	// no space on disk for data
        else
        {
        
            Directory *myDirectory = new Directory(NumDirEntries);
            myDirectory->Initialize(sector, directory->Find("."));
            success = TRUE;
      // everthing worked, flush all changes back to disk
            hdr->is_Directory(TRUE); //we write in the header that this is a directory
            hdr->WriteBack(sector);
        // save new directory in new file to the disk    
            OpenFile *newDir = new OpenFile(sector);
            myDirectory->WriteBack(newDir);     
        // update current folder in global current folder file on the disk    
            directory->WriteBack(file);

            freeMap->WriteBack(freeMapFile);

            delete myDirectory;
            delete newDir;
        }
        delete hdr;
    }
    delete freeMap;
  }
  delete directory;
  return success;
  DEBUG('f', "Created Directory %s \n", name);
}


//----------------------------------------------------------------------
// FileSystem::delDirectory
// 	Delete directory from the file system.
//----------------------------------------------------------------------

bool
FileSystem::RemoveDirectory(const char *name)
{ 
    if( !strcmp(name, ".") || !strcmp(name, "..") ){
        printf("You cannot remove that directory!\n");
        return FALSE;
    }

    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    directory = new Directory(NumDirEntries);
    OpenFile *currDir = new OpenFile(currentDirSector);
    directory->FetchFrom(currDir);

    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
       printf("File or directory not found!\n");
       return FALSE;			 // file not found 
    }
    
    // if( !dirToRemove->isDirectory() )
    // {
    //     printf("It is not a directory!\n");

    //     delete dirToRemove;
    //     delete file;
    //     delete directory;
    //     delete currDir;
    //     return FALSE;
    // }

    if( !directory->isFile(name) )
    {
        Directory *dirToRemove = new Directory(NumDirEntries);
        OpenFile *file = new OpenFile(sector);
        dirToRemove->FetchFrom(file);
    
        if( !(dirToRemove->isEmpty()) )
        {
            printf("Directory %s cannot be removed!\n", name);
            printf("It is either not a directory, or it is not empty\n");
        
            delete dirToRemove;
            delete file;    
            delete directory;
            delete currDir;
            return FALSE; 
        }
        delete dirToRemove;
        delete file;
    }
    
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(currDir);        // flush to disk

    delete fileHdr;
    delete directory;
    delete freeMap;
    delete currDir;

    return TRUE;
} 

//----------------------------------------------------------------------
// FileSystem::MoveToDirectory
//  Move to the directory, given by user in console.
//----------------------------------------------------------------------

bool
FileSystem::MoveToDirectory(const char *name)
{   
    char name1[PathMaxLen];
    strcpy(name1,name);
    
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *file = new OpenFile(currentDirSector);
    directory->FetchFrom(file);   
     
    char * token;
    const char s[2] = "/";
   token = strtok(name1, s);
   while( token != NULL )
   { 
    
    int sector = directory->Find(token);
    if (sector == -1) {
       delete directory;
       delete file;
       printf("No such file or directory!\n");
       return FALSE;             // file not found 
    }
    if ( directory->isFile(token) ) {
       delete directory;
       delete file;
       printf("It is not a directory!\n");
       return FALSE;             // file not found 
    }
    file = new OpenFile(sector);
    directory->FetchFrom(file);
    this->currentDirSector = sector;     
    
    token = strtok(NULL, s);
    
   }

   delete directory;
   delete file;

   return TRUE;
}

//----------------------------------------------------------------------
// FileSystem::MoveToDirectory
//  Move to the directory, given by user in console.
//----------------------------------------------------------------------

bool
FileSystem::MoveToFile(const char *path, char* name)
{   
    char name1[PathMaxLen];
    strcpy(name1, path);
    
    Directory *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);   
    OpenFile *file = NULL;

    char *token;
    const char s[2] = "/";
   token = strtok(name1, s);
   while( token != NULL )
   { 
    
    if( !strcmp(name, token) ){

        delete directory;
        delete file;
        return TRUE;
    }
    int sector = directory->Find(token);

    if (sector == -1) {

       delete directory;
       delete file;
       printf("No such file or directory!\n");
       return FALSE;             // file not found 
    }

    if ( directory->isFile(token) )
    {
        break;
    }
    
    if (file != NULL)
        delete file;
    
    file = new OpenFile(sector);
    directory->FetchFrom(file);
    this->currentDirSector = sector;

    token = strtok(NULL, s);
   }

   delete directory;
   if (file != NULL)
        delete file;

   return TRUE;
}

int
FileSystem::getCurrentSector(void) { return currentDirSector; }

void
FileSystem::setCurrentSector(void){ this->currentDirSector = DirectorySector; }
#endif