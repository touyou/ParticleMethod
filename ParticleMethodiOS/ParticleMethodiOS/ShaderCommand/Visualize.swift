//
//  Visualize.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/13.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct Visualize: ShaderCommand {

    static let functionName: String = "visualize"

    private let pipelineState: MTLComputePipelineState

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    func encode(in buffer: MTLCommandBuffer,
                position: MTLTexture,
                type: MTLTexture,
                inTexture: MTLTexture,
                outTexture: MTLTexture) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        let config = DispatchConfig.init(width: inTexture.width, height: inTexture.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(outTexture, index: 0)
        encoder.setTexture(inTexture, index: 1)
        encoder.setTexture(position, index: 2)
        encoder.setTexture(type, index: 3)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
