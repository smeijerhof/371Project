#pragma once

#include "stdint.h"

#define SERVER_PORT 65502
#define IP "localhost"
#define BUFFER_SIZE 64
#define QUEUE_SIZE 4
#define PLACEHOLDER 1000

#define FISH_NUM 1
#define PLAYER_NUM 4

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define CONNECTION_MESSAGE 0
#define WAIT_LOBBY_MESSAGE 1
#define START_LOBBY_MESSAGE 2
#define CLIENT_UPDATE_MESSAGE 3
#define REQUEST_CATCH_MESSAGE 4
#define CATCH_TERMINATION_MESSAGE 5
#define OVER_MESSAGE 6

#define FISH_NOT_TAKEN 5

const uint16_t MESSAGE_NEGATIVE = 0;
const uint16_t MESSAGE_POSITIVE = 1;

