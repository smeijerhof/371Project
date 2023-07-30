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
	
    while (1) {
	// Read data from the client
	bytesRead = read(clientSocket, clientInput, 2 * BUFFER_SIZE);
	
	// Check if there was an error or if the client disconnected
	if (bytesRead <= 0) {
	    // Handle disconnection
	    printf("Client disconnected or an error occurred.\n");
	    break; // Exit the while loop to close the connection and clean up resources.
	}
	
	// Calculate the number of uint16_t integers read from the client
	int numIntegersRead = bytesRead / 2; // Each uint16_t occupies 2 bytes
	
	for (int i = 0; i < numIntegersRead; i++) {
	    clientInput[i] = ntohs(clientInput[i]);
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
				
			// Token 1 coming in as 768 for some reason
            case 768:
				printf("Handling mouse message\n");
				Vector2 mousePos = { htons(server.input[4]), htons(server.input[5])};
				
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
        write(connectionSocket, response, 2*BUFFER_SIZE);//2*responseLength);  
        
    }

    // close connections
    close(connectionSocket);
    close(server.sock);
}

