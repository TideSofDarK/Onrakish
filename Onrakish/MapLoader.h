/*********************************************************************
Matt Marchant 2013
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format

The zlib license has been used to make this software fully compatible
with SFML. See http://www.sfml-dev.org/license.php

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.

2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#ifndef MAP_LOADER_H_
#define MAP_LOADER_H_

#include <MapObject.h>
#include <iostream>
#include <pugixml/pugixml.hpp>

namespace tmx
{
	enum MapOrientation
	{
		Orthogonal,
		Isometric,
		SteppedIsometric
	};

	class MapLoader
	{
	public:
		MapLoader(const std::string mapDirectory);
		~MapLoader();
		//loads a given tmx file, returns false on failure
		const bool Load(std::string mapFile);
		//returns a vector of map layers
		std::vector<MapLayer>& GetLayers(void) {return m_layers;};
		//draws visible tiles to given target, optionally draw outline of objects for debugging
		void Draw(sf::RenderTarget& rt, bool debug = false);
		//projects orthogonal world coords to isometric world coords if available, else return original value
		//eg: use to convert an isometric world coordinate to a position to be drawn in view space
		const sf::Vector2f IsometricToOrthogonal(const sf::Vector2f& projectedCoords);
		//returns orthogonal world coords from projected coords
		//eg: use to find the orthogonal world coordinates currently under the mouse cursor
		const sf::Vector2f OrthogonalToIsometric(const sf::Vector2f& worldCoords);
	private:
		//properties which correspond to tmx
		sf::Uint16 m_width, m_height; //tile count
		sf::Uint16 m_tileWidth, m_tileHeight; //width / height of tiles
		MapOrientation m_orientation;
		float m_tileRatio; //width / height ratio of isometric tiles

		sf::FloatRect m_bounds; //bounding area of tiles visible on screen
		std::string m_mapDirectory; //directory relative to executable containing map files and images

		std::vector<MapLayer> m_layers; //layers of map, including image and object layers
		std::vector<sf::Texture> m_tileTextures;
		std::vector<sf::Texture> m_imageLayerTextures;

		sf::VertexArray m_gridVertices; //used to draw map grid in debug
		bool m_mapLoaded;

		//resets any loaded map properties
		void m_Unload(void);
		//sets the visible area of tiles to be drawn
		void m_SetDrawingBounds(const sf::View& view);

		//utility functions for parsing map data
		const bool m_ParseMapNode(const pugi::xml_node& mapNode);
		const bool m_ParseTileSets(const pugi::xml_node& mapNode);
		const bool m_ProcessTiles(const pugi::xml_node& tilesetNode);
		const bool m_ParseLayer(const pugi::xml_node& layerNode);
		void m_AddTileToLayer(MapLayer& layer, sf::Uint16 x, sf::Uint16 y, sf::Uint16 gid);
		const bool m_ParseObjectgroup(const pugi::xml_node& groupNode);
		const bool m_ParseImageLayer(const pugi::xml_node& imageLayerNode);
		void m_ParseLayerProperties(const pugi::xml_node& propertiesNode, MapLayer& destLayer);
		void m_SetIsometricCoords(MapLayer& layer);

		//utility method for parsing colour values from hex values
		const sf::Color m_ColourFromHex(const char* hexStr) const;

		//method for decompressing zlib compressed strings
		const bool m_Decompress(const char* source, std::vector<unsigned char>& dest, int inSize, int expectedSize);
		//creates a vertex array used to draw grid lines when using debug output
		void m_CreateDebugGrid(void);
	};


	//method for decoding base64 encoded strings
	static std::string base64_decode(std::string const& string);
};

#endif //MAP_LOADER_H_