#include "syscall.h"

int main () {
    PutString("starting...\n");
    int addr = 228;
    int port;
    int receivingPort = 7;
    if (CreateConnection(addr, 0, receivingPort) != -1) {
        PutString("Connection is established\n");
        int temp;
        ReceiveData(receivingPort, (char*) &port, &temp);
    } else {
        PutString("Can not connect to the server\n");
    }

    char filepath[14] = "bigfile";
    if (SendData(addr, port, filepath, 14) != -1) {
        PutString("File sent successfully\n");
    } else {
        PutString("File sent unsuccessfully\n");
    }
    
    char data[MAX_FILE_SIZE];

    int size;
    PutString("Trying to receive the file\n");
    ReceiveData(receivingPort, data, &size);
    
    PutString("Received the file\n");

    char newfile[12] = "newfile";
    if (UserCreateFile(newfile) != -1) {
        PutString("File is created successfully\n");
    } else {
        PutString("File already exists\n");
    }
    int fd = UserOpenFile(newfile);
    UserWriteFile(fd, data, size);
    UserCloseFile(fd);
    PutString("File was written to the disk\n");
    return 0;
}