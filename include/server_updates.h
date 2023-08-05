#pragma once

#include "server.h"

void handleUpdateMessage(ServerState* server, uint16_t playerNumber, float x, float y, uint16_t score);
void handleCatchMessage(ServerState* server, uint16_t playerNumber, uint16_t requestFish);
void handleTerminateMessage(ServerState* server, uint16_t playerNumber, uint16_t requestFish, uint16_t success);
void handleOverMessage(ServerState* server);
