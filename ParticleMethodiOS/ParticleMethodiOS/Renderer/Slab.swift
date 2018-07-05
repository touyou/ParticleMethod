//
//  Slab.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/11.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import MetalKit

final class Slab {

    var source: MTLTexture
    var dest: MTLTexture

    init(source: MTLTexture, dest: MTLTexture) {

        self.source = source
        self.dest = dest
    }

    func swap() {

        let temp = source
        source = dest
        dest = temp
    }
}
