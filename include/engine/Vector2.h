#ifndef __VECTOR2__
#define __VECTOR2__

// Get access to Math constants (https://stackoverflow.com/questions/26065359/m-pi-flagged-as-undeclared-name)
#define _USE_MATH_DEFINES

#include <string>
#include <initializer_list>
#include <iostream>
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
  Vector2(float x, float y);

  // Base constructor
  Vector2();

  // With initializer_list
  Vector2(std::initializer_list<float> list);

  // Copy constructor
  Vector2(const Vector2 &other);

  // Move constructor
  Vector2(const Vector2 &&other);

  // === ALGEBRAIC OPERATIONS

  Vector2 operator=(const Vector2 &other);

  bool operator==(const Vector2 &other) const;
  bool operator!=(const Vector2 &other) const;

  Vector2 operator+(const Vector2 &other) const;

  Vector2 operator-(const Vector2 &other) const;

  Vector2 operator*(const Vector2 &other) const;

  Vector2 operator/(const Vector2 &other) const;

  Vector2 operator*(float value) const;

  Vector2 operator/(float value) const;

  Vector2 operator+=(const Vector2 &other);

  Vector2 operator-=(const Vector2 &other);

  Vector2 operator*=(const Vector2 &other);

  Vector2 operator/=(const Vector2 &other);

  Vector2 operator*=(float value);

  Vector2 operator/=(float value);

  Vector2 operator-() const;

  // === OTHER OPERATIONS

  float SqrMagnitude() const;

  float Magnitude();

  void SetMagnitude(float magnitude);

  // Sets the magnitude to either it's current value or the given value, in case the current one is greater
  Vector2 CapMagnitude(float value);

  Vector2 Normalized();

  float Angle() const;

  float AngleDegrees() const;

  // Returns a vector rotated by the given angle, in radians
  Vector2 Rotated(float angle) const;

  // Returns this vector with absolute values for it's coordinates
  Vector2 GetAbsolute() const;

  explicit operator bool() const;

  explicit operator std::string() const;

  // === OPERATORS BETWEEN 2 VEC2S

  static float SqrDistance(const Vector2 &v1, const Vector2 &v2);

  static float Distance(const Vector2 &v1, const Vector2 &v2);

  static float AngleBetween(const Vector2 &v1, const Vector2 &v2);

  static float Dot(const Vector2 &v1, const Vector2 &v2);

  // Pivots a point around this vector
  Vector2 Pivot(Vector2 point, float radians) const;

  // === STATIC GETTERS

  static Vector2 Up(float magnitude = 1);
  static Vector2 Left(float magnitude = 1);
  static Vector2 Down(float magnitude = 1);
  static Vector2 Right(float magnitude = 1);
  static Vector2 Zero();
  static Vector2 One();
  static Vector2 Angled(float angle, float magnitude = 1);
  static Vector2 AngledDegrees(float degrees, float magnitude = 1);

  explicit operator SDL_Point() const;

private:
  // Last result of SqrMagnitude
  float lastSqrMagnitude{-1};

  // Last result of magnitude
  float lastMagnitude;
};

Vector2 operator*(float value, const Vector2 &vector);
Vector2 operator/(float value, const Vector2 &vector);
std::ostream &operator<<(std::ostream &stream, const Vector2 &vector);

#endif