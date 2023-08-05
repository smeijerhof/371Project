#pragma once

#include "game.h"
#include "client_message.h"

void* sendConnectionMessage(void* msg);
void* sendWaitLobbyMessage(void* msg);
void* sendStartLobbyMessage(void* msg);
