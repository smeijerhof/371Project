#include "../include/game.h"

Game game;
Actor self;
int* playerNo;

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
	uint16_t score;
};

struct catchMsg {
    int serverSocket;
    uint16_t token = 2;
    uint16_t playerNo;
    uint16_t fishIndex;
};

struct killMsg {
	int serverSocket;
	uint16_t token = 3;
	uint16_t playerNo;
	uint16_t fishIndex;
};

struct connMsg {
    int serverSocket;
    uint16_t token = 0;
};

void* sendKillMessage(void* msg) {
	// Based on message needing to be sent from server, client sends appropriate token, awaits response on new thread
	uint16_t outMsg[BUFFER_SIZE];
	int msgSize = 0;
	
	struct killMsg* myMsg = (struct killMsg*) msg;
	myMsg->token = 3;
	myMsg->fishIndex = (uint16_t) self.target;
	
	outMsg[msgSize++] = htons(myMsg->token);
	outMsg[msgSize++] = htons((uint16_t) *playerNo);
	outMsg[msgSize++] = htons(myMsg->fishIndex);
	
	write(myMsg->serverSocket, outMsg, 2*BUFFER_SIZE);//2*msgSize);
	
	return 0;
}

void* sendCatchMessage(void* msg) {
    // Based on message needing to be sent from server, client sends appropriate token, awaits response on new thread
    uint16_t outMsg[BUFFER_SIZE];
    int msgSize = 0;

    struct catchMsg* myMsg = (struct catchMsg*) msg;
	myMsg->token = 2;
	
    outMsg[msgSize++] = htons(myMsg->token);
    outMsg[msgSize++] = htons((uint16_t) *playerNo);
	
	myMsg->fishIndex = (uint16_t) self.target;
    outMsg[msgSize++] = htons(myMsg->fishIndex);
	
    write(myMsg->serverSocket, outMsg, 2*BUFFER_SIZE);//2*msgSize);

    uint16_t response[BUFFER_SIZE];
	printf("Reading Response!\n");
    read(myMsg->serverSocket, response, 2*BUFFER_SIZE);

	printf("Response = %d\n", ntohs(response[0]));
    
	if (ntohs(response[0]) == 0) {

		printf("Failure in catching fish!\n");
		return 0;
	}
	
	printf("Success in catching fish!\n");
    self.catching = true;
    //self.target = myMsg->fishIndex;
    game.fishes[myMsg->fishIndex].taken = true;
	self.life = 10;
	
    return 0;
}

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
	int responseNum = 0;
	if(ntohs(response[responseNum]) == 1000) {
        printf("Could not establish connection; there are already 4 players in the game\n");
        close(myMsg->serverSocket);
        CloseWindow();
        return 0;
    }

    Actor newActor;
	newActor.p = ntohs(response[responseNum]);
	*playerNo = (int) ntohs(response[responseNum++]);

    game.actors[game.actorNum++] = newActor;

    for(int i = 0; i < FISH_NUM; i++) {
		int fishX = (int) ntohs(response[responseNum++]);
		int fishY = (int) ntohs(response[responseNum++]);
		
		printf("	Received fish spawn at (%d, %d)\n", fishX, fishY);

		game.fishes[i].spawn(fishX, fishY);
		game.fishes[i].texNum = rand() % 3;
		
        if(fishX == 1000 || fishY == 1000) {
            game.fishes[i].alive = false;
        }
    }

    return 0;
}

void* sendMouseMessage(void* msg) {
    // Based on message needing to be sent from server, client sends appropriate token, awaits response on new thread
    uint16_t outMsg[BUFFER_SIZE];
    int msgSize = 0;

    struct crsrMsg* myMsg = (struct crsrMsg*) msg;
		
    outMsg[msgSize++] = htons(myMsg->token);
    outMsg[msgSize++] = htons((uint16_t) *playerNo);
    outMsg[msgSize++] = htons(myMsg->mouseX);
    outMsg[msgSize++] = htons(myMsg->mouseY);
	outMsg[msgSize++] = htons(myMsg->score);
	
    write(myMsg->serverSocket, outMsg, 2*BUFFER_SIZE);//2*msgSize);

    uint16_t response[BUFFER_SIZE];
    read(myMsg->serverSocket, response, 2*BUFFER_SIZE);
	
	// Handle other player data
	int responseNum = 0;
	for (int i = 0; i < PLAYER_NUM; i++) {
		if (i == *playerNo) {
			responseNum += 3;
			continue;
		}
		game.actors[i].pos.x = (float) ntohs(response[responseNum++]);
		game.actors[i].pos.x = (float) ntohs(response[responseNum++]);
		game.actors[i].points = (int) ntohs(response[responseNum++]);
	}

    return 0;
}

