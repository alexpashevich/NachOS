// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

#ifdef CHANGED
#include "thread.h"
#include <sys/time.h>
#ifdef FILESYS
#include "filesys.h"
#endif
#endif


// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

#ifdef NETWORK
void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
#ifndef CHANGED
    printf("Starting MailTest...\n");
    const char *data = "Hello there!";
    const char *ack = "Got it!";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    postOffice->Send(outPktHdr, outMailHdr, data); 

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    postOffice->Send(outPktHdr, outMailHdr, ack); 

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);
#else
    printf("Started the test\n");
    char data[20];
    int times = 10;
    for (int i = 0; i < times; ++i) {
        sprintf(data, "MESSAGE_%d", i);
        outPktHdr.to = farAddr;
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;
        postOffice->SendReliable(outPktHdr, &outMailHdr, data);
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
    }
    currentThread->Sleep(2000);
#endif // CHANGED
    // Then we're done!
    interrupt->Halt();
}

#ifdef CHANGED

void MailCircleTest(int n) {
    printf("Starting MailCircleTest...\n");
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];
    int farAddr = (postOffice->GetNetworkName() + 1) % n;
    outPktHdr.to = farAddr;
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    char data[8] = "MESSAGE";
    char data_final[8] = "FINAL  ";
    if (farAddr == 1) {
        outMailHdr.length = strlen(data) + 1;
        printf("Sent \"%s\" to machine %d, box %d\n",data,outPktHdr.to,outPktHdr.to);
        fflush(stdout);
        postOffice->Send(outPktHdr, &outMailHdr, data);
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from machine %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
    } else {
        outMailHdr.length = strlen(data) + 1;
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from machine %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
        if (farAddr == 0) {
            printf("Sent \"%s\" to machine %d, box %d\n",data_final,inPktHdr.to,inMailHdr.to);
            fflush(stdout);
            postOffice->Send(outPktHdr, &outMailHdr, data_final);
        } else {
            printf("Sent \"%s\" to machine %d, box %d\n",data,inPktHdr.to,inMailHdr.to);
            fflush(stdout);
            postOffice->Send(outPktHdr, &outMailHdr, data);
        }
    }
    Delay(1);
    interrupt->Halt();
}

