#pragma once
#include "GameSession.h"
#include "ClientInfo.h"
#include "list"
class NetworkGameSession :
	public GameSession
{
public:
	std::list<ClientInfo*> *clients;

	NetworkGameSession()
	{
		state = GAME_STATE_LOBBY;
		turnPlayerID = 1;
		clients = new std::list<ClientInfo*>;
	}
};

