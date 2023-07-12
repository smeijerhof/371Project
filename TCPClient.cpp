#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#define SERVER_PORT 65500
#define BUFFER_SIZE 256
#define IP "207.23.163.205"

void printError(const char* message) {
    perror(message);
    exit(-1);
}

int main(int argc, char const *argv[]) {
    // store incoming messages
    char buffer[BUFFER_SIZE];

    struct hostent* serverInfo;
    serverInfo = gethostbyname(IP);
    if(!serverInfo)
        printError("Could not retrieve host information\n");

    // Specify destination socket
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    memcpy(&serverAddress.sin_addr.s_addr, serverInfo->h_addr, serverInfo->h_length);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Create new TCP socket, return file descriptor
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        printError("socket failed\n");

    // Try to connect to server
    int c = connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if(c < 0)
        printError("Could not connect to server\n");

    // variables to track RTT
    float max, average = 0.0;
    float min = 120.0;

    // TODO: write input-determined # times, track max/min/average
    for(int i = 0; i < atoi(argv[1]);i++) {
        write(sock, "hello", 6);    // Send message

        // wait for response, determine time to receive
        time_t before = clock();
        read(sock, buffer, BUFFER_SIZE);
        time_t after = clock() - before;
        float duration = (float) after * 1000 / CLOCKS_PER_SEC;
        if(duration < min) min = duration;
        if(duration > max) max = duration;
        average += duration;
    }
    average /= atoi(argv[1]);
    printf("%s %f %f %f\n", buffer, max, min, average);

    close(sock);
}
