#include "PhysicsLayerHandler.h"
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;

int GetDigitCount(int number);
void PrintMatrix(bool matrix[PHYSICS_LAYER_COUNT][PHYSICS_LAYER_COUNT]);

PhysicsLayerHandler::PhysicsLayerHandler()
{
  // Fill matrix with true
  fill(&collisionMatrix[0][0], &collisionMatrix[0][0] + sizeof(collisionMatrix), true);

  InitializeCollisionMatrix();

#ifdef PRINT_COLLISION_MATRIX
  PrintMatrix(collisionMatrix);
#endif
}

// Disables collision between two layers
void PhysicsLayerHandler::Disable(PhysicsLayer layer1, PhysicsLayer layer2)
{
  collisionMatrix[int(layer1)][int(layer2)] = false;
  collisionMatrix[int(layer2)][int(layer1)] = false;
}

// Enables collision between two layers
void PhysicsLayerHandler::Enable(PhysicsLayer layer1, PhysicsLayer layer2)
{
  collisionMatrix[int(layer1)][int(layer2)] = true;
  collisionMatrix[int(layer2)][int(layer1)] = true;
}

// Disables collision between a layer and all other layers
void PhysicsLayerHandler::DisableAll(PhysicsLayer layer)
{
  for (int other = 0; other < PHYSICS_LAYER_COUNT; other++)
    Disable(layer, PhysicsLayer(other));
}

// Enables collision between a layer and all other layer
void PhysicsLayerHandler::EnableAll(PhysicsLayer layer)
{
  for (int other = 0; other < PHYSICS_LAYER_COUNT; other++)
    Enable(layer, PhysicsLayer(other));
}

// Checks whether the two given layers collide
bool PhysicsLayerHandler::HaveCollision(PhysicsLayer layer1, PhysicsLayer layer2) const
{
  return collisionMatrix[int(layer1)][int(layer2)];
}

string Fill(unsigned count, string character) { return count > 0 ? Fill(count - 1, character) + character : ""; }
string Pad(unsigned count) { return Fill(count, " "); }

void PrintMatrix(bool matrix[PHYSICS_LAYER_COUNT][PHYSICS_LAYER_COUNT])
{
  // Max digits in layer index
  int indexMaxDigits = GetDigitCount(PHYSICS_LAYER_COUNT - 1);

  const static string header = "COLLISION MATRIX";
  const static int padding = 3;
  const static int gap = 2;
  const static string layerLabel = "Layer";

  // === HEADER
  string line0 = Pad(layerLabel.length() + indexMaxDigits + padding);

  // For each layer
  for (int layer = 0; layer < PHYSICS_LAYER_COUNT; layer++)
    line0 += layerLabel + to_string(layer) + Pad(indexMaxDigits - GetDigitCount(layer)) + Pad(gap);

  int headerFilling = (line0.length() - header.length() - 2) / 2;

  if (headerFilling > 0)
    cout << Fill(headerFilling, "=") << " ";

  cout << header;

  if (headerFilling > 0)
    cout << " " << Fill(headerFilling, "=");

  cout << endl
       << line0 << endl;

  // === ROWS
  // Get how many characters each cell has
  int cellLength = layerLabel.length() + indexMaxDigits;

  // How much internal cell padding on left
  string leftAlign = Pad(cellLength / 2);

  // How much internal cell padding on right
  string rightAlign = Pad(cellLength % 2 == 0 ? leftAlign.length() - 1 : leftAlign.length());

  // For each layer
  for (int layer = 0; layer < PHYSICS_LAYER_COUNT; layer++)
  {
    // Print layer label
    cout << layerLabel << layer << Pad(indexMaxDigits - GetDigitCount(layer)) << Pad(padding);

    // For each other layer
    for (int otherLayer = 0; otherLayer < PHYSICS_LAYER_COUNT; otherLayer++)
      cout << leftAlign << matrix[layer][otherLayer] << rightAlign << Pad(gap);
    cout << endl;
  }

  cout << Fill(line0.length() % 2 == 0 ? line0.length() : line0.length() - 1, "=") << endl;
}

int GetDigitCount(int number)
{
  // Count negative sign
  int digits = number < 0 ? 2 : 1;

  number = abs(number);

  while (number > 9)
  {
    digits++;
    number /= 10;
  }

  return digits;
}
