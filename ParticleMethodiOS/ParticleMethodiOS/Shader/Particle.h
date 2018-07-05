//
//  Particle.h
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//



#ifndef Particle_h
#define Particle_h

#include <metal_stdlib>
#include <metal_atomic>

using namespace metal;

constant float particleDistance = 0.02;
constant float maxX = 1.0 + particleDistance * 3;
constant float maxY = 0.2 + particleDistance * 3;
//constant float maxZ = 0.6 + particleDistance * 30;
constant float minX = -particleDistance * 3;
constant float minY = -particleDistance * 3;
//constant float minZ = -particleDistance * 3;
constant float2 gravity = { 0.0, -9.8 };

enum Type {
    ghost = -1,
    fluid = 0,
    wall = 1,
};

float calcWeight(float dist, float re);
float size2(float2 v);
float size(float2 v);
void checkParticle(texture2d<float, access::read_write> position,
                   texture2d<float, access::read_write> velocity,
                   texture2d<short, access::read_write> type,
                   texture2d<float, access::write> pressure,
                   uint2 gid);

#endif /* Particle_h */
