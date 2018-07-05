//
//  InitializeState.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/13.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct InitializeState: ShaderCommand {

    static let functionName: String = "initializeState"

    private let pipelineState: MTLComputePipelineState

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    func encode(in buffer: MTLCommandBuffer,
                inTexture: MTLTexture,
                type: MTLTexture,
                position: MTLTexture) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        let config = DispatchConfig(width: type.width, height: type.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(inTexture, index: 0)
        encoder.setTexture(type, index: 1)
        encoder.setTexture(position, index: 2)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
