#pragma once

// STD and assorted libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
//

// Network Libs
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
//

// Project Libs
#include "raylib.h"
#include "fish.h"
#include "game.h"
#include "def.h"
//

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
#include <vector>
#include <thread>
#include <math.h>


void printError(const char* message) {
    perror(message);
    exit(-1);
}

struct ServerState {
    struct sockaddr_in channel {};
    uint16_t input[BUFFER_SIZE];
    int sock;
	
	bool start = false;

    Vector2 playerCursors[4] = {};
	int playerScores[4] = {};

    int actorNum;

    Fish fishes[FISH_NUM];

    void init() {
        actorNum = 0;
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
    }

    void spawnFish() {
      for (int i = 0; i < FISH_NUM; i++) {
          Fish f;
          f.spawn(
            (float) 100 + rand() % (SCREEN_WIDTH - 200),
            (float) 100 + rand() % (SCREEN_HEIGHT - 200) 
          );

          fishes[i] = f;
      }
    }
};
