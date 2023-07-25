// TODO: See which is necessary?
// Include references (textbook, linuxhowtos)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

#define SERVER_PORT 65501
#define BUFFER_SIZE 10
#define QUEUE_SIZE 10

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
            printf("pos: %f %f\n", pos.x,pos.y);
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

void printError(const char* message) {
    perror(message);
    exit(-1);
}

int main(int argc, char const *argv[]) {
    // Hold data read from socket
    // char buffer[BUFFER_SIZE];
    uint16_t buffer[BUFFER_SIZE];
    char response[] = "back at you";    // response message

    // specify socket to read from 
    struct sockaddr_in channel;
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    channel.sin_addr.s_addr = htonl(INADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    // return file descriptor for socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        printError("Failed to create new socket\n");

    // bind socket to supplied address, port
    int b = bind(sock, (struct sockaddr *) &channel, sizeof(channel));
    if(b < 0) 
        printError("Bind failed\n");

    seed = time(NULL);
    srand(seed);

    // Upon establishment connection, server sends back locations of all the fish in the game world
    fishes = new Fish[fishNum];
    positions = new Vector2[fishNum];
    for (int i = 0; i < fishNum; i++) {
        Vector2 pos = { 100 + rand() % (screenWidth - 200), 100 + rand() % (screenHeight - 200) };
        printf("pos: %f %f\n", pos.x,pos.y);
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

    // listen to socket for connections
    int l = listen(sock, QUEUE_SIZE);
    if(l < 0) 
        printError("Listen failed");

    // wait until connection established, then read message
    int connectionSocket = accept(sock, 0, 0);
    if(connectionSocket < 0)
        printError("Failed to accept\n");

    printf("connection established\n");

    // read data from new connection
    for(int i = 0; i < atoi(argv[1]); i++) {
        read(connectionSocket, buffer, BUFFER_SIZE);
        printf("server: ");
        // TODO: not being received correctly
        for(int j = 0; j < 3; j++)
            printf("%d ", ntohs(buffer[j]));
        printf("\n");
        // printf("%s\n", buffer);
        write(connectionSocket, buffer, BUFFER_SIZE);  

        // if(strcmp(buffer, "hello") == 0) {
        //     // send response to client
        //     write(connectionSocket, response, sizeof(response));  
        // }
    }

    // close connections
    close(connectionSocket);
    close(sock);
}

