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
  ~Vector() = default;
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
  Vector &operator*=(const double r) {
    x *= r;
    y *= r;
    z *= r;
    return *this;
  }
  /// Some Method
  auto size2() -> double { return x * x + y * y + z * z; }
  auto size() -> double { return sqrt(size2()); }
  auto dot(const Vector &v2) -> double {
    return x * v2.x + y * v2.y + z * v2.z;
  }
};

// operator

Vector operator+(const Vector &v1, const Vector &v2) {
  return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector operator-(const Vector &v1, const Vector &v2) {
  return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vector operator*(const Vector &v1, const double r) {
  return Vector(v1.x * r, v1.y * r, v1.z * r);
}

Vector operator*(const Vector &v1, const Vector &v2) {
  return Vector(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

Vector operator/(const Vector &v1, const double r) {
  return Vector(v1.x / r, v1.y / r, v1.z / r);
}

// Particle Structure
class Particle {
public:
  Vector position;
  Vector velocity;
  Vector acceleration;
  double pressure, minPressure;
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

  ~Particle() = default;
};
}; // namespace Mps
