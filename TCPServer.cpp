// TODO: See which is necessary?
// Include references (textbook, linuxhowtos)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define SERVER_PORT 65501
#define BUFFER_SIZE 10
#define QUEUE_SIZE 10

void printError(const char* message) {
    perror(message);
    exit(-1);
}

int main(int argc, char const *argv[]) {
    // Hold data read from socket
    // char buffer[BUFFER_SIZE];
    uint16_t buffer[BUFFER_SIZE];
    char response[] = "back at you";    // response message

    // specify socket to read from 
    struct sockaddr_in channel;
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    // return file descriptor for socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        printError("Failed to create new socket\n");

    // bind socket to supplied address, port
    int b = bind(sock, (struct sockaddr *) &channel, sizeof(channel));
    if(b < 0) 
        printError("Bind failed\n");

    // listen to socket for connections
    int l = listen(sock, QUEUE_SIZE);
    if(l < 0) 
        printError("Listen failed");

    // wait until connection established, then read message
    int connectionSocket = accept(sock, 0, 0);
    if(connectionSocket < 0)
        printError("Failed to accept\n");

    printf("connection established\n");

    // read data from new connection
    for(int i = 0; i < atoi(argv[1]); i++) {
        read(connectionSocket, buffer, BUFFER_SIZE);
        printf("server: ");
        // TODO: not being received correctly
        for(int j = 0; j < 3; j++)
            printf("%d ", ntohs(buffer[j]));
        printf("\n");
        // printf("%s\n", buffer);
        write(connectionSocket, buffer, BUFFER_SIZE);  

        // if(strcmp(buffer, "hello") == 0) {
        //     // send response to client
        //     write(connectionSocket, response, sizeof(response));  
        // }
    }

    // close connections
    close(connectionSocket);
    close(sock);
}

