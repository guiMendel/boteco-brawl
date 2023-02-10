#ifndef __ELLIPSE__
#define __ELLIPSE__

#include "Shape.h"

class Circle : public Shape
{
public:
  // Radius
  float radius;

  // === CONSTRUCTORS

  Circle(Vector2 center, float radius);
  Circle(float radius);
  Circle();
  Circle(const Circle &other);
  Circle(const Circle &&other);

  // === OPERATIONS

  Circle operator=(const Circle &other);

  Circle operator+(const Vector2 &vector) const;

  Circle operator-(const Vector2 &vector) const;

  Circle operator*(float value) const;

  Circle operator/(float value) const;

  Circle operator+=(const Vector2 &vector);

  Circle operator-=(const Vector2 &vector);

  // Indicates if a given coordinate is contained by the Circle
  bool Contains(const Vector2 &vector) const override;

  float GetArea() const override;

  // It's the diameter
  float GetMaxDimension() override;
  
  // It's the diameter
  float GetMinDimension() override;

  // Scale circle's dimensions by the given amount
  // The scale must have the same absolute value for both axes, otherwise an error will be raised
  void Scale(Vector2 scale) override;

  Vector2 SamplePoint() const override;
  
  void DebugDrawAt(Vector2 position, Color color) override;

  explicit operator std::string() const override;
};

Circle operator*(float value, const Circle &Circle);
Circle operator/(float value, const Circle &Circle);
Circle operator+(const Vector2 &vector, const Circle &Circle);
Circle operator-(const Vector2 &vector, const Circle &Circle);

#endif