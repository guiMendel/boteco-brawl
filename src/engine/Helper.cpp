#include "Helper.h"

using namespace std;

void Helper::Assert(bool condition, string message, string explanation)
{
  if (condition == false)
    throw runtime_error(message + ". Reported error: " + explanation);
}

void Helper::Assert(bool condition, string message)
{
  Assert(condition, message, SDL_GetError());
}

float Helper::GetSign(float value, float caseZeroValue)
{
  if (value == 0)
    return caseZeroValue;

  return value > 0 ? 1 : -1;
}

auto Helper::SplitString(string text, string delimiter) -> vector<string>
{
  // Will hold the results
  vector<string> items;

  // Holds the position of the next delimiter found
  size_t next = 0;

  // Remember the last delimiter position
  size_t last = 0;

  // While there are delimiters to go
  while ((next = text.find(delimiter, last)) != string::npos)
  {
    // Push the item right before the found delimiter
    items.push_back(text.substr(last, next - last));

    // Advance the delimiter position
    last = next + delimiter.length();
  }

  // When there's an item left after the last delimiter, push it too
  if (last < text.length())
    items.push_back(text.substr(last));

  return items;
}

float Helper::RadiansToDegrees(float radians) { return radians * 180 / M_PI; }
float Helper::DegreesToRadians(float degrees) { return degrees / 180 * M_PI; }
double Helper::RadiansToDegrees(double radians) { return radians * 180 / M_PI; }
double Helper::DegreesToRadians(double degrees) { return degrees / 180 * M_PI; }
float Helper::RadiansToDegrees(int radians) { return float(radians) * 180 / M_PI; }
float Helper::DegreesToRadians(int degrees) { return float(degrees) / 180 * M_PI; }

int Helper::RandomRange(int min, int max) { return min + rand() % (max - min); }
float Helper::RandomRange(float min, float max)
{
  return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

size_t Helper::HashTwo(size_t a, size_t b) { return a >= b ? a * a + a + b : a + b * b; }

bool Helper::ThrowCoin(float chance)
{
  Assert(chance >= 0 && chance <= 1, "Chance must be a value in range [0, 1]");
  return RandomRange(0.0f, 1.0f) < chance;
}

float Helper::AngleDistance(float radiansA, float radiansB)
{
  // Get a priori distance
  float firstDistance = fmod(radiansB - radiansA, 2 * M_PI);

  // Check if the opposite direction is quicker
  if (2 * M_PI - abs(firstDistance) < abs(firstDistance))
    return firstDistance - 2 * M_PI;

  return firstDistance;
}
