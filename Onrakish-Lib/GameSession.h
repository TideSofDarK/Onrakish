#pragma once

#include <string>

static const int GAME_STATE_LOBBY = 0;
static const int GAME_STATE_GAME = 1;
static const int GAME_STATE_PAUSE = 2;
static const int GAME_STATE_END = 3;

struct GameSession
{
public:
	unsigned int state;
	std::string gameSessionName;
	std::string mapFileName;

	int turnPlayerID;

	GameSession()
	{
		state = GAME_STATE_LOBBY;
	}
};

