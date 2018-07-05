//
//  InitializeState.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/13.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void initializeState(texture2d<half, access::read> inTexture [[ texture(0) ]],
                            texture2d<short, access::write> type [[ texture(1) ]],
                            texture2d<float, access::write> position [[ texture(2) ]],
                            uint2 gid [[ thread_position_in_grid ]]) {

//    half4 color = inTexture.read(gid).rbga;
//    half value = dot(color.rgb, half3(1.0, 1.0, 1.0));
//    half value = color.x + color.y + color.z;
//
//    if (value >= 2.0) {
//
//        type.write(Type::ghost, gid);
//    } else if (value <= 1.0) {
//
//        type.write(Type::wall, gid);
//    } else {
//
//        type.write(Type::fluid, gid);
//    }
    position.write(float4(gid.x * particleDistance, gid.y * particleDistance, 0.0, 0.0), gid);
}
