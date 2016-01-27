#include "syscall.h"

int main () {
    PutString("starting...\n");
    int addr = 228;
    int port;
    int receivingPort = 7;
    if (CreateConnection(addr, 0, receivingPort) != -1) {
        PutString("CreateConnection is done\n");
        int temp;
        ReceiveData(receivingPort, (char*) &port, &temp);
    } else {
        PutString("CreateConnection is not done\n");
    }

    char filepath[12] = "thefile";
    if (SendData(addr, port, filepath, 12) != -1) {
        PutString("File sent successfully\n");
    } else {
        PutString("File sent unsuccessfully\n");
    }
    
    char data[MAX_FILE_SIZE];
    int size;
    ReceiveData(receivingPort, data, &size);
    
    PutString("Received the file\n");

    char newfile[12] = "newfile";
    if (UserCreateFile(newfile) != -1) {
        PutString("File is created successfully\n");
    } else {
        PutString("File was not created successfully\n");
    }
    int fd = UserOpenFile(newfile);
    UserWriteFile(fd, data, size + 1);
    PutString("I am done with writing\n");
    UserCloseFile(fd);
    return 0;
}