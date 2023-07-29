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
#include "game.h"
#include "fish.h"
#include "def.h"
//

void printError(const char* message) {
    perror(message);
    exit(-1);
}

struct ServerState {
    struct sockaddr_in channel {};
    uint16_t input[BUFFER_SIZE];
    int sock;

    void init() {
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

    Vector2* spawnFish() {
      Vector2* positions = new Vector2[FISH_NUM];

      for (int i = 0; i < FISH_NUM; i++)
        positions[i] = {
          100 + rand() % (SCREEN_WIDTH - 200),
          100 + rand() % (SCREEN_HEIGHT - 200)
        };
    }
};
