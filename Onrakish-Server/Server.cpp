#include <string>
#include <iostream>
#include <algorithm>
#include "tclap/CmdLine.h"

#include <SFML/Network.hpp>
#include <SFML/System.hpp>

#include <Config.h>
#include <GameMap.h>
#include <NetworkGameSession.h>
#include <Log.h>
#include "PacketTransmitter.h"

using namespace TCLAP;
using namespace std;

/* Common variables */
bool quit = false;

//Server variables
sf::UdpSocket listener;

//Game variables
NetworkGameSession gameSession;

//Other
static Log logger = Log();

int getRandomInt(int max, int min)
{
	int number = min + (rand() % (int)(max - min + 1));
	return number;
}

void handleClients(void)
{
	if (listener.bind(PORT) == sf::Socket::Done)
	{
		listener.setBlocking(false);
		while(!quit) 
		{
			sf::IpAddress clientAddress;
			unsigned short clientPort;

			sf::Packet receivePacket, gameSessionPacket, portPacket;

			if (listener.receive(receivePacket, clientAddress, clientPort) == sf::Socket::Done)
			{
				ClientInfo newClient;
				if (receivePacket >> newClient)
				{
					newClient.socket = new sf::UdpSocket;

					puts(logger.log("Client trying to connect: " + clientAddress.toString()).c_str());

					gameSessionPacket << gameSession;
					newClient.socket->send(gameSessionPacket, clientAddress, clientPort);

					short unsigned int randomPort = getRandomInt(7001, 7100);

					//Disable error message
					sf::err().rdbuf(NULL);

					do {
						randomPort = getRandomInt(7001, 7100);
					} while(!newClient.socket->bind(randomPort));

					puts(logger.log("Ports are: " + to_string(randomPort) + "/" + to_string(clientPort)).c_str());

					newClient.sendPort = clientPort;
					newClient.receivePort = randomPort;
					newClient.ipAddress = clientAddress;

					sf::Packet packet;
					packet << randomPort;

					newClient.socket->send(packet, clientAddress, clientPort);

					newClient.socket->setBlocking(false);

					gameSession.clients.push_back(&newClient);
				}
			}

			for(std::list<ClientInfo*>::iterator it = gameSession.clients.begin(); it != gameSession.clients.end();)
			{
				ClientInfo& client = **it;
				sf::Packet receive;
				if (client.socket->receive(receive, client.ipAddress, client.receivePort) == sf::Socket::Status::Done)
				{
					string request;
					if (receive >> request)
					{
						if (request == "gameStateRequest")
						{
							sf::Packet gameStatePacket;
							gameStatePacket << gameSession.state;
							client.socket->send(gameStatePacket, client.ipAddress, client.sendPort);
							puts(logger.log("Game state sended to: " + client.ipAddress.toString() + ":" + to_string(client.sendPort)).c_str());
						}
						if (request == "disconnect")
						{
							client.socket->unbind();
							it = gameSession.clients.erase(it);
							puts(logger.log("Client disconnects: " + client.ipAddress.toString() + "...").c_str());
						}
						else
						{
							++it;
						}
					}
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	//Command line arguments parse
	try {  
		CmdLine cmd("Onrakish-Server", ' ', "0.1");

		//Arguments list
		ValueArg<string> portArg("p","port","Server port",true,"12312","string");
		ValueArg<string> mapArg("m","map","Map file name",true,"isometric_grass_and_water","string");
		ValueArg<string> gameSessionNameArg("n","name","Game session name",true,"Default","string");

		cmd.add( portArg );
		cmd.add( mapArg );
		cmd.add( gameSessionNameArg );
		cmd.parse( argc, argv );

		//Static variables assign
		PORT = (unsigned short)(atoi(portArg.getValue().c_str()));

		//Game variables assign
		gameSession.gameSessionName = gameSessionNameArg.getValue();
		gameSession.mapFileName = mapArg.getValue();
		gameSession.state = 0;
	} catch (ArgException &e)
	{ cerr << "error: " << e.error() << " for arg " << e.argId() << endl; }
	
	//Clean console window
	system("cls");

	puts(logger.log("Created game with name \"" + gameSession.gameSessionName + "\"").c_str());

	sf::Thread* thread = 0;
	thread = new sf::Thread(&handleClients);
	thread->launch();

	while (!quit)
	{
		string command;
		cin >> command;
		if(command == "start" && gameSession.clients.size() >= 0)
		{
			for(std::list<ClientInfo*>::iterator it = gameSession.clients.begin(); it != gameSession.clients.end();	++it)
			{
				ClientInfo& client = **it;
			}
		}
	}
	if(thread)
	{
		thread->wait();
		delete thread;
	}
	return 0;
}