// plan-28-C: the SVG export next to the app's own export render. For
// be-6502.schematical and a document with every symbol, writes <name>.svg
// (SchematicExport.svg) and <name>-app.png (the same SchematicScene through
// ScreenPainter's export palette, 4 px per point, same padded rectangle).
// Rasterize the SVG with rsvg-convert -z 4 and diff the two.
//
//   rm -rf /tmp/p28-hc && scripts/harness/build.sh scripts/harness/export-svg.swift /tmp/p28-hc && \
//     /tmp/p28-hc/H.app/Contents/MacOS/harness /tmp/p28-svg
//
// Run from the repo root (it opens be-6502.schematical).
import AppKit
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runExportSVG() }

@MainActor func runExportSVG() {
    let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p28-svg"
    try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
    let catalog = Harness.catalog

    let symbols = Harness.newDocument()
    let names = ["SN74HC00N", "ECS-100A-010", "GENERIC-LED", "TE-GDS0204", "LTC-4727JR", "NHD-0216K1Z-FL-YBW",
                 "PJ-002A", "GENERIC-HEADER-1X02", "5V", "GND", "PULLUP-5V", "PULLDOWN"]
    let spots: [CGPoint] = [
        CGPoint(x: 120, y: 120), CGPoint(x: 420, y: 100), CGPoint(x: 640, y: 80), CGPoint(x: 860, y: 110),
        CGPoint(x: 200, y: 420), CGPoint(x: 620, y: 420), CGPoint(x: 1050, y: 400), CGPoint(x: 140, y: 680),
        CGPoint(x: 300, y: 660), CGPoint(x: 400, y: 680), CGPoint(x: 500, y: 660), CGPoint(x: 600, y: 680),
    ]
    for (name, spot) in zip(names, spots) {
        guard let part = catalog.part(named: name) else {
            print("MISSING part \(name)")
            continue
        }
        symbols.placeChip(part: part, symbol: symbol(for: part), centeredAt: spot)
        if name == "TE-GDS0204", let id = symbols.chips.last?.id { symbols.toggleSwitch(id, position: 1) }
    }
    if symbols.chips.count >= 2 {
        let a = symbols.chips[0], b = symbols.chips[1]
        if let why = symbols.connect(.pin(PinRef(chipID: a.id, index: a.layout.slots[10].pin.index)),
                                     .pin(PinRef(chipID: b.id, index: b.layout.slots[0].pin.index))) {
            print("wire refused: \(why)")
        }
    }

    var documents: [(String, SchematicDocument)] = [("symbols", symbols)]
    do {
        documents.append(("be-6502", try Harness.open("be-6502.schematical")))
    } catch {
        print("open failed: \(error)")
    }

    for (name, document) in documents {
        let scene = SchematicScene(document)
        guard let data = SchematicExport.svg(scene), let rect = SchematicExport.bounds(of: scene) else {
            print("FAIL \(name): no SVG")
            continue
        }
        let svgPath = "\(out)/\(name).svg"
        do {
            try data.write(to: URL(fileURLWithPath: svgPath))
            print("wrote \(svgPath)")
        } catch {
            print("write failed: \(svgPath): \(error)")
        }
        let size = CGSize(width: ceil(rect.width), height: ceil(rect.height))
        let content = Canvas { context, _ in
            var shifted = context
            shifted.translateBy(x: -rect.minX, y: -rect.minY)
            var painter = ScreenPainter(shifted, palette: .export)
            scene.paint(into: &painter)
        }
        .frame(width: size.width, height: size.height)
        .environment(\.colorScheme, .light)
        let renderer = ImageRenderer(content: content)
        renderer.scale = 4
        let pngPath = "\(out)/\(name)-app.png"
        guard let image = renderer.cgImage,
              let png = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:]) else {
            print("FAIL \(name): render")
            continue
        }
        do {
            try png.write(to: URL(fileURLWithPath: pngPath))
            print("wrote \(pngPath) \(image.width)x\(image.height) (\(Int(size.width))x\(Int(size.height)) pt)")
        } catch {
            print("write failed: \(pngPath): \(error)")
        }
    }
}
