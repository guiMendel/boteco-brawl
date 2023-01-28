#ifndef __COLLISION__
#define __COLLISION__

#include "Rectangle.h"
#include "Helper.h"
#include <iostream>
#include <utility>
#include <memory>
#include <unordered_map>
#include <functional>

class Collider;

// Implementation of SAT collision, limited to rectangle polygons
// Based on this guide: https://youtu.be/-EsWKT7Doww
class Collision
{
public:
  struct Data
  {
    // Direction through which contact was initiated
    Vector2 normal;

    // How far along the normal penetration goes
    float penetration;

    // Collider of object that received contact
    std::weak_ptr<Collider> weakSource;

    // Collider of object that made contact
    std::weak_ptr<Collider> weakOther;

    // Returns a number which is unique for each pair of (source, other) colliders (order matters)
    size_t GetHash() const;
  };

  // Finds the minimum distance between both shape's edges
  // Negative values indicate penetration
  static std::pair<float, Vector2> FindMinDistance(std::shared_ptr<Shape> shape1, std::shared_ptr<Shape> shape2);

private:
  using distance_finder_map = std::unordered_map<
      std::string,
      std::function<std::pair<float, Vector2>(std::shared_ptr<Shape>, std::shared_ptr<Shape>)>>;

  static const distance_finder_map distanceFinder;
};

#endif