#include "mymps.h"

namespace Mps {

// constant
const int arraySize = 5000;
const double particleDistance = 0.075;
const double eps = 0.01 * particleDistance;
const double reNumberDensity = 2.1 * particleDistance;
const double reGradient = 2.1 * particleDistance;
const double reLaplacian = 3.1 * particleDistance;
const double fluidDensity = 1000.0;
const double collisionDistance = 0.5 * particleDistance;
const double gravity = -9.8;
const double kinematicViscosity = 1.0e-6;
const double dt = 0.003;
const double coefficientRestitution = 0.2;
const double threshold = 0.97;
const double relaxationCoefficient = 0.2;
const double compressibility = 0.45e-9;
const int outputInterval = 2;
const double finishTime = 2.0;

// variable
Particle particle[arraySize];
auto numberOfParticles = 0;
int fileNumber;
double simTime;
double n0NumberDensity;
double n0Gradient;
double n0Laplacian;
double lambda;
double coefficientMatrix[arraySize * arraySize];
// function
void writeData();
void calcGravity();
void calcViscosity();
void moveParticle();
void collision();
void calcPressure();
void calcPressureGradient();
void moveParticleResult();
void calcNumberDensity();
void setBoundaryCondition();
void setMatrix();
void solvePressure();
void removeNegativePressure();
void setMinPressure();
void exceptionBoundaryCondition();
void checkBoundaryCondition();
void increaseDiagonalTerm();
void setSourceTerm();

auto judgeRegionThree(double x, double y, double z, double xFact, double yFact,
                      double zFact, double xOffset, double yOffset,
                      double zOffset, double yLowOffset = 0.0) -> bool {

  return x > -xFact * particleDistance + eps &&
         x <= xOffset + xFact * particleDistance + eps &&
         y > yLowOffset - yFact * particleDistance + eps &&
         y <= yOffset + eps && z > -zFact * particleDistance + eps &&
         z <= zOffset + zFact * particleDistance + eps;
}

auto weight(double distance, double re) -> double {
  return distance >= re ? 0.0 : (re / distance) - 1.0;
}

void initParticle() {
  auto nX = (int)(1.0 / particleDistance) + 5;
  auto nY = (int)(0.6 / particleDistance) + 5;
  auto nZ = (int)(0.3 / particleDistance) + 5;
  auto i = 0;

  for (auto iX = -4; iX < nX; iX++) {
    for (auto iY = -4; iY < nY; iY++) {
      for (auto iZ = -4; iZ < nZ; iZ++) {
        auto x = particleDistance * (double)iX;
        auto y = particleDistance * (double)iY;
        auto z = particleDistance * (double)iZ;
        auto flagParticleGene = false;

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

void initConstant() {
  n0NumberDensity = n0Gradient = n0Laplacian = lambda = 0.0;
  for (auto iX = -4; iX < 5; iX++) {
    for (auto iY = -4; iY < 5; iY++) {
      for (auto iZ = -4; iZ < 5; iZ++) {
        if (iX == 0 && iY == 0 && iZ == 0) {
          continue;
        }
        Vector distj =
            Vector(particleDistance * (double)iX, particleDistance * (double)iY,
                   particleDistance * (double)iZ);
        n0NumberDensity += weight(distj.size(), reNumberDensity);
        n0Gradient += weight(distj.size(), reGradient);
        n0Laplacian += weight(distj.size(), reLaplacian);
        lambda += distj.size2() * weight(distj.size(), reLaplacian);
      }
    }
  }
  lambda /= n0Laplacian;

  fileNumber = 0;
  simTime = 0.0;
}

void simulate() {
  auto timeStep = 0;
  writeData();
  printf("Number of particles: %d\n", numberOfParticles);

  while (1) {
    calcGravity();
    calcViscosity();
    moveParticle();
    collision();
    calcPressure();
    calcPressureGradient();
    moveParticleResult();
    timeStep++;
    simTime += dt;
    if (timeStep % outputInterval == 0) {
      printf("TimeStepNumber: %4d Time: %lf(s) TimeStamp: %lf(s)\n", timeStep,
             simTime);
      writeData();
    }

    if (simTime >= finishTime) {
      break;
    }
  }
}

inline void calcGravity() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      particle[i].acceleration = Vector(0.0, gravity, 0.0);
    } else {
      particle[i].acceleration = Vector();
    }
  }
}

inline void calcViscosity() {
  double a = kinematicViscosity * 6.0 / (n0Laplacian * lambda);
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type != fluid) {
      continue;
    }
    Vector viscosity = Vector();
    for (auto j = 0; j < numberOfParticles; j++) {
      if (i == j || particle[i].type == ghost) {
        continue;
      }
      Vector distj = particle[j].position - particle[i].position;
      if (distj.size() < reLaplacian) {
        Vector diff = particle[j].velocity - particle[i].velocity;
        double w = weight(distj.size(), reLaplacian);
        viscosity += diff * w;
      }
    }
    viscosity *= a;
    particle[i].acceleration += viscosity;
  }
}

