#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <mutex>

#include "include/game.h"

// Define the port number to listen for connections
#define PORT 8080

// Global mutex to ensure thread-safe access to the game state
std::mutex mtx;

// Function to handle client connections in separate threads
void* clientHandler(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[256] = {0};

    // Create the game instance
    Game game;
    game.start();

    // Send the client's ID to the client
    unsigned char clientID = game.connectClient();
    unsigned char idBuffer[2] = { 'I', clientID };
    write(clientSocket, idBuffer, sizeof(idBuffer));

    // Calculate the size of the message buffer based on the number of fish positions
    size_t bufferSize = 1 + FISH_NUM * 4;
    unsigned char* msgBuffer = new unsigned char[bufferSize];
    msgBuffer[0] = 0; // Points

    while (true) {
        // Receive data from the client (mouse positions in serialized form)
        int bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead <= 0) {
            // Client disconnected or error occurred
            close(clientSocket);
            game.disconnectClient(clientID);
            break;
        }

        // Extract mouse positions (mouseX = buffer[2], mouseY = buffer[3])
        int mouseX = buffer[2];
        int mouseY = buffer[3];

        // Update the game state using the extracted mouse positions
        mtx.lock();
        game.updateGameState(clientID, mouseX, mouseY);
        mtx.unlock();

        // Fill the message buffer with the game fish positions
        mtx.lock();
        for (int i = 0; i < FISH_NUM; i++) {
            int x = (int)game.positions[i].x;
            int y = (int)game.positions[i].y;

            msgBuffer[(i * 4) + 1] = (x >> 8) & 0xFF;
            msgBuffer[(i * 4) + 2] = x & 0xFF;
            msgBuffer[(i * 4) + 3] = (y >> 8) & 0xFF;
            msgBuffer[(i * 4) + 4] = y & 0xFF;
        }
        mtx.unlock();

        // Send the message containing the game fish positions to the client
        write(clientSocket, msgBuffer, bufferSize);
    }

    // Clean up the dynamically allocated buffer
    delete[] msgBuffer;

    return NULL;
}


int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    // Set socket options to allow multiple connections
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed." << std::endl;
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        return 1;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 4) < 0) {
        std::cerr << "Listen failed." << std::endl;
        return 1;
    }

    printf("Server started on port %d\n", PORT);

    std::vector<std::thread> clientThreads;

    // Accept and handle client connections in separate threads
    while ((clientSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen))) {
        // Create a new thread to handle the client
        std::thread clientThread(clientHandler, (void*)&clientSocket);

        // Detach the thread to allow it to run independently
        clientThread.detach();
    }

    return 0;
}
