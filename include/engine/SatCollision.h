#ifndef __SAT_COLLISION__
#define __SAT_COLLISION__

#include "Rectangle.h"
#include <iostream>
#include <utility>
#include <memory>

class Rigidbody;

// Implementation of SAT collision, limited to rectangle polygons
// Based on this guide: https://youtu.be/-EsWKT7Doww
namespace SatCollision
{
  struct CollisionData
  {
    // Direction through which contact was initiated
    Vector2 normal;

    // Body of object that received contact
    std::shared_ptr<Rigidbody> source;

    // Body of object that made contact
    std::shared_ptr<Rigidbody> other;
  };

  // Finds the minimum distance between both rectangle's edges
  // Negative values indicate penetration
  // Rotations are in radians
  static std::pair<float, Vector2> FindMinDistance(Rectangle rect1, Rectangle rect2, float rotation1 = 0.0f, float rotation2 = 0.0f)
  {
    // Will keep track of the best distance found
    float bestDistance = std::numeric_limits<float>::lowest();

    // Will store the normal corresponding to the best distance
    Vector2 bestNormal;

    // Normal to be used in each iteration
    Vector2 normal = Vector2::Angled(rotation1);

    // Loop rect1 vertices
    for (Vector2 vertex1 : rect1.Vertices(rotation1))
    {
      // This vertex's minimum distance to rect2
      float minDistance = std::numeric_limits<float>::max();

      // Loop rect2 vertices
      for (Vector2 vertex2 : rect2.Vertices(rotation2))
      {
        // Check if this distance is smaller (project vertices distance on normal)
        minDistance = std::min(minDistance, Vector2::Dot(vertex2 - vertex1, normal));
      }

      // Check if we got a better distance
      if (minDistance > bestDistance)
      {
        bestDistance = minDistance;
        bestNormal = normal;
      }

      // Advance normal for next iteration (clockwise)
      normal = normal.Rotated(M_PI / 2.0);
    }

    return std::make_pair(bestDistance, bestNormal);
  }
}

#endif