#pragma once

// STD and assorted libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <vector>
//

// Network Libs
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include "stdint.h"
//

// Multithreading libs
#include <pthread.h>
#include <mutex>
#include <thread>
//

// Project Libs
#include "raylib.h"
#include "def.h"
#include "fish.h"
//

struct Actor {
	const float textureScale = 1.f;
	const Color playerColors[4] = {RED, BLUE, GREEN, YELLOW};
	
	uint16_t score;
	uint16_t clientNumber;
	uint16_t lure;

	Vector2 pos;
	
	Actor() {
		score = 0;
		lure = -1;
	}
	
	void init(uint16_t num) {
		score = 0;
		lure = -1;
		clientNumber = num;
	}
	
	void drawScore() {
		DrawText(TextFormat("Player %d Points = %d", clientNumber + 1, score), 20, 20 + clientNumber * 20, 20, playerColors[clientNumber]);
	}
	
	void drawMouse(Texture2D tex) {
		DrawTextureEx(tex, pos, 0.f, textureScale, playerColors[clientNumber]);
	}
};

struct Game {
	float elapsed;
	float lobbyTimer;
	float lobbyLength;
	
	bool lobby;
	bool host;
	
	uint16_t selfNumber;
	uint16_t numberOfClients;
	Actor clients[4];
	
	Fish fishes[FISH_NUM];
	Vector2 fPositions[FISH_NUM];
	
	int fishLife;
	uint16_t target;
	bool catching;
	int score;
	
	int winner;
	bool gameOver;
	
	float catchTimer;
	float catchLength;
	
	Texture2D cursorTexture;
	Texture2D fishTextures[3];
	
	Game() {
		elapsed = 0.f;
		lobbyTimer = 0.f;
		lobbyLength = 5.f;
		
		lobby = false;
		host = false;
		
		selfNumber = FISH_NOT_TAKEN;
		numberOfClients = 0;
		
		fishLife = 10;
		target = 0;
		catching = false;
		score = 0;
		
		winner = 0;
		gameOver = false;
		
		catchTimer = 0.f;
		catchLength = 3.3f;
	}
	
	void syncSelf(Vector2 p) {
		if (numberOfClients < selfNumber || selfNumber > 3) return;
		
		clients[selfNumber].pos = p;
		clients[selfNumber].score = score;
	}
	
	void drawFish() {
		for (int i = 0; i < FISH_NUM; i++) {
			fishes[i].draw(fishTextures[fishes[i].tex]);
		}
	}
};
