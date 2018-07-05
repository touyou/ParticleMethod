//
//  Renderer+Simulator.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

extension Renderer {

    class Simulator {

        private weak var device: MTLDevice?

        // Resources
        private(set) var fluid: Fluid

        // Commands
        private var makeBucket: MakeBucket
        private var viscosityTerm: ViscosityTerm
        private var updateParticle: UpdateParticle
        private var updateParticle2: UpdateParticle2
        private var checkCollision: CheckCollision
        private var makePressure: MakePressure
        private var pressureGradTerm: PressureGradTerm
        private var clearParam: ClearParam
        private var initializeState: InitializeState

        init?(device: MTLDevice, library: MTLLibrary, width: Int, height: Int) {
            self.device = device

            do {

                makeBucket = try MakeBucket(device: device, library: library)
                viscosityTerm = try ViscosityTerm(device: device, library: library)
                updateParticle = try UpdateParticle(device: device, library: library)
                checkCollision = try CheckCollision(device: device, library: library)
                makePressure = try MakePressure(device: device, library: library)
                pressureGradTerm = try PressureGradTerm(device: device, library: library)
                clearParam = try ClearParam(device: device, library: library)
                updateParticle2 = try UpdateParticle2(device: device, library: library)
                initializeState = try InitializeState(device: device, library: library)
            } catch {

                print("Failed to create shader program: \(error)")
                return nil
            }

            guard let fluid = Fluid(device: device, width: width, height: height) else {

                print("Failed to create Fluid")
                return nil
            }

            self.fluid = fluid
        }

        func firstEncode(in buffer: MTLCommandBuffer, texture: MTLTexture) {

            //initializeState.encode(in: buffer, inTexture: texture, type: fluid.type.dest, position: fluid.position.dest)
            //fluid.type.swap()
            //fluid.position.swap()
        }

        func encode(in buffer: MTLCommandBuffer) {

//            clearParam.encode(in: buffer, texture: fluid.bFst.source, value: -1)
//            clearParam.encode(in: buffer, texture: fluid.bLst.source, value: -1)
            fluid.bFst = fluid.bFst.map { _ in -1 }
            fluid.bLst = fluid.bLst.map { _ in -1 }
            clearParam.encode(in: buffer, texture: fluid.nxt.source, value: -1)
            makeBucket.encode(in: buffer,
                              type: fluid.type.source, position: fluid.position.source,
                              bFst: fluid.bFst, bLst: fluid.bLst, nxt: fluid.nxt.source,
                              dBucketInv: fluid.dBucketInv, nBx: fluid.nBx, nBy: fluid.nBy)
            fluid.bFst = makeBucket.bFst
            fluid.bLst = makeBucket.bLst

            viscosityTerm.encode(in: buffer,
                                 type: fluid.type.source, position: fluid.position.source,
                                 velocity: fluid.velocity.source, acceleration: fluid.acceleration.source,
                                 bFst: fluid.bFst, nxt: fluid.nxt.source,
                                 dBucketInv: fluid.dBucketInv, r: fluid.r, a1: fluid.a1, nBx: fluid.nBx, nBy: fluid.nBy)
            updateParticle.encode(in: buffer,
                                  position: fluid.position.source, velocity: fluid.velocity.source,
                                  acceleration: fluid.acceleration.source, type: fluid.type.source,
                                  pressure: fluid.pressure.source, dt: Fluid.dt)
            checkCollision.encode(in: buffer,
                                  type: fluid.type.source, position: fluid.position.source,
                                  velocity: fluid.velocity.source, acceleration: fluid.acceleration.source,
                                  bFst: fluid.bFst, nxt: fluid.nxt.source,
                                  dBucketInv: fluid.dBucketInv, dns: fluid.dns,
                                  rlim2: fluid.rlim * fluid.rlim, col: fluid.col, nBx: fluid.nBx, nBy: fluid.nBy)
            fluid.velocity.source = fluid.acceleration.source
            makePressure.encode(in: buffer,
                                type: fluid.type.source, position: fluid.position.source,
                                velocity: fluid.velocity.source, acceleration: fluid.acceleration.source,
                                pressure: fluid.pressure.source,
                                bFst: fluid.bFst, nxt: fluid.nxt.source,
                                dBucketInv: fluid.dBucketInv,
                                dns: fluid.dns, r: fluid.r, n0: fluid.n0, a2: fluid.a2, nBx: fluid.nBx, nBy: fluid.nBy)
            pressureGradTerm.encode(in: buffer,
                                    type: fluid.type.source, position: fluid.position.source,
                                    velocity: fluid.velocity.source, acceleration: fluid.acceleration.source,
                                    pressure: fluid.pressure.source,
                                    bFst: fluid.bFst, nxt: fluid.nxt.source,
                                    dBucketInv: fluid.dBucketInv,
                                    dns: fluid.dns, invDns: fluid.invDns, r: fluid.r, a3: fluid.a3, nBx: fluid.nBx, nBy: fluid.nBy)
            updateParticle2.encode(in: buffer,
                                  position: fluid.position.source, velocity: fluid.velocity.source,
                                  acceleration: fluid.acceleration.source, type: fluid.type.source,
                                  pressure: fluid.pressure.source, dt: Fluid.dt)
            makePressure.encode(in: buffer,
                                type: fluid.type.source, position: fluid.position.source,
                                velocity: fluid.velocity.source, acceleration: fluid.acceleration.source,
                                pressure: fluid.pressure.source,
                                bFst: fluid.bFst, nxt: fluid.nxt.source,
                                dBucketInv: fluid.dBucketInv,
                                dns: fluid.dns, r: fluid.r, n0: fluid.n0, a2: fluid.a2, nBx: fluid.nBx, nBy: fluid.nBy)
        }

        var currentPosition: MTLTexture {

            return fluid.position.source
        }
    }
}
