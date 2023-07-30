#include "../include/server.h"

int main(int argc, char const *argv[]) {
    time_t seed { 0 };
    seed = time(NULL);
    srand(seed);
    
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

		//printf("%d\n", server.input[0]);
        switch(server.input[0]) {
			
            case 0:
                printf("Handling Connect Message\n");

                if (server.actorNum >= 4) {
                    response[responseLength++] = htons(1000);
                }
                else {
                    response[responseLength++] = htons(server.actorNum++);
                    for(int i = 0; i < FISH_NUM; i++) {
                        if(server.fishes[i].alive) {
							printf("	Sending Fish Spawn at (%d, %d)\n", (int) server.fishes[i].pos.x, (int) server.fishes[i].pos.y);
							
                            response[responseLength++] = htons((uint16_t) server.fishes[i].pos.x);
							response[responseLength++] = htons((uint16_t) server.fishes[i].pos.y);
                        }
                        else {
                            response[responseLength++] = htons((uint16_t) 1000);
                            response[responseLength++] = htons((uint16_t) 1000);
                        }
                    }
                }
                break;
				
            case 1:
				printf("Handling mouse message\n");
				Vector2 mousePos = { (float) server.input[1], (float) server.input[2]};
				
				printf("	Mouse position = %.2f %.2f\n", mousePos.x, mousePos.y);
				
                // printf("server: ");
                // for(int j = 0; j < 3; j++) {
                //     response[responseLength++] = buffer[j];
                //     printf("%d ", ntohs(buffer[j]));
                // }
                // printf("\n");
                break;
        }
        // printf("writing %d\n-----------------------------------------------------\n", 2*responseLength);
        if(write(connectionSocket, response,2*responseLength) != responseLength) {
            printf("Message not sent in its entirety.\n");
        }  
        
    }

    // close connections
    close(connectionSocket);
    close(server.sock);
}

