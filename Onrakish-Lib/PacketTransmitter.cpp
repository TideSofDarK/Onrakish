#include "PacketTransmitter.h"

sf::Packet& operator <<(sf::Packet& packet, const ClientInfo& s)
{
	return packet << s.id << s.state << s.name;
}

sf::Packet& operator >>(sf::Packet& packet, ClientInfo& s)
{
	return packet >> s.id >> s.state >> s.name;
}

sf::Packet& operator <<(sf::Packet& packet, const GameSession& s)
{
	return packet << s.gameSessionName << s.mapFileName << s.state;
}

sf::Packet& operator >>(sf::Packet& packet, GameSession& s)
{
	return packet >> s.gameSessionName >> s.mapFileName >> s.state;
}