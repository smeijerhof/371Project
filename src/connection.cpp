#include "../include/client_connection.h"

void* sendConnectionMessage(void* msg) {
	ClientMessage* message = (ClientMessage*) msg;
	
	message->clearResponse();
	message->setResponse(message->token);
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	message->clearResponse();
	int bytesRead = read(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	if (bytesRead <= 0) {
		// Error occured or conncetion closed
		printf("Error in reading server response\n");
		return nullptr;
	}
	
	uint16_t clientNumber = message->readResponse();
	
	if(clientNumber == PLACEHOLDER) {
		printf("Could not join the lobby: there are already 4 players in the game\n");
		close(message->serverSocket);
		CloseWindow();
		return 0;
	}

	message->state->clients[clientNumber].init(clientNumber);
	
	message->state->selfNumber = clientNumber;
	
	printf("Connected to the server\n");
	if (clientNumber == 0) {
		message->state->host = true;
		printf("Created the lobby\n");
	} else printf("Joined the lobby\n");
	
	message->state->lobby = true;
		
	for(int i = 0; i < FISH_NUM; i++) {
		uint16_t fishX = message->readResponse();
		uint16_t fishY = message->readResponse();
		uint16_t fishTex = message->readResponse();
		
		message->state->fishes[i].setPos((float) fishX, (float) fishY);
		message->state->fishes[i].tex = fishTex;
		
		if(fishX == PLACEHOLDER || fishY == PLACEHOLDER || fishTex == PLACEHOLDER) {
			message->state->fishes[i].alive = false;
		}
	}
	
	
	return 0;
}

void* sendWaitLobbyMessage(void* msg) {
	ClientMessage* message = (ClientMessage*) msg;
	
	message->clearResponse();
	message->setResponse(message->token);
	message->setResponse(message->state->selfNumber);
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	message->clearResponse();
	int bytesRead = read(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	if (bytesRead <= 0) {
		// Error occured or conncetion closed
		printf("Error in reading server response\n");
		return nullptr;
	}
	
	if (message->readResponse() == MESSAGE_POSITIVE) {
		message->state->lobby = false;
		message->state->numberOfClients = message->readResponse();
		
		for (int i = 0; i < message->state->numberOfClients; i++) {
			if (i == message->state->selfNumber) continue;
			message->state->clients[i].init(i);
		}
		
		printf("Joined the game with %d players in the lobby\n", message->state->numberOfClients);
	}
	
	return 0;
}

void* sendStartLobbyMessage(void* msg) {
	ClientMessage* message = (ClientMessage*) msg;
	
	message->clearResponse();
	message->setResponse(message->token);
	message->setResponse(message->state->selfNumber);;
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	message->clearResponse();
	int bytesRead = read(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	if (bytesRead <= 0) {
		// Error occured or conncetion closed
		printf("Error in reading server response\n");
		return nullptr;
	}
	
	message->state->numberOfClients = message->readResponse();
	
	for (int i = 0; i < message->state->numberOfClients; i++) {
		if (i == message->state->selfNumber) continue;
		message->state->clients[i].init(i);
	}
	
	printf("Started the game with %d players in the lobby\n", message->state->numberOfClients);
	
	return 0;
}
