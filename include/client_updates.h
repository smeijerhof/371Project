#pragma once

#include "game.h"
#include "client_message.h"

void* sendUpdateMessage(void* msg);
void* sendCatchMessage(void* msg);
void* sendTerminateMessage(void* msg);
void* sendOverMessage(void* msg);
