debug:
	g++ main.cpp -o bin/debug -O1 -Wall -Wno-missing-braces -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

build:
	g++ main.cpp -o bin/game -O3 -Wno-missing-braces -g -rdynamic -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

server:
	g++ TCPServer.cpp -o bin/server -lraylib -lpthread

clean:
	rm -f ./bin/debug ./bin/game ./bin/server

kill:
	kill -9 $(shell lsof -t -i:8080)