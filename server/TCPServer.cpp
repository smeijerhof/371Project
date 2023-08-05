#include "../include/server.h"

std::mutex mtxMouse;
std::mutex mtxCatch;
std::mutex mtxKill;
std::mutex mtxLobby;
std::mutex mtxWait;
ServerState* server;

int i = 0;

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
					if (server->actorNum == 1) {
						printf("First player has joined! Starting timer.\n");
						response[responseLength++] = htons((uint16_t) 100);
					} else {
						response[responseLength++] = htons((uint16_t) 0);
					}
				}
				break;
				
			case 1:
				{
					// mtxMouse.lock();
					// Read message, update server values
					// printf("Handling mouse message\n");
					int playerNo = (int) threadInputBuffer[1];
					Vector2 mousePos = { (float) threadInputBuffer[2], (float) threadInputBuffer[3]};
					server->playerCursors[playerNo] = mousePos;
					
					int score = (int) threadInputBuffer[4];
					server->playerScores[playerNo] = score;
					
					uint16_t response[BUFFER_SIZE];
					int responseLength = 0;
					for(int i = 0; i < server->actorNum; i++) {
						response[responseLength++] = htons((uint16_t) server->playerCursors[i].x);
						response[responseLength++] = htons((uint16_t) server->playerCursors[i].y);
						response[responseLength++] = htons((uint16_t) server->playerScores[i]);
					}
					// mtxMouse.unlock();
				}
				

				break;
			case 2: // catch message
				
			{				
				mtxCatch.lock();
				int cPlayerNo = (int) threadInputBuffer[1];
				int fishToCatch = (int) threadInputBuffer[2];
				
				if (fishToCatch < 0 || fishToCatch >= FISH_NUM) {
					printf("Error in player %d catching fish %d: corrupted index\n", cPlayerNo, fishToCatch);
					response[responseLength++] = htons((uint16_t) 0);
					mtxCatch.unlock();
					break;
				}
				
				printf("Player %d attemping to catch fish %d.\n", cPlayerNo, fishToCatch);
				if (!server->fishes[fishToCatch].alive) {
					printf("	Fish is dead.\n");
					response[responseLength++] = htons((uint16_t) 0);
					mtxCatch.unlock();
					break;
				}
				if (server->fishes[fishToCatch].taken) {
					printf("	Fish is already being caught.\n");
					response[responseLength++] = htons((uint16_t) 0);
					mtxCatch.unlock();
					break;
				}
				printf("	Succesful.\n");
				
				server->fishes[fishToCatch].taken = true;
				response[responseLength++] = htons((uint16_t) 100);
				
				mtxCatch.unlock();
				break;
			}
			
			case 3: // Kill message
			{
				mtxKill.lock();
				int cPlayerNo = (int) threadInputBuffer[1];
				int fishToCatch = (int) threadInputBuffer[2];
				
				if (fishToCatch < 0 || fishToCatch >= FISH_NUM) {
					printf("Error in player %d killing fish %d: corrupted index\n", cPlayerNo, fishToCatch);
					mtxKill.unlock();
					break;
				}
				
				printf("Player %d killed fish %d.\n", cPlayerNo, fishToCatch);
				
				server->fishes[fishToCatch].alive = false;
				mtxKill.unlock();
				break;
			}	
			break;
			
			case 4: // lobby message
			{
				mtxLobby.lock();
				int cPlayerNo = (int) threadInputBuffer[1];
				
				printf("Lobby started by player %d.\n", cPlayerNo);
				
				server->start = true;
				
				response[responseLength++] = htons((uint16_t) server->actorNum);
				mtxLobby.unlock();
				break;
			}	
			
			case 5: // wait message
			{
				mtxWait.lock();
				int cPlayerNo = (int) threadInputBuffer[1];
				
				//printf("Handling wait message from player %d.\n", cPlayerNo);
				
				if (!server->start) response[responseLength++] = htons((uint16_t) 0);
				else {
					response[responseLength++] = htons((uint16_t) 100);
					response[responseLength++] = htons((uint16_t) server->actorNum);
				}
				//writeResponse(connection, response, responseLength);
				mtxWait.unlock();
				break;
			}
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
	server->start = false;

	int l = listen(server->sock, QUEUE_SIZE);
	if(l < 0) 
		printError("Listen failed");
	
	std::vector<std::thread> clientThreads;
	
	// Accept and handle client connections in separate threads
	while (int clientSocket = accept(server->sock, 0, 0)) {
		if(clientSocket < 0)
			printError("Failed to accept");
		// Create a new thread to handle the client
		if (!server->start) {
			std::thread clientThread(clientHandler, (void*)&clientSocket);
			
			// Detach the thread to allow it to run independently
			clientThread.detach();
		}
		
	}
	
	return 0;
}

