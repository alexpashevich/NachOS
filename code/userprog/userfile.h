#ifndef USERFILE_H
#define USERFILE_H


#ifdef CHANGED
extern int do_UserCreateFile(char* path);
extern int do_UserOpenFile(char* path);
extern int do_UserCloseFile(int id);
extern int do_UserReadFile(int id, int into, int numBytes);
extern int do_UserWriteFile(int id, char *from, int numBytes);
#endif

#endif