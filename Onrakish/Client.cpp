#include <string>
#include <iostream>
#include <algorithm>
#include <list>
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
#include <Debugger.h>
#include <Message.h>

using namespace TCLAP;
using namespace std;

/* Common variables */
bool quit = false;

/* Network variables */ 
sf::TcpSocket server;
sf::Mutex globalMutex;
ClientInfo clientInfo;

sf::IpAddress ipAddress;

/* Game variables */
GameSession gameSession;

float cameraX, cameraY;

/* Other */
static Log logger = Log();

#define LOG logger.log

float getFPS(const sf::Time& time) {
	return (1000000.0f / time.asMicroseconds());
}

int getRandomInt(int max, int min)
{
	int number = min + (rand() % (int)(max - min + 1));
	return number;
}

bool sendMessage(int command)
{
	sf::Packet sendPacket;
	if (sendPacket << command)
	{
		server.send(sendPacket);
		return true;
	} else return false;
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
	sf::RenderWindow window(sf::VideoMode(sf::VideoMode::getDesktopMode().width, sf::VideoMode::getDesktopMode().height), "Onrakish: Early Dev", (sf::Style::Titlebar, sf::Style::Close, sf::Style::Fullscreen));
	//sf::RenderWindow window(sf::VideoMode(1280, 720), "Onrakish: Early Dev", (sf::Style::Titlebar, sf::Style::Close));
	//window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	puts(LOG("Created game window with resolution: " + to_string(window.getSize().x) + "x" + to_string(window.getSize().y)).c_str());

	//Camera init
	sf::View camera(window.getDefaultView());
	window.setView(camera);


	/************************************************************************/
	/* Network																*/
	/************************************************************************/
	if (server.connect(ipAddress, PORT) != sf::Socket::Done)
	{
		cout << "Cant connect to " + ipAddress.toString() << endl;
		return EXIT_FAILURE;
	}

	sf::Packet receivePacket;
	sf::Packet sendPacket;

	server.receive(receivePacket);

	int msg;
	receivePacket >> msg;

	if (msg == MESSAGE_CLIENT_INFO_REQUEST)
	{
		sendPacket << clientInfo;
		server.send(sendPacket);
	}

	sendMessage(MESSAGE_GAME_SESSION_REQUEST);

	receivePacket.clear();
	server.receive(receivePacket);
	receivePacket >> gameSession;

	/************************************************************************/
	/* Tile map																*/
	/************************************************************************/
	tmx::MapLoader ml("maps/");
	ml.Load(gameSession.mapFileName);

	std::vector<tmx::MapLayer> layers;
	layers = ml.GetLayers();

	std::vector<tmx::MapTile> &tiles = layers[0].tiles;


	/************************************************************************/
	/* Debugger setup														*/
	/************************************************************************/

	std::list<std::string*> strs;
	std::string fpsDebugString = "";
	std::string pointerPositionDebugString = "";
	std::string selectedTileDebugString = "";

	strs.push_back(&fpsDebugString);
	strs.push_back(&pointerPositionDebugString);
	strs.push_back(&selectedTileDebugString);
	Debugger debug(strs);


	/************************************************************************/
	/* Other variables                                                      */
	/************************************************************************/
	sf::Vector2i pixelPos = sf::Mouse::getPosition(window);

	GUI gui = GUI();

	sf::Texture pointerTexture;
	pointerTexture.loadFromFile("gfx/pointer.png");

	sf::Sprite defaultPointer = sf::Sprite(pointerTexture);

	sf::Sprite selectedTilePointer = sf::Sprite(pointerTexture);
	sf::Vector2i selectedTile;

	//Click sound

	sf::SoundBuffer clickBuffer;
	if (!clickBuffer.loadFromFile("sfx/click.wav")) return -1;

	sf::Sound sound;
	sound.setBuffer(clickBuffer);

	//Clocks

	sf::Clock deltaClock;
	sf::Clock FPSClock;
	float lastTime = 0;

	while (!quit)
	{
		sf::Event event;
		sf::Clock clock;

		fpsDebugString = "FPS: " + to_string((int)(getFPS(FPSClock.restart())));

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
		/* Input																*/
		/************************************************************************/
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				quit = true;
			if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
				quit = true;
			if (event.type == sf::Event::MouseButtonReleased)
			{
				selectedTilePointer.setPosition(defaultPointer.getPosition());
				selectedTile = convertMouseToMap(window, cameraX, cameraY);
				selectedTileDebugString = "Selected tile position: " + to_string(selectedTile.x) + "x" + to_string(selectedTile.y);
				sound.play();
			}
		}
		
		
		/************************************************************************/
		/* Drawing                                                              */
		/************************************************************************/
		window.clear();
		ml.Draw(window, false);

		//for(std::vector<tmx::MapTile>::iterator it = tiles.begin(); it != tiles.end(); ++it)
		//{
		//	tmx::MapTile& tile = *it;
		//}

		sf::Vector2i pointerVector = convertMouseToMap(window, cameraX, cameraY);
		sf::Vector2f newPos = convertToScreen(pointerVector.x, pointerVector.y);
		newPos.y -= TILE_HEIGHT / 2;
		defaultPointer.setPosition(newPos);

		pointerPositionDebugString = "Pointer position: " + to_string(pointerVector.x) + "x" + to_string(pointerVector.y);

		window.draw(defaultPointer);
		window.draw(selectedTilePointer);
		debug.draw(window, cameraX, cameraY);


		/************************************************************************/
		/* Scrolling                                                            */
		/************************************************************************/
		float deltaTime = deltaClock.restart().asSeconds();
		pixelPos = sf::Mouse::getPosition(window);

		if (pixelPos.x <= 0) 
			cameraX -= SCROLL_SPEED * deltaTime;
		if (pixelPos.x >= window.getSize().x - 1) 
			cameraX += SCROLL_SPEED * deltaTime;
		if (pixelPos.y <= 0) 
			cameraY -= SCROLL_SPEED * deltaTime;
		if (pixelPos.y >= window.getSize().y - 1) 
			cameraY += SCROLL_SPEED * deltaTime;

		camera.reset(sf::FloatRect(cameraX, cameraY, window.getSize().x, window.getSize().y));
		window.setView(camera);

		window.display();
	}


	/************************************************************************/
	/* Sending disconnect packet and close window							*/
	/************************************************************************/
	sf::Packet disconnectPacket;
	disconnectPacket << MESSAGE_DISCONNECT;
	server.send(disconnectPacket);

	window.close();

	return EXIT_SUCCESS;
}