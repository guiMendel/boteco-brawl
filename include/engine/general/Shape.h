#ifndef __SHAPE__
#define __SHAPE__

#include <iostream>
#include <string>
#include "Vector2.h"
#include "Helper.h"
#include "Color.h"

class Shape
{
public:
  Shape(const Vector2 coordinates);

  // === GENERAL METHODS

  // Indicates if a given coordinate is contained by the Shape
  virtual bool Contains(const Vector2 &vector) const = 0;

  // Gets the shape's area
  virtual float GetArea() const = 0;

  // Get the maximum possible length of this shape's projection on a line
  virtual float GetMaxDimension() = 0;

  // Get the minimum possible length of this shape's projection on a line
  virtual float GetMinDimension() = 0;

  // Scale shape's dimensions by the given amount
  // This uses axes relative to the object's rotation, not global axes
  virtual void Scale(Vector2 scale) = 0;

  // Displace it's center coordinates by the given amount
  void Displace(Vector2 displacement);

  // Rotate by the given amount
  void Rotate(float rotation);

  // Gets the offset for a random point inside the shape's area
  // The point is an offset relative to the shape's center of mass
  virtual Vector2 SamplePoint() const = 0;

  // Draws self on screen using debug
  virtual void DebugDrawAt(Vector2 position, Color color) = 0;

  // === CASTS

  virtual explicit operator std::string() const = 0;

  virtual explicit operator Vector2() const;

  // === PROPERTIES

  // The shape's center coordinates
  Vector2 center;

  // It's rotation angle
  // (When shape has vertices) There should always be a vertex at this corresponding angle
  float rotation{0};
};

std::ostream &operator<<(std::ostream &stream, const Shape &Shape);

#endif