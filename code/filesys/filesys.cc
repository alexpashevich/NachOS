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
#define CurrentDirectorySector  2
#define ParentDirectorySector   3

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
    DEBUG('f', "Initializing the file system.\n");
    if (format) {
        BitMap *freeMap = new BitMap(NumSectors);
        Directory *directory = new Directory(NumDirEntries);
	FileHeader *mapHdr = new FileHeader;
	FileHeader *dirHdr = new FileHeader;

#ifdef CHANGED
    Directory *currDirectory = new Directory(NumDirEntries);
    // Directory *parentDirectory = new Directory(NumDirEntries);    

    FileHeader *currDirHdr   = new FileHeader;
    // FileHeader *parentDirHdr = new FileHeader;

    directory->Initialize(DirectorySector, DirectorySector);
    // currentDirSector = DirectorySector;    
#endif

        DEBUG('f', "Formatting the file system.\n");

    // First, allocate space for FileHeaders for the directory and bitmap
    // (make sure no one else grabs these!)
	freeMap->Mark(FreeMapSector);	    
	freeMap->Mark(DirectorySector);
#ifdef CHANGED    
    freeMap->Mark(CurrentDirectorySector);
    // freeMap->Mark(ParentDirectorySector);
#endif
    // Second, allocate space for the data blocks containing the contents
    // of the directory and bitmap files.  There better be enough space!

	ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize));
	ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));
#ifdef CHANGED
    ASSERT(currDirHdr->Allocate(freeMap, DirectoryFileSize));
    // ASSERT(parentDirHdr->Allocate(freeMap, DirectoryFileSize));
#endif
    // Flush the bitmap and directory FileHeaders back to disk
    // We need to do this before we can "Open" the file, since open
    // reads the file header off of disk (and currently the disk has garbage
    // on it!).

        DEBUG('f', "Writing headers back to disk.\n");
#ifdef CHANGED
    dirHdr->is_Directory(TRUE); //Set as a directory
    currDirHdr->is_Directory(TRUE);
    // parentDirHdr->is_Directory(TRUE);
    
    currDirHdr->WriteBack(CurrentDirectorySector);
    // parentDirHdr->WriteBack(ParentDirectorySector);
#endif //CHANGED
	mapHdr->WriteBack(FreeMapSector);    
	dirHdr->WriteBack(DirectorySector);

    // OK to open the bitmap and directory files now
    // The file system operations assume these two files are left open
    // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
#ifdef CHANGED
        currDirFile = new OpenFile(CurrentDirectorySector);
        // parentDirFile = new OpenFile(ParentDirectorySector);
#endif
     
    // Once we have the files "open", we can write the initial version
    // of each file back to disk.  The directory at this point is completely
    // empty; but the bitmap has been changed to reflect the fact that
    // sectors on the disk have been allocated for the file headers and
    // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap and directory back to disk.\n");
	freeMap->WriteBack(freeMapFile);	 // flush changes to disk
	directory->WriteBack(directoryFile);
#ifdef CHANGED
    directory->WriteBack(currDirFile);
    // directory->WriteBack(parentDirFile);
#endif
	if (DebugIsEnabled('f')) {
	    freeMap->Print();
	    directory->Print();

        delete freeMap; 
	    delete directory; 
	    delete mapHdr; 
	    delete dirHdr;
#ifdef CHANGED
        delete currDirectory;
        // delete parentDirectory;
        delete currDirHdr;
        // delete parentDirHdr;
#endif
	}
    } else {
    // if we are not formatting the disk, just open the files representing
    // the bitmap and directory; these are left open while Nachos is running

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
#ifdef CHANGED
        currDirFile = new OpenFile(CurrentDirectorySector);
        // parentDirFile = new OpenFile(ParentDirectorySector);
#endif        
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
    directory->FetchFrom(directoryFile);

    if (directory->Find(name) != -1)
      success = FALSE;			// file is already in directory
    else {	
        freeMap = new BitMap(NumSectors);
        freeMap->FetchFrom(freeMapFile);
        sector = freeMap->Find();	// find a sector to hold the file header
    	if (sector == -1) 		
            success = FALSE;		// no free block for file header 
#ifndef CHANGED       
        else if (!directory->Add(name, sector))
#else            
        else if (!directory->Add(name, sector, FALSE))
#endif    
            success = FALSE;	// no space in directory
	else {
    	    hdr = new FileHeader;
	    if (!hdr->Allocate(freeMap, initialSize))
            	success = FALSE;	// no space on disk for data
	    else {	
	    	success = TRUE;
		// everthing worked, flush all changes back to disk
    	    	hdr->WriteBack(sector); 		
    	    	directory->WriteBack(directoryFile);
    	    	freeMap->WriteBack(freeMapFile);
	    }
            delete hdr;
	}
        delete freeMap;
    }
    delete directory;
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
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); 
    if (sector >= 0) 		
	   openFile = new OpenFile(sector);	// name was found in directory 
    delete directory;
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
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1) {
       delete directory;
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
    directory->WriteBack(directoryFile);        // flush to disk
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
    directory->FetchFrom(currDirFile); 

    directory->List();
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

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
} 


