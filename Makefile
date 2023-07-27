debug:
	g++ main.cpp -o bin/debug -O1 -Wall -Wno-missing-braces -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

build:
	g++ main.cpp -o bin/game -O3 -Wno-missing-braces -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

server:
	g++ TCPServer.cpp -o bin/server
