#include <time.h>
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
#include "raylib.h"

enum Player { NONE = 0, ONE, TWO, THREE, FOUR };

#define FISH_NUM 10

struct Fish {
    Vector2 position { 0 };
    static Vector2 size;

    Color color = RED;

    bool alive = false;

    Player owner = NONE;
    int health = 10;

    bool caught = false;

    void spawn(Vector2 pos) {
        alive = true;
        position = pos;
    }

    void draw() {
        if (!alive) return;
        Vector2 drawPos = position;

        if (caught) {
            drawPos.x += (rand() % 10) - 5;
            drawPos.y += (rand() % 10) - 5;
        }

        DrawRectangleV(drawPos, size, color);

        Rectangle rec = { drawPos.x, drawPos.y, size.x, size.y };
        DrawRectangleLinesEx(rec, 10, BLACK);
    }
};

Vector2 Fish::size = { 70, 35 };

struct Actor {
    int p = -1;
    bool catching = false;
    int target = 0;
    char points = 0;

    struct ActorMessage {
        unsigned char buffer[256];
    };

    ActorMessage createMessage(Vector2 position) {
        ActorMessage msg;
        msg.buffer[0] = points;

        unsigned int x = (unsigned int)position.x;
        unsigned int y = (unsigned int)position.y;

        msg.buffer[1] = (x >> 24) & 0xFF;
        msg.buffer[2] = (x >> 16) & 0xFF;
        msg.buffer[3] = (x >> 8) & 0xFF;
        msg.buffer[4] = x & 0xFF;

        msg.buffer[5] = (y >> 24) & 0xFF;
        msg.buffer[6] = (y >> 16) & 0xFF;
        msg.buffer[7] = (y >> 8) & 0xFF;
        msg.buffer[8] = y & 0xFF;

        return msg;
    }
};

struct Game {
    std::mutex mtx;

    time_t seed { 0 };
    float elapsed = 0.f;

    int screenWidth = 800;
    int screenHeight = 450;

    int fishNum = 10;
    int numFishAlive = fishNum;
    Fish* fishes;
    Vector2* positions;

    int actorNum = 0;
    Actor actors[4] {};
    int totalActors = 0;
    int clientID;

    // New method to update game state based on client's mouse positions
    void updateGameState(int clientID, int mouseX, int mouseY) {
        if (clientID < 0 || clientID >= 4)
            return;

        if (clientID == actorNum) {
            // Update mouse position for the player associated with this clientID
            mtx.lock();
            actors[clientID].p = clientID;
            actors[clientID].catching = false;

            // Check for fish catching condition and update actor state
            for (int i = 0; i < FISH_NUM; i++) {
                if (mouseX > fishes[i].position.x && mouseX < fishes[i].position.x + Fish::size.x &&
                    mouseY > fishes[i].position.y && mouseY < fishes[i].position.y + Fish::size.y &&
                    fishes[i].alive) {
                    // Fish caught, update actor state and fish color
                    actors[clientID].catching = true;
                    actors[clientID].target = i;
                    fishes[i].color = GREEN;
                }
            }
            mtx.unlock();
        }
    }

    unsigned char connectClient() {
        mtx.lock();
        unsigned char clientID = totalActors;
        totalActors++;
        mtx.unlock();
        return clientID;
    }

    // New method to disconnect a client and update game state
    void disconnectClient(int clientID) {
        if (clientID < 0 || clientID >= 4)
            return;

        mtx.lock();
        actors[clientID].p = -1;
        actors[clientID].catching = false;
        mtx.unlock();
    }

    void start() {
        seed = time(NULL);
        srand(seed);

        fishes = new Fish[fishNum];
        positions = new Vector2[fishNum];
        for (int i = 0; i < fishNum; i++) {
            Vector2 pos = { 100 + rand() % (screenWidth - 200), 100 + rand() % (screenHeight - 200) };
            fishes[i].spawn(pos);
            positions[i] = pos;

            // Stop fishes from spawning on each other
            for (int j = 0; j < i; j++) {
                Vector2 oldPosition = fishes[j].position; 
                Vector2 newPosition = fishes[i].position;

                float xDist = abs(oldPosition.x - newPosition.x);
                float yDist = abs(oldPosition.y - newPosition.y);

                int xDir = 1;
                int yDir = 1;

                if (oldPosition.x > newPosition.x) xDir = -1;
                if (oldPosition.y > newPosition.y) yDir = -1;

                if (xDist + yDist < Fish::size.x + Fish::size.y) {
                    newPosition.x += Fish::size.x / 2.f * xDir;
                    newPosition.y += Fish::size.y / 2.f * xDir;
                    positions[i] = newPosition;
                    fishes[i].position = newPosition;

                    oldPosition.x += Fish::size.x / 2.f * yDir * -1;
                    oldPosition.y += Fish::size.y / 2.f * yDir * -1;
                    positions[j] = oldPosition;
                    fishes[j].position = oldPosition;
                }
            }
        }
    }

    void restart() {
        delete[] fishes;
        delete[] positions;

        start();
    }

    void draw() {
        for (int i = 0; i < FISH_NUM; i++) {
            fishes[i].draw();
        }
    }

    ~Game() {
        delete[] fishes;
        delete[] positions;
    }
};
