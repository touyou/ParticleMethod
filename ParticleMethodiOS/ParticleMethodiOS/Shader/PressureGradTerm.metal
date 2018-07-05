//
//  PressureGradTerm.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void pressureGradTerm(texture2d<short, access::read> type [[ texture(0) ]],
                             texture2d<float, access::read> position [[ texture(1) ]],
                             texture2d<float, access::read> velocity [[ texture(2) ]],
                             texture2d<float, access::write> acceleration [[ texture(3) ]],
                             texture2d<float, access::read> pressure [[ texture(4) ]],
                             texture2d<int, access::read> nxt [[ texture(6) ]],
                             constant float &dBucketInv [[ buffer(0) ]],
                             constant float *dns [[ buffer(1) ]],
                             constant float *invDns [[ buffer(2) ]],
                             constant float &r [[ buffer(3) ]],
                             constant float &a3 [[ buffer(4) ]],
                             constant int *bFst [[ buffer(5) ]],
                             constant int &nBx [[ buffer(6) ]],
                             uint2 gid [[ thread_position_in_grid ]]) {

    if (short(type.read(gid).r) == Type::fluid) {
        auto p_acceleration = float2(0.0);
        auto p_position = float2(position.read(gid).rg);
        auto minPressure = float(pressure.read(gid).r);
        auto ix = int((p_position.x - minX) * dBucketInv) + 1;
        auto iy = int((p_position.y - minY) * dBucketInv) + 1;
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
            for (auto jx = ix - 1; jx <= ix + 1; jx++) {
                auto jb = jx + jy * nBx;
                auto jtemp = bFst[jb];
                int2 j;
                if (jtemp == -1) continue;
                j = int2(jtemp % nBx, jtemp / nBx);
                while (j.x != -1) {
                    auto v = float2(position.read(uint2(j)).rg) - p_position;
                    if (size2(v) < r * r) {
                        if ((j.x != int(gid.x) || j.y != int(gid.y)) && short(type.read(gid).r) != Type::ghost) {
                            auto jpressure = float(pressure.read(uint2(j)).r);
                            if (minPressure > jpressure) {
                                minPressure = jpressure;
                            }
                        }
                    }
                    j = int2(nxt.read(uint2(j)).rg);
                }
            }
        }
        for (auto jy = iy - 1; jy <= iy + 1; jy++) {
            for (auto jx = ix - 1; jx <= ix + 1; jx++) {
                auto jb = jx + jy * nBx;
                auto jtemp = bFst[jb];
                int2 j;
                if (jtemp == -1) continue;
                j = int2(jtemp % nBx, jtemp / nBx);
                while (j.x != -1) {
                    auto v = float2(position.read(uint2(j)).rg) - p_position;
                    if (size2(v) < r * r) {
                        if ((j.x != int(gid.x) || j.y != int(gid.y)) && short(type.read(gid).r) != Type::ghost) {
                            float w = calcWeight(size(v), r) * (float(pressure.read(uint2(j)).r) - minPressure) / size2(v);
                            p_acceleration += v * w;
                        }
                    }
                    j = int2(nxt.read(uint2(j)).rg);
                }
            }
        }
        acceleration.write(float4(p_acceleration * invDns[Type::fluid] * a3, 0.0, 0.0), gid);
    }
}

