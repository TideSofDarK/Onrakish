#pragma once

#include <SFML/Graphics.hpp>
#include <MapLoader.h>
#include <Config.h>
#include <string>
#include <math.h>

sf::Vector2f convertToScreen(int x, int y);
sf::Vector2i convertMouseToMap(sf::RenderWindow &window, int cameraX, int cameraY);