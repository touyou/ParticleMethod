//
//  ViewController.swift
//  ParticleMethodiOS
//
//  Created by 藤井陽介 on 2018/05/09.
//  Copyright © 2018 touyou. All rights reserved.
//

import UIKit
import MetalKit

class ViewController: UIViewController {

    @IBOutlet weak var metalView: MTKView!
    @IBOutlet weak var destImageView: UIImageView!

    var render: Renderer?

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)

        render = Renderer(with: metalView, image: #imageLiteral(resourceName: "illust-1.png").cgImage!, resultView: destImageView)
    }
}
