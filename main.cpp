#include <iostream>
#include <pthread.h>
#include <mutex>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <vector>

#include "include/raylib.h"
#include "include/game.h"

// Define the server IP address and port number
#define SERVER_IP "127.0.0.1"
#define PORT 8080

// Define the game time interval to send mouse position to the server (in milliseconds)
#define TIME_INTERVAL_MS 100

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

// Global variables to store mouse position and actor state
Vector2 mousePos = { 0 };
bool leftMouseButtonPressed = false;
bool zKeyPressed = false;
bool sendMousePositionRunning = false;

// // Function to handle keyboard input in a separate thread
// void* keyboardInputHandler(void* args) {
//     while (true) {
//         int ch = nonBlockingRead();
//         if (ch != EOF) {
//             if (ch == 'z') {
//                 mtxZKeyPressed.lock();
//                 zKeyPressed = true;
//                 mtxZKeyPressed.unlock();
//             }
//         }
//     }
// }

// Create a struct to hold both Game and sockfd
struct SendMousePositionArgs {
    Game* game;
    int sockfd;
};

// Function to send mouse position and client ID to the server on a new thread
void* sendMousePositionToServer(void* args) {
    SendMousePositionArgs* sendArgs = static_cast<SendMousePositionArgs*>(args);
    Game* game = sendArgs->game;
    int sockfd = sendArgs->sockfd;
    int clientID = game->clientID;

    while (sendMousePositionRunning) { // Use the flag to control the loop
        usleep(TIME_INTERVAL_MS * 1000); // Sleep for the specified time interval

        mtxMousePos.lock();
        Vector2 mp = mousePos;
        mtxMousePos.unlock();

        // Send mouse position and client ID to the server
        unsigned char buffer[5] = { 0 };
        buffer[0] = 'M';
        buffer[1] = clientID; // Send the client's unique ID to the server
        buffer[2] = (unsigned char)(mp.x);
        buffer[3] = (unsigned char)(mp.y);
        int bytesWritten = write(sockfd, buffer, sizeof(buffer));
        if (bytesWritten == -1) {
            std::cerr << "Error occured while sending data to server." << std::endl;
        } else if ((long unsigned int) bytesWritten < sizeof(buffer)) {
            std::cerr << "Not all data was sent." << std::endl;
        }
    }
    return NULL;
}

int main() {
    // Connect to the server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid server address." << std::endl;
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        return 1;
    }

    // Create the game instance
    Game* game = new Game();
    game->start();

    // // Start keyboard input handler thread
    // pthread_t keyboardThread;
    // pthread_create(&keyboardThread, NULL, keyboardInputHandler, NULL);

    // Create the struct to hold Game and sockfd
    SendMousePositionArgs* sendArgs = new SendMousePositionArgs();
    sendArgs->game = game;
    sendArgs->sockfd = sockfd;

    // Start sending mouse position to the server thread
    pthread_t sendMousePositionThread;
    pthread_create(&sendMousePositionThread, NULL, sendMousePositionToServer, sendArgs);

    // Create the window and set target FPS
    InitWindow(game->screenWidth, game->screenHeight, "TCP Game Client");
    SetTargetFPS(60);

    // Wait until the client receives its unique ID from the server
    while (true) {
        unsigned char buffer[2] = { 0 };
        int bytesRead = read(sockfd, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            // Server disconnected or error occurred
            close(sockfd);
            return 1;
        }

        // Check the message type
        char messageType = buffer[0];

        if (messageType == 'I') {
            // Server sent the client ID, extract and store it
            int clientID = buffer[1];

            // Start sending mouse position to the server thread with the client ID
            sendMousePositionRunning = true; // Set the flag to true
            game->clientID = clientID;
            pthread_create(&sendMousePositionThread, NULL, sendMousePositionToServer, game);

            // Add actors to the vector based on the client ID
            mtxGame.lock();
            game->actorNum = clientID;
            mtxGame.unlock();
            break;
        }
    }

    while (!WindowShouldClose()) {
        // Update mouse position
        mtxMousePos.lock();
        mousePos = GetMousePosition();
        leftMouseButtonPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        mtxMousePos.unlock();

        // Check for fish catching condition and update game state
        if (leftMouseButtonPressed) {
            mtxMousePos.lock();
            mtxGame.lock();
            for (int i = 0; i < FISH_NUM; i++) {
                if (mousePos.x > game->fishes[i].position.x && mousePos.x < game->fishes[i].position.x + Fish::size.x &&
                    mousePos.y > game->fishes[i].position.y && mousePos.y < game->fishes[i].position.y + Fish::size.y &&
                    game->fishes[i].alive) {
                    // Fish caught, update actor state and fish color
                    game->actors[game->actorNum].catching = true;
                    game->actors[game->actorNum].target = i;
                    game->fishes[i].color = GREEN;
                }
            }
            mtxGame.unlock();
            mtxMousePos.unlock();
        }

        // Check for 'z' key press and update fish health and game state
        mtxZKeyPressed.lock();
        if (IsKeyPressed(KEY_Z)) {
            mtxZKeyPressed.unlock();
            mtxGame.lock();
            if (game->actors[game->actorNum].catching) {
                int targetFish = game->actors[game->actorNum].target;
                if (game->fishes[targetFish].health > 0 && game->fishes[targetFish].alive) {
                    game->fishes[targetFish].health--;
                }
                if (game->fishes[targetFish].health <= 0 && game->fishes[targetFish].alive) {
                    // Fish caught successfully, update fish and actor state
                    game->fishes[targetFish].alive = false;
                    game->actors[game->actorNum].catching = false;
                    game->actors[game->actorNum].points++;
                    game->numFishAlive--;
                }
            }
            mtxGame.unlock();
        } else {
            mtxZKeyPressed.unlock();
        }

        // Clear the screen and draw the game elements
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(20, 20);
        DrawText(TextFormat("Points = %d", game->actors[game->actorNum].points), 20, 40, 20, WHITE);
        DrawText(TextFormat("Time = %.1f", game->elapsed), 20, 60, 20, WHITE);

        mtxGame.lock();
        game->draw();
        if (game->actors[game->actorNum].catching) {
            DrawText("Fish Hooked!", (game->screenWidth - MeasureText("Fish Hooked!", 50)) / 2.f, 20, 50, WHITE);
            DrawText("Press Z!", (game->screenWidth - MeasureText("Press Z!", 35)) / 2.f, 70, 35, WHITE);
        }
        mtxGame.unlock();

        EndDrawing();
    }

    // Clean up and close the window
    // pthread_join(keyboardThread, NULL);
    pthread_join(sendMousePositionThread, NULL);
    delete game;
    CloseWindow();

    // Close the socket
    close(sockfd);

    return 0;
}
