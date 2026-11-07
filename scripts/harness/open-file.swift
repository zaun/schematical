// Opens a schematic file as File > Open does and renders the canvas.
//
//   scripts/harness/build.sh scripts/harness/open-file.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness <file> <out.png> [x y w h]
//
// x y is the world point at the top-left (default: the drawing's top-left
// minus 40); w h the image size in points (default 900 × 600).
import CoreGraphics
import Foundation

MainActor.assumeIsolated { openAndRender() }

@MainActor func openAndRender() {
    let args = CommandLine.arguments
    guard args.count >= 3 else {
        print("usage: harness <file> <out.png> [x y w h]")
        exit(2)
    }
    do {
        let document = try Harness.open(args[1])
        print("opened \(args[1]): \(document.chips.count) parts, \(document.wires.count) wires, \(document.buses.count) buses")
        var origin = CGPoint.zero
        var size = CGSize(width: 900, height: 600)
        if args.count >= 7, let x = Double(args[3]), let y = Double(args[4]),
           let w = Double(args[5]), let h = Double(args[6]) {
            origin = CGPoint(x: x, y: y)
            size = CGSize(width: w, height: h)
        } else {
            let points = document.chips.map(\.origin)
                + document.buses.flatMap { $0.lines.flatMap { [$0.a, $0.b] } }
            let xs = points.map(\.x)
            let ys = points.map(\.y)
            origin = CGPoint(x: (xs.min() ?? 0) - 40, y: (ys.min() ?? 0) - 40)
        }
        Harness.renderCanvas(document, to: args[2], size: size, origin: origin)
    } catch {
        print("open failed: \(error)")
        exit(1)
    }
}
