#include <bits/stdc++.h>

namespace Mps {

// constant
const int arraySize = 5000;
const double particleDistance = 0.025;
const double eps = 0.01 * particleDistance;

// Type
enum Type { ghost, fluid, wall, dummy };

// Original Vector Structure
class Vector {
public:
  double x, y, z;
  Vector(double x, double y, double z) : x(x), y(y), z(z) {}
  /// Some Method
  double distance2(Vector *v2) {
    return pow(x - v2->x, 2) + pow(y - v2->y, 2) + pow(z - v2->z, 2);
  }
  double distance(Vector *v2) { return sqrt(distance(v2)); }
};

// Particle Structure
class Particle {
public:
  Vector position;
  Vector velocity;
  Vector acceleration;
  double pressure, minPressure;
  double numberDensity;
  Type type;

  Particle()
      : position(Vector(0, 0, 0)), velocity(Vector(0, 0, 0)),
        acceleration(Vector(0, 0, 0)), pressure(0.0), minPressure(0.0),
        numberDensity(0.0), type(wall) {}
};

// variable
Particle particle[arraySize];
int numberOfParticles = 0;

bool judgeRegionThree(double x, double y, double z, double xFact, double yFact,
                      double zFact, double xOffset, double yOffset,
                      double zOffset, double yLowOffset = 0.0) {

  return x > -xFact * particleDistance + eps &&
         x <= xOffset + xFact * particleDistance + eps &&
         y > yLowOffset - yFact * particleDistance + eps &&
         y <= yOffset + eps && z > -zFact * particleDistance + eps &&
         z <= zOffset + zFact * particleDistance + eps;
}

void initParticle() {
  int nX = (int)(1.0 / particleDistance) + 5;
  int nY = (int)(0.6 / particleDistance) + 5;
  int nZ = (int)(0.3 / particleDistance) + 5;
  int i = 0;

  for (int iX = -4; iX < nX; iX++) {
    for (int iY = -4; iY < nY; iY++) {
      for (int iZ = -4; iZ < nZ; iZ++) {
        double x = particleDistance * (double)iX;
        double y = particleDistance * (double)iY;
        double z = particleDistance * (double)iZ;
        bool flagParticleGene = false;

        if (judgeRegionThree(x, y, z, 4.0, 4.0, 4.0, 1.0, 0.6, 0.3)) {
          particle[i].type = dummy;
          flagParticleGene = true;
        }
        if (judgeRegionThree(x, y, z, 2.0, 2.0, 2.0, 1.0, 0.6, 0.3)) {
          particle[i].type = wall;
          flagParticleGene = true;
        }
        if (judgeRegionThree(x, y, z, 4.0, 4.0, 4.0, 1.0, 0.6, 0.3, 0.6)) {
          particle[i].type = wall;
          flagParticleGene = true;
        }
        if (x > eps && x <= 1.0 + eps && y > eps) {
          flagParticleGene = false;
        }
        if (x > eps && x <= 0.25 + eps && y > eps && y <= 0.5 + eps) {
          particle[i].type = fluid;
          flagParticleGene = true;
        }
        if (flagParticleGene) {
          particle[i].position = Vector(x, y, z);
          i++;
        }
      }
    }
  }
  numberOfParticles = i;
}

}; // namespace Mps

int main(int argc, char **argv) { return 0; }
