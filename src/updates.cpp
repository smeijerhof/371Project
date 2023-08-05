#include "../include/client_updates.h"

void* sendUpdateMessage(void* msg) {
	UpdateMessage* message = (UpdateMessage*) msg;
	message->clearResponse();
	
	message->setResponse(message->token);
	message->setResponse(message->clientNumber);
	message->setResponse(message->mouseX);
	message->setResponse(message->mouseY);
	message->setResponse(message->score);
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	message->clearResponse();
	int bytesRead = read(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	if (bytesRead <= 0) {
		// Error occured or conncetion closed
		printf("Error in reading server response\n");
		return nullptr;
	}
	
	// Handle other player data
	for (int i = 0; i < message->state->numberOfClients; i++) {
		message->state->clients[i].pos.x = (float) message->readResponse();
		message->state->clients[i].pos.y = (float) message->readResponse();
		message->state->clients[i].score = message->readResponse();
		message->state->clients[i].lure = message->readResponse();
	}
	
	// Handle dead fishes
	for (int i = 0; i < FISH_NUM; i++) {
		uint16_t alive = message->readResponse();
		if (alive == 0) message->state->fishes[i].alive = false;
	}
	
	return 0;
}

void* sendCatchMessage(void* msg) {
	printf("Attempting to lure fish\n");
	
	FishMessage* message = (FishMessage*) msg;
	message->clearResponse();
	
	message->setResponse(message->token);
	message->setResponse(message->clientNumber);
	message->setResponse(message->fishIndex);
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	message->clearResponse();
	int bytesRead = read(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	if (bytesRead <= 0) {
		// Error occured or conncetion closed
		printf("Error in reading server response\n");
		return nullptr;
	}
	
	uint16_t success = message->readResponse();
	
	printf("success = %d\n", success);
	
	if (success == MESSAGE_NEGATIVE) {
		printf("	-> Failure in luring fish!\n");
		//message->state->fishes[message->fishIndex].taken = message->readResponse();
		return 0;
	} else if (success == MESSAGE_POSITIVE) {
		printf("	-> Success in luring fish!\n");
		message->state->catching = true;
		message->state->fishes[message->fishIndex].taken = message->clientNumber;
		message->state->fishLife = 10;
		message->state->target = message->fishIndex;
	} else {
		printf("	-> corrupted response from the server");
	}
	
	return 0;
}

void* sendTerminateMessage(void* msg) {
	FishMessage* message = (FishMessage*) msg;
	message->clearResponse();
	
	message->setResponse(message->token);
	message->setResponse(message->clientNumber);
	message->setResponse(message->fishIndex);
	message->setResponse(message->success);
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	if (message->success) printf("	-> Killed fish %d\n", message->fishIndex);
	else printf("	-> Failed to kill fish %d\n", message->fishIndex);
	
	return 0;
}

void* sendOverMessage(void* msg) {
	ClientMessage* message = (ClientMessage*) msg;
	message->clearResponse();
	
	message->setResponse(message->token);
	
	write(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	
	printf("Ended game\n");
	
	message->clearResponse();
	int bytesRead = read(message->serverSocket, message->response, 2 * BUFFER_SIZE);
	if (bytesRead <= 0) {
		// Error occured or conncetion closed
		printf("Error in reading server response\n");
		return nullptr;
	}
	
	message->state->winner = message->readResponse();
	printf("Player %d won the game\n", message->state->winner);
	
	return 0;
}
