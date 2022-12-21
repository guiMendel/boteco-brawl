#include "Color.h"

using namespace std;

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

Color &Color::operator=(const Color &other)
{
  red = other.red;
  green = other.green;
  blue = other.blue;
  alpha = other.alpha;
  return *this;
}

Color Color::operator+(const Color &other)
{
  return Color(
      CAP(red + other.red),
      CAP(green + other.green),
      CAP(blue + other.blue),
      CAP(alpha + other.alpha));
}

Color Color::operator-(const Color &other)
{
  return Color(
      CAP(red - other.red),
      CAP(green - other.green),
      CAP(blue - other.blue),
      CAP(alpha - other.alpha));
}

Color Color::operator*(float value)
{
  return Color(
      CAP(int(red * value)),
      CAP(int(green * value)),
      CAP(int(blue * value)),
      CAP(int(alpha * value)));
}

Color Color::operator/(float value)
{
  Helper::Assert(value != 0, "Division by zero");

  return Color(
      CAP(int(red / value)),
      CAP(int(green / value)),
      CAP(int(blue / value)),
      CAP(int(alpha / value)));
}

// === SDL INTEGRATION

Color::operator SDL_Color() const { return SDL_Color{(Uint8)red, (Uint8)green, (Uint8)blue, (Uint8)alpha}; }
Color::operator std::string() const
{
  return "{ r: " + to_string(red) + ", g: " + to_string(green) + ", b: " + to_string(blue) + ", a: " + to_string(alpha) + " }";
}

Color operator*(float value, const Color &color)
{
  return Color(
      CAP(int(color.red * value)),
      CAP(int(color.green * value)),
      CAP(int(color.blue * value)),
      CAP(int(color.alpha * value)));
}

ostream &operator<<(ostream &stream, const Color &color)
{
  stream << (string)color;
  return stream;
}
