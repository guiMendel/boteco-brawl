#include "Rectangle.h"

using namespace std;

Rectangle::Rectangle(const Vector2 &coordinates, float width, float height) : center(coordinates), width(width), height(height) {}

Rectangle::Rectangle(float x, float y, float width, float height) : Rectangle(Vector2(x, y), width, height) {}

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
bool Rectangle::Contains(const Vector2 &vector) const
{
  return vector.x >= center.x && vector.x <= center.x + width &&
         vector.y >= center.y && vector.y <= center.y + height;
}

Vector2 Rectangle::TopLeft(float pivoted) const
{
  return center.Pivot(Vector2(center.x - width / 2, center.y - height / 2), pivoted);
}
Vector2 Rectangle::BottomLeft(float pivoted) const
{
  return center.Pivot(Vector2(center.x - width / 2, center.y + height / 2), pivoted);
}
Vector2 Rectangle::BottomRight(float pivoted) const
{
  return center.Pivot(Vector2(center.x + width / 2, center.y + height / 2), pivoted);
}
Vector2 Rectangle::TopRight(float pivoted) const
{
  return center.Pivot(Vector2(center.x + width / 2, center.y - height / 2), pivoted);
}

vector<Vector2> Rectangle::Vertices(float pivoted) const
{
  return {TopRight(pivoted), BottomRight(pivoted), BottomLeft(pivoted), TopLeft(pivoted)};
}

// Convert to sdl rect
Rectangle::operator SDL_Rect() const { return SDL_Rect{(int)TopLeft().x, (int)TopLeft().y, (int)width, (int)height}; }

Rectangle::operator Vector2() const { return center; }

Rectangle::operator string() const
{
  return "{ x: " + to_string(center.x) + ", y: " + to_string(center.y) + ", w: " + to_string(width) + ", h: " + to_string(height) + " }";
}

Rectangle operator*(float value, const Rectangle &rectangle) { return rectangle * value; }
Rectangle operator/(float value, const Rectangle &rectangle)
{
  return Rectangle(value / rectangle.center, value / rectangle.width, value / rectangle.height);
}
ostream &operator<<(ostream &stream, const Rectangle &rectangle)
{
  stream << (string)rectangle;
  return stream;
}
