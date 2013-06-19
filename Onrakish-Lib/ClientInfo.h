#pragma once

#include "string"
#include "SFML/Network.hpp"

static const int CLIENT_STATE_LOBBY = 0;
static const int CLIENT_STATE_GAME = 1;
static const int CLIENT_STATE_WAIT = 2;
static const int CLIENT_STATE_PAUSE = 3;
static const int CLIENT_STATE_DISCONNECTED = 4;

struct ClientInfo
{
	sf::UdpSocket* socket;
	unsigned short receivePort;
	unsigned short sendPort;
	sf::IpAddress ipAddress;

	sf::Uint32 id;
	std::string name;
	int state;
	ClientInfo()
	{
		state = CLIENT_STATE_LOBBY;
	}
};

