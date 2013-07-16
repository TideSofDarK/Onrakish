#include "Units.h"
#include <Config.h>

BasicUnit::BasicUnit(tmx::MapObject &referenceToObject)
{
	object = referenceToObject;
}

bool moveObject(tmx::MapObject &object, sf::Uint32 dir)
{
	sf::Vector2f newPosition;
	sf::Vector2f transform;
	switch (dir)
	{
	case DIR_TOP_RIGHT:
		transform.x = (TILE_WIDTH);
		transform.y = -(TILE_HEIGHT);
		newPosition.x = object.GetPosition().x + (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y - (TILE_HEIGHT / 2);
		break;
	case DIR_TOP_LEFT:
		transform.x = -(TILE_WIDTH);
		transform.y = -(TILE_HEIGHT);
		newPosition.x = object.GetPosition().x - (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y - (TILE_HEIGHT / 2);
		break;
	case DIR_BOT_RIGHT:
		transform.x = (TILE_WIDTH);
		transform.y = (TILE_HEIGHT);
		newPosition.x = object.GetPosition().x + (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y + (TILE_HEIGHT / 2);
		break;
	case DIR_BOT_LEFT:
		transform.x = -(TILE_WIDTH);
		transform.y = (TILE_HEIGHT);
		newPosition.x = object.GetPosition().x - (TILE_WIDTH / 2);
		newPosition.y = object.GetPosition().y + (TILE_HEIGHT / 2);
		break;
	default:
		return false;
		break;
	}
	transform.x /= 2;
	transform.y /= 2;
	object.SetPosition2(newPosition, transform);
	return true;
}