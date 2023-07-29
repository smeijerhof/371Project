#include "../include/server.h"

int main(int argc, char const *argv[]) {
    ServerState server;

    server.init();
    server.spawnFish();

    // listen to socket for connections
    int l = listen(server.sock, QUEUE_SIZE);
    if(l < 0) 
        printError("Listen failed");

    // wait until connection established, then read message
    int connectionSocket = accept(server.sock, 0, 0);
    if(connectionSocket < 0)
        printError("Failed to accept\n");

    while(1) {
        // read data from connection, convert to appropriate endian
        // TODO: limit loop to only convert ints read
        read(connectionSocket, server.input, 2*BUFFER_SIZE);

        for(int i = 0; i < BUFFER_SIZE; i++) {
            server.input[i] = ntohs(server.input[i]);
        }
        uint16_t response[BUFFER_SIZE];
        int responseLength = 0;


        switch(input[0]) {
            case 0:
                printf("Sending Connect Message\n");

                if (game.actorNum >= 4) {
                    response[responseLength++] = htons(1000);
                }
                else {
                    response[responseLength++] = htons(server.actorNum++);
                    for(int i = 0; i < FISH_NUM; i++) {
                        if(server.fishes[i].alive) {
                            // printf("{%.1f,%.1f}\n", game.fishes[i].position.x, game.fishes[i].position.y);
                            // printf("host: %f %f\nuint: %d %d\nnetwork: %d %d\n\n",
                            // game.fishes[i].position.x, game.fishes[i].position.y,
                            // (uint16_t) game.fishes[i].position.x, (uint16_t) game.fishes[i].position.y,
                            // htons((uint16_t) game.fishes[i].position.x), htons((uint16_t) game.fishes[i].position.y));
      
                            response[responseLength++] = htons((uint16_t) server.fishes[i].position.x);
                            response[responseLength++] = htons((uint16_t) server.fishes[i].position.y);
                        }
                        else {
                            response[responseLength++] = htons((uint16_t) 1000);
                            response[responseLength++] = htons((uint16_t) 1000);
                        }
                    }
                }
                // for(int i = 0; i < BUFFER_SIZE; i++) {
                //     printf("%d, ",response[i]);
                // }
                // printf("\n");
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
        write(connectionSocket, response, 2*BUFFER_SIZE);//2*responseLength);  
        
    }

    // close connections
    close(connectionSocket);
    close(server.sock);
    
    delete server.positions;
}

