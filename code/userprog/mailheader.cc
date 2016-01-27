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
        id = p->id;
        timestamp = p->timestamp;
        isConfirmation = p->isConfirmation;
    } else {
        id = 0;
        timestamp = 0;
        isConfirmation = true;
    }
}

MailHeaderReliableAnySize::MailHeaderReliableAnySize(const MailHeader *mailHdr) {
    to = mailHdr->to;
    from = mailHdr->from;
    length = mailHdr->length;
    id = 0;
    timestamp = 0;
    isConfirmation = true;
    theLast = false;
}
#endif // CHANGED
#endif // NETWORK