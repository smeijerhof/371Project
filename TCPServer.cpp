// TODO: See which is necessary?
// Include references (textbook, linuxhowtos)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include "include/raylib.h"
#include "include/game.h"

#define SERVER_PORT 65500
#define BUFFER_SIZE 32
#define QUEUE_SIZE 10
#define FISH_NUM 10
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

void printError(const char* message) {
    perror(message);
    exit(-1);
}

int main(int argc, char const *argv[]) {

    // Hold data read from socket
    // char buffer[BUFFER_SIZE];
    uint16_t buffer[BUFFER_SIZE];

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

    // Once connected to socket, load game
    Game game;
    game.start();

    // listen to socket for connections
    int l = listen(sock, QUEUE_SIZE);
    if(l < 0) 
        printError("Listen failed");

    // wait until connection established, then read message
    int connectionSocket = accept(sock, 0, 0);
    if(connectionSocket < 0)
        printError("Failed to accept\n");

    while(1) {
        // read data from connection, convert to appropriate endian
        read(connectionSocket, buffer, BUFFER_SIZE);
        for(int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = ntohs(buffer[i]);
        }
        uint16_t response[BUFFER_SIZE];
        int responseLength = 0;

        switch(buffer[0]) {
            case 0:
            // printf("connect message\n");

                if (game.actorNum >= 4) {
                    response[responseLength++] = htons(1000);
                }
                else {
                    response[responseLength++] = htons(game.actorNum++);
                    for(int i = 0; i < game.fishNum; i++) {
                        if(game.fishes[i].alive) {
                            printf("{%.1f,%.1f}\n", game.fishes[i].position.x, game.fishes[i].position.y);
                            // printf("host: %f %f\nuint: %d %d\nnetwork: %d %d\n\n",
                            // game.fishes[i].position.x, game.fishes[i].position.y,
                            // (uint16_t) game.fishes[i].position.x, (uint16_t) game.fishes[i].position.y,
                            // htons((uint16_t) game.fishes[i].position.x), htons((uint16_t) game.fishes[i].position.y));
      
                            response[responseLength++] = htons((uint16_t) game.fishes[i].position.x);
                            response[responseLength++] = htons((uint16_t) game.fishes[i].position.y);
                        }
                        else {
                            response[responseLength++] = htons((uint16_t) 1000);
                            response[responseLength++] = htons((uint16_t) 1000);
                        }
                    }
                }
                break;
            case 1:
                // printf("mouse message\n");
                // printf("server: ");
                // for(int j = 0; j < 3; j++) {
                //     response[responseLength++] = buffer[j];
                //     printf("%d ", ntohs(buffer[j]));
                // }
                // printf("\n");
                break;
        }
        // printf("writing %d\n-----------------------------------------------------\n", 2*responseLength);
        write(connectionSocket, response, 2*responseLength);  
        
    }

    // close connections
    close(connectionSocket);
    close(sock);
}

