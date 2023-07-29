#pragma once

#include "raylib.h"

struct Fish {
    Vector2 pos {};
    bool alive = false;
    bool taken = false;

    void spawn(Vector2 p) {
        alive = true;
        taken = false;
        pos = p;
    }

    void draw() {
        if (!alive) return;
      
        Vector2 drawPos = pos;
        Color c = RED;
        if (taken) {
            drawPos.x += (rand() % 10) - 5;
            drawPos.y += (rand() % 10) - 5;
            c = GREEN;
        }
        
        DrawRectangleV(drawPos, {FISH_WIDTH, FISH_HEIGHT}, c);

        Rectangle rec = { drawPos.x, drawPos.y, size.x, size.y };
        DrawRectangleLinesEx(rec, 10, BLACK);
    }
};
