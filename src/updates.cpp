// client_updates.h has comments explaining the uses of all the functions defined in this file
#include "../include/client_updates.h"

void* sendUpdateMessage(void* msg) {
	UpdateMessage* message = (UpdateMessage*) msg;
	message->clearResponse();

	// Update messages send the message token (present in all messages), as well as the client number, mouse x and y positions, and score 
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
	
	// We handle all other player's data to update visual information
	// ...clients[i].lure is a variable keeping track of which fish the player is currently luring. This isnt sent in the update message, but it is received
	for (int i = 0; i < message->state->numberOfClients; i++) {
		message->state->clients[i].pos.x = (float) message->readResponse();
		message->state->clients[i].pos.y = (float) message->readResponse();
		message->state->clients[i].score = message->readResponse();
		message->state->clients[i].lure = message->readResponse();
	}
	
	// The server also sends whether or not any fish has died from other users.
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

	// Update messages send the message token (present in all messages), as well as the client number, and the fish being lured 
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

	// The value of success is either positive or negative, and is the servers response as to whether or not luring the fish was successful
	uint16_t success = message->readResponse();
	
	printf("success = %d\n", success);
	
	if (success == MESSAGE_NEGATIVE) {
		printf("	-> Failure in luring fish!\n");
		//message->state->fishes[message->fishIndex].taken = message->readResponse();
		return 0;
	} else if (success == MESSAGE_POSITIVE) {
		printf("	-> Success in luring fish!\n");
		// Update the client to be catching the fish
		message->state->catching = true;
		message->state->fishes[message->fishIndex].taken = message->clientNumber;
		message->state->fishLife = 10;
		message->state->target = message->fishIndex;
	} else {
		// Should only ever happen upon a corrupted message
		printf("	-> corrupted response from the server");
	}
	
	return 0;
}

void* sendTerminateMessage(void* msg) {
	FishMessage* message = (FishMessage*) msg;
	message->clearResponse();

	// Update messages send the message token (present in all messages), as well as the client number, the fish being lured, and whether or not the user succeeded in catching it
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

	// Update messages send the message token. This message is simply to request the winner from the server
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

	// Update which player won the game
	message->state->winner = message->readResponse();
	printf("Player %d won the game\n", message->state->winner);
	
	return 0;
}
