// post.cc 
// 	Routines to deliver incoming network messages to the correct
//	"address" -- a mailbox, or a holding area for incoming messages.
//	This module operates just like the US postal service (in other
//	words, it works, but it's slow, and you can't really be sure if
//	your mail really got through!).
//
//	Note that once we prepend the MailHdr to the outgoing message data,
//	the combination (MailHdr plus data) looks like "data" to the Network 
//	device.
//
// 	The implementation synchronizes incoming messages with threads
//	waiting for those messages.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "post.h"
// #ifndef CHANGED
#include  "system.h" // PROBABLY not needed if CHANGED is not defined
// #endif

#include <strings.h> /* for bzero */

//----------------------------------------------------------------------
// Mail::Mail
//      Initialize a single mail message, by concatenating the headers to
//	the data.
//
//	"pktH" -- source, destination machine ID's
//	"mailH" -- source, destination mailbox ID's
//	"data" -- payload data
//----------------------------------------------------------------------

Mail::Mail(PacketHeader pktH, MailHeader mailH, char *msgData)
{
    ASSERT(mailH.length <= MaxMailSize);

    pktHdr = pktH;
    mailHdr = mailH;
    bcopy(msgData, data, mailHdr.length);
}

//----------------------------------------------------------------------
// MailBox::MailBox
//      Initialize a single mail box within the post office, so that it
//	can receive incoming messages.
//
//	Just initialize a list of messages, representing the mailbox.
//----------------------------------------------------------------------


MailBox::MailBox()
{ 
    messages = new SynchList(); 
}

//----------------------------------------------------------------------
// MailBox::~MailBox
//      De-allocate a single mail box within the post office.
//
//	Just delete the mailbox, and throw away all the queued messages 
//	in the mailbox.
//----------------------------------------------------------------------

MailBox::~MailBox()
{ 
    delete messages; 
}

//----------------------------------------------------------------------
// PrintHeader
// 	Print the message header -- the destination machine ID and mailbox
//	#, source machine ID and mailbox #, and message length.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//----------------------------------------------------------------------

static void 
PrintHeader(PacketHeader pktHdr, MailHeader mailHdr)
{
    printf("From (%d, %d) to (%d, %d) bytes %d\n",
    	    pktHdr.from, mailHdr.from, pktHdr.to, mailHdr.to, mailHdr.length);
}

//----------------------------------------------------------------------
// MailBox::Put
// 	Add a message to the mailbox.  If anyone is waiting for message
//	arrival, wake them up!
//
//	We need to reconstruct the Mail message (by concatenating the headers
//	to the data), to simplify queueing the message on the SynchList.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

void 
MailBox::Put(PacketHeader pktHdr, MailHeader mailHdr, char *data)
{ 
    Mail *mail = new Mail(pktHdr, mailHdr, data); 

    messages->Append((void *)mail);	// put on the end of the list of 
					// arrived messages, and wake up 
					// any waiters
}

//----------------------------------------------------------------------
// MailBox::Get
// 	Get a message from a mailbox, parsing it into the packet header,
//	mailbox header, and data. 
//
//	The calling thread waits if there are no messages in the mailbox.
//
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

void 
MailBox::Get(PacketHeader *pktHdr, MailHeader *mailHdr, char *data) 
{ 
    DEBUG('n', "Waiting for mail in mailbox\n");
    Mail *mail = (Mail *) messages->Remove();	// remove message from list;
						// will wait if list is empty

    *pktHdr = mail->pktHdr;
    *mailHdr = mail->mailHdr;
    if (DebugIsEnabled('n')) {
    	printf("Got mail from mailbox: ");
    	PrintHeader(*pktHdr, *mailHdr);
    }
    bcopy(mail->data, data, mail->mailHdr.length);
					// copy the message data into
					// the caller's buffer
    delete mail;			// we've copied out the stuff we
					// need, we can now discard the message
}

//----------------------------------------------------------------------
// PostalHelper, ReadAvail, WriteDone
// 	Dummy functions because C++ can't indirectly invoke member functions
//	The first is forked as part of the "postal worker thread; the
//	later two are called by the network interrupt handler.
//
//	"arg" -- pointer to the Post Office managing the Network
//----------------------------------------------------------------------