int main() {
    playerNo = (int*) malloc(sizeof(int));

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


    Image cursorImage = LoadImage("assets/Cursor.png");
	Texture2D cursorTexture = LoadTextureFromImage(cursorImage);
	UnloadImage(cursorImage);
	
	Image fish1 = LoadImage("assets/chub.png");
	Image fish2 = LoadImage("assets/pike.png");
	Image fish3 = LoadImage("assets/sardine.png");
	
	Texture2D fish1Tex = LoadTextureFromImage(fish1);
	Texture2D fish2Tex = LoadTextureFromImage(fish2);
	Texture2D fish3Tex = LoadTextureFromImage(fish3);
	
	UnloadImage(fish1);
	UnloadImage(fish2);
	UnloadImage(fish3);
	
	Texture2D fishTextures[3] = {
		fish1Tex,
		fish2Tex,
		fish3Tex
	};

    while (!WindowShouldClose()) {
        //printf("counting time\n");
        game.elapsed += GetFrameTime();
		
		printf("check for catch request\n");
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			printf("	get mouse pos\n");
            Vector2 mp = GetMousePosition();

			printf("	checking all fish\n");
			for (uint16_t i = 0; i < FISH_NUM; i++) {
                if (mp.x > game.fishes[i].pos.x && mp.x < game.fishes[i].pos.x + FISH_WIDTH && mp.y > game.fishes[i].pos.y && mp.y < game.fishes[i].pos.y + FISH_HEIGHT && game.fishes[i].alive) {
					printf("		colliding with fish\n");
                    // REQUEST TO CATCH
					if (self.catching) break;

					printf("		Requesting to catch fish %d\n", i);

					
					struct catchMsg myMsg;
					myMsg.serverSocket = myServerSocket;
					myMsg.token = (uint16_t) 2;
					myMsg.fishIndex = i;
					self.target = (int) i;
					

					printf("		Sending catch message %d\n", i);

					pthread_create(&tcpThread, NULL, sendCatchMessage, (void*) &myMsg);
					
					break;
                }
            }
        }
        

        
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && self.catching) {
			printf("	killing fish\n");

			game.fishes[self.target].taken = true;
            self.life--;
        } 
        
        
        if (self.life <= 0 && self.catching && game.fishes[self.target].alive) {
			printf("	killed fish\n");

			self.catching = false;
			game.fishes[self.target].alive = false;
			self.points++;
			self.life = 0;
			
			struct killMsg myMsg;
			myMsg.serverSocket = myServerSocket;
			myMsg.token = (uint16_t) 3;
			
			myMsg.fishIndex = (uint16_t) self.target;
			
			pthread_create(&tcpThread, NULL, sendKillMessage, (void*) &myMsg);
			printf("Fish %d has been killed\n", self.target);
		}

        // Multi-threaded client Testing
        //printf("%d\n",(int) (fmod(game.elapsed,0.1f) * 100));
        
        if (int(fmod(game.elapsed, 0.1f) * 100) == 0) {
			printf("	sending mouse message\n");
            struct crsrMsg myMsg;
            myMsg.serverSocket = myServerSocket;
            myMsg.token = (uint16_t) 1;
			
            myMsg.mouseX = (uint16_t) GetMousePosition().x;
            myMsg.mouseY = (uint16_t) GetMousePosition().y;
			
            pthread_create(&tcpThread, NULL, sendMouseMessage, (void*) &myMsg);
        }

        BeginDrawing();
		printf("	drawing\n");
            ClearBackground(DARKBLUE);
			
			printf("		draw scores\n");
			for (int i = 0; i < game.actorNum; i++) {
				Actor a = game.actors[i];
				if (i == *playerNo) a = self;
				DrawText(TextFormat("Player %d Points = %d", i+1, a.points), 20, 20 + i * 20, 20, game.colors[i]);
			}
			
			printf("		draw ui\n");
            if (self.catching) {
                DrawText("Fish Hooked!", (SCREEN_WIDTH - MeasureText("Fish Hooked!", 50)) / 2.f, 20, 50, WHITE);
				DrawText("Click the mouse quickly!", (SCREEN_WIDTH - MeasureText("Click the mouse quickly!", 35)) / 2.f, 70, 35, WHITE);
            }
			
			printf("		draw fishes\n");
            game.draw(fishTextures);
			
			printf("		draw actors\n");
			for (int i = 0; i < game.actorNum; i++) {
				if (i == *playerNo) continue;
				DrawTextureEx(cursorTexture, (Vector2) {game.actors[i].pos.x, game.actors[i].pos.y}, 0.f, 5.f, game.colors[i]);
			}
			
			printf("		draw player\n");
			DrawTextureEx(cursorTexture, (Vector2) {GetMousePosition().x, GetMousePosition().y}, 0.f, 5.f, game.colors[*playerNo]);
			

        EndDrawing();
		printf("done drawing\n");
    }
	
	UnloadTexture(fishTextures[0]);
	UnloadTexture(fishTextures[1]);
	UnloadTexture(fishTextures[2]);
	
	UnloadTexture(cursorTexture);
    close(myServerSocket);
    CloseWindow();
    return 0;
}
