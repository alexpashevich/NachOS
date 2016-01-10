#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"


static Semaphore *readAvail;
static Semaphore *writeDone;
static Semaphore *putIsDoneByThread;
static Semaphore *getIsDoneByThread;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    console = new Console(readFile, writeFile,
                          ReadAvail, WriteDone, 0);
    putIsDoneByThread = new Semaphore("put is done by a thread", 1);
    getIsDoneByThread = new Semaphore("get is done by a thread", 1);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeDone;
    delete readAvail;
    delete putIsDoneByThread;
    delete getIsDoneByThread;
}

void SynchConsole::SynchPutChar(const char ch)
{
// the first lock is for synchronisation among different threads
// another thread can not do I/O until the first is done
    putIsDoneByThread->P();
    console->PutChar(ch);
    writeDone->P();
    putIsDoneByThread->V();
}

char SynchConsole::SynchGetChar()
{
// preventing multithread overlaping
    getIsDoneByThread->P();
    readAvail->P();
    char ch = console->GetChar();
    getIsDoneByThread->V();
    return ch;
}

void SynchConsole::SynchPutString(const char s[])
{
// the same idea as in the SynchPutChar
// here the lock before the loop means that outputs of
// several threads can not overlap
// if overlaping is needed, move locks inside
    putIsDoneByThread->P();
    int i;
    for (i = 0; s[i] != '\0'; ++i)
    {
        console->PutChar(s[i]);
        writeDone->P();
    }
    putIsDoneByThread->V();
}

void SynchConsole::SynchGetString(char *s, int n)
{
// preventing multithread overlaping
    getIsDoneByThread->P();
    int i;
    for (i = 0; i < n - 1; ++i)
    {
        readAvail->P();
        s[i] = console->GetChar();
        if (s[i] == '\n')
        {
            s[i + 1] = '\0';
            break;
        }
        if (s[i] == EOF)
        {
            s[i] = '\0';
            break;
        }
    }
    if (i == n - 1)
        s[n - 1] = '\0';
    getIsDoneByThread->V();
}

#endif