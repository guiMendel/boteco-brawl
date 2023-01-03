#include "Collision.h"
#include "Collider.h"

using namespace std;

size_t Collision::Data::GetHash() const
{
  auto source = weakSource.lock();
  auto other = weakOther.lock();

  // Ensure valid pointers
  Helper::Assert(source != nullptr && other != nullptr, "Failed to lock collision data's colliders");
  return Helper::HashTwo(source->id, other->id);
}

pair<float, Vector2> Collision::FindMinDistanceSingleSided(
    Rectangle rect1, Rectangle rect2, float rotation1, float rotation2)
{
  // Will keep track of the best distance found
  float bestDistance = numeric_limits<float>::lowest();

  // Will store the normal corresponding to the best distance
  Vector2 bestNormal;

  // Normal to be used in each iteration
  Vector2 normal = Vector2::Angled(rotation1);

  // Loop rect1 vertices
  for (Vector2 vertex1 : rect1.Vertices(rotation1))
  {
    // This vertex's minimum distance to rect2
    float minDistance = numeric_limits<float>::max();

    // Loop rect2 vertices
    for (Vector2 vertex2 : rect2.Vertices(rotation2))
    {
      // Check if this distance is smaller (project vertices distance on normal)
      minDistance = min(minDistance, Vector2::Dot(vertex2 - vertex1, normal));
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

  return make_pair(bestDistance, bestNormal);
}

pair<float, Vector2> Collision::FindMinDistance(
    Rectangle rect1, Rectangle rect2, float rotation1, float rotation2)
{
  auto distance1 = FindMinDistanceSingleSided(rect1, rect2, rotation1, rotation2);
  auto distance2 = FindMinDistanceSingleSided(rect2, rect1, rotation2, rotation1);

  return distance1.first >= distance2.first ? distance1 : distance2;
}

bool Collision::DetectIntersection(Rectangle rect, Vector2 point, float rotation)
{
  // Detects if point is inside projection of rectangle on a given axis
  auto DetectForAxis = [rect, rotation, point](Vector2 axis)
  {
    // Store lowest projection
    float lowestProjection = numeric_limits<float>::max();

    // Store biggest projection
    float biggestProjection = numeric_limits<float>::lowest();

    // Project each vertex
    for (auto vertex : rect.Vertices())
    {
      float projection = Vector2::Dot(vertex, axis);
      lowestProjection = min(lowestProjection, projection);
      biggestProjection = max(biggestProjection, projection);
    }

    // Get the own point's projection
    float pointProjection = Vector2::Dot(point, axis);

    // It's inside if it's between the lowest & biggest projections
    return lowestProjection <= pointProjection && pointProjection <= biggestProjection;
  };

  // First normal to be used
  Vector2 normal = Vector2::Angled(rotation);

  return DetectForAxis(normal) && DetectForAxis(normal.Rotated(M_PI / 2.0));
}