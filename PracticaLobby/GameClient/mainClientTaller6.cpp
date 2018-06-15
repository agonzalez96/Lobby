#include <PlayerInfo.h>

#define INICIALITZACIO_PLAYER 0
#define INICIALITZACIO_VECTOR_PLAYERS 1
#define DESCONNEXIO 2
#define MOVIMENT 3
#define PING 4
#define SUMAMONEDA 5
#define WIN 6
#define SKILL_1 7
#define SKILL_2 8
#define MOVACK 9

using namespace sf;
using namespace std;

char tablero[SIZE_TABLERO];

Vector2f TransformaCoordenadaACasilla(int _x, int _y)
{
	float xCasilla = _x / LADO_CASILLA;
	float yCasilla = _y / LADO_CASILLA;
	Vector2f casilla(xCasilla, yCasilla);
	return casilla;
}

Vector2f BoardToWindows(Vector2f _position)
{
	return Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}

void receiveData(UdpSocket* socket, vector<Player*>* aPlayers, Player* player1, Coin* coin1) {
	IpAddress senderIP;
	unsigned short senderPort;
	Packet ack;
	string mess;
	int type;
	int count = 0;
	int discID, movID, scoreID, sk1ID;
	int tempX, tempY;
	int Xinter, Yinter;
	int tmpIDPacket, movIDPacket;
	int tmpScore;
	Player* player = new Player;

	while (true) {
		sf::Socket::Status status_r = socket->receive(ack, senderIP, senderPort);

		if (status_r != sf::Socket::Done)
		{
			//std::cout << "failed to receive data" << std::endl;
		}
		else if (status_r == sf::Socket::Disconnected) {
			break;
		}
		else if (status_r == Socket::Done)
		{
			//FUNCIONA
			ack >> type;
			if (type == INICIALITZACIO_PLAYER) {
				ack >> mess;
				cout << mess << endl;
				ack >> player1->ID;
				ack >> player1->posX;
				ack >> player1->posY;
				ack >> player1->tmpposX;
				ack >> player1->tmpposY;
				ack >> coin1->posX;
				ack >> coin1->posY;
				cout << "ID: " << player->ID << endl;
				player1->start = true;
			}
			else if (type == INICIALITZACIO_VECTOR_PLAYERS) {
				aPlayers->resize(0);
				ack >> count;
				for (int i = 0; i < count; i++) {
					player = new Player;
					ack >> player->ID;
					ack >> player->posX;
					ack >> player->posY;
					aPlayers->push_back(player);
					cout << "ID: " << player->ID << endl;
					cout << "posX: " << player->posX << endl;
					cout << "posY: " << player->posY << endl;
					ack >> tmpIDPacket;
					cout << tmpIDPacket << endl;
					Packet critPack;
					critPack << 3;
					critPack << player->ID;
					critPack << tmpIDPacket;
					socket->send(critPack, "localhost", 50000);
				}
			}


			else if (type == DESCONNEXIO) {
				ack >> discID;
				for (int i = 0; i < aPlayers->size(); i++) {
					if (aPlayers->at(i)->ID == discID) {
						aPlayers->erase(aPlayers->begin() + i);
						cout << "ID erased: " << aPlayers->at(i)->ID << endl;
					}
				}
			}
			else if (type == MOVIMENT) {
				ack >> movID;
				ack >> tempX;
				ack >> tempY;
				for (int i = 0; i < aPlayers->size(); i++) {
					if (aPlayers->at(i)->ID == movID) {
						if (aPlayers->at(i)->posX != player1->tmpposX || aPlayers->at(i)->posY != player1->tmpposY) {
							aPlayers->at(i)->prevX = aPlayers->at(i)->posX;
							aPlayers->at(i)->prevY = aPlayers->at(i)->posY;
							aPlayers->at(i)->tmpposX = tempX;
							aPlayers->at(i)->tmpposY = tempY;
							/*aPlayers->at(i)->posX = tempX;
							aPlayers->at(i)->posY = tempY;*/
						}
					}
				}
			}
			else if (type == PING) {
				Packet ping;
				ping << 4;
				ping << player->ID;
				socket->send(ping, "localhost", 50000);
			}
			else if (type == SUMAMONEDA) {
				ack >> coin1->posX;
				ack >> coin1->posY;
				ack >> scoreID;
				ack >> tmpScore;
				player1->score = tmpScore;
				for (int i = 0; i < aPlayers->size(); i++) {
					if (aPlayers->at(i)->ID == scoreID) {
						aPlayers->at(i)->score = player1->score;
						cout << "ID: " << aPlayers->at(i)->ID << " Score: " << aPlayers->at(i)->score << endl;
					}
				}
			}
			else if (type == WIN) {
				ack >> mess;
				cout << mess;
				player1->win = true;
			}
			else if (type == SKILL_1) {
				ack >> coin1->posX;
				ack >> coin1->posY;
			}
			else if (type == SKILL_2) {
				ack >> sk1ID;
				ack >> tempX;
				ack >> tempY;
				for (int i = 0; i < aPlayers->size(); i++) {
					if (aPlayers->at(i)->ID == sk1ID) {
						aPlayers->at(i)->posX = tempX;
						aPlayers->at(i)->posY = tempY;
						player1->posX = tempX;
						player1->posY = tempY;
					}
				}
			}
			else if (type == MOVACK) {
				ack >> movIDPacket;
				ack >> tempX;
				ack >> tempY;
				player1->listAccum.erase(movIDPacket);
				Packet mov;
				int sendType = 2;
				mov << sendType;
				mov << player1->ID;
				mov << tempX;
				mov << tempY;
				socket->send(mov, "localhost", 50000);
			}
		}
	}
}

