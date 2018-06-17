#pragma once
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <map>


#define MAX_MENSAJES 30

#define SIZE_TABLERO 100
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define RADIO_COIN 15.f
#define OFFSET_AVATAR 5

using namespace sf;
using namespace std;

struct Player {
	int posX = -1;
	int posY = -1;
	int tmpposX = -1;
	int tmpposY = -1;
	int ID = -1;
	IpAddress senderIP;
	unsigned short senderPort;
	vector<Packet> aMessages;
	map<int, Packet> ackList;
	int IDPacket = 1;
	int ping;
	int score = -1;
	bool win = false;
	bool sk1Used = false;
	bool sk2Used = false;
	bool start = false;
	map<int, Packet> listAccum;
	map<int, Packet> movAccum;
	int prevX = -1;
	int prevY = -1;
	int worldID = -1;
};

struct Coin {
	int posX = -1;
	int posY = -1;
};


class World {
public:
	vector<Player*> aPlayers;
	int tempID = 0;
	bool win = false;
	int accumID = 0;
	Coin* coin = new Coin;
	bool full = false;
	int maxPlayers;
	int maxCoins;
	int worldid;

	World();
	~World();

};