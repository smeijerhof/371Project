#pragma once

#include "raylib.h"
#include "def.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct Fish {
    Vector2 pos {};
    bool alive = false;
    bool taken = false;
    Color c = RED;

    void spawn(float x, float y) {
        alive = true;
        taken = false;
        pos = {x, y};

    }

    void draw() {
        if (!alive) return;
      
        Vector2 drawPos = pos;
        if (taken) {
            drawPos.x += (rand() % 10) - 5;
            drawPos.y += (rand() % 10) - 5;
            c = GREEN;
        }
        
        DrawRectangleV(drawPos, {FISH_WIDTH, FISH_HEIGHT}, c);

		Rectangle rec = { drawPos.x, drawPos.y, FISH_WIDTH, FISH_HEIGHT };
        DrawRectangleLinesEx(rec, 10, BLACK);
    }
};
