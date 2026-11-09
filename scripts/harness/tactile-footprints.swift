// plan-27-K: every tactile switch footprint in the library, drawn from the
// catalog's own pad data (what the PCB view draws), 12 to an image (3 rows
// of 4), for review against the datasheets: body outline, pads (copper,
// drill holes white), pin numbers, a 1 mm grid, the origin cross, the
// footprint id and a part that uses it.
//
//   rm -rf /tmp/p27-fh && scripts/harness/build.sh scripts/harness/tactile-footprints.swift /tmp/p27-fh && \
//     /tmp/p27-fh/H.app/Contents/MacOS/harness [image dir] [footprint id prefix]
//
// Images: <dir>/p27-fp-01.png, -02, …; <dir>/p27-fp-index.txt.
import AppKit
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runTactileFootprints() }

/// One footprint in its cell: `mm` points per millimetre, origin at the cell centre.
struct FootprintCell: View {
    let footprint: Footprint
    let partName: String
    let mm: CGFloat
    let cellSize: CGSize

    var body: some View {
        Canvas { context, size in
            let c = CGPoint(x: size.width / 2, y: size.height / 2 + 10)
            func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: c.x + x * mm, y: c.y + y * mm) }
            // 1 mm grid, every 5 mm darker.
            for i in -12...12 {
                var v = Path(); v.move(to: p(CGFloat(i), -12)); v.addLine(to: p(CGFloat(i), 12))
                var h = Path(); h.move(to: p(-12, CGFloat(i))); h.addLine(to: p(12, CGFloat(i)))
                let ink = Color.black.opacity(i % 5 == 0 ? 0.18 : 0.06)
                context.stroke(v, with: .color(ink), lineWidth: 0.5)
                context.stroke(h, with: .color(ink), lineWidth: 0.5)
            }
            // Body outline.
            let body = CGRect(x: c.x - footprint.body.width / 2 * mm, y: c.y - footprint.body.height / 2 * mm,
                              width: footprint.body.width * mm, height: footprint.body.height * mm)
            context.stroke(footprint.bodyRound ? Path(ellipseIn: body) : Path(body), with: .color(.blue), lineWidth: 1.2)
            // Pads.
            for pad in footprint.pads {
                let r = CGRect(x: c.x + (pad.centre.x - pad.size.width / 2) * mm,
                               y: c.y + (pad.centre.y - pad.size.height / 2) * mm,
                               width: pad.size.width * mm, height: pad.size.height * mm)
                let shape = pad.shape == .round ? Path(ellipseIn: r) : Path(r)
                context.fill(shape, with: .color(Color(red: 0.85, green: 0.55, blue: 0.1).opacity(0.85)))
                if pad.drill > 0 {
                    let d = pad.drill * mm
                    context.fill(Path(ellipseIn: CGRect(x: r.midX - d / 2, y: r.midY - d / 2, width: d, height: d)),
                                 with: .color(.white))
                }
                context.draw(Text(pad.designator ?? "·").font(.system(size: 9, weight: .bold)).foregroundColor(.black),
                             at: CGPoint(x: r.midX, y: r.midY))
            }
            // Origin.
            var o = Path(); o.move(to: p(-0.6, 0)); o.addLine(to: p(0.6, 0)); o.move(to: p(0, -0.6)); o.addLine(to: p(0, 0.6))
            context.stroke(o, with: .color(.red), lineWidth: 0.8)
            context.draw(Text(footprint.id).font(.system(size: 9, weight: .semibold)), at: CGPoint(x: size.width / 2, y: 7))
            context.draw(Text(partName).font(.system(size: 8)).foregroundColor(.secondary), at: CGPoint(x: size.width / 2, y: 18))
        }
        .frame(width: cellSize.width, height: cellSize.height)
        .border(Color.black.opacity(0.3), width: 0.5)
    }
}

@MainActor func runTactileFootprints() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp"
    let prefix = CommandLine.arguments.count > 2 ? CommandLine.arguments[2] : "SW-TACT-"
    let groups = Harness.catalog.groups(matching: DrawerFilter())
    var all: [CatalogPart] = []
    for category in groups {
        for function in category.functions { all += function.parts }
        for sub in category.subcategories {
            all += sub.parts
            for function in sub.functions { all += function.parts }
        }
    }
    let parts = all.filter { $0.isTactileSwitch }.sorted { $0.name < $1.name }
    var byFootprint: [String: CatalogPart] = [:]
    for part in parts {
        if let id = part.footprint?.id, id.hasPrefix(prefix), byFootprint[id] == nil { byFootprint[id] = part }
    }
    let ids = byFootprint.keys.sorted()
    print("\(ids.count) footprints from \(parts.count) tactile parts")
    let perPage = 12, columns = 4
    let cell = CGSize(width: 300, height: 300)
    let mm: CGFloat = 11
    var index = ""
    for page in stride(from: 0, to: ids.count, by: perPage) {
        let chunk = Array(ids[page..<min(page + perPage, ids.count)])
        let n = page / perPage + 1
        let rows = (chunk.count + columns - 1) / columns
        let view = VStack(spacing: 0) {
            ForEach(0..<rows, id: \.self) { row in
                HStack(spacing: 0) {
                    ForEach(0..<columns, id: \.self) { col in
                        let i = row * columns + col
                        if i < chunk.count, let fp = byFootprint[chunk[i]]!.footprint {
                            FootprintCell(footprint: fp, partName: byFootprint[chunk[i]]!.name, mm: mm, cellSize: cell)
                        } else {
                            Color.clear.frame(width: cell.width, height: cell.height)
                        }
                    }
                }
            }
        }
        .background(Color.white)
        for (i, id) in chunk.enumerated() {
            index += String(format: "%02d  row %d col %d  %@  (%@)\n", n, i / columns + 1, i % columns + 1, id, byFootprint[id]!.name)
        }
        let size = CGSize(width: cell.width * CGFloat(columns), height: cell.height * CGFloat(rows))
        renderSharp(view, size: size, scale: 2, to: String(format: "%@/p27-fp-%02d.png", images, n))
    }
    try? index.write(toFile: "\(images)/p27-fp-index.txt", atomically: true, encoding: .utf8)
    print("wrote \(images)/p27-fp-index.txt")
    exit(0)
}

/// Renders a view to a PNG at `scale` pixels per point, light appearance.
@MainActor func renderSharp<V: View>(_ view: V, size: CGSize, scale: CGFloat, to path: String) {
    let renderer = ImageRenderer(content: view.frame(width: size.width, height: size.height)
        .environment(\.colorScheme, .light))
    renderer.scale = scale
    guard let image = renderer.cgImage,
          let data = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:]) else {
        print("render failed: \(path)")
        return
    }
    try? data.write(to: URL(fileURLWithPath: path))
    print("wrote \(path)")
}
