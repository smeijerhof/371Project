#include "../include/server.h"
#include "../include/server_connection.h"
#include "../include/server_updates.h"

std::mutex mtxWaitLobby;
std::mutex mtxStartLobby;
std::mutex mtxUpdate;
std::mutex mtxCatch;
std::mutex mtxTerminate;
std::mutex mtxGameOver;

void* clientHandler(void* serverArg, void* socketArg) {
	ServerState* server = (ServerState*) serverArg;
	int connectionSocket = *((int*) socketArg);
	
	uint16_t threadInputBuffer[BUFFER_SIZE];
	
	while(1) {
		// read data from connection, convert to appropriate endian
		int bytesRead = read(connectionSocket, threadInputBuffer, 2*BUFFER_SIZE);
		if (bytesRead <= 0) {
			// Client disconnected or error occurred
			close(connectionSocket);
			break;
		}
		
		for(int i = 0; i < BUFFER_SIZE; i++) threadInputBuffer[i] = ntohs(threadInputBuffer[i]);
		
		bool toWrite = false;
		
		uint16_t messageCode = threadInputBuffer[0];
		uint16_t playerNumber = threadInputBuffer[1];
		
		int score, requestFish;
		
		server->clearResponse();
		
		switch(messageCode) {
			case CONNECTION_MESSAGE:
				toWrite = true;
				handleConnectionMessage(server);
				break;
			
			case WAIT_LOBBY_MESSAGE: // wait message
				toWrite = true;
				
				mtxWaitLobby.lock();
				handleWaitLobbyMessage(server);
				mtxWaitLobby.unlock();
				
				break;
			
			case START_LOBBY_MESSAGE: // lobby message
				toWrite = true;
				
				mtxStartLobby.lock();
				handleStartLobbyMessage(server);
				mtxStartLobby.unlock();
				break;
				
			case CLIENT_UPDATE_MESSAGE:
				toWrite = true;
				mtxUpdate.lock();
				handleUpdateMessage(server, playerNumber, (float) threadInputBuffer[2], (float) threadInputBuffer[3], threadInputBuffer[4]);
				mtxUpdate.unlock();
				break;
			
			case REQUEST_CATCH_MESSAGE: // catch message
				toWrite = true;
				
				mtxCatch.lock();
				handleCatchMessage(server, playerNumber, threadInputBuffer[2]);
				mtxCatch.unlock();
				break;
			
			case CATCH_TERMINATION_MESSAGE: // Kill message
				mtxTerminate.lock();
				handleTerminateMessage(server, playerNumber, threadInputBuffer[2], threadInputBuffer[3]);
				mtxTerminate.unlock();
				break;
				
			case OVER_MESSAGE: // game over message
				toWrite = true;
				
				mtxGameOver.lock();
				handleOverMessage(server);
				mtxGameOver.unlock();
				break;
		}
		
		if (toWrite) write(connectionSocket, server->response, 2*server->responseLength);
	}
	
	// close connections
	close(connectionSocket);
	close(server->sock);
	return 0;
}

int main(int argc, char const *argv[]) {
	ServerState server;
	//server = new ServerState();

	std::vector<std::thread> clientThreads;
	
	// Accept and handle client connections in separate threads
	while (int clientSocket = accept(server.sock, 0, 0)) {
		if(clientSocket < 0) {
			server.printError("Failed to accept");
			break;
		}
		
		// Create a new thread to handle the client, and detach the thread to allow it to run independently
		if (!server.start) {
			std::thread clientThread(clientHandler, (void*) &server, (void*) &clientSocket);
			clientThread.detach();
		}
		
	}
	
	return 0;
}

