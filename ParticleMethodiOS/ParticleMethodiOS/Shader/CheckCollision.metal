//
//  CheckCollision.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void checkCollision(texture2d<short, access::read> type [[ texture(0) ]],
                           texture2d<float, access::read> position [[ texture(1) ]],
                           texture2d<float, access::read> velocity [[ texture(2) ]],
                           texture2d<float, access::write> acceleration [[ texture(3) ]],
                           texture2d<int, access::read> nxt [[ texture(4) ]],
                           constant float &dBucketInv [[ buffer(0) ]],
                           constant float *dns [[ buffer(1) ]],
                           constant float &rlim2 [[ buffer(2) ]],
                           constant float &col [[ buffer(3) ]],
                           constant int *bFst [[ buffer(4) ]],
                           constant int &nBx [[ buffer(5) ]],
                           uint2 gid [[ thread_position_in_grid ]]) {

    auto ptype = Type(type.read(gid).r);
    if (ptype == Type::fluid) {

        auto mi = dns[ptype];
        auto p_position = float2(position.read(gid).rg);
        auto p_velocity = float2(velocity.read(gid).rg);
        auto p_velocity2 = p_velocity;
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
                    if (size2(v) < rlim2) {
                        auto jtype = Type(type.read(uint2(j)).r);
                        if ((j.x != int(gid.x) || j.y != int(gid.y)) && jtype != Type::ghost) {
                            float fDt = dot(p_velocity - float2(velocity.read(uint2(j)).rg), v);
                            if (fDt > 0.0) {
                                auto mj = dns[jtype];
                                fDt *= col * mj / (mi + mj) / size2(v);
                                p_velocity2 -= v * fDt;
                            }
                        }
                    }
                    j = int2(nxt.read(uint2(j)).rg);
                }
            }
        }
        acceleration.write(float4(p_velocity2.x, p_velocity2.y, 0.0, 0.0), gid);
    }
}
