#include "Input.h"

bool objectClicked(sf::RenderWindow &window, tmx::MapObject &object)
{
	if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

		if (object.Contains(mouse))
		{
			return true;
		}
	}
	return false;
}