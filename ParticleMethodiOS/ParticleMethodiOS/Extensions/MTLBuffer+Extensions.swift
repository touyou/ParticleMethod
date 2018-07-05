//
//  MTLBuffer+Extensions.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/10.
//  Copyright © 2018 touyou. All rights reserved.
//

import Foundation
import Metal

extension MTLBuffer {

    func set<T>(singleValue: T) {

        let binded = contents().bindMemory(to: T.self, capacity: 1)
        binded[0] = singleValue
    }

    func set<T>(multipleValues: [T]) {
        
        let binded = contents().bindMemory(to: T.self, capacity: multipleValues.count)
        for i in 0 ..< multipleValues.count {
            binded[i] = multipleValues[i]
        }
    }
}
