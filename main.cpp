#include "include/raylib.h"
#include "include/game.h"

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
#define IP "localhost"
#define BUFFER_SIZE 32
#define FISH_NUM 10

Game* game;

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
    uint16_t token = 1;
    uint16_t playerNo;
    uint16_t mouseX;
    uint16_t mouseY;
};

struct connMsg {
    int serverSocket;
    uint16_t token = 0;
};

void* sendConnectMessage(void* msg) {
    uint16_t outMsg[BUFFER_SIZE];
    int msgSize = 0;

    struct connMsg* myMsg = (struct connMsg*) msg;
    outMsg[msgSize++] = htons(myMsg->token);

    // printf("sending %d/%d. %ld bytes\n", ntohs(outMsg[0]), outMsg[0], 
    write(myMsg->serverSocket, outMsg, 2*msgSize);
    // write(myMsg->serverSocket, msg, 2*msgSize);

    uint16_t response[BUFFER_SIZE];
    read(myMsg->serverSocket, response, 2*BUFFER_SIZE);

    // printf("read complete\n");
    if(ntohs(response[0]) == 1000) {
        printf("Could not establish connection; there are already 4 players in the game\n");
        close(myMsg->serverSocket);
        CloseWindow();
        return 0;
    }

    // TESTING: print vectors of all fish
    // for(int i = 0; i < BUFFER_SIZE; i++) {
    //     printf("%d->%d\n",response[i], ntohs(response[i]));
    // }
    // for(int i = 0; i < (2*FISH_NUM)+1; i+=2) {
    //         printf("network: %d %d\nhost: %d %d\nfloat: %.1f %.1f\n\n",
    //                 response[i], response[i+1],
    //                 ntohs(response[i]),ntohs(response[i+1]),
    //                 (float) ntohs(response[i]), (float) ntohs(response[i+1]));
    // }
    Actor newActor;
    newActor.p = ntohs(response[0]);

    

    game->actors[game->actorNum++] = newActor;

    game->fishes = new Fish[FISH_NUM];
    game->positions = new Vector2[FISH_NUM];

    for(int i = 0; i < FISH_NUM; i++) {
        float fishX = (float) ntohs(response[1+(2*i)]);
        float fishY = (float) ntohs(response[1+(2*i)+1]);
        Vector2 fishPos = {fishX, fishY};

        game->fishes[i].spawn(fishPos);
        game->positions[i] = fishPos;
        if(fishX == 1000 || fishY == 1000) {
            game->fishes[i].alive = false;
        }
    }

    for(int i = 0; i < FISH_NUM; i++) {
        // printf("{%.1f, %.1f}\n", game->fishes[i].position.x, game->fishes[i].position.y);
    }

    return 0;
}

void* sendMouseMessage(void* msg) {
    // Based on message needing to be sent from server, client sends appropriate token, awaits response on new thread
    uint16_t outMsg[BUFFER_SIZE];
    int msgSize = 0;

    struct crsrMsg* myMsg = (struct crsrMsg*) msg;

    // printf("%d %d\n", myMsg->mouseX, myMsg->mouseY);
    // printf("%d %d\n", htons(myMsg->mouseX), htons(myMsg->mouseY));

    outMsg[msgSize++] = htons(myMsg->token);
    outMsg[msgSize++] = htons(myMsg->mouseX);
    outMsg[msgSize++] = htons(myMsg->mouseY);

    // free(msg);
    // printf("client: ");
    // for(int i = 0; i < 3; i++) {
    //     printf("%d ", ntohs(buffer[i]));
    // }
    // printf("\n");

    // printf("%ld\n",ptr - buffer);

    write(myMsg->serverSocket, msg, 2*BUFFER_SIZE);//2*msgSize);

    uint16_t response[BUFFER_SIZE];
    read(myMsg->serverSocket, response, 2*BUFFER_SIZE);
    // printf("response: ");
    // for(int i = 0; i < 3; i++)
    //     printf("%d ", ntohs(buffer[i]));
    // printf("\n");

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

    game = (Game*) malloc(sizeof(Game));

    game->screenWidth = 800;
    game->screenHeight = 450;


    int myServerSocket = connectToServer();
    // printf("serverSocket: %d\n", myServerSocket);


    // Upon connection establishment, instantiate local game with provided fish locations
    struct connMsg* myConnMsg = new struct connMsg;
    myConnMsg->serverSocket = myServerSocket;
    pthread_t connThread;

    pthread_create(&connThread, NULL, sendConnectMessage, (void*) myConnMsg);
    pthread_join(connThread, NULL);

    // printf("game starting!\n");
    // for(int i = 0; i < FISH_NUM; i++) {
    //     printf("{%.1f,%.1f} %d\n", game->fishes[i].position.x, game->fishes[i].position.y,game->fishes[i].alive);
    // }

    InitWindow(game->screenWidth, game->screenHeight, "Window Name");

    SetTargetFPS(30); 

    Actor self;

    while (!WindowShouldClose()) {
        
        game->elapsed += GetFrameTime();
        if (IsKeyPressed(KEY_R)) {
            game->restart();
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            for (int i = 0; i < game->fishNum; i++) {
                if (mp.x > game->fishes[i].position.x && mp.x < game->fishes[i].position.x + Fish::size.x && mp.y > game->fishes[i].position.y && mp.y < game->fishes[i].position.y + Fish::size.y && game->fishes[i].alive) {
                    // REQUEST TO CATCH
                    if (self.catching) break;
                    self.catching = true;
                    self.target = i;
                    game->fishes[i].color = GREEN;
                    game->fishes[i].caught = true;
                }
            }
        }

        if (IsKeyPressed(KEY_Z) && self.catching) {
            game->fishes[self.target].health--;
        } 

        if (game->fishes[self.target].health <= 0 && self.catching & game->fishes[self.target].alive) {
            game->fishes[self.target].alive = false;
            self.catching = false;
            self.points++;
        }

        // Multi-threaded client Testing
        // printf("%d\n",(int) (fmod(game.elapsed,0.1f) * 100));
        if (int(fmod(game->elapsed, 0.1f) * 100) == 0) {
            struct crsrMsg myMsg;
            myMsg.serverSocket = myServerSocket;
            myMsg.token = (uint16_t) 1;
            myMsg.mouseX = (uint16_t) GetMousePosition().x;
            myMsg.mouseY = (uint16_t) GetMousePosition().y;

            pthread_create(&tcpThread, NULL, sendMouseMessage, (void*) &myMsg);
        }

        BeginDrawing();

            ClearBackground(DARKBLUE);

            DrawFPS(20, 20);
            DrawText(TextFormat("Points = %d", self.points), 20, 40, 20, WHITE);
            DrawText(TextFormat("Time = %.1f", game->elapsed), 20, 60, 20, WHITE);

            if (self.catching) {
                DrawText("Fish Hooked!", (game->screenWidth - MeasureText("Fish Hooked!", 50)) / 2.f, 20, 50, WHITE);
                DrawText("Press Z!", (game->screenWidth - MeasureText("Press Z!", 35)) / 2.f, 70, 35, WHITE);
            }

            game->draw();

        EndDrawing();
    }

    free(game);
    close(myServerSocket);
    CloseWindow();
    return 0;
}