#include <cmath>
#include "Shape.h"

using namespace std;
using namespace Helper;

Shape::Shape(Vector2 center) : center(center) {}

ostream &operator<<(ostream &stream, const Shape &shape)
{
  return stream << (string)shape;
}

Shape::operator Vector2() const { return center; }

void Shape::Displace(Vector2 displacement) { center += displacement; }

void Shape::Rotate(float rotation) { this->rotation = fmod(this->rotation + rotation, 2 * M_PI); }
