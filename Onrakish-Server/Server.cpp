#include <string>
#include <iostream>
#include <algorithm>
#include "tclap/CmdLine.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>

#include <MapLoader.h>
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

static sf::Uint64 id = 1;

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
	static sf::Mutex mutex;

	sf::Lock lock(mutex);
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
					packet.clear();
					sendPacket.clear();
					switch (msg)
					{
					case MESSAGE_DISCONNECT:
						globalMutex.lock();
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
						switch (gameSession.clients->size())
						{
						case 0:
							puts(logger.log("Currently there are no players in lobby").c_str());
							break;
						case 1:
							puts(logger.log("Currently there is one player in lobby").c_str());
							break;
						default:
							puts(logger.log("Currently there are " + to_string(gameSession.clients->size()) + " players in lobby").c_str());
							break;
						}
						globalMutex.unlock();
						break;

					case MESSAGE_GAME_SESSION_REQUEST:
						globalMutex.lock();
						sendPacket << gameSession;
						client->socket->send(sendPacket);
						globalMutex.unlock();
						break;

					case MESSAGE_END_TURN:
						globalMutex.lock();
						if (gameSession.state == GAME_STATE_GAME)
						{
							if (gameSession.turnPlayerID == client->id)
							{
								sendPacket << MESSAGE_TURN_DATA_REQUEST;
								client->socket->send(sendPacket);

								puts(logger.log("Received \"end turn\" info").c_str());
								packet.clear();
								client->socket->receive(packet);
								string td;
								if (packet >> td)
								{
									if (parseTurnData(td).toString() == td)
									{
										puts(logger.log("Successfully parsed!").c_str());
									}
								}

								if (gameSession.turnPlayerID + 1 > gameSession.clients->size())
								{
									gameSession.turnPlayerID = 1;
								}
								else
								{
									gameSession.turnPlayerID++;
								}	

								puts(logger.log("Player " + client->name + ":" + to_string(client->id) + " with IP " + client->socket->getRemoteAddress().toString() + " ended his turn, next is " + to_string(gameSession.turnPlayerID)).c_str());

								for(auto it = gameSession.clients->begin(); it != gameSession.clients->end();	++it)
								{
									ClientInfo &cl = **it;

									if (cl.id == gameSession.turnPlayerID)
									{
										puts(logger.log("Next turn: " + cl.name + ":" + to_string(cl.id)).c_str());
									}

									sendPacket.clear();

									sendPacket << MESSAGE_UPDATE_TURN_DATA;
									cl.socket->send(sendPacket);

									sendPacket.clear();

									sendPacket << gameSession.turnPlayerID;
									cl.socket->send(sendPacket);

									sendPacket.clear();

									sendPacket << td;
									cl.socket->send(sendPacket);
								}
							}
						}	
						globalMutex.unlock();
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
			globalMutex.lock();
			if (selector.isReady(listener))
			{
				sf::Packet sendPacket;
				sf::Packet receivePacket;

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

					gameSession.clients->push_back(newClient);
					selector.add(*newClient->socket);

					gameSession.clients->back()->id = getUniqueId();
					gameSession.clients->back()->name = receivedInfo.name;

					sf::Thread* thread = 0;
					thread = new sf::Thread(&handleClient, gameSession.clients->back());
					thread->launch();

					puts(logger.log("Player " + gameSession.clients->back()->name + " connected: " + newClient->socket->getRemoteAddress().toString() + ", assigned ID: " + to_string(newClient->id)).c_str());
					switch (gameSession.clients->size())
					{
					case 0:
						puts(logger.log("Currently there are no players in lobby").c_str());
						break;
					case 1:
						puts(logger.log("Currently there is one player in lobby").c_str());
						break;
					default:
						puts(logger.log("Currently there are " + to_string(gameSession.clients->size()) + " players in lobby").c_str());
						break;
					}
				}
				else
				{
					delete newClient->socket;
				}
			}
			globalMutex.unlock();
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

	//Counting players
	tmx::MapLoader ml("maps/");
	ml.Load(gameSession.mapFileName);

	puts(logger.log("Map is for " + ml.GetPropertyString("players") + " players").c_str());

	//Listen port
	puts(logger.log("Listening to: " + to_string(PORT)).c_str());
	listener.listen(PORT);

	//Run the thread that handles clients
	sf::Thread* thread = 0;
	thread = new sf::Thread(&handleClients);
	thread->launch();

	while (!quit)
	{
		string command;
		cin >> command;
		if(command == "getClientInfo")
		{
			globalMutex.lock();
			switch (gameSession.clients->size())
			{
			case 0:
				puts(logger.log("Currently there are no players").c_str());
				break;
			case 1:
				puts(logger.log("Currently there is one player").c_str());
				break;
			default:
				puts(logger.log("Currently there are " + to_string(gameSession.clients->size()) + " players").c_str());
				break;
			}
			for(std::list<ClientInfo*>::iterator it = gameSession.clients->begin(); it != gameSession.clients->end();	++it)
			{
				ClientInfo& client = **it;
				puts(logger.log(" --> " + client.name + ", ID:" + to_string(client.id)).c_str());
			}
			globalMutex.unlock();
		}
		else if (command == "start" && gameSession.clients->size() == atoi(ml.GetPropertyString("players").c_str()) && gameSession.state == GAME_STATE_LOBBY)
		{
			globalMutex.lock();
			int newID = 0;
			for(std::list<ClientInfo*>::iterator it = gameSession.clients->begin(); it != gameSession.clients->end();	++it)
			{
				newID++;

				ClientInfo *client = *it;
				client->id = newID;
				client->state = CLIENT_STATE_GAME;
				puts(logger.log(client->name + " is ready to play! New ID is " + to_string(client->id)).c_str());

				sf::Packet sendPacket;
				if (sendPacket << MESSAGE_START_GAME)
				{
					client->socket->send(sendPacket);
					sendPacket.clear();
					if (sendPacket << *client)
					{
						client->socket->send(sendPacket);
						sendPacket.clear();
						if (sendPacket << gameSession.turnPlayerID)
						{
							client->socket->send(sendPacket);
						}
					}
				}
			}
			gameSession.state = GAME_STATE_GAME;
			puts(logger.log("Battle begins!").c_str());
			globalMutex.unlock();
			if(thread)
			{
				thread->wait();
				delete thread;
			}
		}
	}
	return 0;
}