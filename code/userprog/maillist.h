#ifndef MAILLIST_H
#define MAILLIST_H

#ifdef NETWORK
#ifdef CHANGED

#include "copyright.h"
#include "utility.h"
#include "list.h"
#include <ctime>
#include "mailheader.h"

// The following class defines a "list of mails" -- a singly linked list of
// list elements, each of which points to a single item on the list.
// Implementation of the class in based on List class provided in NachOS.

class ListOfMails {
public:
    ListOfMails();                      // initialize the list
    ~ListOfMails();                     // de-allocate the list
    void Append(MailHeaderReliable *hdr); // Put item at the end of the list
    MailHeaderReliable* Remove(); // Take item off the front of the list

    bool CheckOldMessages(MailHeaderReliable* elem, time_t maxlag, bool deleteIfFound);
                                        // Find the element and delete all elements
                                        // timestamp of which are outdated by more than maxlag
                                        // If the flag deleteIfFound is set, delete also the element
private:
    List *list;                         // list which perform all the appending and removing
};

#endif // CHANGED
#endif // NETWORK

#endif // MAILLIST_H