void ReliableMailTest(int farAddr) {
    printf("Starting ReliableMailTest...\n");
    PacketHeader outPktHdr, inPktHdr;
    MailHeader *outMailHdr, inMailHdr;
    outMailHdr = new MailHeader;
    char buffer[MaxMailSize];

    const char *data = "Hello there!";
    const char *ack = "Got it!";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.from = postOffice->GetNetworkName();
    outPktHdr.to = farAddr;     
    outMailHdr->to = 0;
    outMailHdr->from = 1;
    outMailHdr->length = strlen(data) + 1;

    if (postOffice->SendReliable(outPktHdr, outMailHdr, data) == -1) {
        printf("[NETTEST FUNCTION] Can not send \"%s\"\n", data);
        interrupt->Halt();
    } else {
        printf("[NETTEST FUNCTION] Sent \"%s\" successfully\n", data);
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("[NETTEST FUNCTION] Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr->to = inMailHdr.from;
    outMailHdr->length = strlen(ack) + 1;
    // postOffice->SendReliable(outPktHdr, outMailHdr, ack);
    if (postOffice->SendReliable(outPktHdr, outMailHdr, ack) == -1) {
        printf("[NETTEST FUNCTION] Can not send \"%s\"\n", ack);
        interrupt->Halt();
    } else {
        printf("[NETTEST FUNCTION] Sent \"%s\" successfully\n", ack);
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("[NETTEST FUNCTION] Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);
    currentThread->Sleep(2000);
    interrupt->Halt();
}

void VariableMailTest(int farAddr) {
    printf("Starting VariableMailTest...\n");
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[10000];

    const char *data = "I'd like to repeat the advice that I gave you before, in that I think you really "
    "should make a radical change in your lifestyle and begin to boldly do things which you may previously "
    "never have thought of doing, or been too hesitant to attempt. So many people live within unhappy "
    "circumstances and yet will not take the initiative to change their situation because they are conditioned "
    "to a life of security, conformity, and conservatism, all of which may appear to give one peace of mind, but "
    "in reality nothing is more damaging to the adventurous spirit within a man than a secure future. The very basic "
    "core of a man's living spirit is his passion for adventure. The joy of life comes from our encounters with new "
    "experiences, and hence there is no greater joy than to have an endlessly changing horizon, for each day to have "
    "a new and different sun.\n"
    "If you want to get more out of life, Ron, you must lose your inclination for monotonous security and adopt a "
    "helter-skelter style of life that will at first appear to you to be crazy. But once you become accustomed to "
    "such a life you will see its full meaning and its incredible beauty. And so, Ron, in short, get out of Salton "
    "City and hit the Road. I guarantee you will be very glad you did. But I fear that you will ignore my advice. You "
    "think that I am stubborn, but you are even more stubborn than me. You had a wonderful chance on your drive back to "
    "see one of the greatest sights on earth, the Grand Canyon, something every American should see at least once in his "
    "life. But for some reason incomprehensible to me you wanted nothing but to bolt for home as quickly as possible, right "
    "back to the same situation which you see day after day after day. I fear you will follow this same inclination in the "
    "future and thus fail to discover all the wonderful things that God has placed around us to discover.\n"
    "Don't settle down and sit in one place. Move around, be nomadic, make each day a new horizon. You are still going to "
    "live a long time, Ron, and it would be a shame if you did not take the opportunity to revolutionize your life and move "
    "into an entirely new realm of experience.\n"
    "You are wrong if you think Joy emanates only or principally from human relationships. God has placed it all around us. It "
    "is in everything and anything we might experience. We just have to have the courage to turn against our habitual lifestyle "
    "and engage in unconventional living.\n"
    "My point is that you do not need me or anyone else around to bring this new kind of light in your life. It is simply waiting out "
    "there for you to grasp it, and all you have to do is reach for it. The only person you are fighting is yourself and your "
    "stubbornness to engage in new circumstances.";
    // const char *data = "I'd like to repeat the advice that I gave you before, in that I think you really "
    // "should make a radical change in your lifestyle and begin to boldly do things which you may previously "
    // "never have thought of doing, or been too hesitant to attempt. So many people live within unhappy "
    // "circumstances and yet will not take the initiative to change their situation because they are conditioned\n";
    const char *ack = "Got it!";
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;
    outPktHdr.from = postOffice->GetNetworkName();
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    postOffice->SendReliableAnySize(outPktHdr, &outMailHdr, data); 

    // Wait for the first message from the other machine
    postOffice->ReceiveAnySize(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);
    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    postOffice->SendReliableAnySize(outPktHdr, &outMailHdr, ack); 

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->ReceiveAnySize(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);
    currentThread->Sleep(2000);
    interrupt->Halt();
}
// #endif

// #ifdef CHANGED

#ifdef FILESYS
struct ServClientArgs {
    ServClientArgs(int addr_, int port_, int boxNb_):
        addr(addr_), port(port_), boxNb(boxNb_) {}
    int addr;
    int port;
    int boxNb;
};

void TalkWithClient(int arg) {
    printf("One client is being served\n");
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long begin = tv.tv_sec*1000LL + tv.tv_usec/1000;

    ServClientArgs *realarg = (ServClientArgs*) arg;
    int addr = realarg->addr;
    int port = realarg->port;
    int boxNb = realarg->boxNb;
    delete realarg;

    PacketHeader outPktHdr;
    outPktHdr.to = addr;
    outPktHdr.from = postOffice->GetNetworkName();
    MailHeader outMailHdr(port, boxNb, 4);

    PacketHeader inPktHdr;
    MailHeader inMailHdr;

    postOffice->SendReliableAnySize(outPktHdr, &outMailHdr, (char*) &boxNb);
    char filename[20];
    postOffice->Receive(boxNb, &inPktHdr, &inMailHdr, filename);

    fflush(stdout);
    if (!fileSystem->MoveToFile("", filename) ){
        fileSystem->setCurrentSector();
        // TODO:
        // printf("Couldnt go to directory!\n");
        // return -1;
    }

    OpenFile *file = fileSystem->Open(filename);
    fileSystem->setCurrentSector();

    if (file != NULL) {
        int number_of_bytes = 3750;
        char into_char[number_of_bytes];
        file->Read(into_char, number_of_bytes);

        number_of_bytes = strlen(into_char);;
        outMailHdr.length = number_of_bytes;
        postOffice->SendReliableAnySize(outPktHdr, &outMailHdr, into_char);
    } else {
        printf("Can not open the file\n");
    }
    
    gettimeofday(&tv, NULL);
    long long end = tv.tv_sec*1000LL + tv.tv_usec/1000;
    printf("Speed = %f KB/sec\n", 1.0 * 3750 / (end - begin));

    printf("Finishing the thread...\n");
    currentThread->Finish();
}


void FileServer() {
    printf("Server is started\n");
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    int clientport;
    int connectedMachines = 0;
    while (connectedMachines < postOffice->GetNumberOfBoxes() - 1) {
        postOffice->Receive(0, &inPktHdr, &inMailHdr, (char*)&clientport);
        ServClientArgs *arg = new ServClientArgs(inPktHdr.from, clientport, connectedMachines + 1);
        Thread *t = new Thread("client service");
        t->Fork(TalkWithClient, (int)arg);
        ++connectedMachines;
    }
    interrupt->Halt();
}
// #endif // CHANGED_AGAIN
#endif // FILESYS
#endif // CHANGED
#endif // NETWORK







    