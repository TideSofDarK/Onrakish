#pragma once
#include "GameSession.h"
#include "ClientInfo.h"
#include "list"
class NetworkGameSession :
	public GameSession
{
public:
	std::list<ClientInfo*> clients;
};

