#ifndef __COLOR__
#define __COLOR__

#include "SDL.h"
#include "Helper.h"
#include <iostream>

struct Color
{
  // With each channel
  Color(int red, int green, int blue, int alpha);

  // With only colors
  Color(int red, int green, int blue);

  // Copy constructor
  Color(const Color &other);

  // Default to white
  Color();

  // From SDL
  Color(const SDL_Color &other);

  // === PREFAB COLORS

  static Color White() { return Color(255, 255, 255); }
  static Color Gray() { return Color(127, 127, 127); }
  static Color Black() { return Color(0, 0, 0); }
  static Color Red() { return Color(255, 0, 0); }
  static Color Green() { return Color(0, 255, 0); }
  static Color Blue() { return Color(0, 255, 255); }
  static Color Yellow() { return Color(255, 240, 18); }
  static Color Pink() { return Color(255, 0, 224); }
  static Color Cyan() { return Color(1, 247, 247); }

  // === GENERAL METHODS

  // Tells whether this color has valid values for all it's fields
  bool IsValid() const;

  // Clamps a value to a valid color value
  static float ClampValid(float value);
  static Color ClampValid(Color value);

  // === OPERATORS

  Color &operator=(const Color &other);

  Color operator+(const Color &other) const;

  Color operator-(const Color &other) const;

  Color operator*(float value) const;

  Color operator/(float value) const;

  // === SDL INTEGRATION

  operator SDL_Color() const;
  explicit operator std::string() const;

  // === FIELDS

  int red;
  int green;
  int blue;
  int alpha;
};

Color operator*(float value, const Color &color);
std::ostream &operator<<(std::ostream &stream, const Color &color);

#endif