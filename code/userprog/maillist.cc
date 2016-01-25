#ifdef NETWORK
#ifdef CHANGED

#include "copyright.h"
#include "maillist.h"

//----------------------------------------------------------------------
// List::List
//      Initialize a list, empty to start with.
//      Elements can now be added to the list.
//----------------------------------------------------------------------

ListOfMails::ListOfMails() {
    list = new List;
}

//----------------------------------------------------------------------
// List::~List
//      Prepare a list for deallocation.  If the list still contains any 
//      ListElements, de-allocate them.  However, note that we do *not*
//      de-allocate the "items" on the list -- this module allocates
//      and de-allocates the ListElements to keep track of each item,
//      but a given item may be on multiple lists, so we can't
//      de-allocate them here.
//----------------------------------------------------------------------

ListOfMails::~ListOfMails() {
    delete list;
}

//----------------------------------------------------------------------
// List::Append
//      Append an "item" to the end of the list.
//      
//      Allocate a ListElement to keep track of the item.
//      If the list is empty, then this will be the only element.
//      Otherwise, put it at the end.
//
//      "item" is the thing to put on the list, it can be a pointer to 
//              anything.
//----------------------------------------------------------------------

void ListOfMails::Append(MailHeaderReliable *mailHdr) {
    list->Append((void *) mailHdr);
}

//----------------------------------------------------------------------
// List::Remove
//      Remove the first "item" from the front of the list.
// 
// Returns:
//      Pointer to removed item, NULL if nothing on the list.
//----------------------------------------------------------------------

MailHeaderReliable *ListOfMails::Remove() {
    return (MailHeaderReliable *) list->Remove();
}

//----------------------------------------------------------------------
// ListOfMails::CheckOldMessages
//      Find the element and delete all
//      elements timestamp of which are outdated by more than maxlag.
// 
// Returns:
//      True if the element was found.
//      False otherwise.
//----------------------------------------------------------------------

bool ListOfMails::CheckOldMessages(MailHeaderReliable* elem, time_t maxlag, bool deleteIfFound) {
    time_t now;
    time(&now);
    ListElement *prev = NULL;
    ListElement *next = NULL;
    bool res = false;
    for (ListElement *ptr = list->first; ptr != NULL; ptr = next) {
        next = ptr->next;
        if (now - ((MailHeaderReliable*)ptr->item)->timestamp > maxlag) {
            if (prev == NULL)
                list->first = ptr->next;
            else
                prev->next = ptr->next;
            delete (MailHeaderReliable*) ptr->item;
            delete ptr;
            continue;
        }
        if (((MailHeaderReliable*) ptr->item)->id == elem->id &&
            ((MailHeaderReliable*) ptr->item)->timestamp == elem->timestamp) {
            res = true;
            if (deleteIfFound) {
                if (prev == NULL)
                    list->first = ptr->next;
                else
                    prev->next = ptr->next;
                delete (MailHeaderReliable*) ptr->item;
                delete ptr;
                continue;
            }
        }
        prev = ptr;
    }
    return res;
}

#endif // CHANGED
#endif // NETWORK