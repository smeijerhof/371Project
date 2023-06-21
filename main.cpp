#include <../include/raylib.h>
#include <../include/raymath.h>
#include <vector>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 400
#define SCREEN_H 400

enum State { MENU = 0, PLAY = 1, OVER = 2 };

struct Pond {
    Vector2 position;
    Vector2 size;
    bool seized;
    int total;

    Pond (float x, float y, float w, float h) {
        position.x = x;
        position.y = y;

        size.x = w;
        size.y = h;

        seized = false;

        total = 5;
    }

    bool available(Vector2 pos, Vector2 s) {
        bool ret = false;

        float xDistLeft = abs(position.x - (pos.x + s.x));
        float xDistRight = abs((position.x + size.x) - pos.x);

        float yDistTop = abs(position.y - (pos.y + s.y));
        float yDistBottom = abs((position.y + size.y) - pos.y);

        if ((xDistLeft < 5.f || xDistRight < 5.f) && (pos.y + s.y) > position.y && pos.y < (position.y + size.y)) ret = true;
        if ((yDistTop < 5.f || yDistBottom < 5.f) && (pos.x + s.x) > position.x && pos.x < (position.x + size.x)) ret = true;

        return ret;
    }

    void draw() {
        Color color = DARKBLUE;
        if (total == 0.f) color = RED;

        DrawRectangleV(position, size, color);
        if (seized) DrawRectangle(position.x + size.x / 2.f, position.y + size.y / 2.f, 5, 5, RED);
    }
};

struct Player {
    Vector2 position;
    Vector2 lastPosition;
    Vector2 velocity;
    Vector2 size;

    int fish;
    bool fishing;
    Vector2 fishPos;
    Vector2 fishVel;
    Pond* active;

    Player () {
        position = { 0, 0 };
        lastPosition = { 0, 0 };
        velocity = { 0, 0 };
        size = { 25, 25 };

        fish = 0;
        fishing = false;
        fishPos = { 0, 0 };
        fishVel = { 0, 0 };
        active = NULL;
    }

    void update (std::vector<Pond>& ponds) {
        if (fishing) {
            if (fishPos.y == 0.f) {
                fishPos = { 51, SCREEN_H - 50 - 15 / 2.f };
                fishVel = { 7.f, 0.f };
            }
            if (fishPos.x >= 50 - 15 + SCREEN_W - 100 || fishPos.x <= 50) {
                fishVel.x *= -1;
            }

            fishPos.x += fishVel.x;

            if (IsKeyPressed(KEY_SPACE)) {
                if ((fishPos.x + 15) > (SCREEN_W - 20) / 2.f && fishPos.x < (SCREEN_W + 20) / 2.f) fish++;
                fishing = false;
                active->seized = false;
                active->total--;
                active = NULL;
            }

            return;
        }

        velocity = { 0, 0 };
        if (IsKeyDown(KEY_D)) velocity.x += 1.f;
        if (IsKeyDown(KEY_A)) velocity.x -= 1.f;

        if (IsKeyDown(KEY_S)) velocity.y += 1.f;
        if (IsKeyDown(KEY_W)) velocity.y -= 1.f;
        
        if (fishing) velocity = { 0, 0 };
        velocity = Vector2Normalize(velocity);
        velocity = Vector2Scale(velocity, 5.f);

        lastPosition = position;
        position.x += velocity.x;
        position.y += velocity.y;

        clamp();

        for (Pond& p : ponds) {
            if (collide(p)) revert(p);

            if (IsKeyPressed(KEY_SPACE) && p.available(position, size) && p.total > 0 && !p.seized) { // Attempt to seize a pond
                p.seized = true;
                fishing = true;
                active = &p;
            }
        }
    }

    void clamp () {
        if (position.x < 0.f) position.x = 0.f; // Clamp to screen boundaries
        if (position.x > SCREEN_W - size.x) position.x = SCREEN_W - size.x;

        if (position.y < 0.f) position.y = 0.f;
        if (position.y > SCREEN_H - size.y) position.y = SCREEN_H - size.y;
    }

    bool collide (Pond p) {
        Vector2 obstacleP = p.position;
        Vector2 obstacleS = p.size;

        // Test for collision
        return position.x + size.x > obstacleP.x && position.x < obstacleP.x + obstacleS.x && position.y + size.y > obstacleP.y && position.y < obstacleP.y + obstacleS.y;
    }

    void revert (Pond p) {
        position = lastPosition; // Resolve collision

        position.x += velocity.x; // Test for x or y axis problems (enables sliding)
        if (collide(p)) velocity.x = 0.f;
        else velocity.y = 0.f;

        position = lastPosition; // Resolve sliding
        position.x += velocity.x;
        position.y += velocity.y;
    }

    void draw() {
        DrawRectangleV(position, size, PURPLE);

        if (fishing) {
            DrawRectangle(50, SCREEN_H - 75, SCREEN_W - 100, 50, RED);
            DrawRectangle((SCREEN_W - 20) / 2.f, SCREEN_H - 75, 20, 50, GREEN);
            DrawRectangle(fishPos.x, fishPos.y, 15, 15, BLUE);
        }
    }
};

struct Game {
    State state;

    std::vector<Pond> ponds;

    Player client;
    std::vector<Player> players;

    Game () {
        state = MENU;

        Pond pond1 = Pond(50, 100, 100, 200);
        Pond pond2 = Pond(250, 100, 100, 200);

        ponds.push_back(pond1);
        ponds.push_back(pond2);

        client = Player();
    }
};

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "game");
    SetTargetFPS(60);

    Game game = Game();

    while (!WindowShouldClose()) {
        // This is where client-server synchronization would be done, i.e. server telling client where other players are, and which ponds are seized

        if (game.state == MENU && IsKeyPressed(KEY_ENTER)) game.state = PLAY;
        if (game.state == OVER && IsKeyPressed(KEY_ENTER)) {
            game.state = MENU;
            game.client.fish = 0;
            game.client.position = { 0.f, 0.f };
            for (Pond& p : game.ponds) p.total = 5;
        }
        double dt = GetFrameTime();
        if (game.state == PLAY) {
            game.client.update(game.ponds); // This is updating the player object, not the ponds
            int grandTotal = 0;
            for (Pond p : game.ponds) grandTotal += p.total;
            if (grandTotal == 0) game.state = OVER;
        }

        // This is where client would let server know what its position and score is, as well as if it has seized a pond, to lock it for other players.

        BeginDrawing();
            ClearBackground(BLACK);

            switch (game.state) {
                case MENU:
                DrawText("Press enter to play", 10, 10, 30, WHITE);
                DrawText("WASD to move, SPACE to interact", 10, 40, 20, WHITE);
                break;

                case PLAY:
                for (Pond p : game.ponds) p.draw();
                for (Player p : game.players) p.draw(); // Draws connected players
                game.client.draw(); // Draws client player

                DrawText(TextFormat("fps = %d", (int) (1 / dt)), 10, 10, 20, WHITE);
                DrawText(TextFormat("dt = %.4f", dt), 10, 30, 20, WHITE);
                DrawText(TextFormat("Fish = %d", game.client.fish), 10, 50, 20, WHITE);
                break;

                case OVER:
                DrawText("Game Over", 10, 10, 30, WHITE);
                DrawText(TextFormat("%d out of 10 Fish caught", game.client.fish), 10, 40, 30, WHITE);
                DrawText("Press enter to restart", 10, 70, 20, WHITE);
                break;
            } 
        EndDrawing();
    }
    CloseWindow();
    return 0;
}