int main()
{
	char print;
	UdpSocket socket;
	Packet conn, disc;
	Player* player = new Player;
	vector<Player*> aPlayers;
	Coin* coin = new Coin;
	int movIDPacket = 0;	

	int type = 0;
	conn << type;
	conn << "Holi";


	float secondsPassed = 0.0f;
	clock_t startTime = clock();

	float millisPassed = 0.0f;
	clock_t msgListStartTime = clock();

	float millisPassed2 = 0.0f;
	clock_t startTime2 = clock();

	thread t1(&receiveData, &socket, &aPlayers, player, coin);

	Socket::Status status = socket.send(conn, "localhost", 50000);
	while (player->start != true) {
		if (secondsPassed > 1.0f) {
			status = socket.send(conn, "localhost", 50000);
			if (status != Socket::Done) {

			}
			else {

			}
			cout << "holi";
			startTime = clock();
		}
		secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
	}

	sf::Vector2f casillaOrigen, casillaDestino;

	sf::RenderWindow window(sf::VideoMode(640, 640), "UDP");

	while (window.isOpen() || !player->win)
	{
		sf::Event event;

		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				type = 1;
				disc << type;
				disc << player->ID;
				socket.send(disc, "localhost", 50000);
				break;

			case sf::Event::KeyPressed:
				if (Keyboard::isKeyPressed(Keyboard::A)) {
					if (player->posX > 0 && player->tmpposX > 0) {
						player->tmpposX -= 5;

						//Prediccio
						for (int i = 0; i < aPlayers.size(); i++) {
							if (player->ID == aPlayers[i]->ID) {
								aPlayers[i]->posX = player->tmpposX;
							}
						}
						break;
					}
				}

				//Right
				if (Keyboard::isKeyPressed(Keyboard::D)) {
					if (player->posX < 587 && player->tmpposX < 587) {
						player->tmpposX += 5;

						//Prediccio
						for (int i = 0; i < aPlayers.size(); i++) {
							if (player->ID == aPlayers[i]->ID) {
								aPlayers[i]->posX = player->tmpposX;
							}
						}
						break;
					}
				}

				//Up
				if (Keyboard::isKeyPressed(Keyboard::W)) {
					if (player->posY > 0 && player->tmpposY > 0) {
						player->tmpposY -= 5;

						//Prediccio
						for (int i = 0; i < aPlayers.size(); i++) {
							if (player->ID == aPlayers[i]->ID) {
								aPlayers[i]->posY = player->tmpposY;
							}
						}
						break;
					}
				}

				//Down
				if (Keyboard::isKeyPressed(Keyboard::S)) {
					if (player->posY < 587 && player->tmpposY < 587) {
						player->tmpposY += 5;

						//Prediccio
						for (int i = 0; i < aPlayers.size(); i++) {
							if (player->ID == aPlayers[i]->ID) {
								aPlayers[i]->posY = player->tmpposY;
							}
						}
						break;
					}

				}
				if (Keyboard::isKeyPressed(Keyboard::E)) {
					if (player->sk1Used == false) {
						Packet sk;
						type = 5;
						sk << type;
						socket.send(sk, "localhost", 50000);
						player->sk1Used = true;
						break;
					}

				}
				if (Keyboard::isKeyPressed(Keyboard::Q)) {
					if (player->sk2Used == false) {
						Packet sk;
						type = 6;
						sk << type;
						sk << player->ID;
						socket.send(sk, "localhost", 50000);
						player->sk2Used = true;
						break;
					}

				}
				else break;

			default:
				break;

			}
		}

		//Interpolació
		if (millisPassed2 > 100) {
			for (int i = 0; i < aPlayers.size(); i++) {
				if (aPlayers[i]->ID != player->ID) {
					int dX = aPlayers[i]->tmpposX - aPlayers[i]->prevX;
					int dY = aPlayers[i]->tmpposY - aPlayers[i]->prevY;
						aPlayers[i]->posX += int(dX/5);
						aPlayers[i]->posY += int(dY/5);
				}
			}

			startTime2 = clock();
		}
		

		if (millisPassed > 100) {
			Packet mov;
			type = 7;
			mov << type;
			mov << movIDPacket;
			mov << player->ID;
			mov << player->tmpposX;
			mov << player->tmpposY;
			//cout << "PosX: " << player->posX << endl;
			//cout << "PosY: " << player->posY << endl;
			socket.send(mov, "localhost", 50000);
			player->listAccum[movIDPacket] = mov;
			movIDPacket++;
			msgListStartTime = clock();
		}

		//Interpolació

		

		window.clear();

		//Draw Players
		CircleShape plCircle(RADIO_AVATAR);

		for (int i = 0; i < aPlayers.size(); ++i) {
			//	plCircle.setPosition(BoardToWindows(Vector2f(aPlayers[i]->posX, aPlayers[i]->posY)));
			plCircle.setPosition(Vector2f(aPlayers[i]->posX, aPlayers[i]->posY));
			if (aPlayers[i]->ID == player->ID) plCircle.setFillColor(sf::Color(255, 0, 0, 255));
			else plCircle.setFillColor(sf::Color(0, 0, 255, 255));
			window.draw(plCircle);
		}

		//DrawCoin
		CircleShape coinCircle(RADIO_COIN);
		coinCircle.setPosition(Vector2f(coin->posX, coin->posY));
		coinCircle.setFillColor(sf::Color(255, 255, 0, 255));
		window.draw(coinCircle);

		window.display();

		secondsPassed = (clock() - startTime) / CLOCKS_PER_SEC;
		millisPassed = (clock() - msgListStartTime);
		millisPassed2 = (clock() - startTime2);
	}
	t1.join();
	return 0;
}