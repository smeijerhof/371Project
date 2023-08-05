#pragma once

// STD and assorted libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
//

// Project Libs
#include "raylib.h"
#include "def.h"
//

struct Fish {
	const float textureScale = 1.f;
	const float width = 100.f;
	const float height = 100.f;
	
	const Color fishColors[4] = {RED, BLUE, GREEN, YELLOW};
	
    Vector2 pos;
	uint16_t taken;
    bool alive;
	
	int tex;
	
	Fish() {
		pos = {0.f, 0.f};
		taken = FISH_NOT_TAKEN;
		alive = true;
		
		tex = 0;
	}
	
	void setPos(float x, float y) {
		pos = {x, y};
	}
	
	void setTex(int t) {
		tex = t;
	}
	
	bool colliding(float x, float y) {
		return x > pos.x && x < pos.x + width && y > pos.y && y < pos.y + height && alive;
	}

    void draw(Texture2D texure) {
        if (!alive) return;
      
        Vector2 drawPos = pos;
        if (taken != 5) {
            drawPos.x += (rand() % 10) - 5;
            drawPos.y += (rand() % 10) - 5;
        }
        
        Color c = WHITE;
		if (taken != FISH_NOT_TAKEN) c = fishColors[taken];
        
		DrawTextureEx(texure, (Vector2) {drawPos.x, drawPos.y}, 0.f, textureScale, c);
    }
};
