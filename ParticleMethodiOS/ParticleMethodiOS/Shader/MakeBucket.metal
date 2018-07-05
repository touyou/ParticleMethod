//
//  Main.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/09.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void makeBucket(texture2d<short, access::read> type [[ texture(0) ]],
                       texture2d<float, access::read> position [[ texture(1) ]],
                       texture2d<int, access::write> nxt [[ texture(2) ]],
                       device int *bFst [[ buffer(0) ]],
                       device atomic_int *bLst [[ buffer(1) ]],
                       constant int &nBx [[ buffer(2) ]],
                       constant float &dBucketInv [[ buffer(3) ]],
                       uint2 gid [[ thread_position_in_grid ]]) {

    if (short(type.read(gid).r) != -1) {

        auto p_position = float2(position.read(gid).rg);
        auto ix = int((p_position.x - minX) * dBucketInv) + 1;
        auto iy = int((p_position.y - minY) * dBucketInv) + 1;

        auto ib = ix + iy * nBx;
        int j = atomic_exchange(&bLst[ib], gid.x + gid.y * nBx);
        auto jx = j % nBx;
        auto jy = j / nBx;
        if (jx == -1) {
            bFst[ib] = gid.x + gid.y * nBx;
        } else {
            nxt.write(int4(gid.x, gid.y, 0, 0), uint2(jx, jy));
        }
    }
}

