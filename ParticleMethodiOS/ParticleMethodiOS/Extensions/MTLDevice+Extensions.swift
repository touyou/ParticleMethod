//
//  MTLDevice+Extensions.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

extension MTLDevice {

    func makeSurface(width: Int, height: Int, format: SurfaceFormat, numberOfComponents: Int) -> MTLTexture? {

        let pixelFormat: MTLPixelFormat
        // 格納したいものの型とプロパティの個数でpixelFormatを分ける（そこにデータを入れていく）
        switch (format, numberOfComponents) {
        case (.float, 1):
            pixelFormat = .r32Float
        case (.float, 2):
            pixelFormat = .rg32Float
        case (.float, 3):
            pixelFormat = .rgba32Float
        case (.int, 1):
            pixelFormat = .r8Sint
        case (.int, 2):
            pixelFormat = .r32Sint
        default:
            pixelFormat = .r16Float
        }

        let desc = MTLTextureDescriptor.texture2DDescriptor(pixelFormat: pixelFormat, width: width, height: height, mipmapped: false)
        desc.usage = [.shaderRead, .shaderWrite]
        return makeTexture(descriptor: desc)
    }

    func makeSlab(width: Int, height: Int, format: SurfaceFormat, numberOfComponents: Int) -> Slab? {

        guard let source = makeSurface(width: width, height: height, format: format, numberOfComponents: numberOfComponents),
            let dest = makeSurface(width: width, height: height, format: format, numberOfComponents: numberOfComponents) else {

                return nil
        }

        return Slab(source: source, dest: dest)
    }
}
