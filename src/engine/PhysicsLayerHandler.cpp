#include "PhysicsLayerHandler.h"
#include "GameObject.h"
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;

// Default label for unnamed layer
static const string defaultLayerLabel = "Layer";

int GetDigitCount(int number);

PhysicsLayerHandler::PhysicsLayerHandler()
{
  // Fill matrix with true
  fill(&collisionMatrix[0][0], &collisionMatrix[0][0] + sizeof(collisionMatrix), true);

  InitializeCollisionMatrix();

  // Populate translation table
  ADD_LAYER_TRANSLATION

  // Add missing entries
  for (int layer = 0; layer < PHYSICS_LAYER_COUNT; layer++)
    if (translation.count(PhysicsLayer(layer)) == 0)
      translation[PhysicsLayer(layer)] = defaultLayerLabel + to_string(layer);

#ifdef PRINT_COLLISION_MATRIX
  PrintMatrix();
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

bool PhysicsLayerHandler::HaveCollision(GameObject &object1, GameObject &object2) const
{
  return HaveCollision(object1.GetPhysicsLayer(), object2.GetPhysicsLayer());
}

bool PhysicsLayerHandler::HaveCollision(PhysicsLayer layer1, PhysicsLayer layer2) const
{
  return collisionMatrix[int(layer1)][int(layer2)];
}

string Fill(size_t count, string character) { return count > 0 ? Fill(count - 1, character) + character : ""; }
string Pad(size_t count) { return Fill(count, " "); }

// Returns the given string centered in the the given space length
string CenterFill(string text, size_t space, string filling = " ")
{
  size_t leftSpace = max((space - text.length()) / 2, size_t(0));
  bool evenAlignment = (space - text.length()) % 2 == 0;

  return Fill(leftSpace, filling) + text + Fill(evenAlignment ? leftSpace : leftSpace + 1, filling);
}
string RightFill(string text, size_t space, string filling = " ")
{
  size_t leftSpace = max(space - text.length(), size_t(0));

  return Fill(leftSpace, filling) + text;
}

void PhysicsLayerHandler::PrintMatrix()
{
  const static string header = " COLLISION MATRIX ";
  const static int padding = 1;
  const static int gap = 1;
  const static string layerLabel = "Layer";

  // Get how many characters each cell has
  size_t cellLength = 0;
  for (auto [layer, label] : translation)
    cellLength = max(cellLength, label.length());

  // Returns a filled cell
  auto LabelCell = [cellLength, this](int layer, bool right = false)
  { return right ? RightFill(translation[PhysicsLayer(layer)], cellLength)
                 : CenterFill(translation[PhysicsLayer(layer)], cellLength); };

  auto ValueCell = [cellLength, this](int layer1, int layer2)
  { return CenterFill(to_string(collisionMatrix[layer1][layer2]), cellLength); };

  // === HEADER
  string line0 = Pad(cellLength + padding);

  // For each layer
  for (int layer = 0; layer < PHYSICS_LAYER_COUNT; layer++)
    line0 += LabelCell(layer) + (layer == PHYSICS_LAYER_COUNT - 1 ? "" : Pad(gap));

  cout << CenterFill(header, line0.length(), "=") << endl
       << line0 << endl;

  // === ROWS

  // For each layer
  for (int layer = 0; layer < PHYSICS_LAYER_COUNT; layer++)
  {
    // Print layer label
    cout << LabelCell(layer, true) << Pad(padding);

    // For each other layer
    for (int otherLayer = 0; otherLayer < PHYSICS_LAYER_COUNT; otherLayer++)
      cout << ValueCell(layer, otherLayer) << (otherLayer == PHYSICS_LAYER_COUNT - 1 ? "" : Pad(gap));
    cout << endl;
  }

  cout << Fill(line0.length(), "=") << endl;
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
