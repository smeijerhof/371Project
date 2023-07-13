windowsdebug:
	g++ main.cpp -o bin/debug.exe -O1 -Wall -Wno-missing-braces -I include/ -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm

windows:
	g++ main.cpp -o bin/game.exe -mwindows -O3 -Wno-missing-braces -I include/ -Llib/ -lraylib -lopengl32 -lgdi32 -lwinmm

nixdebug:
	g++ main.cpp -o bin/debug -O1 -Wall -Wno-missing-braces -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11

nix:
	g++ main.cpp -o bin/game -O3 -Wno-missing-braces -I include/ -Llib/ -lnixraylib -lGL -lm -lpthread -ldl -lrt -lX11
