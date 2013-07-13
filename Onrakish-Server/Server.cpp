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
#include "Message.h"

using namespace TCLAP;
using namespace std;

/* Common variables */
bool quit = false;

//Server variables
sf::TcpListener listener;
sf::SocketSelector selector;
sf::Mutex globalMutex;

//Game variables
NetworkGameSession gameSession;

//Other
static Log logger = Log();

int getRandomInt(int max, int min)
{
	int number = min + (rand() % (int)(max - min + 1));
	return number;
}

void handleClient(ClientInfo *client)
{
	bool disconnected = false;
	while (!disconnected)
	{
		if (selector.isReady(*client->socket))
		{
			sf::Packet packet;
			sf::Packet sendPacket;
			if (client->socket->receive(packet) == sf::Socket::Done)
			{
				int msg;
				if (packet >> msg)
				{
					switch (msg)
					{
					case MESSAGE_DISCONNECT:
						disconnected = true;
						puts(logger.log("Client disconnects: " + client->socket->getRemoteAddress().toString()).c_str());
						selector.remove(*client->socket);
						gameSession.clients.remove(client);
						break;
					case MESSAGE_GAME_SESSION_REQUEST:
						sendPacket << gameSession;
						client->socket->send(sendPacket);
						break;
					default:
						break;
					}
				}
			}
		}
	}
}

void handleClients(void)
{
	selector.add(listener);
	while(!quit) 
	{
		if (selector.wait())
		{
			if (selector.isReady(listener))
			{
				sf::Packet sendPacket;
				sf::Packet receivePacket;

				ClientInfo newClient;
				newClient.socket = new sf::TcpSocket;
				if (listener.accept(*newClient.socket) == sf::Socket::Done)
				{
					sendPacket << MESSAGE_CLIENT_INFO_REQUEST;
					newClient.socket->send(sendPacket);

					newClient.socket->receive(receivePacket);
					ClientInfo receivedInfo;
					receivePacket >> receivedInfo;
					newClient.name = receivedInfo.name;

					gameSession.clients.push_back(&newClient);
					selector.add(*newClient.socket);

					sf::Thread* thread = 0;
					thread = new sf::Thread(&handleClient, &newClient);
					thread->launch();

					puts(logger.log("Connected client: " + newClient.socket->getRemoteAddress().toString()).c_str());
				}
				else
				{
					delete newClient.socket;
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
		ValueArg<string> mapArg("m","map","Map file name",true,"tileset.tmx","string");
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
	
	//Clean console window and disable SFML output
	system("cls");
	sf::err().rdbuf(NULL);

	puts(logger.log("Created game with name \"" + gameSession.gameSessionName + "\"").c_str());
	puts(logger.log("Listening to: " + to_string(PORT)).c_str());
	listener.listen(PORT);

	sf::Thread* thread = 0;
	thread = new sf::Thread(&handleClients);
	thread->launch();

	while (!quit)
	{
		string command;
		cin >> command;
		if(command == "start")
		{
			globalMutex.lock();
			for(std::list<ClientInfo*>::iterator it = gameSession.clients.begin(); it != gameSession.clients.end();	++it)
			{
				
				ClientInfo& client = **it;
				//puts(logger.log(client.name).c_str());
				puts(client.socket->getRemoteAddress().toString().c_str());
			}
			globalMutex.unlock();
		}
	}
	if(thread)
	{
		thread->wait();
		delete thread;
	}
	return 0;
}