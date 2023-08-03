#include "../include/server.h"

void writeResponse(int connectionSocket, uint16_t* response, int responseLength) {
	if(write(connectionSocket, response,2*responseLength) != responseLength) {
		//printf("Message not sent in its entirety. %d\n", server.input[0]);
	}
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
			
            case 0: // connect message
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
                    if (server.actorNum == 1) {
						printf("First player has joined! Starting timer.\n");
						response[responseLength++] = htons((uint16_t) 100);
					} else {
						response[responseLength++] = htons((uint16_t) 0);
					}
                }
                writeResponse(connectionSocket, response, responseLength);
                break;
				
            case 1: // cursor update message
				{
					// Read message, update server values
					// printf("Handling mouse message\n");
					int playerNo = (int) server.input[1];
					Vector2 mousePos = { (float) server.input[2], (float) server.input[3]};
					server.playerCursors[playerNo] = mousePos;
					int score = (int) server.input[4];
					server.playerScores[playerNo] = score;
					
					for(int i = 0; i < server.actorNum; i++) {
						response[responseLength++] = htons((uint16_t) server.playerCursors[i].x);
						response[responseLength++] = htons((uint16_t) server.playerCursors[i].y);
						response[responseLength++] = htons((uint16_t) server.playerScores[i]);
					}
					writeResponse(connectionSocket, response, responseLength);
					break;
				}
				
				
			case 2: // catch message

				{

					int cPlayerNo = (int) server.input[1];
					int fishToCatch = (int) server.input[2];
					
					if (fishToCatch < 0 || fishToCatch >= FISH_NUM) {
						printf("Error in player %d catching fish %d: corrupted index\n", cPlayerNo, fishToCatch);
						response[responseLength++] = htons((uint16_t) 0);
						break;
					}
					
					printf("Player %d attemping to catch fish %d.\n", cPlayerNo, fishToCatch);
					if (!server.fishes[fishToCatch].alive) {
						printf("	Fish is dead.\n");
						response[responseLength++] = htons((uint16_t) 0);
						break;
					}
					if (server.fishes[fishToCatch].taken) {
						printf("	Fish is already being caught.\n");
						response[responseLength++] = htons((uint16_t) 0);
						break;
					}
					printf("	Succesful.\n");
					
					server.fishes[fishToCatch].taken = true;
					response[responseLength++] = htons((uint16_t) 100);
					
					writeResponse(connectionSocket, response, responseLength);
					break;
				}
				
			case 3: // Kill message
				{
					int cPlayerNo = (int) server.input[1];
					int fishToCatch = (int) server.input[2];
					
					if (fishToCatch < 0 || fishToCatch >= FISH_NUM) {
						printf("Error in player %d killing fish %d: corrupted index\n", cPlayerNo, fishToCatch);
						break;
					}
					
					printf("Player %d killed fish %d.\n", cPlayerNo, fishToCatch);
					
					server.fishes[fishToCatch].alive = false;
					
					break;
				}	
				break;
				
			case 4: // lobby message
			{
				int cPlayerNo = (int) server.input[1];
				
				printf("Lobby started by player %d.\n", cPlayerNo);
				
				server.start = true;
				
				break;
			}	
			
			case 5: // wait message
			{
				int cPlayerNo = (int) server.input[1];
								
				if (server.start) response[responseLength++] = htons((uint16_t) 0);
				else response[responseLength++] = htons((uint16_t) 100);
				
				writeResponse(connectionSocket, response, responseLength);
				
				break;
			}	
			break;
        }
        // printf("writing %d\n-----------------------------------------------------\n", 2*responseLength);
        //if(write(connectionSocket, response,2*responseLength) != responseLength) {
            //printf("Message not sent in its entirety. %d\n", server.input[0]);
        //}  
        
    }

    // close connections
    close(connectionSocket);
    close(server.sock);
}

