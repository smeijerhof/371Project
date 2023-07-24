#include "include/raylib.h"

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

#define SERVER_PORT 65501
#define IP "142.58.14.7"
#define BUFFER_SIZE 256

enum Player { NONE = 0, ONE, TWO, THREE, FOUR };

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
    Player p = NONE;
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
        for (int i = 0; i < fishNum; i++) {
            fishes[i].draw();
        }
    }

    ~Game() {
        delete[] fishes;
        delete[] positions;
    }
};

bool connectToGame(Game& game, Actor& self) {
    if (game.actorNum >= 4) return false;


    Player num = NONE;
    switch(game.actorNum) {
        case 0:
            num = ONE;
            break;
        case 1:
            num = TWO;
            break;
        case 2:
            num = THREE;
            break;
        case 3:
            num = FOUR;
            break;
        default:
            return false;
    }

    self.p = num;
    game.actors[game.actorNum] = self;
    game.actorNum++;
    return true;
}

void printError(const char* message) {
    perror(message);
    exit(-1);
}

int connectToServer() {
    struct hostent* serverInfo;
    serverInfo = gethostbyname(IP);
    if(!serverInfo)
        printError("Could not retrieve host information");

    // Initialize destination socket
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    memcpy(&serverAddress.sin_addr.s_addr, serverInfo->h_addr, serverInfo->h_length);
    serverAddress.sin_port = htons(SERVER_PORT);

    // Create new TCP socket, return file descriptor
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        printError("socket failed");

    // Try to connect to server
    int c = connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if(c < 0)
        printError("Could not connect to server");

    return sock;

}

struct crsrMsg {
    int serverSocket;
    uint16_t token;
    uint16_t mouseX;
    uint16_t mouseY;
};

void* sendMessage(void* msg) {
    // Based on message needing to be sent from server, client sends appropriate token, awaits response on new thread
    uint16_t buffer[BUFFER_SIZE];
    uint16_t* ptr = buffer;

    struct crsrMsg* myMsg = (struct crsrMsg*) msg;

    // printf("%d %d\n", myMsg->mouseX, myMsg->mouseY);
    // printf("%d %d\n", htons(myMsg->mouseX), htons(myMsg->mouseY));

    buffer[0] = htons(myMsg->token);
    buffer[1] = htons(myMsg->mouseX);
    buffer[2] = htons(myMsg->mouseY);

    ptr = buffer + 6;

    // free(msg);
    printf("client: ");
    for(int i = 0; i < 3; i++) {
        printf("%d ", ntohs(buffer[i]));
    }
    printf("\n");

    // printf("%ld\n",ptr - buffer);

    write(myMsg->serverSocket, buffer, ptr - buffer);

    uint16_t response[BUFFER_SIZE];
    read(myMsg->serverSocket, response, BUFFER_SIZE);
    printf("response: ");
    for(int i = 0; i < 3; i++)
        printf("%d ", ntohs(buffer[i]));
    printf("\n", response);

    // switch((char*) msg) {
    //     case "mouse":
    //         break;
    //     case "catch":
    //         break;
    //     case "catchComplete":
    //         break;
    // }

    return 0;
}

int main() {
    pthread_t tcpThread;

    Game game;

    game.screenWidth = 800;
    game.screenHeight = 450;

    int myServerSocket = connectToServer();
    // printf("serverSocket: %d\n", myServerSocket);


    game.start();

    InitWindow(game.screenWidth, game.screenHeight, "Window Name");

    SetTargetFPS(30); 

    Actor self;
    if (!connectToGame(game, self)) return -1;

    while (!WindowShouldClose()) {
        
        game.elapsed += GetFrameTime();
        if (IsKeyPressed(KEY_R)) {
            game.restart();
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            for (int i = 0; i < game.fishNum; i++) {
                if (mp.x > game.fishes[i].position.x && mp.x < game.fishes[i].position.x + Fish::size.x && mp.y > game.fishes[i].position.y && mp.y < game.fishes[i].position.y + Fish::size.y && game.fishes[i].alive) {
                    // REQUEST TO CATCH
                    if (self.catching) break;
                    self.catching = true;
                    self.target = i;
                    game.fishes[i].color = GREEN;
                    game.fishes[i].caught = true;
                }
            }
        }

        if (IsKeyPressed(KEY_Z) && self.catching) {
            game.fishes[self.target].health--;
        } 

        if (game.fishes[self.target].health <= 0 && self.catching & game.fishes[self.target].alive) {
            game.fishes[self.target].alive = false;
            self.catching = false;
            self.points++;
        }

        // Multi-threaded client Testing
        // TODO: switch back to uint16_t? Problem was with return, not data passed
        // printf("%d\n",(int) (fmod(game.elapsed,0.1f) * 100));
        if (int(fmod(game.elapsed, 0.1f) * 100) == 0) {
            struct crsrMsg myMsg;
            myMsg.serverSocket = myServerSocket;
            myMsg.token = (uint16_t) 1;
            myMsg.mouseX = (uint16_t) GetMousePosition().x;
            myMsg.mouseY = (uint16_t) GetMousePosition().y;

            pthread_create(&tcpThread, NULL, sendMessage, (void*) &myMsg);

            // struct crsrCharMsg myCharMsg;
            // myCharMsg.serverSocket = myServerSocket;
            // snprintf(myCharMsg.mouseX, 4, "%03d", (int) GetMousePosition().x);
            // snprintf(myCharMsg.mouseY, 4, "%03d", (int) GetMousePosition().y);

            // pthread_create(&tcpThread, NULL, sendCharMessage, (void*) &myCharMsg);

            // pthread_join(tcpThread,NULL);
        }


        BeginDrawing();

            ClearBackground(DARKBLUE);

            DrawFPS(20, 20);
            DrawText(TextFormat("Points = %d", self.points), 20, 40, 20, WHITE);
            DrawText(TextFormat("Time = %.1f", game.elapsed), 20, 60, 20, WHITE);

            if (self.catching) {
                DrawText("Fish Hooked!", (game.screenWidth - MeasureText("Fish Hooked!", 50)) / 2.f, 20, 50, WHITE);
                DrawText("Press Z!", (game.screenWidth - MeasureText("Press Z!", 35)) / 2.f, 70, 35, WHITE);
            }

            game.draw();

        EndDrawing();
    }

    close(myServerSocket);
    CloseWindow();
    return 0;
}