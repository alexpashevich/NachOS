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

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

#ifndef CHANGED
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
    char data[20];
    int times = 10, i;
    for (i = 0; i < times; ++i) {
        sprintf(data, "MESSAGE_%d", i);
        outPktHdr.to = farAddr;     
        outMailHdr.to = 0;
        outMailHdr.from = 1;
        outMailHdr.length = strlen(data) + 1;
        postOffice->Send(outPktHdr, outMailHdr, data);
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
    }
#endif
    // Then we're done!
    interrupt->Halt();
}

#ifdef CHANGED
void MailCircleTest(int n) {
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
        postOffice->Send(outPktHdr, outMailHdr, data);
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
            postOffice->Send(outPktHdr, outMailHdr, data_final);
        } else {
            printf("Sent \"%s\" to machine %d, box %d\n",data,inPktHdr.to,inMailHdr.to);
            fflush(stdout);
            postOffice->Send(outPktHdr, outMailHdr, data);
        }
    }
    interrupt->Halt();
}

void ReliableMailTest(int farAddr) {
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char buffer[MaxMailSize];

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
    if (reliableTransfer->Send(outPktHdr, outMailHdr, data) == -1) {
        printf("Machine %d: can not send \"%s\"\n", postOffice->GetNetworkName(), data);
        return;
    }

    // Wait for the first message from the other machine
    reliableTransfer->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    if (reliableTransfer->Send(outPktHdr, outMailHdr, ack) == -1) {
        printf("Machine %d: can not send \"%s\"\n", postOffice->GetNetworkName(), ack);
        return;
    }

    // Wait for the ack from the other machine to the first message we sent.
    reliableTransfer->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);
    interrupt->Halt();
}
#endif















