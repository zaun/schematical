import Foundation

/// The DXF fixtures in `Fixtures/dxf/`, written by planning/plan-65-data/make_fixtures.py (ezdxf 1.4.5; KiCad 10.0.8
/// for `kicad_edge_probe.dxf`). Each is an ASCII DXF; `expected.json` has, per file name, the unit scale to mm, the
/// area (mm^2) and bounding box [minX, minY, maxX, maxY] of each closed loop, and the entities a reader must skip or
/// report.
@MainActor
enum DXFFixtures {
    static var chainBoard: String { text("chain_board.dxf") }
    static var curves: String { text("curves.dxf") }
    static var inchBoard: String { text("inch_board.dxf") }
    static var kicadEdgeProbe: String { text("kicad_edge_probe.dxf") }
    static var openChain: String { text("open_chain.dxf") }
    static var roundedBoard: String { text("rounded_board.dxf") }
    static var expectedJSON: String { text("expected.json") }

    static let names = ["chain_board.dxf", "curves.dxf", "inch_board.dxf", "kicad_edge_probe.dxf", "open_chain.dxf",
                        "rounded_board.dxf"]

    static var all: [(name: String, text: String)] { names.map { ($0, text($0)) } }

    /// The file's text without its final newline (the generator ends every file with one).
    static func text(_ file: String) -> String {
        let url = Fixture.directory.appendingPathComponent("dxf").appendingPathComponent(file)
        let text = (try? String(contentsOf: url, encoding: .utf8)) ?? ""
        return text.hasSuffix("\n") ? String(text.dropLast()) : text
    }
}
