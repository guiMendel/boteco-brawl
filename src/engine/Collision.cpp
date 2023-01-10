#include "Collision.h"
#include "Collider.h"
#include "Rectangle.h"
#include "Circle.h"

#define CAST_SHAPE(oldVar, newVar, NewType)            \
  auto newVar = dynamic_pointer_cast<NewType>(oldVar); \
  Assert(newVar != nullptr, "Failed to get " #NewType " pointer " #newVar " in distance finder call");

#define SHAPE_ID(shape) string(typeid(shape).name())
#define SHAPE_CLASS_ID(Shape) SHAPE_ID(*make_shared<Shape>())

using namespace std;

// === LINE SEGMENT MATH

// Line segment defined by it's 2 extremities
using LineSegment = pair<Vector2, Vector2>;

// Get the point which is the given point's projection on a line segment
Vector2 ProjectPoint(Vector2 point, LineSegment segment);

// === COLLISION IMPLEMENTATION SIGNATURES

// Sat Collision for 2 rectangles
pair<float, Vector2> RectanglesDistance(shared_ptr<Shape> rect1, shared_ptr<Shape> rect2);
pair<float, Vector2> RectanglesDistanceCast(shared_ptr<Rectangle> rect1, shared_ptr<Rectangle> rect2);

// Collision for 2 circles
pair<float, Vector2> CirclesDistance(shared_ptr<Shape> circle1, shared_ptr<Shape> circle2);

// Collision for rectangle and circle
pair<float, Vector2> RectangleCircleDistance(shared_ptr<Shape> rect, shared_ptr<Shape> circle);

// Associate the above implementations to the map
const Collision::distance_finder_map Collision::distanceFinder{
    {SHAPE_CLASS_ID(Rectangle) + SHAPE_CLASS_ID(Rectangle), RectanglesDistance},
    {SHAPE_CLASS_ID(Circle) + SHAPE_CLASS_ID(Circle), CirclesDistance},
    {SHAPE_CLASS_ID(Rectangle) + SHAPE_CLASS_ID(Circle), RectangleCircleDistance}};

// === COLLISION DATA

size_t Collision::Data::GetHash() const
{
  auto source = weakSource.lock();
  auto other = weakOther.lock();

  // Ensure valid pointers
  Helper::Assert(source != nullptr && other != nullptr, "Failed to lock collision data's colliders");
  return Helper::HashTwo(source->id, other->id);
}

// === COLLISION METHODS

pair<float, Vector2> Collision::FindMinDistance(shared_ptr<Shape> shape1, shared_ptr<Shape> shape2)
{
  string shapeId1 = SHAPE_ID(*shape1);
  string shapeId2 = SHAPE_ID(*shape2);

  // Find the associated distance finder
  if (distanceFinder.count(shapeId1 + shapeId2) > 0)
    return distanceFinder.at(shapeId1 + shapeId2)(shape1, shape2);

  // If that order isn't there, the other order must be
  Assert(distanceFinder.count(shapeId2 + shapeId1) > 0,
         "No distance finder was registered for shapes " + shapeId1 + " and " + shapeId2);

  return distanceFinder.at(shapeId2 + shapeId1)(shape2, shape1);
}

// === LOCAL FUNCTION DEFINITIONS

Vector2 ProjectPoint(Vector2 point, LineSegment segment)
{
  // Get segment vector direction
  auto segmentDirection = (segment.second - segment.first).Normalized();

  // Get vector from segment start to point
  auto pointDistance = point - segment.first;

  // Project point vector on segment vector
  float projectionMagnitude = Vector2::Dot(pointDistance, segmentDirection);

  // Clamp magnitude to abide within line segment's limits
  projectionMagnitude = Clamp(projectionMagnitude, 0.0f, (segment.second - segment.first).Magnitude());

  // Get projection point by displacing segment start by the magnitude
  return segment.first + segmentDirection * projectionMagnitude;
}

pair<float, Vector2> RectanglesDistance(shared_ptr<Shape> rect1Shape, shared_ptr<Shape> rect2Shape)
{
  // Get the rectangles
  CAST_SHAPE(rect1Shape, rect1, Rectangle);
  CAST_SHAPE(rect2Shape, rect2, Rectangle);

  // Check from both perspectives
  auto distance1 = RectanglesDistanceCast(rect1, rect2);
  auto distance2 = RectanglesDistanceCast(rect2, rect1);

  return distance1.first >= distance2.first ? distance1 : distance2;
}

pair<float, Vector2> RectanglesDistanceCast(shared_ptr<Rectangle> rect1, shared_ptr<Rectangle> rect2)
{
  // Will keep track of the best distance found
  float bestDistance = numeric_limits<float>::lowest();

  // Will store the normal corresponding to the best distance
  Vector2 bestNormal;

  // Normal to be used in each iteration
  Vector2 normal = Vector2::Angled(rect1->rotation);

  // Loop rect1 vertices
  for (Vector2 vertex1 : rect1->Vertices())
  {
    // This vertex's minimum distance to rect2
    float vertexMinDistance = numeric_limits<float>::max();

    // Loop rect2 vertices
    for (Vector2 vertex2 : rect2->Vertices())
      // Check if this distance is smaller (project vertices distance on normal)
      vertexMinDistance = min(vertexMinDistance, Vector2::Dot(vertex2 - vertex1, normal));

    // Check if we got a better distance
    if (vertexMinDistance > bestDistance)
    {
      bestDistance = vertexMinDistance;
      bestNormal = normal;
    }

    // Advance normal for next iteration (clockwise)
    normal = normal.Rotated(M_PI / 2.0);
  }

  return make_pair(bestDistance, bestNormal);
}

pair<float, Vector2> CirclesDistance(shared_ptr<Shape> circle1Shape, shared_ptr<Shape> circle2Shape)
{
  // Get the circles
  CAST_SHAPE(circle1Shape, circle1, Circle);
  CAST_SHAPE(circle2Shape, circle2, Circle);

  // Get distance between circe's centers
  Vector2 centerDistance = circle2->center - circle1->center;

  // Get circle's distance
  float distance = centerDistance.Magnitude() - (circle2->radius + circle1->radius);

  return make_pair(distance, centerDistance.Normalized());
}

pair<float, Vector2> RectangleCircleDistance(shared_ptr<Shape> rectShape, shared_ptr<Shape> circleShape)
{
  // Get the shapes
  CAST_SHAPE(rectShape, rect, Rectangle);
  CAST_SHAPE(circleShape, circle, Circle);

  // Find rect's edge closest to circle center
  LineSegment bestEdge;
  Vector2 bestEdgeProjection;
  float bestEdgeSqrDistance = numeric_limits<float>::max();

  auto vertices = rect->Vertices();
  auto edgeStartIterator = vertices.end() - 1;
  for (auto edgeEndIterator = vertices.begin();
       edgeEndIterator != vertices.end();
       edgeStartIterator = edgeEndIterator++)
  {
    // Get the edge line segment
    LineSegment edge{*edgeStartIterator, *edgeEndIterator};

    // Project the circle's center on it
    auto circleProjection = ProjectPoint(circle->center, edge);
    float sqrDistance = Vector2::SqrDistance(circleProjection, circle->center);

    if (sqrDistance < bestEdgeSqrDistance)
    {
      bestEdgeSqrDistance = sqrDistance;
      bestEdge = edge;
      bestEdgeProjection = circleProjection;
    }
  }

  auto projectionDistance = circle->center - bestEdgeProjection;

  // Case 1: circle center in rectangle
  if (rect->Contains(circle->center))
    // Distance is from projection all the way to circle's opposite edge, so we sum the radius
    return make_pair(-(projectionDistance.Magnitude() + circle->radius), projectionDistance.Normalized());

  // Case 2: rectangle's edge crossing circle
  // Case 2 returns the exact same calculation as when no intersection is detected
  return make_pair(projectionDistance.Magnitude() - circle->radius, projectionDistance.Normalized());
}
