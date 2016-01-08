#ifdef CHANGED

#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"


static Semaphore *readAvail;
static Semaphore *writeDone;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    console = new Console(readFile, writeFile,
                          ReadAvail, WriteDone, 0);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeDone;
    delete readAvail;
}

void SynchConsole::SynchPutChar(const char ch)
{
    console->PutChar(ch);
    writeDone->P();
}

char SynchConsole::SynchGetChar()
{
    readAvail->P();
    return console->GetChar();
}

void SynchConsole::SynchPutString(const char s[])
{
    int i;
    for (i = 0; s[i] != '\0'; ++i)
    {
        console->PutChar(s[i]);
        writeDone->P();
    }
}

void SynchConsole::SynchGetString(char *s, int n)
{
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
}

#endif