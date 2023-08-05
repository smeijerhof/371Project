#include "../include/server_connection.h"

void handleConnectionMessage (ServerState* server) {	
	if (server->numberOfClients >= 4) {
		server->setResponse(PLACEHOLDER);
		return;
	}
	
	printf("Connected player %d.\n", server->numberOfClients);
		
	server->setResponse(server->numberOfClients);
	server->numberOfClients++;
	for(int i = 0; i < FISH_NUM; i++) {
		uint16_t x = PLACEHOLDER;
		uint16_t y = PLACEHOLDER;
		uint16_t t = 0;
		
		if(server->fishes[i].alive) {
			x = server->fishes[i].pos.x;
			y = server->fishes[i].pos.y;
			t = server->fishes[i].tex;
		}
		
		server->setResponse(x);
		server->setResponse(y);
		server->setResponse(t);
	}
	
	if (server->numberOfClients == 1) printf("	-> Starting lobby.\n");
	else printf("	-> Joining lobby.\n");
}

void handleWaitLobbyMessage (ServerState* server) {
	if (!server->start) server->setResponse(MESSAGE_NEGATIVE);
	else {
		server->setResponse(MESSAGE_POSITIVE);
		server->setResponse(server->numberOfClients);
	}
}

void handleStartLobbyMessage (ServerState* server) {
	printf("Game started.\n");
	server->start = true;
	server->setResponse(server->numberOfClients);
}
