#include "../include/server.h"

// Function to handle client connections in separate threads
void* clientHandler(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[BUFFER_SIZE] = {0};

    // Create the game instance
    Game game;
    game.start();

    // Send the client's ID to the client
    unsigned char clientID = game.connectClient();
    unsigned char idBuffer[2] = { 'I', clientID };
    write(clientSocket, idBuffer, sizeof(idBuffer));

    // Calculate the size of the message buffer based on the number of fish positions
    size_t bufferSize = 1 + FISH_NUM * 4;
    unsigned char* msgBuffer = new unsigned char[bufferSize];
    msgBuffer[0] = 0; // Points

    while (true) {
        // Receive data from the client (mouse positions in serialized form)
        int bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            // Client disconnected or error occurred
            close(clientSocket);
            game.disconnectClient(clientID);
            break;
        }

        // Extract mouse positions (mouseX = buffer[2], mouseY = buffer[3])
        int mouseX = buffer[2];
        int mouseY = buffer[3];

        // Update the game state using the extracted mouse positions
        mtx.lock();
        game.updateGameState(clientID, mouseX, mouseY);
        mtx.unlock();

        // Fill the message buffer with the game fish positions
        mtx.lock();
        for (int i = 0; i < FISH_NUM; i++) {
            int x = (int)game.positions[i].x;
            int y = (int)game.positions[i].y;

            msgBuffer[(i * 4) + 1] = (x >> 8) & 0xFF;
            msgBuffer[(i * 4) + 2] = x & 0xFF;
            msgBuffer[(i * 4) + 3] = (y >> 8) & 0xFF;
            msgBuffer[(i * 4) + 4] = y & 0xFF;
        }
        mtx.unlock();

        // Send the message containing the game fish positions to the client
        write(clientSocket, msgBuffer, bufferSize);
    }

    // Clean up the dynamically allocated buffer
    delete[] msgBuffer;

    return NULL;
}

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

    std::vector<std::thread> clientThreads;

    // Accept and handle client connections in separate threads
    while ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen))) {
        if(clientSocket < 0)
            printError("Failed to accept\n");
        // Create a new thread to handle the client
        std::thread clientThread(clientHandler, (void*)&clientSocket);

        // Detach the thread to allow it to run independently
        clientThread.detach();
    }

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
                // Read message, update server values
				// printf("Handling mouse message\n");
                int playerNo = (int) server.input[1];
                Vector2 mousePos = { (float) server.input[2], (float) server.input[3]};
                server.playerCursors[playerNo] = mousePos;


                uint16_t response[BUFFER_SIZE];
                int responseLength = 0;
                for(int i = 0; i < server.actorNum; i++) {
                    response[responseLength++] = htons((uint16_t) server.playerCursors[i].x);
                    response[responseLength++] = htons((uint16_t) server.playerCursors[i].y);
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
        if(write(connectionSocket, response,2*responseLength) != responseLength) {
            printf("Message not sent in its entirety. %d\n", server.input[0]);
        }  
        
    }

    // close connections
    close(connectionSocket);
    close(server.sock);
}

