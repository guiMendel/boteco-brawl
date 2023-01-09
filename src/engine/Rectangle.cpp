#include "Rectangle.h"

using namespace std;

Rectangle::Rectangle(const Vector2 &coordinates, float width, float height) : Shape(coordinates), width(width), height(height) {}

Rectangle::Rectangle() : Rectangle(Vector2::Zero(), 0, 0) {}

Rectangle::Rectangle(const Rectangle &other) : Rectangle(other.center, other.width, other.height) {}

Rectangle::Rectangle(const Rectangle &&other) : Rectangle(other.center, other.width, other.height) {}

Rectangle::Rectangle(const SDL_Rect &rect) : Rectangle(Vector2(rect.x + rect.w / 2, rect.y + rect.h / 2), rect.w, rect.h) {}

Rectangle::Rectangle(TopLeftInitializerLabel, const Vector2 &coordinates, float width, float height)
    : Rectangle(coordinates + Vector2(width / 2, height / 2), width, height) {}

// === OPERATIONS

Rectangle Rectangle::operator=(const Rectangle &other)
{
  center = other.center;
  width = other.width;
  height = other.height;
  return *this;
}

Rectangle Rectangle::operator+(const Vector2 &vector) const
{
  return Rectangle(center + vector, width, height);
}

Rectangle Rectangle::operator-(const Vector2 &vector) const { return *this + -vector; }

Rectangle Rectangle::operator*(float value) const { return Rectangle(center * value, width * value, height * value); }
Rectangle Rectangle::operator/(float value) const { return Rectangle(center / value, width / value, height / value); }

// Indicates if a given coordinate is contained by the rectangle
bool Rectangle::Contains(const Vector2 &point) const
{
  // Detects if point is inside projection of rectangle on a given axis
  auto DetectForAxis = [this, point](Vector2 axis)
  {
    // Store lowest projection
    float lowestProjection = numeric_limits<float>::max();

    // Store biggest projection
    float biggestProjection = numeric_limits<float>::lowest();

    // Project each vertex
    for (auto vertex : Vertices())
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

float Rectangle::GetArea() const { return width * height; }

float Rectangle::GetMaxDimension()
{
  float partialDiagonal = width * width + height * height;

  if (lastPartialDiagonal != partialDiagonal)
  {
    lastPartialDiagonal = partialDiagonal;
    lastDiagonal = sqrt(lastPartialDiagonal);
  }

  return lastDiagonal;
}

float Rectangle::GetMinDimension() { return min(width, height); }

Vector2 Rectangle::TopLeft(float pivoted) const
{
  return PivotAroundCenter(Vector2(center.x - width / 2, center.y - height / 2), pivoted);
}
Vector2 Rectangle::BottomLeft(float pivoted) const
{
  return PivotAroundCenter(Vector2(center.x - width / 2, center.y + height / 2), pivoted);
}
Vector2 Rectangle::BottomRight(float pivoted) const
{
  return PivotAroundCenter(Vector2(center.x + width / 2, center.y + height / 2), pivoted);
}
Vector2 Rectangle::TopRight(float pivoted) const
{
  return PivotAroundCenter(Vector2(center.x + width / 2, center.y - height / 2), pivoted);
}

vector<Vector2> Rectangle::Vertices(float pivoted) const
{
  return {TopRight(pivoted), BottomRight(pivoted), BottomLeft(pivoted), TopLeft(pivoted)};
}

// Convert to sdl rect
Rectangle::operator SDL_Rect() const { return SDL_Rect{(int)TopLeft().x, (int)TopLeft().y, (int)width, (int)height}; }

Rectangle::operator string() const
{
  return "{ x: " + to_string(center.x) + ", y: " + to_string(center.y) + ", w: " + to_string(width) + ", h: " + to_string(height) + " }";
}

Rectangle operator*(float value, const Rectangle &rectangle) { return rectangle * value; }
Rectangle operator/(float value, const Rectangle &rectangle)
{
  return Rectangle(value / rectangle.center, value / rectangle.width, value / rectangle.height);
}

void Rectangle::Scale(Vector2 scale)
{
  width *= scale.x;
  height *= scale.y;
}

Vector2 Rectangle::PivotAroundCenter(Vector2 point, float angleOffset) const
{
  return center.Pivot(point, rotation + angleOffset);
}
