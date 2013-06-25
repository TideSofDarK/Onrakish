#include "TileMapUtil.h"

sf::Vector2f convertToScreen(int x, int y)
{
	sf::Vector2f screen;
	screen.x = 0 - (y * TILE_WIDTH/2) + (x * TILE_WIDTH/2) - (TILE_WIDTH/2);
	screen.y = 0 + (y * TILE_HEIGHT/2) + (x * TILE_HEIGHT/2);
	return screen;
}

sf::Vector2i convertMouseToMap(sf::RenderWindow &window, int cameraX, int cameraY)
{
	sf::Vector2i map;

	int mouseX = sf::Mouse::getPosition().x;
	int mouseY = sf::Mouse::getPosition().y;

	sf::Vector2i windowPosition = window.getPosition();

	int dx = mouseX + cameraX;
	int dy = mouseY + cameraY + (TILE_HEIGHT/2);

	map.x = (dy + dx/2)/(TILE_WIDTH/2);
	map.y = (dy - dx/2)/TILE_HEIGHT;

	return map;
}