#include <bits/stdc++.h>

// MARK: - Constant
namespace {
const int DIMENSION = 3;
const double L0 = 0.075;
const double DT = 0.003;
const int OUTPUT_INTERVAL = 2;
const double FINISH_TIME = 2.0;

const int ARRAY_SIZE = 5000;
const double FINISH_TIME = 2.0;
const double KINEMATIC_VISCOSITY = 1.0E-6;
const double FLUID_DENSITY = 1000.0;
const double EPS = 0.01 * PARTICLE_DISTANCE;
const int GHOST = -1;
const int FLUID = 0;
const int WALL = 2;
const int DUMMY_WALL = 3;
const double G_X = 0.0;
const double G_Y = -9.8;
const double G_Z = 0.0;
const double COEFFICIENT_OF_RESTITUTION = 0.2;
} // namespace

// MARK: - Variables
static double acceleration[3 * ARRAY_SIZE];
static double position[3 * ARRAY_SIZE];
static double velocity[3 * ARRAY_SIZE];
static int particleTypes[ARRAY_SIZE];

// MARK: - Particle class

class Particle {
public:
  void initParticleState(int dimension);
  void calcConstantParam();
  void calcNZeroLambda();
  double weight(double distance, double re);
  void simulate();
  void calcGravity();
  void calcViscosity();
  void moveParticle();
  void collision();
  void calcPressure();
  void calcDensity();
  void setBoundaryCond();
  void setSourceTerm();
  void setMatrix();
  void processBoundaryCond();
  void checkBoundaryCond();
  void increaseDiagonalTerm();
  void solveGaussElim();
  void removeNegPressure();
  void setMinimumPressure();
  void calcPressureGradient();
  void moveParticleByPressureGradient();
  void writeData(string type);

private:
  int fileNum;
  double times;
  int numberOfParticles;
  double reNumDensity, re2NumDensity;
  double reGradient, re2Gradient;
  double reLaplacian, re2Laplacian;
  double fluidDensity;
  double colDistance, colDistance2;
  double n0NumDensity;
  double n0Gradient;
  double n0Laplacian;
  double lambda;
};

int main(int argc, char **argv) {}

bool judgeRegionThree(double x, double y, double z, double xFact, double yFact,
                      double zFact, double xOffset, double yOffset,
                      double zOffset, double yLowOffset = 0.0) {

  return x > -xFact * PARTICLE_DISTANCE + EPS &&
         x <= xOffset + xFact * PARTICLE_DISTANCE + EPS &&
         y > yLowOffset - yFact * PARTICLE_DISTANCE + EPS &&
         y <= yOffset + EPS && z > -zFact * PARTICLE_DISTANCE + EPS &&
         z <= zOffset + zFact * PARTICLE_DISTANCE + EPS
}

void Particle::initParticleState(int dimension) {

  if (dimension == 3) {
    int nx, ny, nz;
    double x, y, z;
    int i = 0;
    bool flagParticleGen;

    nx = (int)(1.0 / PARTICLE_DISTANCE) + 5;
    ny = (int)(0.6 / PARTICLE_DISTANCE) + 5;
    nz = (int)(0.3 / PARTICLE_DISTANCE) + 5;
    for (int ix = -4; ix < nx; ix++) {
      for (int iy = -4; iy < ny; iy++) {
        for (int iz = -4; iz < nz; iz++) {
          x = PARTICLE_DISTANCE * ix;
          y = PARTICLE_DISTANCE * iy;
          z = PARTICLE_DISTANCE * iz;
          flagParticleGen = false;

          /// dummy wall region
          if (judgeRegionThree(x, y, z, 4.0, 4.0, 4.0, 1.0, 0.6, 0.3)) {
            particleTypes[i] = DUMMY_WALL;
            flagParticleGen = true;
          }
          /// wall region
          if (judgeRegionThree(x, y, z, 2.0, 2.0, 2.0, 1.0, 0.6, 0.3)) {
            particleTypes[i] = WALL;
            flagParticleGen = true;
          }
          if (judgeRegionThree(x, y, z, 4.0, 4.0, 4.0, 1.0, 0.6, 0.3, 0.6)) {
            particleTypes[i] = WALL;
            flagParticleGen = true;
          }
          /// empty region
          if (x > EPS && x <= 1.0 + EPS && y > EPS && z > EPS &&
              z <= 0.3 + EPS) {
            flagParticleGen = false;
          }
          /// fluid region
          if (x > EPS && x <= 0.25 + EPS && y > EPS && y < 0.5 + EPS &&
              z > EPS && z <= 0.3 + EPS) {
            particleTypes[i] = FLUID;
            flagParticleGen = true;
          }

          if (flagParticleGen) {
            position[i * 3] = x;
            position[i * 3 + 1] = y;
            pos[i * 3 + 2] = z;
            i++;
          }
        }
      }
    }
    numberOfParticles = i;
    for (i = 0; i < numberOfParticles * 3; i++) {
      vel[i] = 0.0;
    }
  }
}

