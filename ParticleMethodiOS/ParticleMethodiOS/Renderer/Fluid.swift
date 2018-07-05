//
//  Fluid.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

enum ParticleType: Int {
    case ghost = -1
    case fluid = 0
    case wall = 1
}

enum SurfaceFormat {
    case float
    case int
}

struct Fluid {
    static let optFrequency = 100
    static let courantNumber: Float = 0.1
    static let kinematicViscosity: Float = 0.000001
    static let dimension = 2
    static let sound: Float = 22.0
    static let dnsFluid = 1000
    static let dnsWall = 1000
    static let distLimitRate: Float = 0.9
    static let collisionRate: Float = 0.2
    static let particleDistance: Float = 0.2
    static let maxX = 1.0 + Fluid.particleDistance * 3
    static let maxY = 0.2 + Fluid.particleDistance * 3
    static let minX = -Fluid.particleDistance * 3
    static let minY = -Fluid.particleDistance * 3
    static let dt: Float = 0.0005

    let width: Int
    let height: Int
    let r: Float
    let dBucket: Float
    let dBucketInv: Float
    let nBx: Int
    let nBy: Int
    var n0: Float = 0.0
    var lambda: Float = 0.0
    var a1: Float = 0.0
    var a2: Float = 0.0
    var a3: Float = 0.0
    var dns: [Float] = []
    var invDns: [Float] = []
    var rlim: Float = 0.0
    var col: Float = 0.0
    var bFst: [Int] = []
    var bLst: [Int] = []

    let position: Slab
    let velocity: Slab
    let acceleration: Slab
    let pressure: Slab
    let type: Slab
//    let bFst: Slab
//    let bLst: Slab
    let nxt: Slab

    init?(device: MTLDevice, width: Int, height: Int) {

        self.r = Fluid.particleDistance * 2.1
        self.dBucket = r * (1.0 + Fluid.courantNumber)
        self.dBucketInv = 1.0 / dBucket
        self.nBx = Int((Fluid.maxX - Fluid.minX) * dBucketInv) + 3
        self.nBy = Int((Fluid.maxX - Fluid.minX) * dBucketInv) + 3

        guard
            let position = device.makeSlab(width: width, height: height, format: .float, numberOfComponents: 2),
            let velocity = device.makeSlab(width: width, height: height, format: .float, numberOfComponents: 2),
            let acceleration = device.makeSlab(width: width, height: height, format: .float, numberOfComponents: 2),
            let pressure = device.makeSlab(width: width, height: height, format: .float, numberOfComponents: 1),
            let type = device.makeSlab(width: width, height: height, format: .int, numberOfComponents: 1),
//            let bFst = device.makeSlab(width: nBx, height: nBy, format: .int, numberOfComponents: 2),
//            let bLst = device.makeSlab(width: nBx, height: nBy, format: .int, numberOfComponents: 2),
            let nxt = device.makeSlab(width: width, height: height, format: .int, numberOfComponents: 2)
            else {

                return nil
        }

        self.width = width
        self.height = height

        self.position = position
        self.velocity = velocity
        self.acceleration = acceleration
        self.pressure = pressure
        self.type = type
//        self.bFst = bFst
//        self.bLst = bLst
        self.nxt = nxt

        var tN0: Float = 0.0
        var tLambda: Float = 0.0
        for ix in -4 ..< 5 {
            for iy in -4 ..< 5 {
                let x = Fluid.particleDistance * Float(ix)
                let y = Fluid.particleDistance * Float(iy)
                let dist2 = x * x + y * y
                if dist2 <= r * r {
                    if dist2 == 0.0 {
                        continue
                    }
                    let dist = sqrt(dist2)
                    tN0 += weight(dist, r)
                    tLambda += dist2 * weight(dist, r)
                }
            }
        }

        self.n0 = tN0
        self.lambda = tLambda / tN0
        self.a1 = 2.0 * Fluid.kinematicViscosity * Float(Fluid.dimension) / self.n0 / self.lambda
        self.a2 = Fluid.sound * Fluid.sound / self.n0
        self.a3 = -Float(Fluid.dimension) / self.n0
        self.dns = [Float(Fluid.dnsFluid), Float(Fluid.dnsWall), 0.0, 0.0]
        self.invDns = [1.0 / Float(Fluid.dnsFluid), 1.0 / Float(Fluid.dnsWall), 0.0, 0.0]
        self.rlim = Fluid.particleDistance * Fluid.distLimitRate
        self.col = 1.0 + Fluid.collisionRate

        createInitialState()
    }