inline void moveParticle() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      particle[i].velocity += particle[i].acceleration * dt;
      particle[i].position += particle[i].velocity * dt;
    }
    particle[i].acceleration = Vector();
  }
}

inline void collision() {
  Vector velocities[numberOfParticles];
  for (auto i = 0; i < numberOfParticles; i++) {
    velocities[i] = particle[i].velocity;
  }
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      Vector velocity = particle[i].velocity;
      for (auto j = 0; j < numberOfParticles; j++) {
        if (j == i || particle[j].type == ghost) {
          continue;
        }
        Vector distj = particle[j].position - particle[i].position;
        if (distj.size2() < collisionDistance * collisionDistance) {
          double distance = distj.size();
          Vector force = (velocity - particle[j].velocity) * (distj / distance);
          double forceDT = force.x + force.y + force.z;
          if (forceDT > 0.0) {
            forceDT *= (1.0 + coefficientRestitution) / 2;
            velocity -= distj / distance * forceDT;
          }
        }
      }
      velocities[i] = velocity;
    }
  }
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      particle[i].position += (velocities[i] - particle[i].velocity) * dt;
      particle[i].velocity = velocities[i];
    }
  }
}

void calcPressure() {
  calcNumberDensity();
  setBoundaryCondition();
  setSourceTerm();
  setMatrix();
  solvePressure();
  removeNegativePressure();
  setMinPressure();
}

inline void calcNumberDensity() {
  for (auto i = 0; i < numberOfParticles; i++) {
    particle[i].numberDensity = 0.0;
    if (particle[i].type == ghost) {
      continue;
    }
    for (auto j = 0; j < numberOfParticles; j++) {
      if (j == i || particle[j].type == ghost) {
        continue;
      }
      Vector distj = particle[j].position - particle[i].position;
      particle[i].numberDensity += weight(distj.size(), reNumberDensity);
    }
  }
}

inline void setBoundaryCondition() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == ghost || particle[i].type == dummy) {
      particle[i].boundaryCondition = immovable;
    } else if (particle[i].numberDensity < threshold * n0NumberDensity) {
      particle[i].boundaryCondition = surface;
    } else {
      particle[i].boundaryCondition = inner;
    }
  }
}

inline void setSourceTerm() {
  for (auto i = 0; i < numberOfParticles; i++) {
    particle[i].sourceTerm = 0.0;
    if (particle[i].type == ghost || particle[i].type == dummy) {
      continue;
    }
    if (particle[i].boundaryCondition == inner) {
      particle[i].sourceTerm =
          relaxationCoefficient * (1.0 / (dt * dt)) *
          ((particle[i].numberDensity - n0NumberDensity) / n0NumberDensity);
    } else if (particle[i].boundaryCondition == surface) {
      particle[i].sourceTerm = 0.0;
    }
  }
}

