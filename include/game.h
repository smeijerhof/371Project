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

enum Player { NONE = 0, ONE, TWO, THREE, FOUR };

struct Actor {
    int p = -1;
    bool catching = false;
    int target = 0;
    int points = 0;
	int life = 0;
	
	Vector2 pos;

    struct ActorMessage {
        unsigned char buffer[256];
    };
};

struct Game {
    time_t seed { 0 };
    float elapsed = 0.f;

    Fish fishes[FISH_NUM];
    Vector2 fPositions[FISH_NUM];
	
	Color colors[4] = {RED, BLUE, GREEN, YELLOW};

    int actorNum = 0;
    Actor actors[4] {};

    void draw(Texture2D* texs) {
		for (int i = 0; i < FISH_NUM; i++) {
			Color col = WHITE;
			if (fishes[i].taken) col = RED;
			fishes[i].draw(texs[fishes[i].texNum], col);
		}
    }
};
