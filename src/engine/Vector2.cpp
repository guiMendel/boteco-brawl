#include "Vector2.h"
#include "Helper.h"

using namespace std;
using namespace Helper;

// Initialize constructor
Vector2::Vector2(float x, float y) : x(x), y(y) {}

// Base constructor
Vector2::Vector2() : Vector2(0, 0) {}

// With initializer_list
Vector2::Vector2(initializer_list<float> list) : Vector2(*list.begin(), *(list.begin() + 1))
{
  Assert(list.size() == 2, "Vector2 initializer list must have exactly 2 values");
}

// Copy constructor
Vector2::Vector2(const Vector2 &other) : Vector2(other.x, other.y) {}

// Move constructor
Vector2::Vector2(const Vector2 &&other) : Vector2(other.x, other.y) {}

// === ALGEBRAIC OPERATIONS

Vector2 Vector2::operator=(const Vector2 &other)
{
  x = other.x;
  y = other.y;
  return *this;
}

bool Vector2::operator==(const Vector2 &other) const { return x == other.x && y == other.y; }
bool Vector2::operator!=(const Vector2 &other) const { return !(*this == other); }

Vector2 Vector2::operator+(const Vector2 &other) const { return Vector2(x + other.x, y + other.y); }

Vector2 Vector2::operator-(const Vector2 &other) const { return Vector2(x - other.x, y - other.y); }

Vector2 Vector2::operator*(const Vector2 &other) const { return Vector2(x * other.x, y * other.y); }

Vector2 Vector2::operator/(const Vector2 &other) const { return Vector2(x / other.x, y / other.y); }

Vector2 Vector2::operator*(float value) const { return Vector2(x * value, y * value); }

Vector2 Vector2::operator/(float value) const { return Vector2(x / value, y / value); }

Vector2 Vector2::operator+=(const Vector2 &other) { return *this = *this + other; }

Vector2 Vector2::operator-=(const Vector2 &other) { return *this = *this - other; }

Vector2 Vector2::operator*=(const Vector2 &other) { return *this = *this * other; }

Vector2 Vector2::operator/=(const Vector2 &other) { return *this = *this / other; }

Vector2 Vector2::operator*=(float value) { return *this = *this * value; }

Vector2 Vector2::operator/=(float value) { return *this = *this / value; }

Vector2 Vector2::operator-() const { return Vector2(*this) * -1; }

Vector2 operator*(float value, const Vector2 &vector) { return vector * value; }

Vector2 operator/(float value, const Vector2 &vector) { return Vector2(value / vector.x, value / vector.y); }

// === OTHER OPERATIONS

float Vector2::SqrMagnitude() const { return x * x + y * y; }

float Vector2::Magnitude()
{
  float sqrMagnitude = SqrMagnitude();

  if (lastSqrMagnitude != sqrMagnitude)
  {
    lastSqrMagnitude = sqrMagnitude;
    lastMagnitude = sqrt(lastSqrMagnitude);
  }

  return lastMagnitude;
}

void Vector2::SetMagnitude(float magnitude)
{
  float angle = Angle();
  x = magnitude * cos(angle);
  y = magnitude * sin(angle);
}

// Sets the magnitude to either it's current value or the given value, in case the current one is greater
Vector2 Vector2::CapMagnitude(float value)
{
  // Check that it does not exceed the limits
  if (SqrMagnitude() > value * value)
  {
    // Cap it
    SetMagnitude(value);
  }

  return *this;
}

Vector2 Vector2::Normalized()
{
  float sqrMagnitude = SqrMagnitude();

  if (sqrMagnitude == 0)
    return Vector2::Zero();

  return Vector2(*this) * InverseRoot(sqrMagnitude);
}

float Vector2::Angle() const { return atan2(y, x); }

float Vector2::AngleDegrees() const { return Angle() * 180 / M_PI; }

// Returns a vector rotated by the given angle, in radians
Vector2 Vector2::Rotated(float angle) const
{
  return Vector2(x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));
}

Vector2 Vector2::GetAbsolute() const { return {abs(x), abs(y)}; }

Vector2::operator bool() const { return x != 0 || y != 0; }

Vector2::operator string() const
{
  return "{ x: " + to_string(x) + ", y: " + to_string(y) + " }";
}

// === OPERATORS BETWEEN 2 VEC2S

float Vector2::SqrDistance(const Vector2 &v1, const Vector2 &v2) { return (v1 - v2).SqrMagnitude(); }

float Vector2::Distance(const Vector2 &v1, const Vector2 &v2) { return (v1 - v2).Magnitude(); }

float Vector2::AngleBetween(const Vector2 &v1, const Vector2 &v2) { return (v2 - v1).Angle(); }

float Vector2::Dot(const Vector2 &v1, const Vector2 &v2) { return v1.x * v2.x + v1.y * v2.y; }

// Pivots a point around this vector
Vector2 Vector2::Pivot(Vector2 point, float radians) const
{
  return (point - *this).Rotated(radians) + *this;
}

// === STATIC GETTERS

Vector2 Vector2::Up(float magnitude) { return Vector2(0, -magnitude); }
Vector2 Vector2::Left(float magnitude) { return Vector2(-magnitude, 0); }
Vector2 Vector2::Down(float magnitude) { return Vector2(0, magnitude); }
Vector2 Vector2::Right(float magnitude) { return Vector2(magnitude, 0); }
Vector2 Vector2::Zero() { return Vector2(0, 0); }
Vector2 Vector2::One() { return Vector2(1, 1); }
Vector2 Vector2::Angled(float angle, float magnitude) { return Vector2::Right(magnitude).Rotated(angle); }
Vector2 Vector2::AngledDegrees(float degrees, float magnitude) { return Vector2::Right(magnitude).Rotated(DegreesToRadians(degrees)); }

Vector2::operator SDL_Point() const { return {(int)round(x), (int)round(y)}; }

ostream &operator<<(ostream &stream, const Vector2 &vector)
{
  stream << (string)vector;
  return stream;
}