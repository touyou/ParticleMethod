//
//  MakePressure.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void makePressure(texture2d<short, access::read> type [[ texture(0) ]],
                         texture2d<float, access::read> position [[ texture(1) ]],
                         texture2d<float, access::read> velocity [[ texture(2) ]],
                         texture2d<float, access::write> acceleration [[ texture(3) ]],
                         texture2d<float, access::write> pressure [[ texture(4) ]],
                         texture2d<int, access::read> nxt [[ texture(5) ]],
                         constant float &dBucketInv [[ buffer(0) ]],
                         constant float *dns [[ buffer(1) ]],
                         constant float &r [[ buffer(2) ]],
                         constant float &n0 [[ buffer(3) ]],
                         constant float &a2 [[ buffer(4) ]],
                         constant int *bFst [[ buffer(5) ]],
                         constant int &nBx [[ buffer(6) ]],
                         uint2 gid [[ thread_position_in_grid ]]) {

    auto ptype = short(type.read(gid).r);
    if (ptype != Type::ghost) {

        auto p_position = position.read(gid).rg;
        auto ni = 0.0;
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
                    float2 v = float2(position.read(uint2(j)).rg) - p_position;
                    if (size2(v) < r * r) {
                        if ((j.x != int(gid.x) || j.y != int(gid.y)) && short(type.read(uint2(j)).r) != -1) {
                            ni += calcWeight(size(v), r);
                        }
                    }
                    j = int2(nxt.read(uint2(j)).rg);
                }
            }
        }
        auto mi = dns[ptype];
        pressure.write((ni > n0) * (ni - n0) * a2 * mi, gid);
    }
}
