//
//  ClearParam.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/13.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void clearParam(texture2d<int, access::write> target [[ texture(0) ]],
                      constant int &value [[ buffer(0) ]],
                      uint2 gid [[ thread_position_in_grid ]]) {

    target.write(int4(value), gid);
}


