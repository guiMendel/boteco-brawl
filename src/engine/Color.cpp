#include "Color.h"

using namespace std;
using namespace Helper;

// With each channel
Color::Color(int red, int green, int blue, int alpha) : red(red), green(green), blue(blue), alpha(alpha) {}

// With only colors
Color::Color(int red, int green, int blue) : Color(red, green, blue, 256) {}

// Copy constructor
Color::Color(const Color &other) : Color(other.red, other.green, other.blue, other.alpha) {}

// Default to white
Color::Color() : Color(White()) {}

// From SDL
Color::Color(const SDL_Color &other) : Color(other.r, other.g, other.b, other.a) {}

// Tells whether this color has valid values for all it's fields
bool Color::IsValid() const
{
  return red >= 0 && red < 255 && green >= 0 && green < 255 && blue >= 0 && blue < 255 && alpha >= 0 && alpha < 255;
}

float Color::ClampValid(float value) { return Clamp(value, 0.0f, 255.0f); }
Color Color::ClampValid(Color value)
{
  return Color(ClampValid(value.red), ClampValid(value.green), ClampValid(value.blue), ClampValid(value.alpha));
}

Color &Color::operator=(const Color &other)
{
  red = other.red;
  green = other.green;
  blue = other.blue;
  alpha = other.alpha;
  return *this;
}

Color Color::operator+(const Color &other) const
{
  return Color(
      red + other.red,
      green + other.green,
      blue + other.blue,
      alpha + other.alpha);
}

Color Color::operator-(const Color &other) const
{
  return Color(
      red - other.red,
      green - other.green,
      blue - other.blue,
      alpha - other.alpha);
}

Color Color::operator*(float value) const
{
  return Color(
      int(red * value),
      int(green * value),
      int(blue * value),
      int(alpha * value));
}

Color Color::operator/(float value) const
{
  Helper::Assert(value != 0, "Division by zero");

  return Color(
      int(red / value),
      int(green / value),
      int(blue / value),
      int(alpha / value));
}

// === SDL INTEGRATION

Color::operator SDL_Color() const { return SDL_Color{(Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha}; }
Color::operator std::string() const
{
  return "{ r: " + to_string(red) + ", g: " + to_string(green) + ", b: " + to_string(blue) + ", a: " + to_string(alpha) + " }";
}

Color operator*(float value, const Color &color)
{
  return color * value;
}

ostream &operator<<(ostream &stream, const Color &color)
{
  stream << (string)color;
  return stream;
}
