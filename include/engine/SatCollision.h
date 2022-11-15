#ifndef __SAT_COLLISION__
#define __SAT_COLLISION__

#include "Rectangle.h"
#include <iostream>

// Implementation of SAT collision, limited to rectangle polygons
// Based on this guide: https://youtu.be/-EsWKT7Doww
namespace SatCollision
{
  // Finds the minimum distance between both rectangle's edges
  // Negative values indicate penetration
  // Rotations are in radians
  static float FindMinDistance(Rectangle rect1, Rectangle rect2, float rotation1 = 0.0f, float rotation2 = 0.0f)
  {
    // Will keep track of the best distance found
    float bestDistance = std::numeric_limits<float>::lowest();

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
        bestDistance = minDistance;

      // Advance normal for next iteration (clockwise)
      normal = normal.Rotated(M_PI / 2.0);
    }

    return bestDistance;
  }

  // Checks if both rect's have any overlapping area
  static bool IsColliding(const Rectangle &rect1, const Rectangle &rect2, float rotation1, float rotation2)
  {
    return FindMinDistance(rect1, rect2, rotation1, rotation2) <= 0.0f && FindMinDistance(rect2, rect1, rotation2, rotation1) <= 0.0f;
  }

  // Additionally first does a quick check to ensure the rectangles are close enough so that collision is possible
  static bool IsColliding(const Rectangle &rect1, const Rectangle &rect2, float rotation1, float rotation2, float minDistanceSquared)
  {
    // std::cout << minDistanceSquared << " < " << Vector2::SqrDistance(rect1.Center(), rect2.Center()) << " == " << (Vector2::SqrDistance(rect1.Center(), rect2.Center()) > minDistanceSquared) << std::endl;

    // if (Vector2::SqrDistance(rect1.Center(), rect2.Center()) > minDistanceSquared)
    //   return false;

    return IsColliding(rect1, rect2, rotation1, rotation2);
  }
}

#endif