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


using namespace sf;
using namespace std;


int main()
{

	srand(time(NULL));

	UdpSocket socket;

	Packet conn;
	string mes;
	vector<Player*> aPlayers;
	int tempID = 0;
	int sendType = 0;
	int recType = -1;
	int discID, movID, movX, movY;
	int tmpIDPacket, tmpID;
	float dist;
	bool win = false;
	int movIDPacket;
	int accumID = 0;
	bool add = true;

	Coin* coin = new Coin;
	coin->posX = rand() % 587;
	coin->posY = rand() % 587;

	float secondsPassed = 0.0f;
	float millisPassed = 0.0f;
	float millisPassed2 = 0.0f;

	Socket::Status status = socket.bind(50000);

	clock_t startTime = clock();
	clock_t startTime2 = clock();
	clock_t msgListStartTime = clock();


	socket.setBlocking(false);
	while (win != true) {
		Packet welcome;
		sendType = 0;
		welcome << sendType;
		welcome << "Welcome";
		Player* player = new Player;
		status = socket.receive(conn, player->senderIP, player->senderPort);
		if (status == sf::Socket::Done) {
			conn >> recType;

			//Entra nou jugador
			if (recType == ENTRA_NOU_JUGADOR) {
				for (int i = 0; i < aPlayers.size(); i++) {
						if (player->senderPort == aPlayers[i]->senderPort) {
							welcome << aPlayers[i]->ID;
							welcome << aPlayers[i]->posX;
							welcome << aPlayers[i]->posY;
							welcome << coin->posX;
							welcome << coin->posY;
							socket.send(welcome, player->senderIP, player->senderPort);
							add = false;
						}
						else {
							add = true;
						}
					}
				if (add == true) {
					tempID++;
					player->ID = tempID;
					player->posX = rand() % 587;
					player->posY = rand() % 587;

					for (int i = 0; i < aPlayers.size(); i++) { //Si la posicio esta ocupada
						if (aPlayers[i]->posX == player->posX && aPlayers[i]->posY == player->posY) {
							player->posX += 100;
							player->posY += 100;
						}
					}
					welcome << player->ID;
					welcome << player->posX;
					welcome << player->posY;
					welcome << coin->posX;
					welcome << coin->posY;
					socket.send(welcome, player->senderIP, player->senderPort);
					aPlayers.push_back(player);
				 }
				//Creem packet amb la posicio del jugador i fem un send
				conn >> mes;
				

				cout << "IP: " << player->senderIP << endl;
				cout << "Port: " << player->senderPort << endl;
				cout << "mensaje: " << mes << endl;

				//Fem un packet amb la info de tots els jugadors
				Packet newInfo;
				sendType = 1;
				newInfo << sendType;
				newInfo << player->ID;  //Per saber quanta gent hi ha

				for (int j = 0; j < aPlayers.size(); j++) {
					newInfo << aPlayers[j]->ID;
					newInfo << aPlayers[j]->posX;
					newInfo << aPlayers[j]->posY;
					newInfo << aPlayers[j]->IDPacket;
					aPlayers[j]->ackList[aPlayers[j]->IDPacket] = newInfo;
					aPlayers[j]->IDPacket++;
				}

				//Enviem el packet
				for (int i = 0; i < aPlayers.size(); i++) {
					socket.send(newInfo, aPlayers[i]->senderIP, aPlayers[i]->senderPort);


				}
				recType = -1;
			}

			//Marxa un jugador
			else if (recType == DESCONNEXIO) {
				conn >> discID;
				for (int i = 0; i < aPlayers.size(); i++) {
					if (discID == aPlayers[i]->ID) {
						aPlayers.erase(aPlayers.begin() + i);
					}
				}
				Packet newInfo;
				sendType = 2;
				newInfo << sendType;
				newInfo << discID;
				for (int i = 0; i < aPlayers.size(); i++) {
					socket.send(newInfo, aPlayers[i]->senderIP, aPlayers[i]->senderPort);
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
						aPlayers[i]->movAccum[accumID] = newInfo;
						accumID++;
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
				coin->posX = rand() % 587;
				coin->posY = rand() % 587;
				Packet newInfo;
				sendType = 7;
				newInfo << sendType;
				newInfo << coin->posX;
				newInfo << coin->posY;
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
		for (int i = 0; i < aPlayers.size(); i++) {
			dist = sqrt(pow((coin->posX - aPlayers[i]->posX), 2) + pow((coin->posY - aPlayers[i]->posY), 2));
			if (dist <= RADIO_COIN + RADIO_AVATAR) {
				Packet newInfo;
				coin->posX = rand() % 587;
				coin->posY = rand() % 587;
				aPlayers[i]->score++;
				sendType = 5;
				newInfo << sendType;
				newInfo << coin->posX;
				newInfo << coin->posY;
				newInfo << aPlayers[i]->ID;
				newInfo << aPlayers[i]->score;
				for (int j = 0; j < aPlayers.size(); j++) {
					socket.send(newInfo, aPlayers[j]->senderIP, aPlayers[j]->senderPort);
				}
			}
		}

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
				win = true;
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