#ifndef __TILESET__
#define __TILESET__

#include <memory>
#include <string>
#include "Sprite.h"
#include "GameObject.h"

class Sprite;

class TileSet
{
public:
  // Default constructor
  TileSet(int tileWidth, int tileHeight, std::string filename);
  ~TileSet();

  // Render a tile
  void RenderTile(unsigned index, Vector2 position);

  // Get the width of each tile
  int GetTileWidth() const { return tileWidth; }

  // Get the height of each tile
  int GetTileHeight() const { return tileHeight; }

private:
  int tileWidth;
  int tileHeight;

  // A game object with a sprite which contains the tile set
  std::weak_ptr<GameObject> objectWeak;

  // The tileset sprite reference
  std::weak_ptr<Sprite> tileSpriteWeak;

  // The amount of rows in the tileset
  int rows;

  // The amount of columns in the tileset
  int columns;
};

#endif