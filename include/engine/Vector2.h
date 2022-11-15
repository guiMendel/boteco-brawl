#ifndef __VECTOR2__
#define __VECTOR2__

// Get access to Math constants (https://stackoverflow.com/questions/26065359/m-pi-flagged-as-undeclared-identifier)
#define _USE_MATH_DEFINES

#include <string>
#include <initializer_list>
#include <cmath>
#include <SDL.h>

class Vector2
{
public:
  // X coordinate
  float x;

  // Y coordinate
  float y;

  // Initialize constructor
  Vector2(float x, float y) : x(x), y(y) {}

  // Base constructor
  Vector2() : Vector2(0, 0) {}

  // With initializer_list
  Vector2(std::initializer_list<float> list) : x(*list.begin()), y(*(list.begin() + 1)) {}

  // Copy constructor
  Vector2(const Vector2 &other) : x(other.x), y(other.y) {}

  // Move constructor
  Vector2(const Vector2 &&other) : x(other.x), y(other.y) {}

  // === ALGEBRAIC OPERATIONS

  Vector2 operator=(const Vector2 &other)
  {
    x = other.x;
    y = other.y;
    return *this;
  }

  Vector2 operator+(const Vector2 &other) const { return Vector2(x + other.x, y + other.y); }

  Vector2 operator-(const Vector2 &other) const { return Vector2(x - other.x, y - other.y); }

  Vector2 operator*(const Vector2 &other) const { return Vector2(x * other.x, y * other.y); }

  Vector2 operator/(const Vector2 &other) const { return Vector2(x / other.x, y / other.y); }

  Vector2 operator*(float value) const { return Vector2(x * value, y * value); }

  Vector2 operator/(float value) const { return Vector2(x / value, y / value); }

  Vector2 operator+=(const Vector2 &other) { return *this = *this + other; }

  Vector2 operator-=(const Vector2 &other) { return *this = *this - other; }

  Vector2 operator*=(const Vector2 &other) { return *this = *this * other; }

  Vector2 operator/=(const Vector2 &other) { return *this = *this / other; }

  Vector2 operator*=(float value) { return *this = *this * value; }

  Vector2 operator/=(float value) { return *this = *this / value; }

  Vector2 operator-() const { return Vector2(*this) * -1; }

  // === OTHER OPERATIONS

  float SqrMagnitude() const { return x * x + y * y; }

  float Magnitude() const { return sqrt(SqrMagnitude()); }

  void SetMagnitude(float magnitude)
  {
    float angle = Angle();
    x = magnitude * cos(angle);
    y = magnitude * sin(angle);
  }

  // Sets the magnitude to either it's current value or the given value, in case the current one is greater
  void CapMagnitude(float value)
  {
    // Check that it does not exceed the limits
    if (SqrMagnitude() > value * value)
    {
      // Cap it
      SetMagnitude(value);
    }
  }

  Vector2 Normalized() const
  {
    float magnitude = Magnitude();

    if (!magnitude)
      return Vector2::Zero();

    return Vector2(*this) / magnitude;
  }

  float Angle() const { return atan2(y, x); }

  float AngleDegrees() const { return Angle() * 180 / M_PI; }

  // Returns a vector rotated by the given angle, in radians
  Vector2 Rotated(float angle) const
  {
    return Vector2(x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));
  }

  explicit operator bool() const { return x != 0 || y != 0; }

  explicit operator std::string() const
  {
    return "{ x: " + std::to_string(x) + ", y: " + std::to_string(y) + " }";
  }

  // === OPERATORS BETWEEN 2 VEC2S

  static float SqrDistance(const Vector2 &v1, const Vector2 &v2) { return (v1 - v2).SqrMagnitude(); }

  static float Distance(const Vector2 &v1, const Vector2 &v2) { return (v1 - v2).Magnitude(); }

  static float AngleBetween(const Vector2 &v1, const Vector2 &v2) { return (v2 - v1).Angle(); }

  static float Dot(const Vector2 &v1, const Vector2 &v2) { return v1.x * v2.x + v1.y * v2.y; }

  // === STATIC GETTERS

  static Vector2 Up(float magnitude = 1) { return Vector2(0, magnitude); }
  static Vector2 Left(float magnitude = 1) { return Vector2(-magnitude, 0); }
  static Vector2 Down(float magnitude = 1) { return Vector2(0, -magnitude); }
  static Vector2 Right(float magnitude = 1) { return Vector2(magnitude, 0); }
  static Vector2 Zero() { return Vector2(0, 0); }
  static Vector2 One() { return Vector2(1, 1); }
  static Vector2 Angled(float angle, float magnitude = 1) { return Vector2::Right(magnitude).Rotated(angle); }

  explicit operator SDL_Point() const { return {(int)round(x), (int)round(y)}; }
};

#endif