#ifdef CHANGED
//----------------------------------------------------------------------
// FileSystem::Create Directory
//----------------------------------------------------------------------

bool
FileSystem::CreateDirectory(const char *name)
{  
  Directory *directory;
  BitMap *freeMap;
  FileHeader *hdr;
  int sector;
  bool success;

  DEBUG('f', "Creating directory %s \n", name);

  directory = new Directory(NumDirEntries);
  directory->FetchFrom(currDirFile);

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
            directory->WriteBack(currDirFile);
        // update current folder in its own file on the disk
            OpenFile *updateDir = new OpenFile(directory->Find("."));   // '.' has current folder sector
            directory->WriteBack(updateDir);

            freeMap->WriteBack(freeMapFile);

            delete myDirectory;
            delete newDir;
            delete updateDir;
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
    Directory *directory;
    BitMap *freeMap;
    FileHeader *fileHdr;
    int sector;
    
    directory = new Directory(NumDirEntries);
    directory->FetchFrom(currDirFile);

    sector = directory->Find(name);
    printf("Sektor: %d\n", sector);
    if (sector == -1) {
       delete directory;
       printf("File or directory not found!\n");
       return FALSE;			 // file not found 
    }
    
    Directory *dirToRemove = new Directory(NumDirEntries);
    OpenFile *file = new OpenFile(sector);
    dirToRemove->FetchFrom(file);


    if( !dirToRemove->isDirectory() )
    {
        printf("It is not a directory!\n");

        delete dirToRemove;
        delete file;
        return FALSE;
    }

    if( !(dirToRemove->isEmpty()) )
    {
        printf("Directory %s cannot be removed!\n", name);
        printf("It is either not a directory, or it is not empty\n");
        
        delete dirToRemove;
        delete file;    
        delete directory;
        return FALSE; 
    }
    
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new BitMap(NumSectors);
    freeMap->FetchFrom(freeMapFile);

    fileHdr->Deallocate(freeMap);  		// remove data blocks
    freeMap->Clear(sector);			// remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);		// flush to disk
    directory->WriteBack(currDirFile);        // flush to disk
// update current folder in its own file on the disk
    OpenFile *updateDir = new OpenFile(directory->Find("."));   // '.' has current folder sector
    directory->WriteBack(updateDir);

    delete dirToRemove;
    delete file;

    delete fileHdr;
    delete directory;
    delete freeMap;
    delete updateDir;

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
    
    char * token;
    const char s[2] = "/";
   token = strtok(name1, s);
   while( token != NULL )
   {
    
    // if(token == NULL)
    // {
    //  break;  
    // }
    
    Directory *directory = new Directory(NumDirEntries);
    directory->FetchFrom(currDirFile);    
    
    int sector = directory->Find(token);
    if (sector == -1) {
       delete directory;
       printf("No such file or directory!\n");
       return FALSE;             // file not found 
    }
    
    Directory *newDir = new Directory(NumDirEntries);
    OpenFile *newDirFile = new OpenFile(sector);
    newDir->FetchFrom(newDirFile);
    newDir->WriteBack(currDirFile);

    delete newDir;
    delete directory;
    delete newDirFile;
    
    token = strtok(NULL, s);
   // printf("moving to directory: %s \n", token);
   
    
   }//while;

    return TRUE;
}

#endif