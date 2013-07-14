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
static sf::Mutex globalMutex;

//Game variables
NetworkGameSession gameSession;

//Other
static Log logger = Log();

int getRandomInt(int max, int min)
{
	int number = min + (rand() % (int)(max - min + 1));
	return number;
}

sf::Uint64 getUniqueId()
{
	static sf::Uint64 id = 1;

	sf::Lock lock(globalMutex);
	return id++;
}

bool sendMessage(int command, ClientInfo *client)
{
	sf::Packet sendPacket;
	if (sendPacket << command)
	{
		client->socket->send(sendPacket);
		return true;
	} else return false;
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
						puts(logger.log(client->name + " disconnects: " + client->socket->getRemoteAddress().toString()).c_str());
						selector.remove(*client->socket);
						for(std::list<ClientInfo*>::iterator it = gameSession.clients->begin(); it != gameSession.clients->end();)
						{
							ClientInfo& cl = **it;
							if (cl.id == client->id)
							{
								it = gameSession.clients->erase(it);
								break;
							}
							else
							{
								++it;
							}
						}
						break;
					case MESSAGE_GAME_SESSION_REQUEST:
						sendPacket << gameSession;
						client->socket->send(sendPacket);
						break;
					case MESSAGE_END_TURN:
						if (gameSession.turnPlayerID == client->id)
						{
							puts(logger.log("Player " + client->name + ":" + to_string(client->id) + " with IP " + client->socket->getRemoteAddress().toString() + " ended his turn").c_str());
							gameSession.turnPlayerID = 2;
						}
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

				globalMutex.lock();
				ClientInfo *newClient = new ClientInfo;
				newClient->socket = new sf::TcpSocket;
				if (listener.accept(*newClient->socket) == sf::Socket::Done)
				{
					sendPacket << MESSAGE_CLIENT_INFO_REQUEST;
					newClient->socket->send(sendPacket);

					newClient->socket->receive(receivePacket);
					ClientInfo receivedInfo;
					receivePacket >> receivedInfo;
					newClient->name = receivedInfo.name;
					newClient->id = getUniqueId();

					gameSession.clients->push_back(newClient);
					selector.add(*newClient->socket);

					gameSession.clients->back()->name = receivedInfo.name;

					//gameSession.clients->sort();

					sf::Thread* thread = 0;
					thread = new sf::Thread(&handleClient, gameSession.clients->back());
					thread->launch();

					puts(logger.log("Player " + gameSession.clients->back()->name + " connected: " + newClient->socket->getRemoteAddress().toString() + ", assigned ID: " + to_string(newClient->id)).c_str());
				}
				else
				{
					delete newClient->socket;
				}
				globalMutex.unlock();
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
		if(command == "get")
		{
			globalMutex.lock();
			puts(to_string(gameSession.clients->size()).c_str());
			for(std::list<ClientInfo*>::iterator it = gameSession.clients->begin(); it != gameSession.clients->end();	++it)
			{
				ClientInfo& client = **it;
			}
			globalMutex.unlock();
		}
		else if (command == "start" && gameSession.clients->size() >= 2)
		{
			globalMutex.lock();
			for(std::list<ClientInfo*>::iterator it = gameSession.clients->begin(); it != gameSession.clients->end();	++it)
			{
				ClientInfo *client = *it;
				client->state = CLIENT_STATE_GAME;
				puts(logger.log(client->name + " is ready to play!").c_str());
				
				sf::Packet sendPacket;
				if (sendPacket << MESSAGE_START_GAME)
				{
					client->socket->send(sendPacket);
				}
			}
			puts(logger.log("Battle begins!").c_str());
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