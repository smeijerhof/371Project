#pragma once

#include "def.h"
#include "game.h"

struct ClientMessage {
	Game* state = nullptr;
	uint16_t response[BUFFER_SIZE];
	uint16_t responseLength = 0;
	
	int serverSocket = 0;
	uint16_t token = CONNECTION_MESSAGE;

	void clearResponse() {
		for (int i = 0; i < BUFFER_SIZE - 1; i++) response[i] = 0;
		responseLength = 0;
	}
	
	uint16_t setResponse(uint16_t val) {
		response[responseLength] = htons(val);
		responseLength++;
		return responseLength;
	}
	
	uint16_t readResponse() {
		uint16_t ret = ntohs(response[responseLength]);
		responseLength++;
		return ret;
	}
	
	void construct(Game* st, int sock, uint16_t tok) {
		state = st;
		serverSocket = sock;
		token = tok;
	}
};

struct UpdateMessage: ClientMessage {
	uint16_t clientNumber;
	uint16_t mouseX;
	uint16_t mouseY;
	uint16_t score;
	
	void construct(Game* st, int sock, uint16_t tok, uint16_t num, uint16_t x, uint16_t y, uint16_t scr) {
		state = st;
		serverSocket = sock;
		token = tok;
		
		clientNumber = num;
		mouseX = x;
		mouseY = y;
		score = scr;
	}
};

struct FishMessage: ClientMessage {
	uint16_t clientNumber;
	uint16_t fishIndex;
	uint16_t success;
	
	void construct(Game* st, int sock, uint16_t tok, uint16_t num, uint16_t fish, bool suc = false) {
		state = st;
		serverSocket = sock;
		token = tok;
		
		clientNumber = num;
		fishIndex = fish;
		
		success = 0;
		if (suc) success = 1;
	}
};
