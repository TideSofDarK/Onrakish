#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <list>
#include <SFML/Graphics.hpp>
#include "MapLoader.h"

struct Building
{
	std::string buildingName;
};

struct Move
{
	std::string unitName;
	sf::Vector2f transform;
};

struct TurnData
{
	TurnData(){};

	sf::Uint32 playerID;
	std::list<Move> moves;
	std::string toString();
};

TurnData parseTurnData(std::string td);
bool applyTurnData(TurnData td, std::vector<tmx::MapObject> &objects);

