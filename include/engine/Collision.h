#ifndef __COLLISION__
#define __COLLISION__

#include "Rectangle.h"
#include "Helper.h"
#include <iostream>
#include <utility>
#include <memory>

class Collider;

// Implementation of SAT collision, limited to rectangle polygons
// Based on this guide: https://youtu.be/-EsWKT7Doww
namespace Collision
{
  struct Data
  {
    // Direction through which contact was initiated
    Vector2 normal;

    // How far along the normal penetration goes
    float penetration;

    // Collider of object that received contact
    std::weak_ptr<Collider> weakSource;

    // Collider of object that made contact
    std::weak_ptr<Collider> weakOther;

    // Returns a number which is unique for each pair of (source, other) colliders (order matters)
    size_t GetHash() const;
  };

  // Finds the minimum distance between both rectangle's edges
  // Negative values indicate penetration
  // Rotations are in radians
  std::pair<float, Vector2> FindMinDistanceSingleSided(
      Rectangle rect1, Rectangle rect2, float rotation1 = 0.0f, float rotation2 = 0.0f);

  std::pair<float, Vector2> FindMinDistance(
      Rectangle rect1, Rectangle rect2, float rotation1 = 0.0f, float rotation2 = 0.0f);

  // Indicates whether point is inside the perimeter of the rectangle
  // Rotation in radians
  bool DetectIntersection(Rectangle rect, Vector2 point, float rotation = 0);
}

#endif