//
//  Visualize.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/13.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void visualize(texture2d<half, access::write> outTexture [[ texture(0) ]],
                      texture2d<half, access::read> inTexture [[ texture(1) ]],
                      texture2d<float, access::read> position [[ texture(2) ]],
                      texture2d<short, access::read> type [[ texture(3) ]],
                      uint2 gid [[ thread_position_in_grid ]]) {

    auto newPos = float2(position.read(gid).rg);
    uint2 texturePos = uint2(uint(newPos.x / particleDistance), uint(newPos.y / particleDistance));
    half4 color = inTexture.read(texturePos).rbga;

//    outTexture.write(inTexture.read(gid), gid);
    outTexture.write(color, gid);

//    if (Type(type.read(gid).r) == Type::wall) {
//
//        outTexture.write(half4(0.0, 0.0, 1.0, 1.0), gid);
//    } else if (Type(type.read(gid).r) == Type::fluid) {
//
//        outTexture.write(half4(0.0, 1.0, 0.0, 1.0), gid);
//    } else {
//
//        outTexture.write(half4(1.0, 0.0, 0.0, 1.0), gid);
//    }
}
