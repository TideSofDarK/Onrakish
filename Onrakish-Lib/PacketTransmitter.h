#pragma once

#include "SFML/Network.hpp"
#include "GameSession.h"
#include "ClientInfo.h"
#include "TurnData.h"

sf::Packet& operator <<(sf::Packet& packet, const GameSession& s);
sf::Packet& operator >>(sf::Packet& packet, GameSession& s);

sf::Packet& operator <<(sf::Packet& packet, const ClientInfo& s);
sf::Packet& operator >>(sf::Packet& packet, ClientInfo& s);