#include <bits/stdc++.h>

namespace Mps {
// Type
enum Type { ghost = -1, fluid = 0, wall = 1, dummy = 2 };
enum Condition { immovable, surface, inner };
enum CheckCondition { ignored, connected, noconnect, checked };

// Original Vector Structure
class Vector {
public:
  double x, y, z;
  Vector() : x(0.0), y(0.0), z(0.0) {}
  Vector(double x, double y, double z) : x(x), y(y), z(z) {}
  ~Vector() {}
  /// operator
  Vector &operator+=(const Vector &v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }
  Vector &operator-=(const Vector &v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }
  Vector &operator*=(double r) {
    x *= r;
    y *= r;
    z *= r;
    return *this;
  }
  /// Some Method
  auto distance2(Vector *v2) -> double {
    return pow(x - v2->x, 2) + pow(y - v2->y, 2) + pow(z - v2->z, 2);
  }
  auto distance(Vector *v2) -> double { return sqrt(distance(v2)); }
  auto size2() -> double { return x * x + y * y + z * z; }
  auto size() -> double { return sqrt(size2()); }
};

// operator

Vector operator+(const Vector &v1, const Vector &v2) {
  return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector operator-(const Vector &v1, const Vector &v2) {
  return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vector operator*(const Vector &v1, double r) {
  return Vector(v1.x * r, v1.y * r, v1.z * r);
}

Vector operator*(const Vector &v1, const Vector &v2) {
  return Vector(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

Vector operator/(const Vector &v1, double r) {
  return Vector(v1.x / r, v1.y / r, v1.z / r);
}

// Particle Structure
class Particle {
public:
  Vector position;
  Vector velocity;
  Vector acceleration;
  double pressure, minPressure;
  double avgPressure;
  double numberDensity;
  Type type;
  Condition boundaryCondition;
  double sourceTerm;
  CheckCondition flagCondition;

  Particle()
      : position(Vector(0, 0, 0)), velocity(Vector(0, 0, 0)),
        acceleration(Vector(0, 0, 0)), pressure(0.0), minPressure(0.0),
        numberDensity(0.0), type(wall), boundaryCondition(immovable),
        sourceTerm(0.0), flagCondition(ignored) {}

  ~Particle() {}
};
}; // namespace Mps
