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

struct ServerState {
    Vector2* spawnFish() {
      Vector2* positions = new Vector2[FISH_NUM];

      for (int i = 0; i < FISH_NUM; i++)
        positions[i] = {
          100 + rand() % (SCREEN_WIDTH - 200),
          100 + rand() % (SCREEN_HEIGHT - 200)
        };
    }
};
