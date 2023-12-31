# Fishing Frenzy

Made by
- John Jamora
- Julian Martin
- Sebastian Meijerhof
- Japneet Sason
- Angela Yung

## Description

Fishing Frenzy is a LAN multiplayer game where players compete to catch the most fish. 
Players can attempt to catch fish by clicking on them, but each fish can only be attempted by one player at a time.
Players must perform an action within a time limit to succesfully catch a fish. If they did succeed, they gain a point.
The player with the most points when there are no fish remaining win the game.

## Installation

Go to [release](https://github.com/smeijerhof/371Project/releases/tag/release) and install the zip file.

## Running the Game

*Note: game only runs on GNU Linux*

Download `FishingFrenzy.zip` into your directory of choice. Extract its contents using your file manager or with
```
unzip FishingFrenzy.zip
```
You can start a server by opening a terminal in `FishingFrenzy/` and running
```
./server
```
Then run the exectuable by opening a terminal in `FishingFrenzy/` and running
```
./frenzy {IP ADDRESS}
```

Where {IP ADDRESS} is the IP address of the machine running the server.

Fishing Frenzy is confirmed to work on Debian-based systems. We reccomend using Ubuntu 20.04 LTS.

## Instructions

You can start a server by opening a terminal in `FishingFrenzy/` and running
```
./server
```

Before that, you should run
```
hostname -I
```
To find out the server machine's IP address. This address is needed as an argument in running the game executable.

After and only after a server has been started, each player, including the one who started the server, need to run the game executable with
```
./frenzy {IP ADDRESS}
```

Where {IP ADDRESS} is the IP address of the machine running the server. Note that the machine running the server can use "localhost" as the argument, instead of an IP address.

Once in the game, players can press the j key to join the game.
The first player to join is the host. The host can start the game by pressing the z key, after which a short timer will start before actually commencing the game. The host does not need to be the same user running the server.

All of this information is also found within the game.
