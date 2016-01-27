#include "syscall.h"

int main () {
    int addr = 228;
    int port;
    int receivingPort = 7;
    if (CreateConnection(addr, 0, receivingPort) == -1) {
        int temp;
        ReceiveData(receivingPort, (char*) &port, &temp);
        PutString("CreateConnection failed\n");
    }
    char filepath[12] = "thefile.txt";
    if (SendData(addr, port, filepath, 12) != -1) {
        PutString("File sent successfully\n");
    } else {
        PutString("File sent unsuccessfully\n");
    }
    
    char data[MAX_FILE_SIZE];
    int size;
    ReceiveData(receivingPort, data, &size);
    

    int fd = UserOpenFile(filepath);
    UserWriteFile(fd, data, size);
    UserCloseFile(fd);
    return 0;
}