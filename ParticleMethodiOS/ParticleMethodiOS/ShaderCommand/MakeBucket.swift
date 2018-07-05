//
//  MakeBucket.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

struct MakeBucket: ShaderCommand {

    static let functionName: String = "makeBucket"

    private let pipelineState: MTLComputePipelineState

    var bFst: [Int] = []
    var bLst: [Int] = []

    init(device: MTLDevice, library: MTLLibrary) throws {

        pipelineState = try type(of: self).makePipelineState(device: device, library: library)
    }

    mutating func encode(in buffer: MTLCommandBuffer,
                type: MTLTexture,
                position: MTLTexture,
                bFst: [Int],
                bLst: [Int],
                nxt: MTLTexture,
                dBucketInv: Float,
                nBx: Int,
                nBy: Int) {

        guard let encoder = buffer.makeComputeCommandEncoder() else {
            return
        }

        var _dBucketInv = dBucketInv
        var _nBx = nBx
        self.bFst = bFst
        self.bLst = bLst

        let config = DispatchConfig(width: type.width, height: type.height)
        encoder.setComputePipelineState(pipelineState)
        encoder.setTexture(type, index: 0)
        encoder.setTexture(position, index: 1)
        encoder.setTexture(nxt, index: 2)
        encoder.setBytes(&_dBucketInv, length: MemoryLayout<Float>.size, index: 3)
        encoder.setBytes(&_nBx, length: MemoryLayout<Int>.size, index: 2)
        encoder.setBytes(self.bFst, length: MemoryLayout<Int>.size * nBx * nBy, index: 0)
        encoder.setBytes(self.bLst, length: MemoryLayout<Int>.size * nBx * nBy, index: 1)
        encoder.dispatchThreadgroups(config.threadgroupCount, threadsPerThreadgroup: config.threadsPerThreadgroup)
        encoder.endEncoding()
    }
}
