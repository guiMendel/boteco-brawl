#ifndef __HELPER__
#define __HELPER__

#include <cmath>
#include <stdexcept>
#include <vector>
#include <memory>
#include <string>
#include <SDL.h>

namespace Helper
{
  // Alias for unique pointer with destructor function
  template <class T>
  using auto_unique_ptr = std::unique_ptr<T, void (*)(T *)>;

  // Throws exception if condition is false
  [[maybe_unused]] static void Assert(bool condition, std::string message, std::string explanation)
  {
    if (condition == false)
    {
      throw std::runtime_error(message + ". Reported error: " + explanation);
    }
  }

  // Throws exception if condition is false
  [[maybe_unused]] static void Assert(bool condition, std::string message)
  {
    Assert(condition, message, SDL_GetError());
  }

  // Splits the given string into an array of strings, using the given delimiter as the separator token
  [[maybe_unused]] static auto SplitString(std::string text, std::string delimiter) -> std::vector<std::string>
  {
    // Will hold the results
    std::vector<std::string> items;

    // Holds the position of the next delimiter found
    size_t next = 0;

    // Remember the last delimiter position
    size_t last = 0;

    // While there are delimiters to go
    while ((next = text.find(delimiter, last)) != std::string::npos)
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

  // Converts radians to degrees
  [[maybe_unused]] static float RadiansToDegrees(float radians) { return radians * 180 / M_PI; }
  // Converts degrees to radians
  [[maybe_unused]] static float DegreesToRadians(float degrees) { return degrees / 180 * M_PI; }
  // Converts radians to degrees
  [[maybe_unused]] static double RadiansToDegrees(double radians) { return radians * 180 / M_PI; }
  // Converts degrees to radians
  [[maybe_unused]] static double DegreesToRadians(double degrees) { return degrees / 180 * M_PI; }

  // Gets a random number in the range [min, max[
  [[maybe_unused]] static int RandomRange(int min, int max) { return min + rand() % (max - min); }
  // Gets a random number in the range [min, max[
  [[maybe_unused]] static float RandomRange(float min, float max)
  {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
  }

  // Gets a random valid index of the array
  template <typename T>
  [[maybe_unused]] static int SampleIndex(T array[]) { return RandomRange(0, sizeof(array) / sizeof(T)); }
  // Gets a random valid index of the vector
  template <typename T>
  [[maybe_unused]] static int SampleIndex(std::vector<T> array) { return RandomRange(0, array.size()); }

  // Gets a random member from the array
  template <typename T>
  [[maybe_unused]] static int Sample(T array[]) { return array[SampleIndex(array)]; }
  // Gets a random member from the vector
  template <typename T>
  [[maybe_unused]] static int Sample(std::vector<T> array) { return array[SampleIndex(array)]; }

}

#endif