/// Reなどを定義
void Particle::calcConstantParam() {

  reNumDensity = 2.1 * PARTICLE_DISTANCE;
  reGradient = 2.1 * PARTICLE_DISTANCE;
  reLaplacian = 3.1 * PARTICLE_DISTANCE;
  re2NumDensity = pow(reNumDensity, 2.0);
  re2Gradient = pow(reGradient, 2.0);
  re2Laplacian = pow(reLaplacian, 2.0);
  calcNZeroLambda();
  fluidDensity = 1000.0;
  colDistance = 0.5 * PARTICLE_DISTANCE;
  colDistance2 = pow(colDistance, 2.0);
  fileNum = 0;
  times = 0.0;
}

/// n0とlambda0を計算
void Particle::calcNZeroLambda() {

  int izStart, izEnd;
  double xj, yj, zj, distance, distance2;
  double xi, yi, zi;

  if (DIMENSION == 2) {
    izStart = 0;
    izEnd = 1;
  } else {
    izStart = -4;
    izEnd = 5;
  }

  n0NumDensity = 0.0;
  n0Gradient = 0.0;
  n0Laplacian = 0.0;
  lambda = 0.0;
  xi = yi = zi = 0.0;

  for (int ix = -4; ix < 5; ix++) {
    for (int iy = -4; iy < 5; iy++) {
      for (int iz = izStart; iz < izEnd; iz++) {
        if (ix == 0 && iy == 0 && iz == 0)
          continue;

        xj = PARTICLE_DISTANCE * (double)ix;
        yj = PARTICLE_DISTANCE * (double)iy;
        zj = PARTICLE_DISTANCE * (double)iz;

        distance2 = pow(xj - xi, 2.0) + pow(yj - yi, 2.0) + pow(zj - zi, 2.0);
        distance = sqrt(distance2);

        n0NumDensity += weight(distance, reNumDensity);
        n0Gradient += weight(distance, reGradient);
        n0Laplacian += weight(distance, reLaplacian);
        lambda += distance2 * weight(distance, reLaplacian);
      }
    }
  }

  lambda /= n0Laplacian;
}

/// 重み付けの w(x)を計算
double Particle::weight(double distance, double re) {

  if (distance >= re) {
    return 0.0
  } else {
    return (re / distance) - 1.0;
  }
}

/// シミュレーションのメイン過程
void Particle::simulate() {

  int timeStep = 0;

  writeData("vtu");

  while (1) {
    calcGravity();
    calcViscosity();
    moveParticle();
    collision();
    calcPressure();
    calcPressureGradient();
    moveParticleByPressureGradient();

    timeStep++;
    times += DT;
    if (timeStep % OUTPUT_INTERVAL == 0) {
      printf("TimeStepNumber: %4d  Time: %lf(s)  NumberOfParticle: %d\n",
             timeStep, times, numberOfParticles);
      writeData("vtu");
    }

    if (times >= FINISH_TIME) {
      break;
    }
  }
}

/// まず重力のかかる部分に重力をかける
void Particle::calcGravity() {
  for (int i = 0; i < numberOfParticles; i++) {
    if (particleTypes[i] == FLUID) {
      acceleration[i * 3] = G_X;
      acceleration[i * 3 + 1] = G_Y;
      acceleration[i * 3 + 2] = G_Z;
    } else {
      acceleration[i * 3] = acceleration[i * 3 + 1] = acceleration[i * 3 + 2] =
          0.0;
    }
  }
}

/// 粘性を計算する
void Particle::calcViscosity() {
  double viscosityX, viscosityY, viscosityZ;
  double distance, distance2;
  double w;
  double xij, yij, zij;
  double a;

  /// 2.35式の係数部分
  a = KINEMATIC_VISCOSITY * 2.0 * DIMENSION / (n0Laplacian * lambda);
  for (int i = 0; i < numberOfParticles; i++) {
    if (particleTypes[i] != FLUID)
      continue;
    viscosityX = viscosityY = viscosityZ = 0.0;

    for (int j = 0; j < numberOfParticles; j++) {
      if (j == i || particleTypes[j] == GHOST)
        continue;
      xij = position[j * 3] - position[i * 3];
      yij = position[j * 3 + 1] - position[i * 3 + 1];
      zij = position[j * 3 + 2] - position[i * 3 + 2];
      distance2 = pow(xij, 2) + pow(yij, 2) + pow(zij, 2);
      distance = sqrt(distance2);
      if (distance < reLaplacian) {
        w = weight(distance, reLaplacian);

        viscosityX += (velocity[j * 3] - velocity[i * 3]) * w;
        viscosityY += (velocity[j * 3 + 1] - velocity[i * 3 + 1]) * w;
        viscosityZ += (velocity[j * 3 + 2] - velocity[i * 3 + 2]) * w;
      }
      viscosityX = viscosityX * a;
      viscosityY = viscosityY * a;
      viscosityZ = viscosityZ * a;
      acceleration[i * 3] += viscosityX;
      acceleration[i * 3 + 1] += viscosityY;
      acceleration[i * 3 + 2] += viscosityZ;
    }
  }
}