static void PostalHelper(int arg)
{ PostOffice* po = (PostOffice *) arg; po->PostalDelivery(); }
static void ReadAvail(int arg)
{ PostOffice* po = (PostOffice *) arg; po->IncomingPacket(); }
static void WriteDone(int arg)
{ PostOffice* po = (PostOffice *) arg; po->PacketSent(); }

//----------------------------------------------------------------------
// PostOffice::PostOffice
// 	Initialize a post office as a collection of mailboxes.
//	Also initialize the network device, to allow post offices
//	on different machines to deliver messages to one another.
//
//      We use a separate thread "the postal worker" to wait for messages 
//	to arrive, and deliver them to the correct mailbox.  Note that
//	delivering messages to the mailboxes can't be done directly
//	by the interrupt handlers, because it requires a Lock.
//
//	"addr" is this machine's network ID 
//	"reliability" is the probability that a network packet will
//	  be delivered (e.g., reliability = 1 means the network never
//	  drops any packets; reliability = 0 means the network never
//	  delivers any packets)
//	"nBoxes" is the number of mail boxes in this Post Office
//----------------------------------------------------------------------

PostOffice::PostOffice(NetworkAddress addr, double reliability, int nBoxes)
{
// First, initialize the synchronization with the interrupt handlers
    messageAvailable = new Semaphore("message available", 0);
    messageSent = new Semaphore("message sent", 0);
    sendLock = new Lock("message send lock");

// Second, initialize the mailboxes
    netAddr = addr; 
    numBoxes = nBoxes;
    boxes = new MailBox[nBoxes];

// Third, initialize the network; tell it which interrupt handlers to call
    network = new Network(addr, reliability, ReadAvail, WriteDone, (int) this);


// Finally, create a thread whose sole job is to wait for incoming messages,
//   and put them in the right mailbox. 
    Thread *t = new Thread("postal worker");

    t->Fork(PostalHelper, (int) this);
}

//----------------------------------------------------------------------
// PostOffice::~PostOffice
// 	De-allocate the post office data structures.
//----------------------------------------------------------------------

PostOffice::~PostOffice()
{
    delete network;
    delete [] boxes;
    delete messageAvailable;
    delete messageSent;
    delete sendLock;
}

//----------------------------------------------------------------------
// PostOffice::PostalDelivery
// 	Wait for incoming messages, and put them in the right mailbox.
//
//      Incoming messages have had the PacketHeader stripped off,
//	but the MailHeader is still tacked on the front of the data.
//----------------------------------------------------------------------

void
PostOffice::PostalDelivery()
{
#ifdef CHANGED
    PacketHeader pktHdr;
    MailHeader *mailHdr;
    char *buffer = new char[MaxPacketSize];

    for (;;) {
        // first, wait for a message
        messageAvailable->P();  
        pktHdr = network->Receive(buffer);

        MailHeaderReliable* hdr = NULL;
        MailHeaderReliableAnySize *hdrAnySize = NULL;
        mailHdr = (MailHeader *)buffer;
        if (pktHdr.length == mailHdr->length + sizeof(MailHeaderReliable)) {
            hdr = (MailHeaderReliable *)buffer;
        }
        if (pktHdr.length == mailHdr->length + sizeof(MailHeaderReliableAnySize)) {
            hdrAnySize = (MailHeaderReliableAnySize *)buffer;
        }

        if (DebugIsEnabled('n')) {
            printf("Putting mail into mailbox: ");
            PrintHeader(pktHdr, *mailHdr);
        }

        // check that arriving message is legal!
        ASSERT(0 <= mailHdr->to && mailHdr->to < numBoxes);
        ASSERT(mailHdr->length <= MaxMailSize);

        // put into mailbox
        if (hdr != NULL) {
            // reliable transfer is happening

            PostOfficeReliable* p = static_cast<PostOfficeReliable*>(this);
            // it means that we are PostOfficeReliable
            if (hdr->isConfirmation) {
                // confirmation mail was received

                DEBUG('u', "confirmation received, id = %d\n", hdr->id);
                MailHeaderReliable *hdrRel = new MailHeaderReliable(hdr);
                p->confirmationMails->Append(hdrRel);
            } else {
                // normal mail was received

                DEBUG('u', "normal mail received, text = \"%s\", ", buffer + sizeof(MailHeaderReliable));
                time_t maxlag = TEMPO * (MAXREEMISSIONS + 1);
                
                // TODO: check from here
                if (!p->oldMessages->CheckOldMessages(hdr, maxlag, false)) {
                    // if mailHdr is NOT in oldMessages
                    DEBUG('u', "NOT repeated\n");
                    boxes[mailHdr->to].Put(pktHdr, *mailHdr, buffer + sizeof(MailHeaderReliable));
                    MailHeaderReliable *newMailHdr = new MailHeaderReliable(hdr);
                    p->oldMessages->Append(newMailHdr);
                } else {
                    DEBUG('u', "repeated\n");
                }

                // but send the confirmation anyway

                // setting headers to send the confirmation
                MailHeaderReliable mailHdrConf(hdr);
                mailHdrConf.length = 0;
                mailHdrConf.isConfirmation = true;

                PacketHeader pktHdrConf;
                pktHdrConf.to = pktHdr.from;
                pktHdrConf.from = pktHdr.to;
                pktHdrConf.length = mailHdrConf.length + sizeof(MailHeaderReliable);

            // sending the confirmation
                this->Send(pktHdrConf, &mailHdrConf, NULL);
            }
        } else
            boxes[mailHdr->to].Put(pktHdr, *mailHdr, buffer + sizeof(MailHeader));
    }
#else
    PacketHeader pktHdr;
    MailHeader mailHdr;
    char *buffer = new char[MaxPacketSize];

    for (;;) {
        // first, wait for a message
        messageAvailable->P();  
        pktHdr = network->Receive(buffer);

        mailHdr = *(MailHeader *)buffer;
        if (DebugIsEnabled('n')) {
            printf("Putting mail into mailbox: ");
            PrintHeader(pktHdr, mailHdr);
        }

        // check that arriving message is legal!
        ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);
        ASSERT(mailHdr.length <= MaxMailSize);

        
        boxes[mailHdr.to].Put(pktHdr, mailHdr, buffer + sizeof(MailHeader));
    }
