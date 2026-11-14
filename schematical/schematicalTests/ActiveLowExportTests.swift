import Foundation
import XCTest
@testable import schematical

/// plan-39-L: four shipped parts reviewed by plan-39 (an SRAM, a UART, an
/// ADC and an MCU) placed on one schematic and exported. The overlined pieces
/// of the SVG are exactly the alternatives `scripts/pin-polarity.js pins`
/// lists as flagged for those pinouts; the files are kept for a look.
@MainActor
final class ActiveLowExportTests: XCTestCase {
    /// From `node scripts/pin-polarity.js pins <device> <pinout>` (2026-11-14):
    /// AS6C1008 p32, TL16C550D tqfp48, ADS8548 p64, at90can128 quad64.
    private let expected: [String: [String]] = [
        "AS6C1008-55PCN": ["CE", "OE", "WE"],
        "TL16C550DIPT": ["CS2", "BAUDOUT", "WR1", "RD1", "TXRDY", "ADS", "RXRDY", "OUT2", "RTS", "DTR",
                         "OUT1", "CTS", "DSR", "DCD", "RI"],
        "ADS8548SPM": ["PAR", "STBY", "WR", "RD", "CS", "FS", "REFBUFEN", "HW"],
        "AT90CAN128-16AU": ["RESET"],
    ]

    /// The text of every `<tspan … text-decoration="overline">`.
    private func overlined(_ svg: String) -> [String] {
        let regex = try! NSRegularExpression(pattern: "<tspan[^>]*text-decoration=\"overline\">([^<]*)</tspan>")
        return regex.matches(in: svg, range: NSRange(svg.startIndex..., in: svg)).map {
            String(svg[Range($0.range(at: 1), in: svg)!])
        }
    }

    func testFourReviewedPartsExportTheirOverlines() throws {
        let names = ["AS6C1008-55PCN", "TL16C550DIPT", "ADS8548SPM", "AT90CAN128-16AU"]
        var all: [String] = []
        let doc = TestSupport.newDocument()
        for (index, name) in names.enumerated() {
            // One part alone: its overlines are exactly the expected ones.
            let single = TestSupport.newDocument()
            _ = try TestSupport.place(single, name, at: CGPoint(x: 300, y: 300))
            let svg = String(decoding: try XCTUnwrap(SchematicExport.svg(SchematicScene(single))), as: UTF8.self)
            XCTAssertEqual(overlined(svg).sorted(), try XCTUnwrap(expected[name]).sorted(), name)
            XCTAssertFalse(svg.contains("CE#") || svg.contains("OE#") || svg.contains("WE#"), "\(name): a # is drawn")
            all += expected[name]!
            _ = try TestSupport.place(doc, name, at: CGPoint(x: 300 + 500 * (index % 2), y: 400 + 900 * (index / 2)))
        }
        let svgData = try XCTUnwrap(SchematicExport.svg(SchematicScene(doc)))
        XCTAssertEqual(overlined(String(decoding: svgData, as: UTF8.self)).sorted(), all.sorted())
        let pngData = try XCTUnwrap(SchematicExport.png(SchematicScene(doc), white: true))
        let dir = URL(fileURLWithPath: NSTemporaryDirectory()).appendingPathComponent("p39-landing")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        try svgData.write(to: dir.appendingPathComponent("four-parts.svg"))
        try pngData.write(to: dir.appendingPathComponent("four-parts.png"))
        print("P39-LANDING-EXPORT \(dir.path)")
    }
}
