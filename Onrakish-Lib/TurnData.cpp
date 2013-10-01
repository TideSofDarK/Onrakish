#include "TurnData.h"

std::string TurnData::toString()
{
	std::string finalString = std::to_string(playerID) + ";";
	for(auto it = moves.begin(); it != moves.end();	++it)
	{
		Move &move = *it;
		finalString += move.unitName + ":" + std::to_string((int)move.transform.x) + "x" + std::to_string((int)move.transform.y) + ";";
	}
	return finalString;
}

TurnData parseTurnData(std::string td)
{
	TurnData turnData;
	std::string turnDataString = td;
	std::string idString;
	for(int i = 0; i < turnDataString.size(); i++)
	{
		if (turnDataString[i] != ';') idString.push_back(turnDataString[i]);
		else break;
	}
	sf::Uint32 id = atoi(idString.c_str());
	turnData.playerID = id;

	turnDataString.replace(turnDataString.find(idString + ";"), idString.size() + 1, "");

	size_t n = std::count(turnDataString.begin(), turnDataString.end(), ';');

	for(int i = 0; i < n; i++)
	{
		Move move;
		std::string newName;
		sf::Vector2f newTransform;
		for(int a = 0; a < turnDataString.size(); a++)
		{
			if (turnDataString[a] != ':') newName.push_back(turnDataString[a]);
			else break;
		}

		move.unitName = newName;

		turnDataString.replace(turnDataString.find(newName + ":"), newName.size() + 1, "");

		sf::Uint32 xPos;
		sf::Uint32 yPos;
		std::string xPosString;
		std::string yPosString;
		for(int a = 0; a < turnDataString.size(); a++)
		{
			if (turnDataString[a] != 'x') xPosString.push_back(turnDataString[a]);
			else break;
		}

		xPos = atoi(xPosString.c_str());

		turnDataString.replace(turnDataString.find(xPosString + "x"), xPosString.size() + 1, "");

		for(int a = 0; a < turnDataString.size(); a++)
		{
			if (turnDataString[a] != ';') yPosString.push_back(turnDataString[a]);
			else break;
		}

		yPos = atoi(yPosString.c_str());

		turnDataString.replace(turnDataString.find(yPosString + ";"), yPosString.size() + 1, "");
		
		move.transform = sf::Vector2f(xPos, yPos);
		turnData.moves.push_back(move);
	}
	return turnData;
}

bool applyTurnData(TurnData td, std::vector<tmx::MapObject> &objects)
{
	for(auto it = objects.begin(); it != objects.end();	++it)
	{
		tmx::MapObject &object = *it;
		if (object.GetName() == td.moves.front().unitName && atoi(object.GetPropertyString("player").c_str()) == td.playerID)
		{
			object.SetPosition(td.moves.front().transform);
		}
	}
	return true;
}