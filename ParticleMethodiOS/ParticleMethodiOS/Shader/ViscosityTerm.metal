//
//  ViscosityTerm.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void viscosityTerm(texture2d<short, access::read> type [[ texture(0) ]],
                          texture2d<float, access::read> position [[ texture(1) ]],
                          texture2d<float, access::read> velocity [[ texture(2) ]],
                          texture2d<float, access::write> acceleration [[ texture(3) ]],
                          texture2d<int, access::read> nxt [[ texture(4) ]],
                          constant float &dBucketInv [[ buffer(0) ]],
                          constant float &r [[ buffer(1) ]],
                          constant float &a1 [[ buffer(2) ]],
                          constant int *bFst [[ buffer(3) ]],
                          constant int &nBx [[ buffer(4) ]],
                          uint2 gid [[ thread_position_in_grid ]]){

    if (short(type.read(gid).r) == 0) {

        auto p_acceleration = float2(0.0);
        auto p_position = float2(position.read(gid).rg);
        auto p_velocity = float2(velocity.read(gid).rg);

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
                        if ((j.x != int(gid.x) || j.y != int(gid.y)) && Type(type.read(uint2(j)).r) != Type::ghost) {
                            float w = calcWeight(size(v), r);
                            p_acceleration += (float2(velocity.read(uint2(j)).rg) - p_velocity) * w;
                        }
                    }
                    j = int2(nxt.read(uint2(j)).rg);
                }
            }
        }
        acceleration.write(float4(p_acceleration * a1 + gravity, 0.0, 0.0), gid);
    }
}
