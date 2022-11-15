#include <ctype.h>
#include <algorithm>
#include <fstream>
#include "TileMap.h"
#include "Helper.h"
#include "Camera.h"

using namespace std;
using namespace Helper;

TileMap::TileMap(
    GameObject &associatedObject, std::string filename,
    shared_ptr<TileSet> tileSet, int onlyLayer, RenderLayer renderLayer)
    : Component(associatedObject), tileSet(tileSet), renderLayer(renderLayer), targetLayer(onlyLayer)
{
  Load(filename);

  // Validate target layer
  Assert(targetLayer < mapDepth, "Target layer must not exceed the depth of the tilemap file configuration");
}

void TileMap::Load(std::string filename)
{
  // Flush the current loaded matrix
  tileMatrix.clear();

  // Open the file
  ifstream mapFile(filename.c_str());

  // Ensure all's good
  Assert(mapFile.is_open(), "Unable to open tilemap configuration file " + filename);

  // Will hold each line from the document
  string line;

  // Get the first line, which holds the maps dimensions
  getline(mapFile, line);

  auto mapDimensions = SplitString(line, ",");

  try
  {
    // Extract the dimensions
    mapWidth = stoi(mapDimensions[0]);
    mapHeight = stoi(mapDimensions[1]);
    mapDepth = stoi(mapDimensions[2]);

    // Loop through each item of each line
    while (getline(mapFile, line))
      for (auto item : SplitString(line, ","))
      {
        // Disregard empty or whitespace only strings
        if (item.empty() || item.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
          continue;

        // Convert from string and subtract 1 (so that empty tiles are mapped to -1)
        // Push the result to the matrix
        tileMatrix.push_back(stoi(item) - 1);
      }
  }
  // Catch stoi errors
  catch (const invalid_argument &error)
  {
    // Clean up file
    mapFile.close();

    throw runtime_error("Map file at " + filename + " has invalid syntax");
  }

  // Close file
  mapFile.close();
}

void TileMap::RenderTileLayer(int layer)
{
  // Offset to center tilemap
  Vector2 centerOffset = Vector2(-GetWidth() / 2, -GetHeight() / 2);

  // Offset to apply to matrix index to account for layer
  int layerOffset = layer * mapWidth * mapHeight;

  // Get camera position
  auto [cameraX, cameraY] = Camera::GetInstance().GetPosition();

  // Parallax to apply on X coordinates
  float parallaxX = layer * parallaxIntensity * cameraX;

  // Parallax to apply on Y coordinates
  float parallaxY = layer * parallaxIntensity * cameraY;

  // For each tile in matrix
  for (int i = 0; i < mapWidth * mapHeight; i++)
  {
    int matrixIndex = i + layerOffset;

    // Skip empty tiles
    if (tileMatrix[matrixIndex] == -1)
      continue;

    // X coord where to render it
    float x = (i % mapWidth) * tileSet->GetTileWidth() - parallaxX;

    // Y coord where to render it
    float y = (i / mapWidth) * tileSet->GetTileHeight() - parallaxY;

    // Render it
    tileSet->RenderTile(tileMatrix[matrixIndex], Vector2(x, y) + centerOffset);
  }
}

void TileMap::Render()
{
  // If only needs to render one layer, do it and stop
  if (targetLayer >= 0)
  {
    RenderTileLayer(targetLayer);

    return;
  }

  // Render each layer
  for (int layer = 0; layer < mapDepth; ++layer)
  {
    RenderTileLayer(layer);
  }
}

float TileMap::GetWidth() const { return HorizontalTileCount() * tileSet->GetTileWidth(); }
float TileMap::GetHeight() const { return VerticalTileCount() * tileSet->GetTileHeight(); }
