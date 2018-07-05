//
//  PressureGradTerm.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct PressureGradTerm: ShaderCommand {

    static let functionName: String = "pressureGradTerm"

    private let pipelineState: MTLComputePipelineState

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    func encode(in buffer: MTLCommandBuffer,
                type: MTLTexture,
                position: MTLTexture,
                velocity: MTLTexture,
                acceleration: MTLTexture,
                pressure: MTLTexture,
                bFst: [Int],
                nxt: MTLTexture,
                dBucketInv: Float,
                dns: [Float],
                invDns: [Float],
                r: Float,
                a3: Float,
                nBx: Int,
                nBy: Int) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        var _dBucketInv = dBucketInv
        let _dns = dns
        let _invDns = invDns
        var _r = r
        var _a3 = a3
        var _nBx = nBx

        let config = DispatchConfig.init(width: type.width, height: type.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(type, index: 0)
        encoder.setTexture(position, index: 1)
        encoder.setTexture(velocity, index: 2)
        encoder.setTexture(acceleration, index: 3)
        encoder.setTexture(pressure, index: 4)
        encoder.setTexture(nxt, index: 5)
        encoder.setBytes(&_dBucketInv, length: MemoryLayout<Float>.size, index: 0)
        encoder.setBytes(_dns, length: MemoryLayout<Float>.size * 2, index: 1)
        encoder.setBytes(_invDns, length: MemoryLayout<Float>.size, index: 2)
        encoder.setBytes(&_r, length: MemoryLayout<Float>.size, index: 3)
        encoder.setBytes(&_a3, length: MemoryLayout<Float>.size, index: 4)
        encoder.setBytes(bFst, length: MemoryLayout<Int>.size * nBx * nBy, index: 5)
        encoder.setBytes(&_nBx, length: MemoryLayout<Int>.size, index: 6)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
