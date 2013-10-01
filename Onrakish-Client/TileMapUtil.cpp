#include "TileMapUtil.h"

sf::Vector2f convertToScreen(int x, int y)
{
	sf::Vector2f screen;
	screen.x = 0 - (y * TILE_WIDTH/2) + (x * TILE_WIDTH/2) - (TILE_WIDTH/2);
	screen.y = 0 + (y * TILE_HEIGHT/2) + (x * TILE_HEIGHT/2);

	return screen;
}

sf::Vector2f convertToMap(int x, int y)
{
	sf::Vector2f map;

	map.x = (x / TILE_WIDTH_HALF + y / TILE_HEIGHT_HALF) /2;
	map.y = (y / TILE_HEIGHT_HALF -(x / TILE_WIDTH_HALF)) /2;

	return map;
}

sf::Vector2i convertMouseToMap(sf::RenderWindow &window)
{
	sf::Vector2i map;

	sf::Vector2f mousePosScreen = window.mapPixelToCoords(sf::Mouse::getPosition(window));

	//map.x = (mousePosScreen.y + mousePosScreen.x/2)/(TILE_WIDTH/2);
	//map.y = (mousePosScreen.y - mousePosScreen.x/2)/TILE_HEIGHT;

	map.x = (mousePosScreen.x / TILE_WIDTH_HALF + mousePosScreen.y / TILE_HEIGHT_HALF) /2;
	map.y = (mousePosScreen.y / TILE_HEIGHT_HALF -(mousePosScreen.x / TILE_WIDTH_HALF)) /2;

	return map;
}