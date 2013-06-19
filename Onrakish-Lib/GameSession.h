#pragma once

#include <string>

static const int GAME_STATE_LOBBY = 0;
static const int GAME_STATE_GAME = 0;
static const int GAME_STATE_PAUSE = 0;
static const int GAME_STATE_END = 0;

struct GameSession
{
public:
	unsigned int state;
	std::string gameSessionName;
	std::string mapFileName;
};

