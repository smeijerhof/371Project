debug:
	g++ src/*.cpp -o bin/debug -O1 -Wall -Wno-missing-braces -Wno-unused-variable -Wno-unused-result -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

build:
	g++ src/*.cpp -o bin/frenzy -O3 -Wno-missing-braces -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

host:
	g++ server/*.cpp -o bin/server -I include/ -lpthread

kill:
	kill -9 $(shell lsof -t -i:8080)

clean:
	rm -f ./bin/debug ./bin/game ./bin/server
