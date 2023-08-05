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

struct ServerState {	
    struct sockaddr_in channel {};
    uint16_t input[BUFFER_SIZE];
    int sock;
	
	bool start;

	uint16_t numberOfClients;
	Vector2 playerCursors[PLAYER_NUM] = {};
	uint16_t playerScores[PLAYER_NUM] = {};
	uint16_t playerLures[PLAYER_NUM] = {};

    Fish fishes[FISH_NUM];
	
	uint16_t response[BUFFER_SIZE];
	uint16_t responseLength;
	
	int connections = -1;

	void printError(const char* message) {
		perror(message);
		exit(-1);
	}
	
    ServerState() {
		numberOfClients = 0;
		
        // specify socket to read from 
        memset(&channel, 0, sizeof(channel));
        channel.sin_family = AF_INET;
        channel.sin_addr.s_addr = htonl(INADDR_ANY);
        channel.sin_port = htons(SERVER_PORT);
    
        // return file descriptor for socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0)
            printError("Failed to create new socket\n");
    
        // bind socket to supplied address, port
        int b = bind(sock, (struct sockaddr *) &channel, sizeof(channel));
        if(b < 0) 
            printError("Bind failed\n");
		
		int l = listen(sock, QUEUE_SIZE);
		if(l < 0) 
			printError("Listen failed");
		
		start = false;
		
		spawnFishes();
		clearResponse();
		
		for (int i = 0; i < PLAYER_NUM; i++) playerLures[i] = -1;
    }

    void spawnFishes() {
		srand(time(NULL));
		
		for (int i = 0; i < FISH_NUM; i++) {
			float x = 100 + rand() % (SCREEN_WIDTH - 200);
			float y = 100 + rand() % (SCREEN_HEIGHT - 200);
			
			fishes[i].setPos(x, y);
			
			int t = rand() % 3;
			
			fishes[i].setTex(t);
		}
	}
	
	void clearResponse() {
		for (int i = 0; i < BUFFER_SIZE - 1; i++) response[i] = 0;
		responseLength = 0;
	}
	
	uint16_t setResponse(uint16_t val) {
		response[responseLength] = htons(val);
		responseLength++;
		return responseLength;
	}

};
