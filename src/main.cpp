#include "../include/game.h"

Game* game;
int* playerNo;

// Function to read keyboard input (non-blocking)
// int nonBlockingRead() {
//     struct termios oldt, newt;
//     int ch;
//     int oldf;

//     tcgetattr(STDIN_FILENO, &oldt);
//     newt = oldt;
//     newt.c_lflag &= ~(ICANON | ECHO);
//     tcsetattr(STDIN_FILENO, TCSANOW, &newt);
//     oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
//     fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

//     ch = getchar();

//     tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
//     fcntl(STDIN_FILENO, F_SETFL, oldf);

//     return ch;
// }

// Global mutexes to ensure thread-safe access to the game state
std::mutex mtxMousePos;
std::mutex mtxLeftMouseButton;
std::mutex mtxZKeyPressed;
std::mutex mtxGame;

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

    if(write(myMsg->serverSocket, outMsg, 2*msgSize) != 2*msgSize) {
        printf("Message not sent in its entirety.\n");
    }

    uint16_t response[BUFFER_SIZE];
    read(myMsg->serverSocket, response, 2*BUFFER_SIZE);

    // printf("read complete\n");
    if(ntohs(response[0]) == PLACEHOLDER) {
        printf("Could not establish connection; there are already 4 players in the game\n");
        close(myMsg->serverSocket);
        CloseWindow();
        return 0;
    }

    Actor newActor;
    newActor.p = ntohs(response[0]);
    *playerNo = (int) ntohs(response[0]);

    game->actors[game->actorNum++] = newActor;

    for(int i = 0; i < FISH_NUM; i++) {
		int fishX = (int) ntohs(response[1+(2*i)]);
		int fishY = (int) ntohs(response[1+(2*i)+1]);
		
		printf("	Received fish spawn at (%d, %d)\n", fishX, fishY);

		game->fishes[i].spawn(fishX, fishY);
        if(fishX == PLACEHOLDER || fishY == PLACEHOLDER) {
            game->fishes[i].alive = false;
        }
    }

    return 0;
}

void* sendMouseMessage(void* msg) {
    // Based on message needing to be sent from server, client sends appropriate token, awaits response on new thread
    uint16_t outMsg[BUFFER_SIZE];
    int msgSize = 0;

    struct crsrMsg* myMsg = (struct crsrMsg*) msg;
	
	// printf("positions: %d %d %d\n", myMsg->mouseX, htons(myMsg->mouseX), ntohs(htons(myMsg->mouseX)));
	
    outMsg[msgSize++] = htons(myMsg->token);
    outMsg[msgSize++] = htons((uint16_t) *playerNo);
    outMsg[msgSize++] = htons(myMsg->mouseX);
    outMsg[msgSize++] = htons(myMsg->mouseY);
	
    write(myMsg->serverSocket, outMsg, 2*BUFFER_SIZE);//2*msgSize);

    uint16_t response[BUFFER_SIZE];
    read(myMsg->serverSocket, response, 2*BUFFER_SIZE);

    return 0;
}

int main() {
    playerNo = (int*) malloc(sizeof(int));

    game = new Game();

    pthread_t tcpThread;

    int myServerSocket = connectToServer();


    // Upon connection establishment, instantiate local game with provided fish locations
    struct connMsg* myConnMsg = new struct connMsg;
    myConnMsg->serverSocket = myServerSocket;
    pthread_t connThread;

    pthread_create(&connThread, NULL, sendConnectMessage, (void*) myConnMsg);
    pthread_join(connThread, NULL);


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fishing Frenzy");

    SetTargetFPS(30); 

    Actor self;

    while (!WindowShouldClose()) {
        
        game->elapsed += GetFrameTime();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mp = GetMousePosition();
            for (int i = 0; i < FISH_NUM; i++) {
                if (mp.x > game->fishes[i].pos.x && mp.x < game->fishes[i].pos.x + FISH_WIDTH && mp.y > game->fishes[i].pos.y && mp.y < game->fishes[i].pos.y + FISH_HEIGHT && game->fishes[i].alive) {
                    // REQUEST TO CATCH
                    if (self.catching) break;
                    self.catching = true;
                    self.target = i;
                    game->fishes[i].taken = true;
                }
            }
        }

        if (IsKeyPressed(KEY_Z) && self.catching) {
            //game->fishes[self.target].health--;
        } 

        /*if (game->fishes[self.target].health <= 0 && self.catching & game->fishes[self.target].alive) {
            game->fishes[self.target].alive = false;
            self.catching = false;
            self.points++;
        }*/

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
                DrawText("Fish Hooked!", (SCREEN_WIDTH - MeasureText("Fish Hooked!", 50)) / 2.f, 20, 50, WHITE);
                DrawText("Press Z!", (SCREEN_WIDTH - MeasureText("Press Z!", 35)) / 2.f, 70, 35, WHITE);
            }

            game->draw();

        EndDrawing();
    }

    // Program gets blocked when below function is run
    // pthread_join(tcpThread, NULL);
    delete game;
    close(myServerSocket);
    CloseWindow();
    return 0;
}
