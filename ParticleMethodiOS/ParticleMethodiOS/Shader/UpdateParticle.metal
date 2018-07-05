//
//  UpdateParticle.metal
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

#include "Particle.h"

kernel void updateParticle1(texture2d<float, access::read_write> position [[ texture(0) ]],
                            texture2d<float, access::read_write> velocity [[ texture(1) ]],
                            texture2d<float, access::read_write> acceleration [[ texture(2) ]],
                            texture2d<short, access::read_write> type [[ texture(3) ]],
                            texture2d<float, access::write> pressure [[ texture(4) ]],
                            constant float &dt [[ buffer(0) ]],
                            uint2 gid [[ thread_position_in_grid ]]) {

    if (Type(type.read(gid).r) == Type::fluid) {

        auto p_acceleration = float2(acceleration.read(gid).rg);
        auto p_velocity = float2(velocity.read(gid).rg);
        auto p_position = float2(position.read(gid).rg);
        velocity.write(float4(p_velocity + p_acceleration * dt, 0.0, 0.0), gid);
        position.write(float4(p_position + (p_velocity + p_acceleration * dt) * dt, 0.0, 0.0), gid);
        acceleration.write(float4(0.0), gid);
        checkParticle(position, velocity, type, pressure, gid);
    }
}

kernel void updateParticle2(texture2d<float, access::read_write> position [[ texture(0) ]],
                            texture2d<float, access::read_write> velocity [[ texture(1) ]],
                            texture2d<float, access::read_write> acceleration [[ texture(2) ]],
                            texture2d<short, access::read_write> type [[ texture(3) ]],
                            texture2d<float, access::write> pressure [[ texture(4) ]],
                            constant float &dt [[ buffer(0) ]],
                            uint2 gid [[ thread_position_in_grid ]]) {

    if (Type(type.read(gid).r) == Type::fluid) {

        auto p_acceleration = float2(acceleration.read(gid).rg);
        auto p_velocity = float2(velocity.read(gid).rg);
        auto p_position = float2(position.read(gid).rg);
        velocity.write(float4(p_velocity + p_acceleration * dt, 0.0, 0.0), gid);
        position.write(float4(p_position + p_acceleration * dt * dt, 0.0, 0.0), gid);
        acceleration.write(float4(0.0), gid);
        checkParticle(position, velocity, type, pressure, gid);
    }
}
