#include "myemps.h"

namespace Emps {

/// constant

const char *fileName = "dambreak.prof";
// 粒子間距離
const double particleDistance = 0.02;
// 解析方向の制限
const double maxX = 1.0 + particleDistance * 3;
const double maxY = 0.2 + particleDistance * 3;
const double maxZ = 0.6 + particleDistance * 30;
const double minX = -particleDistance * 3;
const double minY = -particleDistance * 3;
const double minZ = -particleDistance * 3;
// 出力間隔を決める反復数
const int optFrequency = 100;
// クーラン条件数
const double courantNumber = 0.1;
// 動粘性係数
const double kinematicViscosity = 0.000001;
// 次元数
const int dimension = 3;
// 音速
const double sound = 22.0;
// 粒子の密度
const int dnsFluid = 1000;
const int dnsWall = 1000;
// 接近を許さない距離
const double distLimitRate = 0.9;
// 反発率
const double collisionRate = 0.2;
const Vector gravity = Vector(0.0, 0.0, -9.8);
const double dt = 0.0005;
const double finishTime = 2.0;

/// variable

Particle *particle;
auto numberOfParticles = 0;
auto fileNumber = 0;
auto nBx = 0;
auto nBy = 0;
auto nBz = 0;
auto nBxy = 0;
auto nBxyz = 0;
int *bFst, *bLst, *nxt;
double dns[2], invDns[2];
auto loopTime = 0;
double r, r2, dBucket, dBucketInv;
double n0, lambda, a1, a2, a3, rlim, rlim2, col;
double simTime = 0.0;

/// function

auto weight(double dist, double re) -> double { return (re / dist) - 1.0; }

void checkParticle(int i) {
  if (particle[i].position.x > maxX || particle[i].position.x < minX ||
      particle[i].position.y > maxY || particle[i].position.y < minY ||
      particle[i].position.z > maxZ || particle[i].position.z < minZ) {
    particle[i].type = ghost;
    particle[i].pressure = 0.0;
    particle[i].velocity = Vector();
  }
}

void readDat() {
  auto fp = fopen(fileName, "r");
  fscanf(fp, "%d", &numberOfParticles);
  printf("number of particle: %d\n", numberOfParticles);
  particle = new Particle[numberOfParticles];
  for (auto i = 0; i < numberOfParticles; i++) {
    int a[2];
    double b[8];
    fscanf(fp, " %d %d %lf %lf %lf %lf %lf %lf %lf %lf", &a[0], &a[1], &b[0],
           &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]);
    particle[i].type = Type(a[1]);
    particle[i].position = Vector(b[0], b[1], b[2]);
    particle[i].velocity = Vector(b[3], b[4], b[5]);
    particle[i].pressure = b[6];
    particle[i].avgPressure = b[7];
  }
  fclose(fp);
  for (auto i = 0; i < numberOfParticles; i++) {
    checkParticle(i);
  }
  for (auto i = 0; i < numberOfParticles; i++) {
    particle[i].acceleration = Vector();
  }
}

void writeDat() {
  char outputFileName[256];
  sprintf(outputFileName, "empsdata/output%05d.prof", fileNumber);
  auto fp = fopen(outputFileName, "w");
  fprintf(fp, "%d\n", numberOfParticles);
  for (auto i = 0; i < numberOfParticles; i++) {
    fprintf(fp, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf\n", i, particle[i].type,
            particle[i].position.x, particle[i].position.y,
            particle[i].position.z, particle[i].velocity.x,
            particle[i].velocity.y, particle[i].velocity.z,
            particle[i].pressure, particle[i].avgPressure / optFrequency);
    particle[i].avgPressure = 0.0;
  }
  fclose(fp);
  fileNumber++;
}

void analysisBucket() {
  r = particleDistance * 2.1;
  r2 = r * r;
  dBucket = r * (1.0 + courantNumber);
  dBucketInv = 1.0 / dBucket;
  nBx = int((maxX - minX) * dBucketInv) + 3;
  nBy = int((maxY - minY) * dBucketInv) + 3;
  nBz = int((maxZ - minZ) * dBucketInv) + 3;
  nBxy = nBx * nBy;
  nBxyz = nBx * nBy * nBz;
  printf("nBx:%d nBy:%d nBz:%d nBxy:%d nBxyz:%d\n", nBx, nBy, nBz, nBxy, nBxyz);
  bFst = new int[nBxyz];
  bLst = new int[nBxyz];
  nxt = new int[numberOfParticles];
}

