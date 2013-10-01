#pragma once

#include <string>
#include <list>
#include <SFML/Graphics.hpp>

class Debugger
{
private:
	sf::Font debuggerFont;

	std::list<std::string*> strings;
	std::list<sf::Text> renderStrings;
public:
	Debugger(std::list<std::string*> &strs);

	void draw(sf::RenderTarget& rt, int cameraX, int cameraY);
};

