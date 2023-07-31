#pragma once

// STD and assorted libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <vector>
//

// Network Libs
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
//

// Mutex libs
#include <mutex>
#include <thread>
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
    int sock;

    Vector2 playerCursors[4] = {};

    int actorNum;

    Fish fishes[FISH_NUM];

    void init() {
        int opt = 1;
        actorNum = 0;
        // specify socket to read from 
        memset(&channel, 0, sizeof(channel));
        channel.sin_family = AF_INET;
        channel.sin_addr.s_addr = htonl(INADDR_ANY);
        channel.sin_port = htons(SERVER_PORT);
    
        // return file descriptor for socket
        // Create the server socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0)
            printError("Failed to create new socket");

        // Set socket options to allow multiple connections
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            printError("setsockopt failed.");
        }
    
        // bind socket to supplied address, port
        // Bind the socket to the specified port

        int b = bind(sock, (struct sockaddr *) &channel, sizeof(channel));
        if(b < 0) 
            printError("Bind failed");  
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
