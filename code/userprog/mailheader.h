#ifdef NETWORK
#ifdef CHANGED

#include <ctime>

// Mailbox address -- uniquely identifies a mailbox on a given machine.
// A mailbox is just a place for temporary storage for messages.
typedef int MailBoxAddress;

// The following class defines part of the message header.  
// This is prepended to the message by the PostOffice, before the message 
// is sent to the Network.

class MailHeader {
  public:
    MailHeader() {}
    virtual ~MailHeader() {}
    MailBoxAddress to;      // Destination mail box
    MailBoxAddress from;    // Mail box to reply to
    unsigned length;        // Bytes of message data (excluding the 
                // mail header)
};

class MailHeaderReliable: public MailHeader {
  public:
    MailHeaderReliable() {}
    MailHeaderReliable(const MailHeader *mailHdr);
    time_t timestamp;
    int id;
    bool isConfirmation;
};
#endif // CHANGED
#endif // NETWORK