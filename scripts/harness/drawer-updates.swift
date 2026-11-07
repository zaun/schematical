// Checks that the parts library still redraws on its own (plan-17): it fills
// in once the catalog loads, and it filters on search.
//
//   scripts/harness/build.sh scripts/harness/drawer-updates.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness <out dir>
//
// Hosts SchematicSidebarView (350 × 700) with a catalog that has not loaded
// yet, as at app launch, then calls catalog.load() and types "74HC00" into
// the search field. Writes 1-before-load.png, 2-after-load.png and
// 3-search.png to <out dir>. 2 should list the categories (Connector …
// Memory) with a "… parts from backend" footer; 3 only Logic › NAND.
import AppKit
import SwiftUI

MainActor.assumeIsolated { run() }

@MainActor func run() {
    let args = CommandLine.arguments
    guard args.count >= 2 else {
        print("usage: harness <out dir>")
        exit(2)
    }
    let out = URL(fileURLWithPath: args[1], isDirectory: true)
    try! FileManager.default.createDirectory(at: out, withIntermediateDirectories: true)
    _ = NSApplication.shared
    let catalog = PartCatalogStore()  // not loaded yet, as at app launch
    let document = SchematicDocument(library: nil)
    let view = SchematicSidebarView(catalog: catalog, document: document, simulation: Harness.simulation)
        .frame(width: 350, height: 700)
        .environment(\.colorScheme, .light)
    let host = NSHostingView(rootView: view)
    host.frame = CGRect(x: 0, y: 0, width: 350, height: 700)
    let window = NSWindow(contentRect: host.frame, styleMask: [.titled], backing: .buffered, defer: false)
    window.appearance = NSAppearance(named: .aqua)
    window.contentView = host
    window.makeKeyAndOrderFront(nil)
    RunLoop.main.run(until: Date().addingTimeInterval(0.3))

    func snap(_ name: String) {
        RunLoop.main.run(until: Date().addingTimeInterval(0.4))
        host.layoutSubtreeIfNeeded()
        let rep = host.bitmapImageRepForCachingDisplay(in: host.bounds)!
        host.cacheDisplay(in: host.bounds, to: rep)
        let url = out.appendingPathComponent(name)
        try! rep.representation(using: .png, properties: [:])!.write(to: url)
        print("wrote \(url.path)")
    }
    snap("1-before-load.png")
    catalog.load()
    snap("2-after-load.png")

    func textFields(_ view: NSView) -> [NSTextField] {
        ((view as? NSTextField).map { [$0] } ?? []) + view.subviews.flatMap(textFields)
    }
    guard let field = textFields(host).first(where: \.isEditable) else {
        print("no search field found")
        exit(1)
    }
    window.makeFirstResponder(field)
    (window.firstResponder as? NSTextView)?
        .insertText("74HC00", replacementRange: NSRange(location: NSNotFound, length: 0))
    snap("3-search.png")
    exit(0)
}