#endif
}

//----------------------------------------------------------------------
// PostOffice::Send
// 	Concatenate the MailHeader to the front of the data, and pass 
//	the result to the Network for delivery to the destination machine.
//
//	Note that the MailHeader + data looks just like normal payload
//	data to the Network.
//
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

#ifdef CHANGED
void PostOffice::Send(PacketHeader pktHdr, const MailHeader *mailHdr, const char* data) {
    char* buffer = new char[MaxPacketSize]; // space to hold concatenated
                        // mailHdr + data

    if (DebugIsEnabled('n')) {
        printf("Post send: ");
        PrintHeader(pktHdr, *mailHdr);
    }
    ASSERT(mailHdr->length <= MaxMailSize);
    ASSERT(0 <= mailHdr->to && mailHdr->to < numBoxes);
    
    // fill in pktHdr, for the Network layer
    pktHdr.from = netAddr;
    if (MailHeaderReliable *mailHdrReliable = dynamic_cast<MailHeaderReliable*>(const_cast<MailHeader*>(mailHdr))) {
        pktHdr.length = mailHdrReliable->length + sizeof(MailHeaderReliable);

        // concatenate MailHeader and data
        memcpy(buffer, mailHdrReliable, sizeof(MailHeaderReliable));
        memcpy(buffer + sizeof(MailHeaderReliable), data, mailHdrReliable->length);
    } else if (MailHeaderReliableAnySize *mailHdrReliableAnySize = dynamic_cast<MailHeaderReliableAnySize*>(const_cast<MailHeader*>(mailHdr))) {
        pktHdr.length = mailHdrReliableAnySize->length + sizeof(MailHeaderReliableAnySize);

        // concatenate MailHeader and data
        memcpy(buffer, mailHdrReliableAnySize, sizeof(MailHeaderReliableAnySize));
        memcpy(buffer + sizeof(MailHeaderReliableAnySize), data, mailHdrReliableAnySize->length);
    } else {
        pktHdr.length = mailHdr->length + sizeof(MailHeader);

        // concatenate MailHeader and data
        bcopy(mailHdr, buffer, sizeof(MailHeader));
        bcopy(data, buffer + sizeof(MailHeader), mailHdr->length);
    }

    sendLock->Acquire();        // only one message can be sent
                    // to the network at any one time

    network->Send(pktHdr, buffer);
    messageSent->P();           // wait for interrupt to tell us
                    // ok to send the next message
    sendLock->Release();

    delete [] buffer;           // we've sent the message, so
                    // we can delete our buffer
}
#else
void PostOffice::Send(PacketHeader pktHdr, MailHeader mailHdr, const char* data)
{
    char* buffer = new char[MaxPacketSize];	// space to hold concatenated
						// mailHdr + data

    if (DebugIsEnabled('n')) {
    	printf("Post send: ");
    	PrintHeader(pktHdr, mailHdr);
    }
    ASSERT(mailHdr.length <= MaxMailSize);
    ASSERT(0 <= mailHdr.to && mailHdr.to < numBoxes);
    
    // fill in pktHdr, for the Network layer
    pktHdr.from = netAddr;

    pktHdr.length = mailHdr.length + sizeof(MailHeader);

    // concatenate MailHeader and data
    bcopy(&mailHdr, buffer, sizeof(MailHeader));
    bcopy(data, buffer + sizeof(MailHeader), mailHdr.length);

    sendLock->Acquire();   		// only one message can be sent
					// to the network at any one time
    network->Send(pktHdr, buffer);
    messageSent->P();			// wait for interrupt to tell us
					// ok to send the next message
    sendLock->Release();

    delete [] buffer;			// we've sent the message, so
					// we can delete our buffer
}
#endif

