//
//  UpdateParticle2.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/14.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct UpdateParticle2: ShaderCommand {

    static var functionName: String = "updateParticle2"

    private let pipelineState: MTLComputePipelineState

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    func encode(in buffer: MTLCommandBuffer,
                position: MTLTexture,
                velocity: MTLTexture,
                acceleration: MTLTexture,
                type: MTLTexture,
                pressure: MTLTexture,
                dt: Float) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        var _dt = dt

        let config = DispatchConfig(width: type.width, height: type.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(position, index: 0)
        encoder.setTexture(velocity, index: 1)
        encoder.setTexture(acceleration, index: 2)
        encoder.setTexture(type, index: 3)
        encoder.setTexture(pressure, index: 4)
        encoder.setBytes(&_dt, length: MemoryLayout<Float>.size, index: 0)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