void setParameter() {
  auto tn0 = 0.0;
  auto tlambda = 0.0;
  for (auto ix = -4; ix < 5; ix++) {
    for (auto iy = -4; iy < 5; iy++) {
      for (auto iz = -4; iz < 5; iz++) {
        auto x = particleDistance * (double)ix;
        auto y = particleDistance * (double)iy;
        auto z = particleDistance * (double)iz;
        auto dist2 = x * x + y * y + z * z;
        if (dist2 <= r * r) {
          if (dist2 == 0.0)
            continue;
          double dist = sqrt(dist2);
          tn0 += weight(dist, r);
          tlambda += dist2 * weight(dist, r);
        }
      }
    }
  }
  n0 = tn0;
  lambda = tlambda / tn0;
  a1 = 2.0 * kinematicViscosity * dimension / n0 / lambda;
  a2 = sound * sound / n0;
  a3 = -dimension / n0;
  dns[fluid] = dnsFluid;
  dns[wall] = dnsWall;
  invDns[fluid] = 1.0 / dnsFluid;
  invDns[wall] = 1.0 / dnsWall;
  rlim = particleDistance * distLimitRate;
  rlim2 = rlim * rlim;
  col = 1.0 + collisionRate;
  loopTime = 0;
  fileNumber = 0;
  simTime = 0.0;
  printf("n0: %lf lambda: %lf\na1: %lf a2:%lf a3:%lf\n", n0, lambda, a1, a2,
         a3);
}

void makeBucket() {
  for (auto i = 0; i < numberOfParticles; i++)
    bFst[i] = -1;
  for (auto i = 0; i < numberOfParticles; i++)
    bLst[i] = -1;
  for (auto i = 0; i < numberOfParticles; i++)
    nxt[i] = -1;
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == ghost)
      continue;
    auto ix = int((particle[i].position.x - minX) * dBucketInv) + 1;
    auto iy = int((particle[i].position.y - minY) * dBucketInv) + 1;
    auto iz = int((particle[i].position.z - minZ) * dBucketInv) + 1;
    auto ib = iz * nBxy + iy * nBx + ix;
    auto j = bLst[ib];
    bLst[ib] = i;
    if (j == -1)
      bFst[ib] = i;
    else
      nxt[j] = i;
  }
}

void viscosityTerm() {
#pragma omp parallel for schedule(dynamic, 64)
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      auto acceleration = Vector();
      auto position = particle[i].position;
      auto velocity = particle[i].velocity;

      auto ix = int((position.x - minX) * dBucketInv) + 1;
      auto iy = int((position.y - minY) * dBucketInv) + 1;
      auto iz = int((position.z - minZ) * dBucketInv) + 1;
      for (auto jz = iz - 1; jz <= iz + 1; jz++) {
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
          for (auto jx = ix - 1; jx <= ix + 1; jx++) {
            auto jb = jz * nBxy + jy * nBx + jx;
            auto j = bFst[jb];
            while (j != -1) {
              Vector v = particle[j].position - position;
              if (v.size2() < r2) {
                if (j != i && particle[j].type != ghost) {
                  double w = weight(v.size(), r);
                  acceleration += (particle[j].velocity - velocity) * w;
                }
              }
              j = nxt[j];
            }
          }
        }
      }
      particle[i].acceleration = acceleration * a1 + gravity;
    }
  }
}

void updateParticle1() {
#pragma omp parallel for
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      particle[i].velocity += particle[i].acceleration * dt;
      particle[i].position += particle[i].velocity * dt;
      particle[i].acceleration = Vector();
      checkParticle(i);
    }
  }
}

void checkCollision() {
#pragma omp parallel for schedule(dynamic, 64)
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      auto mi = dns[particle[i].type];
      auto position = particle[i].position;
      auto velocity = particle[i].velocity;
      auto velocity2 = particle[i].velocity;
      auto ix = int((position.x - minX) * dBucketInv) + 1;
      auto iy = int((position.y - minY) * dBucketInv) + 1;
      auto iz = int((position.z - minZ) * dBucketInv) + 1;
      for (auto jz = iz - 1; jz <= iz + 1; jz++) {
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
          for (auto jx = ix - 1; jx <= ix + 1; jx++) {
            auto jb = jz * nBxy + jy * nBx + jx;
            auto j = bFst[jb];
            while (j != -1) {
              Vector v = particle[j].position - position;
              if (v.size2() < rlim2) {
                if (j != i && particle[j].type != ghost) {
                  double fDt = (velocity - particle[j].velocity).dot(v);
                  if (fDt > 0.0) {
                    auto mj = dns[particle[j].type];
                    fDt *= col * mj / (mi + mj) / v.size2();
                    velocity2 -= v * fDt;
                  }
                }
              }
              j = nxt[j];
            }
          }
        }
      }
      particle[i].acceleration = velocity2;
    }
  }
  for (auto i = 0; i < numberOfParticles; i++) {
    particle[i].velocity = particle[i].acceleration;
  }
}

