import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-16-A: the board's named copper layers, their styles, the file keys
/// and the PCB page's start zoom.
@MainActor
final class BoardLayersTests: XCTestCase {
    private func doc() -> SchematicDocument { TestSupport.newDocument() }

    /// Saves `doc` and opens the bytes into a fresh document.
    static func roundTrip(_ doc: SchematicDocument) throws -> SchematicDocument {
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(data).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    /// Opens a file whose `board` object is `board` (JSON), returning the
    /// document or throwing the file's error.
    static func open(board: String, parts: String = "[]") throws -> SchematicDocument {
        let json = """
        { "format": "\(SchematicFile.formatName)", "version": 3, "nextReference": 1,
          "parts": \(parts), "junctions": [], "wires": [], "board": \(board) }
        """
        let opened = TestSupport.newDocument()
        try SchematicFile.decode(Data(json.utf8)).apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        return opened
    }

    /// The `badPCB` detail `body` throws, or a failure.
    static func pcbError(_ body: () throws -> Void, file: StaticString = #filePath,
                         line: UInt = #line) -> String? {
        do {
            try body()
            XCTFail("no error", file: file, line: line)
        } catch let SchematicFileError.badPCB(detail) {
            return detail
        } catch {
            XCTFail("unexpected \(error)", file: file, line: line)
        }
        return nil
    }

    func testCopperLayersForEachCount() {
        func layers(_ n: Int) -> [CopperLayer] {
            var board = BoardSettings()
            board.layers = n
            return board.copperLayers
        }
        XCTAssertEqual(layers(1), [.top])
        XCTAssertEqual(layers(2), [.top, .bottom])
        XCTAssertEqual(layers(4), [.top, .inner(2), .inner(3), .bottom])
        XCTAssertEqual(layers(6), [.top, .inner(2), .inner(3), .inner(4), .inner(5), .bottom])
        XCTAssertEqual(layers(6).map(\.title), ["Top", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Bottom"])
        XCTAssertEqual(layers(6).map(\.rawValue), ["top", "inner2", "inner3", "inner4", "inner5", "bottom"])
        for layer in layers(6) { XCTAssertEqual(CopperLayer(rawValue: layer.rawValue), layer) }
        XCTAssertNil(CopperLayer(rawValue: "inner1"))
        XCTAssertNil(CopperLayer(rawValue: "inner6"))
        XCTAssertNil(CopperLayer(rawValue: "inner02"))
    }

    func testDefaultStyles() {
        let board = BoardSettings()
        XCTAssertEqual(board.style(of: .top), LayerStyle(color: LayerColor(hex: "#C83737")!, type: .component))
        XCTAssertEqual(board.style(of: .bottom), LayerStyle(color: LayerColor(hex: "#3A6FD8")!, type: .component))
        XCTAssertEqual([2, 3, 4, 5].map { board.style(of: .inner($0)).color.hex },
                       ["#D8B23A", "#3AAE5C", "#B04AD0", "#3AB8C8"])
        XCTAssertEqual(board.style(of: .inner(3)).type, .plane)
    }

    func testHexRoundTrip() throws {
        let colour = try XCTUnwrap(LayerColor(hex: "#0a7FfE"))
        XCTAssertEqual(colour, LayerColor(red: 0x0A, green: 0x7F, blue: 0xFE))
        XCTAssertEqual(colour.hex, "#0A7FFE")
        XCTAssertEqual(LayerColor(hex: colour.hex), colour)
        for bad in ["red", "#12345", "#1234567", "123456", "#12345G", "#+12345"] {
            XCTAssertNil(LayerColor(hex: bad), bad)
        }
        // Through SwiftUI's Color and back.
        XCTAssertEqual(LayerColor(colour.color), colour)
    }

    func testFourLayerBoardRoundTrip() throws {
        let doc = doc()
        doc.setLayerCount(4)
        let green = LayerColor(red: 0x20, green: 0xC0, blue: 0x40)
        doc.setLayerColor(.top, green)
        doc.setLayerType(.inner(3), .wire)
        let opened = try Self.roundTrip(doc)
        XCTAssertEqual(opened.board.layers, 4)
        XCTAssertEqual(opened.board.style(of: .top).color, green)
        XCTAssertEqual(opened.board.style(of: .inner(3)).type, .wire)
        XCTAssertEqual(opened.board.style(of: .inner(2)), LayerStyle.defaultStyle(of: .inner(2)))

        // Every layer on the board is written, in stack order.
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.board?.layerStyles?.map(\.layer), ["top", "inner2", "inner3", "bottom"])
        XCTAssertEqual(file.board?.layerStyles?.first?.color, "#20C040")
    }

    func testFileWithoutLayerStylesGetsDefaults() throws {
        let opened = try Self.open(board: #"{ "layers": 6 }"#)
        for layer in opened.board.copperLayers {
            XCTAssertEqual(opened.board.style(of: layer), LayerStyle.defaultStyle(of: layer))
        }
        // A partial entry keeps the default for what it leaves out.
        let partial = try Self.open(board: #"{ "layers": 4, "layerStyles": [ { "layer": "inner2", "type": "wire" } ] }"#)
        XCTAssertEqual(partial.board.style(of: .inner(2)).type, .wire)
        XCTAssertEqual(partial.board.style(of: .inner(2)).color, LayerStyle.defaultStyle(of: .inner(2)).color)
    }

    func testEveryErrorDetail() {
        let cases: [(String, String)] = [
            (#"{ "layers": 4, "layerStyles": [ { "layer": "inner4" } ] }"#,
             #"layer "inner4" is not on a 4-layer board"#),
            (#"{ "layers": 4, "layerStyles": [ { "layer": "middle" } ] }"#,
             #"layer "middle" must be top, inner2 to inner5, or bottom"#),
            (#"{ "layers": 2, "layerStyles": [ { "layer": "top", "type": "plane" } ] }"#,
             #"layer top type "plane" must be component"#),
            (#"{ "layers": 2, "layerStyles": [ { "layer": "bottom", "type": "plane" } ] }"#,
             #"layer bottom type "plane" must be component"#),
            (#"{ "layers": 4, "layerStyles": [ { "layer": "inner2", "type": "component" } ] }"#,
             #"layer inner2 type "component" must be plane or wire"#),
            (#"{ "layers": 2, "layerStyles": [ { "layer": "top", "color": "red" } ] }"#,
             #"layer top color "red" must be #RRGGBB"#),
            (#"{ "layers": 2, "layerStyles": [ { "layer": "top" }, { "layer": "top" } ] }"#,
             "layer top is listed twice"),
        ]
        for (board, message) in cases {
            XCTAssertEqual(Self.pcbError { _ = try Self.open(board: board) }, message, board)
        }
    }

    func testFourToTwoToFourRestoresLayer2() {
        let doc = doc()
        doc.setLayerCount(4)
        let pink = LayerColor(red: 0xF0, green: 0x80, blue: 0xC0)
        doc.setLayerColor(.inner(2), pink)
        doc.setLayerType(.inner(2), .wire)
        doc.setLayerCount(2)
        XCTAssertEqual(doc.board.copperLayers, [.top, .bottom])
        doc.setLayerCount(4)
        XCTAssertEqual(doc.board.style(of: .inner(2)), LayerStyle(color: pink, type: .wire))
    }

    func testLayerTypeRefusals() {
        let doc = doc()
        doc.setLayerCount(4)
        doc.setLayerType(.top, .plane)
        XCTAssertEqual(doc.board.style(of: .top).type, .component)
        doc.setLayerType(.inner(2), .component)
        XCTAssertEqual(doc.board.style(of: .inner(2)).type, .plane)
        doc.setLayerType(.inner(5), .wire)
        XCTAssertNil(doc.board.styles[.inner(5)], "not on a 4-layer board")
        for count in BoardSettings.layerCounts { XCTAssertNil(doc.canSetLayerCount(count)) }
    }

    /// Viewports made by tests, kept alive for the same reason as
    /// `TestSupport`'s documents: freeing one aborts the host.
    private static var keptViewports: [CanvasViewport] = []

    func testViewportStartZoom() {
        let viewports = [CanvasViewport(zoom: 0.25), CanvasViewport(zoom: 9), CanvasViewport()]
        Self.keptViewports += viewports
        XCTAssertEqual(viewports.map(\.zoom), [0.25, 3, 1])
    }

    /// The schematic's 100 % draws at 0.75 points per unit; zoom steps
    /// scale from there, and screen/world mapping uses the drawn scale.
    func testViewportBaseScale() {
        let viewport = CanvasViewport(baseScale: 0.75)
        Self.keptViewports.append(viewport)
        XCTAssertEqual(viewport.scale, 0.75)
        viewport.setZoom(2)
        XCTAssertEqual(viewport.zoom, 2)
        XCTAssertEqual(viewport.scale, 1.5)
        XCTAssertEqual(viewport.world(CGPoint(x: 30, y: 15)), CGPoint(x: 20, y: 10))
    }
}
