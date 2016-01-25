#ifdef NETWORK
#ifdef CHANGED

#include "mailheader.h"
#include <cstdio>

MailHeaderReliable::MailHeaderReliable(const MailHeader *mailHdr) {
    to = mailHdr->to;
    from = mailHdr->from;
    length = mailHdr->length;
    MailHeaderReliable *p;
    if ((p = dynamic_cast<MailHeaderReliable*>(const_cast<MailHeader*>(mailHdr))) != NULL) {
        printf("Creating from MailHeaderReliable\n");
        id = p->id;
        timestamp = p->timestamp;
        isConfirmation = p->isConfirmation;
    } else {
        printf("Creating not from MailHeaderReliable\n");
        id = 0;
        timestamp = 0;
        isConfirmation = true;
    }
}
#endif // CHANGED
#endif // NETWORK