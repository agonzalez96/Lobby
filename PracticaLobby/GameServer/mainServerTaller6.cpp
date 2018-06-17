#include <PlayerInfo.h>

#define MAX_MENSAJES 30

#define ENTRA_NOU_JUGADOR 0
#define DESCONNEXIO 1
#define MOVIMENT 2
#define ACK_CRITICAL_PACKET 3
#define PING_RESET 4
#define SKILL1 5
#define SKILL2 6
#define MOVACK 7
#define INIT_PLAYER 8



using namespace sf;
using namespace std;


int main()
{
	srand(time(NULL));
	map<int, World> worldManager;
	int worldID = 0;

	UdpSocket socket;

	Packet conn;
	int sendType = 0;
	int recType = -1;
	int create = -1;
	int join = -1;
	bool add = true;
	string mes;
	int discID, movID, movX, movY;	
	int tmpIDPacket, tmpID;
	float dist;
	int movIDPacket;
	int maxCoins, maxPlayers;
	int tmpIDWorld;

	float secondsPassed = 0.0f;
	float millisPassed = 0.0f;
	float millisPassed2 = 0.0f;

	Socket::Status status = socket.bind(50000);

	clock_t startTime = clock();
	clock_t startTime2 = clock();
	clock_t msgListStartTime = clock();


	socket.setBlocking(false);
	while (true) {
		Packet welcome;
		vector<Player*> aPlayers;
		sendType = 0;
		welcome << sendType;
		welcome << "Welcome";
		Player* player = new Player;
		status = socket.receive(conn, player->senderIP, player->senderPort);
		if (status == sf::Socket::Done) {
			conn >> recType;

			//Entra nou jugador
			if (recType == ENTRA_NOU_JUGADOR) {
			conn >> mes;
			cout << "IP: " << player->senderIP << endl;
			cout << "Port: " << player->senderPort << endl;
			cout << "mensaje: " << mes << endl;
			conn >> create;
			if (create == 0) {
				int tmp;
				World newWorld;
				Coin* coin = new Coin;
				coin->posX = rand() % 587;
				coin->posY = rand() % 587;
				worldManager[worldID] = newWorld;
				conn >> maxPlayers;
				conn >> maxCoins;
				worldManager[worldID].maxPlayers = maxPlayers;
				worldManager[worldID].maxCoins = maxCoins;
				worldManager[worldID].coin->posX = coin->posX;
				worldManager[worldID].coin->posY = coin->posY;
				worldManager[worldID].tempID++;
				worldManager[worldID].worldid = worldID;
				player->ID = worldManager[worldID].tempID;
				player->posX = rand() % 587;
				player->posY = rand() % 587;
				player->tmpposX = player->posX;
				player->tmpposY = player->posY;

				for (int i = 0; i < aPlayers.size(); i++) { //Si la posicio esta ocupada
					if (aPlayers[i]->posX == player->posX && aPlayers[i]->posY == player->posY) {
						player->posX += 100;
						player->posY += 100;
					}
				}
				welcome << player->ID;
				welcome << player->posX;
				welcome << player->posY;
				welcome << player->tmpposX;
				welcome << player->tmpposY;

				welcome << worldManager[worldID].coin->posX;
				welcome << worldManager[worldID].coin->posY;

				welcome << worldManager[worldID].worldid;

				socket.send(welcome, player->senderIP, player->senderPort);
				aPlayers.push_back(player);
				worldManager[worldID].aPlayers.push_back(player);

				//Fem un packet amb la info de tots els jugadors
				Packet newInfo;
				sendType = 1;
				newInfo << sendType;
				newInfo << player->ID;  //Per saber quanta gent hi ha

				for (int j = 0; j < worldManager[worldID].aPlayers.size(); j++) {
					newInfo << worldManager[worldID].aPlayers[j]->ID;
					newInfo << worldManager[worldID].aPlayers[j]->posX;
					newInfo << worldManager[worldID].aPlayers[j]->posY;
					newInfo << worldManager[worldID].worldid;
					newInfo << worldManager[worldID].aPlayers[j]->IDPacket;
					worldManager[worldID].aPlayers[j]->ackList[aPlayers[j]->IDPacket] = newInfo;
					worldManager[worldID].aPlayers[j]->IDPacket++;
				}

				//Enviem el packet
				for (int i = 0; i < aPlayers.size(); i++) {
					socket.send(newInfo, worldManager[worldID].aPlayers[i]->senderIP, worldManager[worldID].aPlayers[i]->senderPort);
				}
				worldID++;

			}
			else if (create == 1) {
				conn >> join;
				if (join == 0) {
					Packet games;
					games << 10;
					games << worldID;;
					for (map<int, World>::iterator it = worldManager.begin(); it != worldManager.end(); ++it) {
						games << it->second.worldid;
					}
					socket.send(games, player->senderIP, player->senderPort);
				}
				else if (join != 0) {
					Packet games;
					games << 10;
					conn >> maxPlayers;
					conn >> maxCoins;
					int size = 0;
					for (map<int, World>::iterator it = worldManager.begin(); it != worldManager.end(); ++it) {
						if (it->second.maxCoins == maxCoins) {
							if (it->second.maxPlayers == maxPlayers) {
								size++;
							}
						}
					}
					games << size;
					for (map<int, World>::iterator it = worldManager.begin(); it != worldManager.end(); ++it) {
						if (it->second.maxCoins == maxCoins) {
							if (it->second.maxPlayers == maxPlayers) {
								games << it->second.worldid;
							}
						}
					}
					socket.send(games, player->senderIP, player->senderPort);
				}
			}						
			recType = -1;
		}

			else if (recType == INIT_PLAYER) {
				conn >> tmpIDWorld;

				//Mirem si el jugador ja existia
				for (int i = 0; i < worldManager[tmpIDWorld].aPlayers.size(); i++) {
					if (player->senderPort == worldManager[tmpIDWorld].aPlayers[i]->senderPort) {
						welcome << aPlayers[i]->ID;
						welcome << aPlayers[i]->posX;
						welcome << aPlayers[i]->posY;
						welcome << worldManager[tmpIDWorld].coin->posX;
						welcome << worldManager[tmpIDWorld].coin->posY;
						socket.send(welcome, player->senderIP, player->senderPort);
						add = false;
					}
					else {
						add = true;
					}
				}

				//Si no existia el creem
				if (add == true) {
					//Creem la info del jugador
					worldManager[tmpIDWorld].tempID++;
					player->ID = worldManager[tmpIDWorld].tempID;
					player->posX = rand() % 587;
					player->posY = rand() % 587;
					player->tmpposX = player->posX;
					player->tmpposY = player->posY;

					for (int i = 0; i < worldManager[tmpIDWorld].aPlayers.size(); i++) { //Si la posicio esta ocupada
						if (worldManager[tmpIDWorld].aPlayers[i]->posX == player->posX && worldManager[tmpIDWorld].aPlayers[i]->posY == player->posY) {
							player->posX += 100;
							player->posY += 100;
						}
					}
					welcome << player->ID;
					welcome << player->posX;
					welcome << player->posY;

					welcome << player->tmpposX;
					welcome << player->tmpposY;

					welcome << worldManager[tmpIDWorld].coin->posX;
					welcome << worldManager[tmpIDWorld].coin->posY;

					welcome << worldManager[tmpIDWorld].worldid;

					//Enviem info i afegim als vectors
					socket.send(welcome, player->senderIP, player->senderPort);
					aPlayers.push_back(player);
					worldManager[tmpIDWorld].aPlayers.push_back(player);

					cout << "IP: " << player->senderIP << endl;
					cout << "Port: " << player->senderPort << endl;
					cout << "mensaje: " << mes << endl;

					//Fem un packet amb la info de tots els jugadors
					Packet newInfo;
					sendType = 1;
					newInfo << sendType;
					newInfo << player->ID;  //Per saber quanta gent hi ha

					for (int j = 0; j < worldManager[tmpIDWorld].aPlayers.size(); j++) {
						newInfo << worldManager[tmpIDWorld].aPlayers[j]->ID;
						newInfo << worldManager[tmpIDWorld].aPlayers[j]->posX;
						newInfo << worldManager[tmpIDWorld].aPlayers[j]->posY;
						newInfo << worldManager[tmpIDWorld].worldid;
						newInfo << worldManager[tmpIDWorld].aPlayers[j]->IDPacket;
						worldManager[tmpIDWorld].aPlayers[j]->ackList[worldManager[tmpIDWorld].aPlayers[j]->IDPacket] = newInfo;
						worldManager[tmpIDWorld].aPlayers[j]->IDPacket++;
					}

					//Enviem el packet
					for (int i = 0; i < worldManager[tmpIDWorld].aPlayers.size(); i++) {
						socket.send(newInfo, worldManager[tmpIDWorld].aPlayers[i]->senderIP, worldManager[tmpIDWorld].aPlayers[i]->senderPort);
					}
				}
			}

			//Marxa un jugador
			else if (recType == DESCONNEXIO) {
				conn >> discID;
				for (map<int, World>::iterator it = worldManager.begin(); it != worldManager.end(); ++it) {
					for (int i = 0; i < it->second.aPlayers.size(); i++) {
						if (discID == it->second.aPlayers[i]->ID) {
							it->second.aPlayers.erase(aPlayers.begin() + i);
						}
					}
					Packet newInfo;
					sendType = 2;
					newInfo << sendType;
					newInfo << discID;
					for (int i = 0; i < it->second.aPlayers.size(); i++) {
						socket.send(newInfo, it->second.aPlayers[i]->senderIP, it->second.aPlayers[i]->senderPort);
					}
				}
				recType = -1;
			}

			//Es  mou un jugador
			else if (recType == MOVIMENT) {
				conn >> movID;
				Packet newInfo;
				sendType = 3;
				newInfo << sendType;
				for (int i = 0; i < aPlayers.size(); i++) {
					if (movID == aPlayers[i]->ID) {
						conn >> movX;
						conn >> movY;
						aPlayers[i]->posX = movX;
						aPlayers[i]->posY = movY;
						newInfo << aPlayers[i]->ID;
						newInfo << aPlayers[i]->posX;
						newInfo << aPlayers[i]->posY;
						worldManager[worldID].aPlayers[i]->movAccum[worldManager[worldID].accumID] = newInfo;
						worldManager[worldID].accumID++;
					}
				}
				recType = -1;
			}
			if (recType == MOVACK) {
				conn >> movIDPacket;
				int tmpX, tmpY;
				conn >> tmpID;
				conn >> tmpX;
				conn >> tmpY;
				Packet movAck;
				sendType = 9;
				movAck << sendType;
				movAck << movIDPacket;
				
				if (tmpX > 0 && tmpX < 587 && tmpY > 0 && tmpY < 587) {
					movAck << tmpX;
					movAck << tmpY;
				}
				else {
					for (int i = 0; i < aPlayers.size(); i++) {
						if (aPlayers[i]->ID == tmpID) {
							movAck << aPlayers[i]->posX;
							movAck << aPlayers[i]->posY;
						}
					}
				}
				socket.send(movAck, player->senderIP, player->senderPort);
			}

			//Critical packet ACK
			else if (recType == ACK_CRITICAL_PACKET) {
				conn >> tmpID;
				conn >> tmpIDPacket;

				for (int i = 0; i < aPlayers.size(); i++) {
					if (aPlayers[i]->ID == tmpID) {
						aPlayers[i]->ackList.erase(tmpIDPacket);
					}
				}
				recType = -1;
			}

			//PING reset
			else if (recType == PING_RESET) {
				conn >> tmpID;
				for (int i = 0; i < aPlayers.size(); i++) {
					if (aPlayers[i]->ID == tmpID) {
						aPlayers[i]->ping = 0;
					}
				}
				recType = -1;
			}

			//Skill Move Positions
			else if (recType == SKILL1) {
				worldManager[worldID].coin->posX = rand() % 587;
				worldManager[worldID].coin->posY = rand() % 587;
				Packet newInfo;
				sendType = 7;
				newInfo << sendType;
				newInfo << worldManager[worldID].coin->posX;
				newInfo << worldManager[worldID].coin->posY;
				for (int j = 0; j < aPlayers.size(); j++) {
					socket.send(newInfo, aPlayers[j]->senderIP, aPlayers[j]->senderPort);
				}
				recType = -1;
			}

			//Skill cambiar moneda de posicion
			else if (recType == SKILL2) {
				conn >> tmpID;
				int tmpX, tmpY;
				tmpX = rand() % 587;
				tmpY = rand() % 587;
				for (int i = 0; i < aPlayers.size(); i++) {
					if (aPlayers[i]->ID == tmpID) {
						aPlayers[i]->posX = tmpX;
						aPlayers[i]->posY = tmpY;
						Packet sk;
						sendType = 8;
						sk << sendType;
						sk << aPlayers[i]->ID;
						sk << aPlayers[i]->posX;
						sk << aPlayers[i]->posY;
						for (int j = 0; j < aPlayers.size(); j++) {
							socket.send(sk, aPlayers[j]->senderIP, aPlayers[j]->senderPort);
						}
					}
				}
				recType = -1;
			}
			else {
				//Nothing
			}
		}
		else if (status == sf::Socket::NotReady) {
		}

		//Agafar monedes
		//for (int i = 0; i < aPlayers.size(); i++) {
		//	dist = sqrt(pow((worldManager[worldID].coin->posX - aPlayers[i]->posX), 2) + pow((worldManager[worldID].coin->posY - aPlayers[i]->posY), 2));
		//	if (dist <= RADIO_COIN + RADIO_AVATAR) {
		//		Packet newInfo;
		//		worldManager[worldID].coin->posX = rand() % 587;
		//		worldManager[worldID].coin->posY = rand() % 587;
		//		aPlayers[i]->score++;
		//		sendType = 5;
		//		newInfo << sendType;
		//		newInfo << worldManager[worldID].coin->posX;
		//		newInfo << worldManager[worldID].coin->posY;
		//		newInfo << aPlayers[i]->ID;
		//		newInfo << aPlayers[i]->score;
		//		for (int j = 0; j < aPlayers.size(); j++) {
		//			socket.send(newInfo, aPlayers[j]->senderIP, aPlayers[j]->senderPort);
		//		}
		//	}
		//}

		//Condicio victoria
		for (int i = 0; i < aPlayers.size(); i++) {
			if (aPlayers[i]->score == 10) {
				string victorymes;
				victorymes = "Player " + to_string(aPlayers[i]->ID) + " wins";
				Packet victory;
				sendType = 6;
				victory << sendType;
				victory << victorymes;
				for (int j = 0; j < aPlayers.size(); j++) {
					socket.send(victory, aPlayers[j]->senderIP, aPlayers[j]->senderPort);
				}
				worldManager[worldID].win = true;
			}
		}


		//Acumulacio moviment
		if (millisPassed2 > 500) {
			for (int i = 0; i < aPlayers.size(); i++) {
				if (aPlayers[i]->movAccum.size() != 0) {
					for (map<int, Packet>::reverse_iterator it = aPlayers[i]->movAccum.rbegin(); it != aPlayers[i]->movAccum.rend(); ++it) {

						int tmpX2, tmpY2, tmpID2, tmpSendType;
						it->second >> tmpSendType;
						it->second >> tmpID2;
						it->second >> tmpX2;
						it->second >> tmpY2;
						if (tmpX2 > 0 && tmpX2 < 587 && tmpY2 > 0 && tmpY2 < 587) {
							for (int j = 0; j< aPlayers.size(); j++) {
								socket.send(it->second, aPlayers[j]->senderIP, aPlayers[j]->senderPort);
							}
							aPlayers[i]->movAccum.clear();
						}
						
					}
				}
			}
			startTime2 = clock();

		}

		//Critical Pakcets 
		if (millisPassed > 500) {
			for (int i = 0; i < aPlayers.size(); i++) {
				if (aPlayers[i]->ackList.size() != 0) {
					for (map<int, Packet>::iterator it = aPlayers[i]->ackList.begin(); it != aPlayers[i]->ackList.end(); ++it) {
						socket.send(it->second, aPlayers[i]->senderIP, aPlayers[i]->senderPort);
					}
				}
			}
			msgListStartTime = clock();

		}

		//PING
		if (secondsPassed > 1) {
			startTime = clock();
			for (int i = 0; i < aPlayers.size(); i++) {
				Packet _ping;
				sendType = 4;
				_ping << sendType;
				aPlayers[i]->ping++;
				for (int i = 0; i < aPlayers.size(); i++) {
					socket.send(_ping, aPlayers[i]->senderIP, aPlayers[i]->senderPort);
				}
			}
		}

		for (int i = 0; i < aPlayers.size(); i++) {
			if (aPlayers[i]->ping >= 60) {
				Packet newInfo;
				sendType = 2;
				newInfo << sendType;
				newInfo << aPlayers[i]->ID;;
				aPlayers.erase(aPlayers.begin() + i);
				for (int i = 0; i < aPlayers.size(); i++) {
					socket.send(newInfo, aPlayers[i]->senderIP, aPlayers[i]->senderPort);
				}
			}

		}

		secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
		millisPassed = (clock() - msgListStartTime);
		millisPassed2 = (clock() - startTime2);
		//cout << to_string(millisPassed) << endl;
	}
	return 0;
}