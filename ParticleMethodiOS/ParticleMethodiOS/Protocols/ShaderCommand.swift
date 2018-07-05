//
//  ShaderCommand.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/10.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

protocol ShaderCommand {
    static var functionName: String { get }
}

enum ShaderCommandError: Error {
    case failedToCreateFunction
}

extension ShaderCommand {
    static func makePipelineState(device: MTLDevice, library: MTLLibrary) throws -> MTLComputePipelineState {
        guard let function = library.makeFunction(name: functionName) else {
            throw ShaderCommandError.failedToCreateFunction
        }
        return try device.makeComputePipelineState(function: function)
    }
}