void makePressure() {
#pragma omp parallel for schedule(dynamic, 64)
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type != ghost) {
      auto position = particle[i].position;
      auto ni = 0.0;
      auto ix = int((position.x - minX) * dBucketInv) + 1;
      auto iy = int((position.y - minY) * dBucketInv) + 1;
      auto iz = int((position.z - minZ) * dBucketInv) + 1;
      for (auto jz = iz - 1; jz <= iz + 1; jz++) {
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
          for (auto jx = ix - 1; jx <= ix + 1; jx++) {
            auto jb = jz * nBxy + jy * nBx + jx;
            auto j = bFst[jb];
            while (j != -1) {
              Vector v = particle[j].position - position;
              if (v.size2() < r2) {
                if (j != i && particle[j].type != ghost) {
                  ni += weight(v.size(), r);
                }
              }
              j = nxt[j];
            }
          }
        }
      }
      auto mi = dns[particle[i].type];
      particle[i].pressure = (ni > n0) * (ni - n0) * a2 * mi;
    }
  }
}

void pressureGradTerm() {
#pragma omp parallel for schedule(dynamic, 64)
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      auto acceleration = Vector();
      auto position = particle[i].position;
      auto minPressure = particle[i].pressure;
      auto ix = int((position.x - minX) * dBucketInv) + 1;
      auto iy = int((position.y - minY) * dBucketInv) + 1;
      auto iz = int((position.z - minZ) * dBucketInv) + 1;
      for (auto jz = iz - 1; jz <= iz + 1; jz++) {
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
          for (auto jx = ix - 1; jx <= ix + 1; jx++) {
            auto jb = jz * nBxy + jy * nBx + jx;
            auto j = bFst[jb];
            while (j != -1) {
              Vector v = particle[j].position - position;
              if (v.size2() < r2) {
                if (j != i && particle[j].type != ghost) {
                  if (minPressure > particle[j].pressure)
                    minPressure = particle[j].pressure;
                }
              }
              j = nxt[j];
            }
          }
        }
      }
      for (auto jz = iz - 1; jz <= iz + 1; jz++) {
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
          for (auto jx = ix - 1; jx <= ix + 1; jx++) {
            auto jb = jz * nBxy + jy * nBx + jx;
            auto j = bFst[jb];
            while (j != -1) {
              Vector v = particle[j].position - position;
              if (v.size2() < r2) {
                if (j != i && particle[j].type != ghost) {
                  double w = weight(v.size(), r) *
                             (particle[j].pressure - minPressure) / v.size2();
                  acceleration += v * w;
                }
              }
              j = nxt[j];
            }
          }
        }
      }
      particle[i].acceleration = acceleration * invDns[fluid] * a3;
    }
  }
}

void updateParticle2() {
#pragma omp parallel for
  for (auto i = 0; i < numberOfParticles; i++) {
    if (particle[i].type == fluid) {
      particle[i].velocity += particle[i].acceleration * dt;
      particle[i].position += particle[i].acceleration * dt * dt;
      particle[i].acceleration = Vector();
      checkParticle(i);
    }
  }
}

void calcEmps() {
  while (1) {
    if (loopTime % 100 == 0) {
      auto pNum = 0;
      for (auto i = 0; i < numberOfParticles; i++) {
        if (particle[i].type != ghost)
          pNum++;
      }
      printf("%5d th TIME: %lf / pNum: %d\n", loopTime, simTime, pNum);
    }
    if (loopTime % optFrequency == 0) {
      writeDat();
      if (simTime >= finishTime)
        break;
    }

    makeBucket();
    viscosityTerm();
    updateParticle1();
    checkCollision();
    makePressure();
    pressureGradTerm();
    updateParticle2();
    makePressure();
    for (auto i = 0; i < numberOfParticles; i++) {
      particle[i].avgPressure += particle[i].pressure;
    }
    loopTime++;
    simTime += dt;
  }
}

}; // namespace Emps

int main(int argc, char **argv) {
  printf("*** START EMPS ***\n");
  Emps::readDat();
  Emps::analysisBucket();
  Emps::setParameter();

  auto timerStart = std::chrono::system_clock::now();
  Emps::calcEmps();
  auto timerEnd = std::chrono::system_clock::now();
  printf("Total: %13.6lf sec\n",
         std::chrono::duration<double>(timerEnd - timerStart).count());
}
