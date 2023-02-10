#ifndef __RECTANGLE__
#define __RECTANGLE__

#include <initializer_list>
#include <vector>
#include <SDL.h>
#include <iostream>
#include "Shape.h"

class Rectangle : public Shape
{
public:
  // Used for initializing rectangle with top left coordinates
  enum TopLeftInitializerLabel
  {
    TopLeftInitialize
  };

  // Width
  float width;

  // Height
  float height;

  // === CONSTRUCTORS

  Rectangle(const Vector2 &coordinates, float width, float height);
  Rectangle(float width, float height);
  Rectangle(std::initializer_list<float> list) = delete;
  Rectangle();
  Rectangle(const Rectangle &other);
  Rectangle(const Rectangle &&other);
  Rectangle(const SDL_Rect &rect);
  // Interprets coordinates as top left
  Rectangle(TopLeftInitializerLabel, const Vector2 &coordinates, float width, float height);

  // === OPERATIONS

  Rectangle operator=(const Rectangle &other);
  Rectangle operator+(const Vector2 &vector) const;
  Rectangle operator-(const Vector2 &vector) const;
  Rectangle operator*(float value) const;
  Rectangle operator/(float value) const;

  // Indicates if a given coordinate is contained by the rectangle
  bool Contains(const Vector2 &vector) const override;

  // Gets the rect's area
  float GetArea() const override;

  // Get the rect's diagonal
  float GetMaxDimension() override;

  // Get the length of the minimum side
  float GetMinDimension() override;

  // Scale rect's dimensions by the given amount
  // This uses axes relative to the object's rotation, not global axes
  void Scale(Vector2 scale) override;

  Vector2 SamplePoint() const override;

  void DebugDrawAt(Vector2 position, Color color) override;

  Vector2 TopLeft(float pivoted = 0.0f) const;
  Vector2 BottomLeft(float pivoted = 0.0f) const;
  Vector2 BottomRight(float pivoted = 0.0f) const;
  Vector2 TopRight(float pivoted = 0.0f) const;

  std::vector<Vector2> Vertices(float pivoted = 0.0f) const;

  // Convert to sdl rect
  explicit operator SDL_Rect() const;

  explicit operator std::string() const override;

private:
  // Pivot's a point around the rectangle's center to the angle of the rect's rotation plus an offset
  Vector2 PivotAroundCenter(Vector2 point, float angleOffset = 0) const;

  // Last value of partial diagonal calculation
  float lastPartialDiagonal{-1};

  // Last value of complete diagonal calculation
  float lastDiagonal;
};

Rectangle operator*(float value, const Rectangle &rectangle);
Rectangle operator/(float value, const Rectangle &rectangle);

#endif