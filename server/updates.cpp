#include "../include/server_updates.h"

void handleUpdateMessage(ServerState* server, uint16_t playerNumber, float x, float y, uint16_t score) {
	// Update server info with incoming values
	server->playerCursors[playerNumber] = (Vector2) {x, y};
	server->playerScores[playerNumber] = score;
	
	// Add each player's cursor location, score, fish being caught to response
	for(int i = 0; i < server->numberOfClients; i++) {
		server->setResponse(server->playerCursors[i].x);
		server->setResponse(server->playerCursors[i].y);
		server->setResponse(server->playerScores[i]);
		server->setResponse(server->playerLures[i]);
	}
	
	// Add fish status (caught or not) to response
	for (int i = 0; i < FISH_NUM; i++) {
		uint16_t alive = 0;
		if (server->fishes[i].alive) alive = 1;
		server->setResponse(alive);
	}
}

void handleCatchMessage(ServerState* server, uint16_t playerNumber, uint16_t requestFish) {
	printf("Player attemping to catch fish %d.\n", requestFish);
	
	// Deny request if fish has been caught, allocated to different player, or index invalid
	if (!server->fishes[requestFish].alive || server->fishes[requestFish].taken != 5 || requestFish < 0 || requestFish >= FISH_NUM) {
		printf("	-> Refusing catch request.\n");
		printf("	-> alive = %d.\n", server->fishes[requestFish].alive);
		printf("	-> taken = %d.\n", server->fishes[requestFish].taken);
		server->setResponse(MESSAGE_NEGATIVE);
		server->setResponse(5);
		return;
	}
	printf("	-> Accepting catch request.\n");
	
	// Accept request: update server
	server->setResponse(MESSAGE_POSITIVE);
	
	server->fishes[requestFish].taken = playerNumber;
	server->playerLures[playerNumber] = requestFish;
	
	for (int i = 0; i < FISH_NUM; i++) {
		if (i == requestFish) continue;
		if (server->fishes[i].taken == playerNumber) server->fishes[i].taken = FISH_NOT_TAKEN;
	}
	
	server->setResponse(playerNumber);
}

void handleTerminateMessage(ServerState* server, uint16_t playerNumber, uint16_t requestFish, uint16_t success) {
	if (requestFish < 0 || requestFish >= FISH_NUM) {
		printf("Error in killing fish %d: corrupted index\n", requestFish);
		return;
	}
	
	if (success == 1) {
		printf("Killed fish %d.\n", requestFish);
		server->fishes[requestFish].alive = false;
		server->playerLures[playerNumber] = -1;
	} else if (success == 0) {
		printf("Failed to kill fish %d.\n", requestFish);
		server->fishes[requestFish].taken = FISH_NOT_TAKEN;
		server->playerLures[playerNumber] = -1;
	}
	
}

void handleOverMessage(ServerState* server) {
	uint16_t winner = 0;
	uint16_t maxScore = server->playerScores[0];
	
	for (int i = 1; i < server->numberOfClients; i++) {
		if (server->playerScores[i] > maxScore) {
			maxScore = server->playerScores[i];
			winner = i;
		}
	}
	
	server->setResponse(winner);
}