inline void setMatrix() {
  auto n = numberOfParticles;
  auto a = 2.0 * 3.0 / (n0Laplacian * lambda);
  for (auto i = 0; i < numberOfParticles; i++) {
    for (auto j = 0; j < numberOfParticles; j++) {
      coefficientMatrix[i * n + j] = 0.0;
    }
  }
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].boundaryCondition != inner) {
      continue;
    }
    for (auto j = 0; j < numberOfParticles; j++) {
      if (j == i || particle[j].boundaryCondition == immovable) {
        continue;
      }
      Vector distj = particle[j].position - particle[i].position;
      if (distj.size() >= reLaplacian) {
        continue;
      }
      auto coefficientIJ = a * weight(distj.size(), reLaplacian) / fluidDensity;
      coefficientMatrix[i * n + j] = -1.0 * coefficientIJ;
      coefficientMatrix[i * n + i] += coefficientIJ;
    }
    coefficientMatrix[i * n + i] += compressibility / (dt * dt);
  }
  exceptionBoundaryCondition();
}

void exceptionBoundaryCondition() {
  checkBoundaryCondition();
  increaseDiagonalTerm();
}

inline void checkBoundaryCondition() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].boundaryCondition == immovable) {
      particle[i].flagCondition = ignored;
    } else if (particle[i].boundaryCondition == surface) {
      particle[i].flagCondition = connected;
    } else {
      particle[i].flagCondition = noconnect;
    }
  }

  auto count = 0;
  do {
    count = 0;
    for (auto i = 0; i < numberOfParticles; i++) {
      if (particle[i].flagCondition == connected) {
        for (auto j = 0; j < numberOfParticles; j++) {
          if (j == i || particle[j].type == ghost ||
              particle[j].type == dummy) {
            continue;
          }
          if (particle[j].flagCondition == noconnect) {
            Vector distj = particle[j].position - particle[i].position;
            if (distj.size2() >= reLaplacian * reLaplacian) {
              continue;
            }
            particle[j].flagCondition = connected;
          }
        }
        particle[i].flagCondition = checked;
        count++;
      }
    }
  } while (count != 0);

  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].flagCondition == noconnect) {
      fprintf(stderr,
              "WARNING: There is no dirichlet boundary condition for %d-th "
              "particle.\n",
              i);
    }
  }
}

inline void increaseDiagonalTerm() {
  auto n = numberOfParticles;
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].flagCondition == noconnect) {
      coefficientMatrix[i * n + i] *= 2.0;
    }
  }
}

inline void solvePressure() {
  auto n = numberOfParticles;
  for (auto i = 0; i < n; i++) {
    particle[i].pressure = 0.0;
  }
  for (auto i = 0; i < n - 1; i++) {
    if (particle[i].boundaryCondition != inner) {
      continue;
    }
    for (auto j = i + 1; j < n; j++) {
      if (particle[j].boundaryCondition == immovable) {
        continue;
      }
      auto c = coefficientMatrix[j * n + i] / coefficientMatrix[i * n + i];
      for (auto k = i + 1; k < n; k++) {
        coefficientMatrix[j * n + k] -= c * coefficientMatrix[i * n + k];
      }
      particle[j].sourceTerm -= c * particle[i].sourceTerm;
    }
  }
  for (auto i = n - 1; i >= 0; i--) {
    if (particle[i].boundaryCondition != inner) {
      continue;
    }
    auto sumOfTerms = 0.0;
    for (auto j = i + 1; j < n; j++) {
      if (particle[j].boundaryCondition == immovable) {
        continue;
      }
      sumOfTerms += coefficientMatrix[i * n + j] * particle[j].pressure;
    }
    particle[i].pressure =
        (particle[i].sourceTerm - sumOfTerms) / coefficientMatrix[i * n + i];
  }
}

inline void removeNegativePressure() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].pressure < 0.0)
      particle[i].pressure = 0.0;
  }
}

