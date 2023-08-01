#include "../include/server.h"

// Global mutex to ensure thread-safe access to the game state
std::mutex mtx;

ServerState* server;

void* clientHandler(void* arg) {
    int connectionSocket = *((int*) arg);
    uint16_t threadInputBuffer[BUFFER_SIZE];

    while(1) {
        // read data from connection, convert to appropriate endian
        // TODO: limit loop to only convert ints read
        int bytesRead = read(connectionSocket, threadInputBuffer, 2*BUFFER_SIZE);
		
        if (bytesRead <= 0) {
            // Client disconnected or error occurred
            close(connectionSocket);
            // game.disconnectClient(clientID);
            break;
        }
		
        for(int i = 0; i < BUFFER_SIZE; i++) {
           threadInputBuffer[i] = ntohs(threadInputBuffer[i]);
        }
        uint16_t response[BUFFER_SIZE];
        int responseLength = 0;

		//printf("%d\n", server.input[0]);
        switch(threadInputBuffer[0]) {
			
            case 0:
                printf("Handling Connect Message\n");

                if (server->actorNum >= 4) {
                    response[responseLength++] = htons(PLACEHOLDER);
                }
                else {
                    response[responseLength++] = htons(server->actorNum++);
                    for(int i = 0; i < FISH_NUM; i++) {
                        if(server->fishes[i].alive) {
							printf("	Sending Fish Spawn at (%d, %d)\n", (int) server->fishes[i].pos.x, (int) server->fishes[i].pos.y);
							
                            response[responseLength++] = htons((uint16_t) server->fishes[i].pos.x);
							response[responseLength++] = htons((uint16_t) server->fishes[i].pos.y);
                        }
                        else {
                            response[responseLength++] = htons((uint16_t) PLACEHOLDER);
                            response[responseLength++] = htons((uint16_t) PLACEHOLDER);
                        }
                    }
                }
                break;
				
            case 1:
                // Read message, update server values
				// printf("Handling mouse message\n");
                int playerNo = (int) threadInputBuffer[1];
                Vector2 mousePos = { (float) threadInputBuffer[2], (float) threadInputBuffer[3]};
                server->playerCursors[playerNo] = mousePos;


                uint16_t response[BUFFER_SIZE];
                int responseLength = 0;
                for(int i = 0; i < server->actorNum; i++) {
                    response[responseLength++] = htons((uint16_t) server->playerCursors[i].x);
                    response[responseLength++] = htons((uint16_t) server->playerCursors[i].y);
                }

				
				// printf("	Mouse position = %.2f %.2f\n", mousePos.x, mousePos.y);
				
                // printf("server: ");
                // for(int j = 0; j < 3; j++) {
                //     response[responseLength++] = buffer[j];
                //     printf("%d ", ntohs(buffer[j]));
                // }
                // printf("\n");
                break;
        }
        // printf("writing %d\n-----------------------------------------------------\n", 2*responseLength);
        if(write(connectionSocket, response,2*responseLength) != 2*responseLength) {
            printf("Message not sent in its entirety. %d\n", threadInputBuffer[0]);
        }  
        
    }

    // close connections
    close(connectionSocket);
    close(server->sock);
    return 0;
}

int main(int argc, char const *argv[]) {
    time_t seed { 0 };
    seed = time(NULL);
    srand(seed);
    
    server = new ServerState();

    server->init();
    server->spawnFish();


    // Start listening for incoming connections
    // listen to socket for connections
    int l = listen(server->sock, QUEUE_SIZE);
    if(l < 0) 
        printError("Listen failed");

    // wait until connection established, then read message
    // int connectionSocket = accept(server->sock, 0, 0);
    // if(connectionSocket < 0)
    //     printError("Failed to accept\n");



    std::vector<std::thread> clientThreads;

    // Accept and handle client connections in separate threads
    while (int clientSocket = accept(server->sock, 0, 0)) {
        if(clientSocket < 0)
            printError("Failed to accept");
        // Create a new thread to handle the client
        std::thread clientThread(clientHandler, (void*)&clientSocket);

        // Detach the thread to allow it to run independently
        clientThread.detach();
    }

    return 0;
}

