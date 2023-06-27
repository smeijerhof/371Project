debug:
	g++ -o dbug src/*.cpp -O1 -Wall -Wno-missing-braces -I include/ -L lib/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

linux:
	g++ -o game src/*.cpp -O1 -w -Wno-missing-braces -I include/ -L lib/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

windows:
	g++ -o game src/*.cpp -O1 -w -mwindows -Wno-missing-braces -I include/ -L lib/ lib/libraylibwindows.a -lopengl32 -lgdi32 -lwinmm