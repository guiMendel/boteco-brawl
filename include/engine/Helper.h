#ifndef __HELPER__
#define __HELPER__

#include <cmath>
#include <stdexcept>
#include <vector>
#include <list>
#include <memory>
#include <string>
#include <iostream>
#include <utility>
#include <SDL.h>

#define RAD2DEG (180 / M_PI)

namespace Helper
{
  // Alias for unique pointer with destructor function
  template <class T>
  using auto_unique_ptr = std::unique_ptr<T, void (*)(T *)>;

  // Throws exception if condition is false
  void Assert(bool condition, std::string message, std::string explanation);

  // Throws exception if condition is false, uses SDL_Error as explanation
  void Assert(bool condition, std::string message);

  // Limits a value to a given range
  template <class T>
  T Clamp(T value, T minValue, T maxValue) { return std::max(std::min(value, maxValue), minValue); }

  // Returns -1 if value is negative, 1 otherwise
  float GetSign(float value);

  // Splits the given string into an array of strings, using the given delimiter as the separator token
  auto SplitString(std::string text, std::string delimiter) -> std::vector<std::string>;

  // Converts radians to degrees
  float RadiansToDegrees(float radians);
  // Converts degrees to radians
  float DegreesToRadians(float degrees);
  // Converts radians to degrees
  double RadiansToDegrees(double radians);
  // Converts degrees to radians
  double DegreesToRadians(double degrees);
  // Converts radians to degrees
  float RadiansToDegrees(int radians);
  // Converts degrees to radians
  float DegreesToRadians(int degrees);

  // Gets a random number in the range [min, max[
  int RandomRange(int min, int max);
  // Gets a random number in the range [min, max[
  float RandomRange(float min, float max);

  template <typename T>
  T Lerp(T min, T max, float amount)
  {
    // Range distance
    T rangeDistance = max - min;

    return min + amount * rangeDistance;
  }

  template <typename T>
  T Lerp(std::pair<T, T> range, float amount)
  {
    return Lerp(range.first, range.second, amount);
  }

  template <typename T>
  T RandomRange(T min, T max)
  {
    return Lerp(min, max, RandomRange(0.0f, 1.0f));
  }
  template <typename T>
  T RandomRange(std::pair<T, T> range)
  {
    return RandomRange(range.first, range.second);
  }

  // Gets a random valid index of the array
  template <typename T>
  int SampleIndex(T array[]) { return RandomRange(0, sizeof(array) / sizeof(T)); }
  // Gets a random valid index of the vector
  template <typename T>
  int SampleIndex(std::vector<T> array) { return RandomRange(0, array.size()); }

  // Gets a random member from the array
  template <typename T>
  int Sample(T array[]) { return array[SampleIndex(array)]; }
  // Gets a random member from the vector
  template <typename T>
  int Sample(std::vector<T> array) { return array[SampleIndex(array)]; }

  // Iterates through each weak pointer, removing those that are expired, and collecting the rest in a shared ptr collection
  template <typename T>
  std::list<std::shared_ptr<T>> ParseWeakIntoShared(std::list<std::weak_ptr<T>> &weakList)
  {
    std::list<std::shared_ptr<T>> finalList;

    // Start iterating
    auto weakIterator = weakList.begin();

    while (weakIterator != weakList.end())
    {
      // If it's expired remove it
      if (weakIterator->expired())
        weakIterator = weakList.erase(weakIterator);

      // Otherwise, add it to the list
      else
        finalList.push_back((weakIterator++)->lock());
    }

    return finalList;
  }

  // Apply Szudzik's hash function (https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way)
  size_t HashTwo(size_t a, size_t b);
}

#endif