inline void setMinPressure() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == ghost || particle[i].type == dummy) {
      continue;
    }
    particle[i].minPressure = particle[i].pressure;
    for (auto j = 0; j < numberOfParticles; j++) {
      if (j == i || particle[j].type == ghost || particle[j].type == dummy) {
        continue;
      }
      Vector distj = particle[j].position - particle[i].position;
      if (distj.size2() >= reGradient * reGradient) {
        continue;
      }
      if (particle[i].minPressure > particle[j].pressure) {
        particle[i].minPressure = particle[j].pressure;
      }
    }
  }
}

inline void calcPressureGradient() {
  auto a = 3.0 / n0Gradient;
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type != fluid) {
      continue;
    }
    Vector gradient = Vector();
    for (auto j = 0; j < numberOfParticles; j++) {
      if (j == i || particle[j].type == ghost || particle[j].type == dummy) {
        continue;
      }
      Vector distj = particle[j].position - particle[i].position;
      if (distj.size() < reGradient) {
        auto w = weight(distj.size(), reGradient);
        auto pij =
            (particle[j].pressure - particle[i].minPressure) / distj.size2();
        gradient += distj * pij * w;
      }
    }
    gradient *= a;
    particle[i].acceleration = gradient / fluidDensity * (-1.0);
  }
}

inline void moveParticleResult() {
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      particle[i].velocity += particle[i].acceleration * dt;
      particle[i].position += particle[i].acceleration * dt * dt;
    }
    particle[i].acceleration = Vector();
  }
}

void writeData() {
  FILE *fp;
  char fileName[1024];

  sprintf(fileName, "mpsdata/particle_%04d.vtu", fileNumber);
  fp = fopen(fileName, "w");
  fprintf(fp, "<?xml version='1.0' encoding='UTF-8'?>\n");
  fprintf(fp, "<VTKFile xmlns='VTK' byte_order='LittleEndian' version='0.1' "
              "type='UnstructuredGrid'>\n");
  fprintf(fp, "<UnstructuredGrid>\n");
  fprintf(fp, "<Piece NumberOfCells='%d' NumberOfPoints='%d'>\n",
          numberOfParticles, numberOfParticles);
  fprintf(fp, "<Points>\n");
  fprintf(fp, "<DataArray NumberOfComponents='3' type='Float32' "
              "Name='Position' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%lf %lf %lf\n", particle[i].position.x, particle[i].position.y,
            particle[i].position.z);
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "</Points>\n");
  fprintf(fp, "<PointData>\n");
  fprintf(fp, "<DataArray NumberOfComponents='1' type='Int32' "
              "Name='ParticleType' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%d\n", particle[i].type);
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "<DataArray NumberOfComponents='1' type='Float32' "
              "Name='Velocity' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%f\n", (float)particle[i].velocity.size());
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "<DataArray NumberOfComponents='1' type='Float32' "
              "Name='pressure' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%f\n", (float)particle[i].pressure);
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "</PointData>\n");
  fprintf(fp, "<Cells>\n");
  fprintf(fp, "<DataArray type='Int32' Name='connectivity' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%d\n", i);
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "<DataArray type='Int32' Name='offsets' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%d\n", i + 1);
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "<DataArray type='UInt8' Name='types' format='ascii'>\n");
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "1\n");
  }
  fprintf(fp, "</DataArray>\n");
  fprintf(fp, "</Cells>\n");
  fprintf(fp, "</Piece>\n");
  fprintf(fp, "</UnstructuredGrid>\n");
  fprintf(fp, "</VTKFile>\n");
  fclose(fp);
  fileNumber++;
}

}; // namespace Mps

int main(int argc, char **argv) {

  printf("\n*** START PARTICLE-SIMULATION ***\n");
  Mps::initParticle();
  printf("*** INITIALIZE PARTICLE ***\n");
  Mps::initConstant();
  printf("*** INITIALIZE CONSTANT ***\n*** START MAIN LOOP ***\n");
  auto timerStart = std::chrono::system_clock::now();
  Mps::simulate();
  auto timerEnd = std::chrono::system_clock::now();
  printf("Total: %13.6lf sec\n",
         std::chrono::duration_cast<std::chrono::seconds>(timerEnd - timerStart)
             .count());
  printf("*** END ***\n\n");
  return 0;
}