//----------------------------------------------------------------------
// PostOffice::Receive
// 	Retrieve a message from a specific box if one is available, 
//	otherwise wait for a message to arrive in the box.
//
//	Note that the MailHeader + data looks just like normal payload
//	data to the Network.
//
//
//	"box" -- mailbox ID in which to look for message
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

void
PostOffice::Receive(int box, PacketHeader *pktHdr, 
				MailHeader *mailHdr, char* data)
{
    ASSERT((box >= 0) && (box < numBoxes));

    boxes[box].Get(pktHdr, mailHdr, data);
    ASSERT(mailHdr->length <= MaxMailSize);
}

//----------------------------------------------------------------------
// PostOffice::IncomingPacket
// 	Interrupt handler, called when a packet arrives from the network.
//
//	Signal the PostalDelivery routine that it is time to get to work!
//----------------------------------------------------------------------

void
PostOffice::IncomingPacket()
{ 
    messageAvailable->V(); 
}

//----------------------------------------------------------------------
// PostOffice::PacketSent
// 	Interrupt handler, called when the next packet can be put onto the 
//	network.
//
//	The name of this routine is a misnomer; if "reliability < 1",
//	the packet could have been dropped by the network, so it won't get
//	through.
//----------------------------------------------------------------------

void 
PostOffice::PacketSent()
{ 
    messageSent->V();
}

#ifdef CHANGED
//----------------------------------------------------------------------
// PostOffice::GetNetworkName
//  Get network name
//----------------------------------------------------------------------
int PostOffice::GetNetworkName() {
    return (int) netAddr;
}

PostOfficeReliable::PostOfficeReliable(NetworkAddress addr, double reliability, int nBoxes):
                    PostOffice(addr, reliability, nBoxes) {
    confirmationMails = new ListOfMails;
    oldMessages = new ListOfMails;
}

PostOfficeReliable::~PostOfficeReliable() {
    delete confirmationMails;
    delete oldMessages;
}

int PostOfficeReliable::SendReliable(PacketHeader pktHdr, const MailHeader *mailHdr, const char *data) {
// setting internal fields
    MailHeaderReliable *mailHdrReliable = new MailHeaderReliable(mailHdr);
    // if (mailHdrReliable = (MailHeaderReliable*) dynamic_cast<MailHeaderReliableAnySize*>(const_cast<MailHeader*>(mailHdr))) {
        // it means that we are trying to use PostOffice with variable size features
        // TODO: do smth here ???
    // } else {
        // normal PostOfficeReliable usage
        // MailHeaderReliable *mailHdrReliable = new MailHeaderReliable(mailHdr);
    // }
    mailHdrReliable->isConfirmation = false;
    mailHdrReliable->id = Random() + (int) data;
    time(&mailHdrReliable->timestamp);

// sending the mail
    this->Send(pktHdr, mailHdrReliable, data);
    DEBUG('u', "Sending \"%s\" the first time\n", data);

// waiting for some time to give the other machine chance to send the confirmation

    currentThread->Sleep(TEMPO);
    int counter = 1;
    bool confirmed = false;
    DEBUG('u', "I am awaken\n");
    while (!(confirmed = this->CheckConfirmation(pktHdr, *mailHdrReliable)) && counter < MAXREEMISSIONS) { // TODO: check if this works with MailHeaderReliableAnySize
        DEBUG('u', "Didn't receive the confirmation, sending \"%s\" again\n", data);
// if the confirmation is not received then we think that our message was not delivered
// so send it again
        this->Send(pktHdr, mailHdrReliable, data);
        ++counter;

// and wait again
        currentThread->Sleep(TEMPO);
    }

    if (confirmed)
        printf("[SUCCESS] Confirmation was received, sent from %d time\n", counter);
    delete mailHdrReliable;
    // printf("Received the confirmation? counter = %d\n", counter);
// return the result
    if (counter == MAXREEMISSIONS)
        return -1;
    return 0;
}

