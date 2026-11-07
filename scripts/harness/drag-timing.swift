// Times drag steps on the canvas with the sidebar shown (plan-17).
//
//   scripts/harness/build.sh scripts/harness/drag-timing.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness <file>
//
// Opens <file>, selects the part with the most pins, and hosts
// SchematicCanvasView (1200 × 900) plus SchematicSidebarView (350 pt) in an
// NSWindow. Prints the median and p90 of 120 steps each for `pan`,
// `moveChip` (±10-pt group-drag steps of the selected part) and `dragBusLine` (first
// bus, first line; only when the file has a bus). Each step pumps the run
// loop, lays out, displays and flushes. A drag step should cost about as
// much as a pan step; plan-17 measured ~9 ms for a drag in the debug build.
import AppKit
import QuartzCore
import SwiftUI

MainActor.assumeIsolated { run() }

@MainActor func run() {
    let args = CommandLine.arguments
    guard args.count >= 2 else {
        print("usage: harness <file>")
        exit(2)
    }
    let document = try! Harness.open(args[1])
    guard let target = document.chips.max(by: { $0.part.pins.count < $1.part.pins.count }) else {
        print("no parts in \(args[1])")
        exit(1)
    }
    print("parts \(document.chips.count) wires \(document.wires.count); dragging \(target.reference) \(target.part.name) (\(target.part.pins.count) pins)")
    document.select(.chip(target.id))
    _ = NSApplication.shared
    let viewport = CanvasViewport()
    let size = CGSize(width: 1550, height: 900)
    let root = HStack(spacing: 0) {
        SchematicCanvasView(catalog: Harness.catalog, document: document, viewport: viewport,
                            tool: .pointer, simulation: Harness.simulation)
        SchematicSidebarView(catalog: Harness.catalog, document: document, simulation: Harness.simulation)
            .frame(width: 350)
    }.frame(width: size.width, height: size.height)
    let host = NSHostingView(rootView: root)
    host.frame = CGRect(origin: .zero, size: size)
    let window = NSWindow(contentRect: host.frame, styleMask: [.titled], backing: .buffered, defer: false)
    window.contentView = host
    window.orderFront(nil)
    RunLoop.main.run(until: Date().addingTimeInterval(0.5))
    let points = document.chips.map(\.origin)
    viewport.pan(to: CGSize(width: -(points.map(\.x).min()! - 40), height: -(points.map(\.y).min()! - 40)))
    RunLoop.main.run(until: Date().addingTimeInterval(0.3))

    func frame() {
        for _ in 0..<3 { RunLoop.main.run(mode: .default, before: Date()) }
        host.layoutSubtreeIfNeeded()
        host.displayIfNeeded()
        CATransaction.flush()
    }
    func measure(_ name: String, steps: Int = 120, _ step: (Int) -> Void) {
        for i in 0..<10 { step(i); frame() }  // warm up
        var times: [Double] = []
        for i in 0..<steps {
            let start = CACurrentMediaTime()
            step(i)
            frame()
            times.append((CACurrentMediaTime() - start) * 1000)
        }
        times.sort()
        print(String(format: "%@: median %.2f ms  p90 %.2f ms  max %.2f ms",
                     name, times[steps / 2], times[steps * 9 / 10], times.last!))
    }

    let base = viewport.offset
    measure("pan") { i in
        viewport.pan(to: CGSize(width: base.width + CGFloat(i % 40) * 3, height: base.height))
    }
    // A part drag is a group drag of the one selected part, as the canvas
    // does it (moveSchematicGroup, since plan-15).
    let group = document.schematicGroupBase()
    measure("moveChip") { i in
        let k = CGFloat(i % 20) - 10
        document.moveSchematicGroup(group, by: CGSize(width: k * 10, height: 0))
    }
    if let bus = document.buses.first, let line = bus.lines.first {
        document.selectedBusID = bus.id
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        // Even steps move the line 10 pt, odd steps put it back to the start;
        // so read the p90, as half the steps may be no-ops.
        measure("dragBusLine") { i in
            let k = CGFloat(i % 2 == 0 ? 1 : 0)
            let original = document.bus(bus.id)!
            _ = document.dragBus(bus.id, from: .zero, to: CGSize(width: 0, height: k * 10)) { offset in
                BusEdit.moveLine(original, line: line.id, by: offset)
            }
        }
    }
    exit(0)
}
