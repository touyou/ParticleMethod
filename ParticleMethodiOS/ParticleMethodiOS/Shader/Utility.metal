//
//  Utility.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/14.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

float calcWeight(float dist, float re) { return (re / dist) - 1.0; }
float size2(float2 v) { return v.x * v.x + v.y * v.y; }
float size(float2 v) { return rsqrt(size2(v)); }

void checkParticle(texture2d<float, access::read_write> position,
                   texture2d<float, access::read_write> velocity,
                   texture2d<short, access::read_write> type,
                   texture2d<float, access::write> pressure,
                   uint2 gid) {

    auto p_position = position.read(gid);
    if (short(type.read(gid).r) == -1) return;
    if (p_position.x > maxX || p_position.x < minX ||
        p_position.y > maxY || p_position.y < minY) {
        type.write(-1, gid);
        pressure.write(0.0, gid);
        velocity.write(float4(0.0), gid);
    }
}