void PostOfficeReliable::ReceiveReliable(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data) {
// receiving the mail
    // TODO:
    DEBUG('u', "Trying to receive a mail\n");
    this->Receive(box, pktHdr, mailHdr, data); // or change to MailHeaderReliable  ???
}

bool PostOfficeReliable::CheckConfirmation(PacketHeader pktHdr, MailHeaderReliable mailHdr) {
    DEBUG('u', "Checking confirmation\n");
    int maxlag = TEMPO * MAXREEMISSIONS;
    bool res = confirmationMails->CheckOldMessages(&mailHdr, maxlag, true);
    return res;
}





















PostOfficeReliableAnySize::PostOfficeReliableAnySize(NetworkAddress addr, double reliability, int nBoxes):
                    PostOfficeReliable(addr, reliability, nBoxes) {
    // nothing to do here
}

PostOfficeReliableAnySize::~PostOfficeReliableAnySize() {
    // nothing to do here as well
}

void PostOfficeReliableAnySize::SendReliableAnySize(PacketHeader pktHdr, const MailHeader *mailHdr, const char *data) {
#ifdef NOT_COMPILED
    int length = mailHdr->length;

    for (int p = 0; p < length; p += MaxMailSize) {
        MailHeaderReliableAnySize *tempMailHdr = new MailHeaderReliableAnySize(mailHdr);
        tempMailHdr->length = (length - p < (int) MaxMailSize ? length - p : MaxMailSize);
        tempMailHdr->theLast = (p + (int) MaxMailSize >= length ? true : false);
        char tempData[tempMailHdr->length];
        memcpy(tempData, data + p, tempMailHdr->length);
        this->SendReliable(pktHdr, tempMailHdr, tempData);
        delete tempMailHdr;
        printf("send it\n");
    }
    printf("done with sending\n");
#endif // NOT_COMPILED
}

void PostOfficeReliableAnySize::ReceiveReliableAnySize(int box, PacketHeader *pktHdr, MailHeader *mailHdr, char *data) {
#ifdef NOT_COMPILED
    PacketHeader tempPktHdr;
    MailHeader tempMailHdr;
    char *firstData = new char[MaxMailSize];
    List dataList;
    int size = 0;
    printf("trying to receive\n");
    postOffice->Receive(box, &tempPktHdr, &tempMailHdr, firstData);
    printf("can not receive\n");
    size += tempMailHdr.length;
    dataList.Append((int*) firstData);
    while (!tempMailHdr.theLast) {
        char *tempData = new char[MaxMailSize];
        postOffice->Receive(box, &tempPktHdr, &tempMailHdr, tempData);
        size += tempMailHdr.length;
        dataList.Append((int*) tempData);
        printf("received one\n");
    }

    printf("received everything\n");
    pktHdr->to = tempPktHdr.to;
    pktHdr->from = tempPktHdr.from;
    pktHdr->length = size + sizeof(MailHeader);

    mailHdr->to = tempMailHdr.to;
    mailHdr->from = tempMailHdr.from;
    mailHdr->length = size;

    char *tempData;
    int bytescopied = 0;
    while ((tempData = (char*) dataList.Remove()) != NULL) {
        memcpy(data + bytescopied, tempData, (bytescopied + (int) MaxMailSize >= size ? size - bytescopied : MaxMailSize));
        delete[] tempData;
        bytescopied += MaxMailSize;
    }
#endif // NOT_COMPILED
}
#endif // CHANGED

















