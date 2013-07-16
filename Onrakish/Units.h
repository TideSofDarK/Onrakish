#pragma once

#include <SFML/Graphics.hpp>
#include <MapLoader.h>

static const sf::Uint32 DIR_TOP_RIGHT = 0;
static const sf::Uint32 DIR_TOP_LEFT = 1;
static const sf::Uint32 DIR_BOT_RIGHT = 2;
static const sf::Uint32 DIR_BOT_LEFT = 3;

class BasicUnit
{
private:
	tmx::MapObject object;
public:
	BasicUnit(tmx::MapObject &referenceToObject);
};

bool moveObject(tmx::MapObject &object, sf::Uint32 dir);

