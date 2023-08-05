#pragma once

#include "../include/raylib.h"
#include "../include/fish.h"
#include "../include/def.h"

#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#include <mutex>

enum Player { NONE = 0, ONE, TWO, THREE, FOUR };

struct Actor {
	int p = -1;
	bool catching = false;
	int target = 0;
	char points = 0;
	int life = 10;
	
	Vector2 pos;
	
	struct ActorMessage {
		unsigned char buffer[256];
	};
};

struct Game {
	std::mutex mtx;
	
	time_t seed { 0 };
	float elapsed = 0.f;
	float waitingTimer = 0.f;
	
	bool canPlay = false;
	
	bool waiting = false;
	
	Color colors[4] = {RED, BLUE, GREEN, YELLOW};
	
	Fish fishes[FISH_NUM];
	Vector2 fPositions[FISH_NUM];
	
	int actorNum = 0;
	Actor actors[4] {};
	int totalActors = 0;
	unsigned char clientID;
	
	// New method to update game state based on client's mouse positions
	void updateGameState(int clientID, int mouseX, int mouseY) {
		if (clientID < 0 || clientID >= 4)
			return;
		
		if (clientID == actorNum) {
			// Update mouse position for the player associated with this clientID
			mtx.lock();
			actors[clientID].p = clientID;
			actors[clientID].catching = false;
			
			// Check for fish catching condition and update actor state
			for (int i = 0; i < FISH_NUM; i++) {
				if (mouseX > fishes[i].pos.x && mouseX < fishes[i].pos.x + FISH_WIDTH &&
					mouseY > fishes[i].pos.y && mouseY < fishes[i].pos.y + FISH_HEIGHT &&
					fishes[i].alive) {
					// Fish caught, update actor state and fish color
					actors[clientID].catching = true;
					actors[clientID].target = i;
					fishes[i].c = GREEN;
				}
			}
			mtx.unlock();
		}
	}
	
	unsigned char connectClient() {
		mtx.lock();
		unsigned char clientID = totalActors;
		totalActors++;
		mtx.unlock();
		return clientID;
	}
	
	// New method to disconnect a client and update game state
	void disconnectClient(int clientID) {
		if (clientID < 0 || clientID >= 4)
			return;
		
		mtx.lock();
		actors[clientID].p = -1;
		actors[clientID].catching = false;
		mtx.unlock();
	}
	
	void draw(Texture2D* texs) {
		for (int i = 0; i < FISH_NUM; i++) {
			Color col = WHITE;
			if (fishes[i].taken) col = RED;
			fishes[i].c = RED;
			fishes[i].draw(texs[fishes[i].texNum]);
			fishes[i].c = WHITE;
		}
	}
};
