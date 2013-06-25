#include <string>
#include <iostream>
#include <algorithm>
#include "tclap/CmdLine.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>

#include <MapLoader.h>
#include <Config.h>
#include <GameSession.h>
#include <ClientInfo.h>
#include <Log.h>
#include <PacketTransmitter.h>
#include <Input.h>
#include <GUI.h>
#include <TileMapUtil.h>

using namespace TCLAP;
using namespace std;

/* Common variables */
bool quit = false;

/* Network variables */ 
sf::UdpSocket server;
sf::Mutex globalMutex;
ClientInfo clientInfo;

sf::IpAddress ipAddress;
short unsigned int receivePort;
short unsigned int sendPort;

/* Game variables */
GameSession gameSession;

float cameraX, cameraY;

/* Other */
static Log logger = Log();

#define LOG logger.log

int getRandomInt(int max, int min)
{
	int number = min + (rand() % (int)(max - min + 1));
	return number;
}

int main(int argc, char** argv)
{
	//Command line arguments parse
	try {  
		CmdLine cmd("Onrakish-Client", ' ', "0.1");

		//Arguments list
		ValueArg<string> portArg("p","port","Server port",true,"12312","string");
		ValueArg<string> ipArg("a","address","IP Address",true,"12312","string");
		ValueArg<string> nameArg("n","name","Client name",true,"TideS","string");

		cmd.add( portArg );
		cmd.add( ipArg );
		cmd.add( nameArg );
		cmd.parse( argc, argv );

		//Static variables assign
		PORT = (unsigned short)(atoi(portArg.getValue().c_str()));

		//Other variables assign
		clientInfo.name = nameArg.getValue();
		ipAddress = ipArg.getValue();
		receivePort = PORT + 1;

	} catch (ArgException &e)
	{ cerr << "error: " << e.error() << " for arg " << e.argId() << endl; }

	/************************************************************************/
	/* Preparing															*/
	/************************************************************************/
	//Clean console window and disable SFML output
	std::cerr.rdbuf(NULL);
	system("cls");


	/************************************************************************/
	/* SFML Engine init														*/
	/************************************************************************/
	sf::RenderWindow renderWindow(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "Onrakish: Early Dev", (sf::Style::Titlebar, sf::Style::Close, sf::Style::Fullscreen));
	//sf::RenderWindow renderWindow(sf::VideoMode(1280, 720), "Onrakish: Early Dev", (sf::Style::Titlebar, sf::Style::Close));
	renderWindow.setFramerateLimit(240);

	puts(LOG("Created game window with resolution: " + to_string(renderWindow.getSize().x) + "x" + to_string(renderWindow.getSize().y)).c_str());

	//Camera init
	sf::View camera(renderWindow.getDefaultView());
	renderWindow.setView(camera);


	/************************************************************************/
	/* Network																*/
	/************************************************************************/
	if (server.bind(receivePort) != sf::Socket::Done)
	{
		cout << "Cant connect to " + ipAddress.toString() << endl;
		return EXIT_FAILURE;
	}

	//Send client info
	sf::Packet clientInfoPacket;
	clientInfoPacket << clientInfo;
	server.send(clientInfoPacket, ipAddress, PORT);

	//Receive game session
	sf::Packet gameSessionPacket;
	server.receive(gameSessionPacket, ipAddress, receivePort);
	gameSessionPacket >> gameSession;

	sf::Packet sendPortPacket;
	server.receive(sendPortPacket, ipAddress, receivePort);
	sendPortPacket >> sendPort;

	//Reverse ports
	short unsigned int buffer;
	buffer = receivePort;
	receivePort = sendPort;
	sendPort = buffer;


	/************************************************************************/
	/* Tile map																*/
	/************************************************************************/
	tmx::MapLoader ml("maps/");
	ml.Load(gameSession.mapFileName);

	std::vector<tmx::MapLayer> layers;
	layers = ml.GetLayers();

	std::vector<tmx::MapTile> &tiles = layers[0].tiles;


	/************************************************************************/
	/* Other variables                                                      */
	/************************************************************************/
	sf::Vector2i pixelPos = sf::Mouse::getPosition(renderWindow);

	GUI gui = GUI();

	sf::Texture pointerTexture;
	pointerTexture.loadFromFile("gfx/pointer.png");

	sf::Sprite defaultPointer = sf::Sprite(pointerTexture);

	sf::Sprite selectedTilePointer = sf::Sprite(pointerTexture);
	sf::Vector2i selectedTile;

	sf::SoundBuffer clickBuffer;
	if (!clickBuffer.loadFromFile("sfx/click.wav")) return -1;

	sf::Sound sound;
	sound.setBuffer(clickBuffer);

	renderWindow.resetGLStates();

	while (!quit)
	{
		sf::Event event;
		sf::Clock clock;

		/************************************************************************/
		/* Network                                                              */
		/************************************************************************/
		//if (gameState == -1)
		//{
		//	server.setBlocking(true);

		//	sf::Packet gameStateRequestPacket, gameStatePacket;
		//	gameStateRequestPacket << "gameStateRequest";
		//	server.send(gameStateRequestPacket, ipAddress, sendPort);

		//	server.receive(gameStatePacket, ipAddress, receivePort);
		//	gameStatePacket >> gameState;

		//	server.setBlocking(false);
		//}

		//sf::Packet requestPacket;
		//string request;
		//server.receive(requestPacket, ipAddress, receivePort);

		//if (requestPacket >> request)
		//{
		//	if (request == "")
		//	{
		//	}
		//}


		/************************************************************************/
		/* Scrolling                                                            */
		/************************************************************************/
		pixelPos = sf::Mouse::getPosition(renderWindow);

		if (pixelPos.x <= (renderWindow.getSize().x / 100) * 5 && pixelPos.x >= 0) 
			cameraX -= SCROLL_SPEED;
		if (pixelPos.x >= renderWindow.getSize().x - (renderWindow.getSize().x / 100) * 5 && pixelPos.x <= renderWindow.getSize().x) 
			cameraX += SCROLL_SPEED;
		if (pixelPos.y <= (renderWindow.getSize().y / 100) * 9 && pixelPos.y >= 0) 
			cameraY -= SCROLL_SPEED;
		if (pixelPos.y >= renderWindow.getSize().y - (renderWindow.getSize().y / 100) * 9 && pixelPos.y <= renderWindow.getSize().y) 
			cameraY += SCROLL_SPEED;

		camera.reset(sf::FloatRect(cameraX, cameraY, renderWindow.getSize().x, renderWindow.getSize().y));
		renderWindow.setView(camera);


		/************************************************************************/
		/* Input																*/
		/************************************************************************/
		while (renderWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				quit = true;
			if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				quit = true;
			if (event.type == sf::Event::MouseButtonReleased)
			{
				selectedTilePointer.setPosition(defaultPointer.getPosition());
				selectedTile = convertMouseToMap(renderWindow, cameraX, cameraY);
				sound.play();
			}
		}

		
		/************************************************************************/
		/* Drawing                                                              */
		/************************************************************************/
		renderWindow.clear();
		ml.Draw(renderWindow, false);

		//for(std::vector<tmx::MapTile>::iterator it = tiles.begin(); it != tiles.end(); ++it)
		//{
		//	tmx::MapTile& tile = *it;
		//}

		sf::Vector2i pointerVector = convertMouseToMap(renderWindow, cameraX, cameraY);
		sf::Vector2f newPos = convertToScreen(pointerVector.x, pointerVector.y);
		newPos.y -= TILE_HEIGHT / 2;
		defaultPointer.setPosition(newPos);

		renderWindow.draw(defaultPointer);
		renderWindow.draw(selectedTilePointer);

		renderWindow.display();
	}


	/************************************************************************/
	/* Sending disconnect packet and close window							*/
	/************************************************************************/
	sf::Packet disconnectPacket;
	disconnectPacket << "disconnect";
	server.send(disconnectPacket, ipAddress, sendPort);

	renderWindow.close();

	return EXIT_SUCCESS;
}