import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AC: the Part Editor's Block pane draws the part: centred when it
/// fits, in a scroll view when it does not.
@MainActor
final class BlockPreviewRenderTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    /// How many pixels of the window's content differ clearly from its corner pixel.
    private func drawnPixels(_ probe: ViewProbe) throws -> Int {
        let view = try XCTUnwrap(probe.window.contentView)
        let rep = try XCTUnwrap(view.bitmapImageRepForCachingDisplay(in: view.bounds))
        view.cacheDisplay(in: view.bounds, to: rep)
        let background = try XCTUnwrap(rep.colorAt(x: 1, y: 1)?.usingColorSpace(.sRGB))
        var count = 0
        for x in stride(from: 0, to: rep.pixelsWide, by: 2) {
            for y in stride(from: 0, to: rep.pixelsHigh, by: 2) {
                guard let color = rep.colorAt(x: x, y: y)?.usingColorSpace(.sRGB) else { continue }
                let difference = abs(color.redComponent - background.redComponent)
                    + abs(color.greenComponent - background.greenComponent)
                    + abs(color.blueComponent - background.blueComponent)
                if difference > 0.3 { count += 1 }
            }
        }
        return count
    }

    private func show(_ name: String, size: NSSize) throws -> ViewProbe {
        let chip = PartPreview.chip(for: try TestSupport.part(name))
        let probe = ViewProbe(BlockPreviewView(chip: chip), size: size)
        probes.append(probe)
        return probe
    }

    func testASmallPartIsDrawnCentred() throws {
        let probe = try show("SN74HC00N", size: NSSize(width: 600, height: 500))
        XCTAssertGreaterThan(try drawnPixels(probe), 50, "the block is drawn")
        XCTAssertTrue(probe.views(NSScrollView.self).isEmpty, "it fits: no scroll view")
        XCTAssertEqual(BlockPreviewView.scale, 0.75)
    }

    func testALargePartScrolls() throws {
        let probe = try show("W65C02S6TPLG-14", size: NSSize(width: 160, height: 120))
        XCTAssertFalse(probe.views(NSScrollView.self).isEmpty, "too large to fit: a scroll view")
        XCTAssertGreaterThan(try drawnPixels(probe), 10, "the block is drawn")
    }
}
