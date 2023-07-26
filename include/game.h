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
    int points = 0;
};

struct Game {
    time_t seed { 0 };
    float elapsed = 0.f;

    int screenWidth = 0;
    int screenHeight = 0;

    int fishNum = 10;
    Fish* fishes;
    Vector2* positions;

    int actorNum = 0;
    Actor actors[4] {};

    void start() {
        seed = time(NULL);
        srand(seed);

        // TODO: get fish locations from server, in order to synchronize locations across clients
        // Upon establishment connection, server sends back locations of all the fish in the game world
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