/// ���������������������������際に一度仮で動かしてみる部分
void Particle::moveParticle() {

  for (int i = 0; i < numberOfParticles; i++) {
    if (particleTypes[i] == FLUID) {
      velocity[i * 3] += acceleration[i * 3] * DT;
      velocity[i * 3 + 1] += acceleration[i * 3 + 1] * DT;
      velocity[i * 3 + 2] += acceleration[i * 3 + 2] * DT;

      position[i * 3] += velocity[i * 3] * DT;
      position[i * 3 + 1] += velocity[i * 3 + 1] * DT;
      position[i * 3 + 2] += velocity[i * 3 + 2] * DT;
    }

    acceleration[i * 3] = 0.0;
    acceleration[i * 3 + 1] = 0.0;
    acceleration[i * 3 + 2] = 0.0;
  }
}

/// 剛体衝突を起こして近づきすぎるのを防ぐ
void Particle::collision() {
  double xij, yij, zij;
  double distance, distance2;
  // 粒子間に働くインパルス
  double forceDT;
  double mi, mj;
  double velocityX, velocityY, velocityZ;
  double e = COEFFICIENT_OF_RESTITUTION;
  static double velocityCollision[3 * ARRAY_SIZE];

  for (int i = 0; i < 3 * numberOfParticles; i++) {
    velocityCollision[i] = velocity[i];
  }
  for (int i = 0; i < 3 * numberOfParticles; i++) {
    if (particleTypes[i] == FLUID) {
      mi = fluidDensity;
      velocityX = velocity[i * 3];
      velocityY = velocity[i * 3 + 1];
      velocityZ = velocity[i * 3 + 2];
      for (int j = 0; j < numberOfParticles; j++) {
        if (j == i || particleTypes[j] == GHOST)
          continue;
        xij = position[j * 3] - position[i * 3];
        yij = position[j * 3 + 1] - position[i * 3 + 1];
        zij = position[j * 3 + 2] - position[i * 3 + 2];
        distance2 = pow(xij, 2.0) + pow(yij, 2.0) + pow(zij, 2.0);
        if (distance2 < collisionDistance2) {
          distance = sqrt(distance2);
          // 衝突した力
          forceDT = (velocityX - velocity[j * 3]) * xij / distance +
                    (velocityY - velocity[j * 3 + 1]) * yij / distance +
                    (velocityZ - velocity[j * 3 + 2]) * zij / distance;
          if (forceDT > 0.0) {
            mj = fluidDensity;
            forceDT *= (1.0 + e) * mi * mj / (mi + mj);
            velocityX -= forceDT / mi * xij / distance;
            velocityY -= forceDT / mi * yij / distance;
            velocityZ -= forceDT / mi * zij / distance;
            if (j > i) {
              fprintf(
                  stderr,
                  "WARNING: Collision occured between %d and %d particles.\n",
                  i, j);
            }
          }
        }
        velocityCollision[i * 3] = velocityX;
        velocityCollision[i * 3 + 1] = velocityY;
        velocityCollision[i * 3 + 2] = velocityZ;
      }
    }
    for (int i = 0; i < numberOfParticles; i++) {
      if (particleTypes[i] == FLUID) {
        position[i * 3] += (velocityCollision[i * 3] - velocity[i * 3]) * DT;
        position[i * 3 + 1] +=
            (velocityCollision[i * 3 + 1] - velocity[i * 3 + 1]) * DT;
        position[i * 3 + 2] +=
            (velocityCollision[i * 3 + 2] - velocity[i * 3 + 2]) * DT;
        velocity[i * 3] = velocityCollision[i * 3];
        velocity[i * 3 + 1] = velocityCollision[i * 3 + 1];
        velocity[i * 3 + 2] = velocityCollision[i * 3 + 2];
      }
    }
  }
}

/// 非圧縮条件にもとづいて圧力を計算する
void Particle::calcPressure() {
  calcDensity();
  setBoundaryCond();
  setSourceTerm();
  setMatrix();
  solveGaussElim();
  removeNegPressure();
  setMinimumPressure();
}

void Particle::calcDensity() {}

void Particle::setBoundaryCond() {}

void Particle::setSourceTerm() {}

void Particle::setMatrix() {}

void Particle::processBoundaryCond() {}

void Particle::checkBoundaryCond() {}

void Particle::increaseDiagonalTerm() {}

void Particle::solveGaussElim() {}

void Particle::removeNegPressure() {}

void Particle::setMinimumPressure() {}

void Particle::calcPressureGradient() {}

void Particle::moveParticleByPressureGradient() {}

void Particle::writeData(string type) {}
