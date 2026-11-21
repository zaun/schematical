// Times the edits that felt laggy on a big schematic, with the real canvas
// and sidebar hosted (like drag-timing.swift): a keystroke in a net label's
// name, a label type change, a bus wire rename, and adding a wire (then
// undoing it). Prints the median / p90 per edit for the canvas and sidebar
// together, or for each alone, with `pan` as the baseline.
//
//   scripts/harness/build.sh scripts/harness/edit-lag.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness examples/6502.schematical [both|canvas|sidebar]
// (one hosting per run; default both)
import AppKit
import QuartzCore
import SwiftUI

MainActor.assumeIsolated { run() }

@MainActor func run() {
    let args = CommandLine.arguments
    guard args.count >= 2 else { print("usage: harness <file>"); exit(2) }
    let document = try! Harness.open(args[1])
    print("parts \(document.chips.count) wires \(document.wires.count) labels \(document.netLabels.count) buses \(document.buses.count)")
    _ = NSApplication.shared
    let viewport = CanvasViewport()
    let size = CGSize(width: 1550, height: 900)
    enum Hosted: String, CaseIterable { case both, canvas, sidebar }
    let label = document.netLabels.first { $0.isNet }!
    let bus = document.buses.first { $0.card.isCustom } ?? document.buses.first
    var host: NSHostingView<AnyView>!
    var window: NSWindow!

    func show(_ hosted: Hosted) {
        let canvas = SchematicCanvasView(catalog: Harness.catalog, document: document, viewport: viewport,
                                         tool: .pointer, simulation: Harness.simulation)
        let sidebar = SchematicSidebarView(catalog: Harness.catalog, document: document, simulation: Harness.simulation)
            .frame(width: 350)
        let root: AnyView
        switch hosted {
        case .both: root = AnyView(HStack(spacing: 0) { canvas; sidebar })
        case .canvas: root = AnyView(canvas)
        case .sidebar: root = AnyView(sidebar.frame(maxWidth: .infinity))
        }
        host = NSHostingView(rootView: AnyView(root.frame(width: size.width, height: size.height)))
        host.frame = CGRect(origin: .zero, size: size)
        window = NSWindow(contentRect: host.frame, styleMask: [.titled], backing: .buffered, defer: false)
        window.contentView = host
        window.orderFront(nil)
        RunLoop.main.run(until: Date().addingTimeInterval(0.5))
    }
    func frame() {
        for _ in 0..<3 { RunLoop.main.run(mode: .default, before: Date()) }
        host.layoutSubtreeIfNeeded()
        host.displayIfNeeded()
        CATransaction.flush()
    }
    func measure(_ name: String, steps: Int = 40, _ step: (Int) -> Void) {
        for i in 0..<6 { step(i); frame() }
        var times: [Double] = []
        for i in 0..<steps {
            let start = CACurrentMediaTime()
            step(i)
            frame()
            times.append((CACurrentMediaTime() - start) * 1000)
        }
        times.sort()
        print(String(format: "  %@: median %.1f ms  p90 %.1f ms", name, times[steps / 2], times[steps * 9 / 10]))
    }

    let points = document.chips.map(\.origin)
    viewport.pan(to: CGSize(width: -(points.map(\.x).min()! - 40), height: -(points.map(\.y).min()! - 40)))
    let base = viewport.offset
    // One hosting per process: a window left behind keeps redrawing and skews the next.
    let only = args.count > 2 ? args[2] : "both"
    for hosted in Hosted.allCases where hosted.rawValue == only {
        print(hosted.rawValue)
        show(hosted)
        measure("pan") { i in viewport.pan(to: CGSize(width: base.width + CGFloat(i % 40) * 3, height: base.height)) }
        document.select(.label(label.id))
        frame()
        measure("label name") { i in
            _ = document.setNetLabelName(label.id, i % 2 == 0 ? "ZZ1" : "ZZ2")
        }
        document.endCoalescing()
        measure("label type") { i in document.setNetLabelStyle(label.id, i % 2 == 0 ? .output : .input) }
        if let bus {
            document.selectedBusID = bus.id
            frame()
            measure("bus wire name") { i in
                _ = document.renameBusWire(bus: bus.id, wire: bus.wires[0].id, to: i % 2 == 0 ? "QA" : "QB")
            }
        }
        document.clearAllSelections()
        frame()
        measure("add wire + undo") { i in
            let y = CGFloat(-300 - (i % 5) * 40)
            if i % 2 == 0 {
                _ = document.connect(.point(CGPoint(x: -400, y: y)), .point(CGPoint(x: -200, y: y)))
            } else {
                _ = document.undo(on: .schematic)
            }
        }
        // The model alone, with no views hosted work to wait for.
        var connectMs: [Double] = [], undoMs: [Double] = []
        for i in 0..<10 {
            let y = CGFloat(-300 - (i % 5) * 40)
            var t = CACurrentMediaTime()
            _ = document.connect(.point(CGPoint(x: -400, y: y)), .point(CGPoint(x: -200, y: y)))
            connectMs.append((CACurrentMediaTime() - t) * 1000)
            t = CACurrentMediaTime()
            _ = document.undo(on: .schematic)
            undoMs.append((CACurrentMediaTime() - t) * 1000)
        }
        print(String(format: "  model only: connect %.1f ms, undo %.1f ms", connectMs.sorted()[5], undoMs.sorted()[5]))
        window.orderOut(nil)
    }
    exit(0)
}
