//
//  ClearParam.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/13.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct ClearParam: ShaderCommand {

    static let functionName: String = "clearParam"

    private let pipelineState: MTLComputePipelineState

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    func encode(in buffer: MTLCommandBuffer,
                texture: MTLTexture,
                value: Int) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        var _value = value

        let config = DispatchConfig.init(width: texture.width, height: texture.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(texture, index: 0)
        encoder.setBytes(&_value, length: MemoryLayout<Int>.size, index: 0)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
