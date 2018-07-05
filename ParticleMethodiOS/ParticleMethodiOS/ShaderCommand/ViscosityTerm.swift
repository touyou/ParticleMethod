//
//  ViscosityTerm.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct ViscosityTerm: ShaderCommand {

    static let functionName: String = "viscosityTerm"

    private let pipelineState: MTLComputePipelineState

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    func encode(in buffer: MTLCommandBuffer,
                type: MTLTexture,
                position: MTLTexture,
                velocity: MTLTexture,
                acceleration: MTLTexture,
                bFst: [Int],
                nxt: MTLTexture,
                dBucketInv: Float,
                r: Float,
                a1: Float,
                nBx: Int,
                nBy: Int) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        var _dBucketInv = dBucketInv
        var _r = r
        var _a1 = a1
        var _nBx = nBx

        let config = DispatchConfig.init(width: type.width, height: type.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(type, index: 0)
        encoder.setTexture(position, index: 1)
        encoder.setTexture(velocity, index: 2)
        encoder.setTexture(acceleration, index: 3)
        encoder.setTexture(nxt, index: 4)
        encoder.setBytes(&_dBucketInv, length: MemoryLayout<Float>.size, index: 0)
        encoder.setBytes(&_r, length: MemoryLayout<Float>.size, index: 1)
        encoder.setBytes(&_a1, length: MemoryLayout<Float>.size, index: 2)
        encoder.setBytes(bFst, length: MemoryLayout<Int>.size * nBx * nBy, index: 2)
        encoder.setBytes(&_nBx, length: MemoryLayout<Int>.size, index: 2)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