    private mutating func createInitialState() {

        let vectorRowBytes = MemoryLayout<Float>.size * width * 2
        let floatRowBytes = MemoryLayout<Float>.size * width
        let int8RowBytes = MemoryLayout<Int8>.size * width
        let intRowBytes = MemoryLayout<Int>.size * width
        let intVecRowBytes = MemoryLayout<Int>.size * width * 2
        // Position
        var initialPosition: [Float] = Array(repeating: 0.0, count: width * height * 2)
        for i in 0 ..< initialPosition.count {

            if i % 2 == 0 {
                let x = i % width
                let y = i / width
                initialPosition[i] = Float(x) * Fluid.particleDistance
                initialPosition[i+1] = Float(y) * Fluid.particleDistance
            }
        }
        let positionRegion = MTLRegionMake2D(0, 0, width, height)
        position.source.replace(region: positionRegion,
                                mipmapLevel: 0,
                                withBytes: &initialPosition,
                                bytesPerRow: vectorRowBytes)

        // Velocity
        var initialVelocity: [Float] = Array(repeating: 0.0, count: width * height * 2)
        let velocityRegion = MTLRegionMake2D(0, 0, width, height)
        velocity.source.replace(region: velocityRegion,
                                mipmapLevel: 0,
                                withBytes: &initialVelocity,
                                bytesPerRow: vectorRowBytes)
        // Acceleration
        var initialAcceleration: [Float] = Array(repeating: 0.0, count: width * height * 2)
        let accelerateRegion = MTLRegionMake2D(0, 0, width, height)
        acceleration.source.replace(region: accelerateRegion,
                                    mipmapLevel: 0,
                                    withBytes: &initialAcceleration,
                                    bytesPerRow: vectorRowBytes)

        // Pressure
        var initialPressure: [Float] = Array(repeating: 0.0, count: width * height)
        let pressureRegion = MTLRegionMake2D(0, 0, width, height)
        pressure.source.replace(region: pressureRegion,
                                mipmapLevel: 0,
                                withBytes: &initialPressure,
                                bytesPerRow: floatRowBytes)

        // Type
        var initialType: [Int8] = Array(repeating: -1, count: width * height)
        for i in 0 ..< initialType.count {

            let x = i % width
            let y = i / width
            if (x > 5 && x < 10) || (x < width - 5 && x > width - 10)  {

                initialType[i] = 1
            } else if x > 10 && x < width - 10 && y < height / 2 {

                initialType[i] = 0
            } else if y > height - 10 {

                initialType[i] = 1
            }
        }
        let typeRegion = MTLRegionMake2D(0, 0, width, height)
        type.source.replace(region: typeRegion,
                            mipmapLevel: 0,
                            withBytes: &initialType,
                            bytesPerRow: int8RowBytes)

        // bFst, bLst, nxt
        self.bFst = Array(repeating: -1, count: nBx * nBy)
        self.bLst = Array(repeating: -1, count: nBx * nBy)
//        var initialBFst: [Int] = Array(repeating: 0, count: nBx * nBy * 2)
//        var initialBLst: [Int] = Array(repeating: 0, count: nBx * nBy * 2)
        var initialNxt: [Int] = Array(repeating: 0, count: width * height * 2)
//        let bFstRegion = MTLRegionMake2D(0, 0, nBx, nBy)
//        let bLstRegion = MTLRegionMake2D(0, 0, nBx, nBy)
        let nxtRegion = MTLRegionMake2D(0, 0, width, height)
//        bFst.source.replace(region: bFstRegion,
//                            mipmapLevel: 0,
//                            withBytes: &initialBFst,
//                            bytesPerRow: intVecRowBytes)
//        bLst.source.replace(region: bLstRegion,
//                            mipmapLevel: 0,
//                            withBytes: &initialBLst,
//                            bytesPerRow: intVecRowBytes)
        nxt.source.replace(region: nxtRegion,
                            mipmapLevel: 0,
                            withBytes: &initialNxt,
                            bytesPerRow: intRowBytes)
    }

    private func weight(_ dist: Float, _ re: Float) -> Float {

        return (re / dist) - 1.0
    }
}
