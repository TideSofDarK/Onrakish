#include "Units.h"
#include <Config.h>

BasicUnit::BasicUnit(tmx::MapObject &referenceToObject)
{
	object = referenceToObject;
}

bool moveObject(tmx::MapObject &object, sf::Uint32 dir)
{
	sf::Vector2f newPosition;
	switch (dir)
	{
	case DIR_TOP_RIGHT:
		newPosition.x = object.GetPosition().x + (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y - (TILE_HEIGHT / 2);
		break;
	case DIR_TOP_LEFT:
		newPosition.x = object.GetPosition().x - (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y - (TILE_HEIGHT / 2);
		break;
	case DIR_BOT_RIGHT:
		newPosition.x = object.GetPosition().x + (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y + (TILE_HEIGHT / 2);
		break;
	case DIR_BOT_LEFT:
		newPosition.x = object.GetPosition().x - (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y + (TILE_HEIGHT / 2);
		break;
	default:
		return false;
		break;
	}
	object.SetPosition2(newPosition);
	return true;
}