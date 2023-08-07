#include "../include/game.h"
#include "../include/client_message.h"
#include "../include/client_connection.h"
#include "../include/client_updates.h"

std::mutex mtxMousePos;
std::mutex mtxLeftMouseButton;
std::mutex mtxZKeyPressed;
std::mutex mtxGame;
//std::mutex fishCatch;

void printError(const char* message) {
    perror(message);
    exit(-1);
}

int connectToServer(const char* IP) {
    struct hostent* serverInfo;
    serverInfo = gethostbyname(IP);
	if(serverInfo == NULL)
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

void initRaylib(Game* state) {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Fishing Frenzy");
	
	SetTargetFPS(30);
	
	Image cursorImage = LoadImage("assets/Cursor.png");
	Texture2D cursorTexture = LoadTextureFromImage(cursorImage);
	UnloadImage(cursorImage);
	
	state->cursorTexture = cursorTexture;
	
	Image fish1 = LoadImage("assets/chub.png");
	Image fish2 = LoadImage("assets/pike.png");
	Image fish3 = LoadImage("assets/sardine.png");
	
	Texture2D fish1Tex = LoadTextureFromImage(fish1);
	Texture2D fish2Tex = LoadTextureFromImage(fish2);
	Texture2D fish3Tex = LoadTextureFromImage(fish3);
	
	UnloadImage(fish1);
	UnloadImage(fish2);
	UnloadImage(fish3);
	
	state->fishTextures[0] = fish1Tex;
	state->fishTextures[1] = fish2Tex;
	state->fishTextures[2] = fish3Tex;
}

void handleLobby(Game* state, pthread_t* tcpThread, int clientSocket) {
	if (state->host) {
		if (state->lobbyTimer > 0.f) {
			state->lobbyTimer += GetFrameTime();
			
			if (state->lobbyTimer >= state->lobbyLength) {
				ClientMessage startLobbyMessage;
				startLobbyMessage.construct(state, clientSocket, START_LOBBY_MESSAGE);
				
				pthread_create(tcpThread, NULL, sendStartLobbyMessage, (void*) &startLobbyMessage);
				pthread_join(*tcpThread, NULL);
				
				state->lobby = false;
			}
			
			int timeRemaining = state->lobbyLength - state->lobbyTimer;
			int titleFont = 50;
			int subFont = 30;
			
			BeginDrawing();
			ClearBackground(DARKBLUE);
			DrawText(TextFormat("Game starting in %d seconds", timeRemaining), (SCREEN_WIDTH - MeasureText(TextFormat("Game starting in %d seconds", timeRemaining), titleFont)) / 2.f, 20, titleFont, WHITE);
			EndDrawing();
		} else {
			if (IsKeyPressed(KEY_Z)) state->lobbyTimer = 0.01f;
			
			int titleFont = 50;
			int subFont = 30;
			
			BeginDrawing();
			ClearBackground(DARKBLUE);
			DrawText("You are the host.", (SCREEN_WIDTH - MeasureText("You are the host.", titleFont)) / 2.f, 20, titleFont, WHITE);
			DrawText("Press z to start the game!", (SCREEN_WIDTH - MeasureText("Press z to start the game!", subFont)) / 2.f, 20 + titleFont + 20, subFont, WHITE);
			
			EndDrawing();
		}
		return;
	}
	
	ClientMessage waitLobbyMessage;
	waitLobbyMessage.construct(state, clientSocket, WAIT_LOBBY_MESSAGE);
	
	pthread_create(tcpThread, NULL, sendWaitLobbyMessage, (void*) &waitLobbyMessage);
	pthread_join(*tcpThread, NULL);
	
	int subFont = 30;
	
	BeginDrawing();
	ClearBackground(DARKBLUE);
	DrawText("Waiting for host to start the game...", (SCREEN_WIDTH - MeasureText("Waiting for host to start the game...", subFont)) / 2.f, 20, subFont, WHITE);
	EndDrawing();
}

void handleGameOver(Game state) {
	BeginDrawing();
		ClearBackground(DARKBLUE);
		
		DrawText("Game over!", (SCREEN_WIDTH - MeasureText("Game over!", 50)) / 2.f, 20, 50, WHITE);
		DrawText(TextFormat("Player %d wins!!!", state.winner + 1), (SCREEN_WIDTH - MeasureText(TextFormat("Player %d wins!!!", state.winner + 1), 70)) / 2.f, 70, 70, state.clients[state.winner].playerColors[state.winner]);
	
	EndDrawing();
}

void joinLobby(Game* state, pthread_t* connectionThread, int clientSocket) {
	// Upon connection establishment, instantiate local game with provided fish locations
	ClientMessage connectMessage;
	connectMessage.construct(state, clientSocket, CONNECTION_MESSAGE);
	
	pthread_create(connectionThread, NULL, sendConnectionMessage, (void*) &connectMessage);
	pthread_join(*connectionThread, NULL);
}

void* startServer(void* arg) {
	printf("Started server\n");
	std::system("./server");
	
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Incorrect number of arguments: please input server IP\n");
		return 0;
	}
	
	const char* IP = argv[1];
	
	Game state;
	initRaylib(&state);
	
	pthread_t serverThread;
	pthread_t tcpThread;
	pthread_t connectionThread;
	int clientSocket = 0;
	
    while (!WindowShouldClose()) {
		
		if (state.menu) {
			
			if (IsKeyPressed(KEY_H)) {
				//pthread_create(&serverThread, NULL, startServer, NULL);
			}
			
			if (IsKeyPressed(KEY_J)) {
				state.menu = false;
				clientSocket = connectToServer(IP);
				joinLobby(&state, &connectionThread, clientSocket);
				pthread_join(connectionThread, NULL);
			}
			BeginDrawing();
				ClearBackground(DARKBLUE);
				DrawText("Fishing Frenzy", (SCREEN_WIDTH - MeasureText("Fishing Frenzy", 50)) / 2.f, 20, 50, WHITE);
				//DrawText("Press the H KEY to host a server", (SCREEN_WIDTH - MeasureText("Press the H KEY to host a server", 30)) / 2.f, 70, 30, WHITE);
				DrawText("Press the J KEY to join the server", (SCREEN_WIDTH - MeasureText("Press the J KEY to join the server", 30)) / 2.f, 100, 30, WHITE);
			EndDrawing();
			continue;
		}
		
		state.elapsed += GetFrameTime();
		if (state.lobby) {
			handleLobby(&state, &tcpThread, clientSocket);
			continue;
		}
		
		for (int i = 0; i < FISH_NUM; i++) {
			if (state.fishes[i].alive) break;
			
			if (i == FISH_NUM - 1) {
				ClientMessage overMessage;
				overMessage.construct(&state, clientSocket, OVER_MESSAGE);
				
				pthread_create(&tcpThread, NULL, sendOverMessage, (void*) &overMessage);
				pthread_join(tcpThread, NULL);
				
				state.gameOver = true;
			}
			
		}

		if (state.gameOver) {
			handleGameOver(state);
			continue;
		}
		
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && state.catching) {
			state.fishes[state.target].taken = state.selfNumber;
			state.fishLife--;
		}
		
		if (state.fishLife <= 0 && state.catching && state.fishes[state.target].alive) {			
			state.catching = false;
			state.fishes[state.target].alive = false;
			state.score++;
			state.fishLife = 10;
			state.catchTimer = 0.f;
			state.clients[state.selfNumber].lure = -1;
			
			FishMessage terminateMessage;
			terminateMessage.construct(&state, clientSocket, CATCH_TERMINATION_MESSAGE, state.selfNumber, state.target, true);
			
			pthread_create(&tcpThread, NULL, sendTerminateMessage, (void*) &terminateMessage);
			pthread_join(tcpThread, NULL);
		}
		
		if (state.catchTimer > state.catchLength && state.catching) {
			state.catching = false;
			state.fishLife = 10;
			state.catchTimer = 0.f;
			state.fishes[state.target].taken = FISH_NOT_TAKEN;
			state.clients[state.selfNumber].lure = -1;
			
			FishMessage terminateMessage;
			terminateMessage.construct(&state, clientSocket, CATCH_TERMINATION_MESSAGE, state.selfNumber, state.target, false);
			
			pthread_create(&tcpThread, NULL, sendTerminateMessage, (void*) &terminateMessage);
			pthread_join(tcpThread, NULL);
		}
		
		if (state.catchTimer > 0.f) state.catchTimer += GetFrameTime();
		
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Vector2 mp = GetMousePosition();
			for (uint16_t i = 0; i < FISH_NUM; i++) {
				if (state.fishes[i].colliding(mp.x, mp.y)) {
					if (state.catching) break;
					FishMessage catchMessage;
					catchMessage.construct(&state, clientSocket, REQUEST_CATCH_MESSAGE, state.selfNumber, i);
					
					pthread_create(&tcpThread, NULL, sendCatchMessage, (void*) &catchMessage);
					pthread_join(tcpThread, NULL);
					
					state.catchTimer = 0.01f;
					break;
				}
			}
		}
		
		// Update Message
		state.syncSelf(GetMousePosition());
		UpdateMessage updateMessage;
		updateMessage.construct(&state, clientSocket, CLIENT_UPDATE_MESSAGE, state.selfNumber, state.clients[state.selfNumber].pos.x, state.clients[state.selfNumber].pos.y, state.clients[state.selfNumber].score);
		pthread_create(&tcpThread, NULL, sendUpdateMessage, (void*) &updateMessage);
		pthread_join(tcpThread, NULL);
		
		for (int i = 0; i < FISH_NUM; i++) {
			for (int j = 0; j < state.numberOfClients; j++) {
				if (j == state.selfNumber) continue;
				
				if (state.clients[j].lure == i) state.fishes[i].taken = j;
				else state.fishes[i].taken = FISH_NOT_TAKEN;
			}
		}
		if (state.catching) state.fishes[state.target].taken = state.selfNumber;
		
        BeginDrawing();
            ClearBackground(DARKBLUE);
			
			state.syncSelf(GetMousePosition());
			
			for (int i = 0; i < state.numberOfClients; i++) state.clients[i].drawScore();
			
			if (state.catching) {
                DrawText("Fish Hooked!", (SCREEN_WIDTH - MeasureText("Fish Hooked!", 50)) / 2.f, 20, 50, WHITE);
				DrawText("Click the mouse quickly!", (SCREEN_WIDTH - MeasureText("Click the mouse quickly!", 20)) / 2.f, 70, 20, WHITE);
				DrawText(TextFormat("%.2fs left!!!", state.catchLength - state.catchTimer), (SCREEN_WIDTH - MeasureText(TextFormat("%.2fs left!!!", state.catchLength - state.catchTimer), 35)) / 2.f, 90, 35, RED);
            }
			
            state.drawFish();
			
			for (int i = 0; i < state.numberOfClients; i++) state.clients[i].drawMouse(state.cursorTexture);
						
        EndDrawing();
    }
    
	
	UnloadTexture(state.fishTextures[0]);
	UnloadTexture(state.fishTextures[1]);
	UnloadTexture(state.fishTextures[2]);
	UnloadTexture(state.cursorTexture);
	
    close(clientSocket);
    CloseWindow();
    return 0;
}
