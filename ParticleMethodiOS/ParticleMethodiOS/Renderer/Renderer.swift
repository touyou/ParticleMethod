//
//  Renderer.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/09.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import UIKit
import MetalKit

class Renderer: NSObject {

    weak var view: MTKView?
    weak var device: MTLDevice?
    weak var resultView: UIImageView?
    var library: MTLLibrary
    var commandQueue: MTLCommandQueue

    private var inflightSemaphore = DispatchSemaphore(value: 1)

    private var simulator: Simulator?

    private var initializeState: InitializeState?
    private var visualize: Visualize?
    private var originalImageTexture: MTLTexture?

    init?(with view: MTKView, image: CGImage, resultView: UIImageView) {
        self.view = view

        self.device = MTLCreateSystemDefaultDevice()
        self.view?.device = self.device

        guard let device = self.device else {
            print("Failed to create device")
            return nil
        }

        guard let library = device.makeDefaultLibrary() else {
            print("Failed to create Library")
            return nil
        }
        self.library = library

        guard let commandQueue = device.makeCommandQueue() else {
            print("Failed to create command queue")
            return nil
        }
        self.commandQueue = commandQueue
        self.resultView = resultView

        simulator = Simulator(device: device, library: library, width: image.width, height: image.height)

        do {

            initializeState = try InitializeState(device: device, library: library)
            visualize = try Visualize(device: device, library: library)
        } catch {

            print("Failed to create shader program")
        }

        super.init()

        view.delegate = self

        guard let commandBuffer = commandQueue.makeCommandBuffer() else {

            return
        }

        let textureLoader = MTKTextureLoader(device: device)
        guard let texture = try? textureLoader.newTexture(cgImage: image, options: nil) else {

            return
        }
        self.originalImageTexture = texture

        commandBuffer.addCompletedHandler { _ in
            _ = self.inflightSemaphore.signal()
        }

        simulator?.firstEncode(in: commandBuffer, texture: texture)
    }

    private func renderer(_ texture: MTLTexture) {

        let bitPerComponent = 8
        let bitsPerPixel = bitPerComponent * 4
        let imageSize = CGSize(width: texture.width, height: texture.height)
        let imageByteCount = Int(imageSize.width * imageSize.height) * 4
        let bytesPerRow = 4 * Int(imageSize.width)
        var imageBytes = [UInt8](repeating: 0, count: imageByteCount)
        let region = MTLRegionMake2D(0, 0, Int(imageSize.width), Int(imageSize.height))
        texture.getBytes(&imageBytes, bytesPerRow: Int(bytesPerRow), from: region, mipmapLevel: 0)
        let providerRef = CGDataProvider(data: Data(bytes: &imageBytes, count: imageBytes.count * MemoryLayout<UInt8>.size) as CFData)
        let colorSpaceRef = CGColorSpaceCreateDeviceRGB()
        let bitmapInfo = CGBitmapInfo(rawValue: CGBitmapInfo.byteOrder32Big.rawValue)
        let renderingInstent = CGColorRenderingIntent.perceptual
        let imageRef = CGImage(width: Int(imageSize.width), height: Int(imageSize.height), bitsPerComponent: bitPerComponent, bitsPerPixel: bitsPerPixel, bytesPerRow: bytesPerRow, space: colorSpaceRef, bitmapInfo: bitmapInfo, provider: providerRef!, decode: nil, shouldInterpolate: false, intent: renderingInstent)
        resultView?.image = UIImage(cgImage: imageRef!)
        
    }
}

extension Renderer: MTKViewDelegate {

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
    }

    func draw(in view: MTKView) {
        _ = inflightSemaphore.wait(timeout: .distantFuture)

        guard let commandBuffer = commandQueue.makeCommandBuffer() else {
            return
        }

        commandBuffer.addCompletedHandler { _ in
            _ = self.inflightSemaphore.signal()
        }

        simulator?.encode(in: commandBuffer)

        let outTextureDescriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: .rgba8Unorm,
            width: (originalImageTexture?.width)!, height: (originalImageTexture?.height)!,
            mipmapped: false)
        outTextureDescriptor.usage = [.shaderWrite]
        if let texture = simulator?.currentPosition,
            let typeTexture = simulator?.fluid.type.source,
            let originalTexture = self.originalImageTexture,
            let outTexture = device?.makeTexture(descriptor: outTextureDescriptor)
        {


            visualize?.encode(in: commandBuffer,
                              position: texture, type: typeTexture,
                              inTexture: originalTexture,
                              outTexture: outTexture)
            commandBuffer.commit()
            commandBuffer.waitUntilCompleted()

            renderer(outTexture)
        } else {

            commandBuffer.commit()
        }
    }
}
