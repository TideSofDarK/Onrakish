#include "Debugger.h"

Debugger::Debugger(std::list<std::string*> &strs)
{
	strings = strs;

	debuggerFont.loadFromFile("fnt/ARIAL.TTF");

	for(std::list<std::string*>::iterator it = strings.begin(); it != strings.end();	++it)
	{
		std::string& str = **it;
		sf::Text renderString(str, debuggerFont);
		renderStrings.push_back(renderString);
	}
}

void Debugger::draw(sf::RenderTarget& rt, int cameraX, int cameraY)
{
	for(std::list<sf::Text>::iterator it = renderStrings.begin(); it != renderStrings.end();	++it)
	{
		sf::Text& renderString = *it;

		int i = std::distance(renderStrings.begin(), it);

		std::list<std::string*>::iterator ite = strings.begin();
		std::advance(ite, i);

		std::string& str = **ite;

		renderString.setString(str);
	}
	for(std::list<sf::Text>::iterator it = renderStrings.begin(); it != renderStrings.end();	++it)
	{
		sf::Text& renderString = *it;

		renderString.setPosition(cameraX, cameraY + (std::distance(renderStrings.begin(), it) * 30));
		rt.draw(renderString);
	}
}