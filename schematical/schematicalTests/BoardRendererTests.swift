import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// What the board preview draws flat on the board: edge fingers, card pads
/// and holes, a Plane, traces, silkscreen outlines and texts; and the
/// picture's fitted size (plan-68-Z).
@MainActor
final class BoardRendererTests: XCTestCase {
    nonisolated deinit {}

    static func pixels(_ doc: SchematicDocument, face: BoardRenderFace = .front, angle: BoardRenderAngle = .top,
                       size: CGSize = CGSize(width: 300, height: 220)) throws -> Data {
        let view = BoardPreviewView.canvas(document: doc, face: face, angle: angle).frame(width: size.width, height: size.height)
        let image = try XCTUnwrap(ImageRenderer(content: view).cgImage)
        return try XCTUnwrap(image.dataProvider?.data as Data?)
    }

    /// How many bytes of two renders differ (antialiasing can move a few: compare amounts).
    static func differing(_ a: Data, _ b: Data) -> Int {
        zip(a, b).reduce(0) { $0 + ($1.0 == $1.1 ? 0 : 1) } + abs(a.count - b.count)
    }

    /// `change` makes the render (of `face`, `angle`) differ by more than `least` bytes.
    private func assertShows(_ doc: SchematicDocument, face: BoardRenderFace = .front, angle: BoardRenderAngle = .top,
                             least: Int = 200, _ what: String, change: () throws -> Void,
                             file: StaticString = #filePath, line: UInt = #line) throws {
        _ = try Self.pixels(doc, face: face, angle: angle)
        let before = try Self.pixels(doc, face: face, angle: angle)
        try change()
        let after = try Self.pixels(doc, face: face, angle: angle)
        XCTAssertGreaterThan(Self.differing(before, after), least, what, file: file, line: line)
    }

    func testTheOptionsTexts() {
        XCTAssertEqual(BoardRenderFace.allCases.map(\.id), ["front", "back"])
        XCTAssertEqual(BoardRenderFace.allCases.map(\.title), ["Front", "Back"])
        XCTAssertEqual(BoardRenderFace.back.side, .bottom)
        XCTAssertEqual(BoardRenderAngle.allCases.map(\.id), ["top", "angled"])
        XCTAssertEqual(BoardRenderAngle.allCases.map(\.title), ["Top", "Angled"])
        XCTAssertEqual(BoardRenderAngle.allCases.map(\.symbol), ["square", "cube"])
        XCTAssertEqual(Tint(white: 0.4), Tint(0.4, 0.4, 0.4))
        XCTAssertEqual(Tint(0.5, 0.5, 0.5).shaded(0.5), Tint(0.25, 0.25, 0.25))
        XCTAssertEqual(Tint(0.5, 0.5, 0.5).shaded(1.5), Tint(0.75, 0.75, 0.75))
    }

    /// The size `draw` fills at a scale: the board (plus the tallest part's
    /// lift, angled) and the margin on each side.
    func testFittedSize() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        let top = BoardRenderer(document: doc, face: .front, angle: .top)
        XCTAssertEqual(top.fittedSize(pointsPerMM: 10), CGSize(width: 500 + 72, height: 300 + 72))
        var tight = top
        tight.margin = 0
        XCTAssertEqual(tight.fittedSize(pointsPerMM: 2), CGSize(width: 100, height: 60))
        let bare = BoardRenderer(document: doc, face: .front, angle: .angled).fittedSize(pointsPerMM: 10)
        let chip = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        doc.moveFootprint(chip.id, to: CGPoint(x: 25, y: 15))
        let tall = BoardRenderer(document: doc, face: .front, angle: .angled).fittedSize(pointsPerMM: 10)
        XCTAssertGreaterThan(tall.height, bare.height, "the LED stands taller than the 2 mm minimum")
        // On the back the LED is not seen: the fit is the bare board's.
        XCTAssertEqual(BoardRenderer(document: doc, face: .back, angle: .angled).fittedSize(pointsPerMM: 10).height,
                       bare.height)
    }

    func testEdgeFingersShowInGold() throws {
        let doc = TestSupport.newDocument()
        try assertShows(doc, "the ISA card's fingers") { doc.setBoardType(.isa8) }
        XCTAssertTrue(doc.board.fingers.contains { !$0.isThroughHole && $0.side == .top })
    }

    func testCardPadsAndHolesShow() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardType(.arduinoShield)
        XCTAssertFalse(doc.board.holes.isEmpty)
        try assertShows(doc, "the ICSP header's plated holes") {
            var options = doc.board.cardOptions
            options["icsp"] = .bool(true)
            doc.setCardOptions(options)
        }
        XCTAssertEqual(doc.board.extraPads.count, 6)
    }

    func testATraceShowsOnItsSide() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 30, height: 20)
        try assertShows(doc, "a trace on Top") {
            _ = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 3, y: 10), CGPoint(x: 27, y: 10)], layer: .top, width: 1000))
        }
        // On the back it is not there.
        let back = try Self.pixels(doc, face: .back)
        let bare = TestSupport.newDocument()
        bare.setBoardSize(width: 30, height: 20)
        _ = try Self.pixels(bare, face: .back)
        XCTAssertLessThan(Self.differing(back, try Self.pixels(bare, face: .back)), 200)
    }

    func testASilkscreenOutlineShows() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 30, height: 20)
        let chip = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        doc.moveFootprint(chip.id, to: CGPoint(x: 15, y: 10))
        doc.setFootprintMarks(chip.id, outline: .document, pin1: .document, reference: .document)
        try assertShows(doc, least: 50, "the body outline in silkscreen") {
            doc.setFootprintMarks(chip.id, outline: .silkscreen)
        }
    }

    func testSilkscreenTextOnBothSides() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 40, height: 20)
        doc.setLayerCount(2)
        var id: UUID?
        try assertShows(doc, least: 50, "a text on the front") {
            id = doc.addText(at: CGPoint(x: 5, y: 5))
            doc.setText(try XCTUnwrap(id), "HELLO")
            doc.setTextSize(try XCTUnwrap(id), 3000)
        }
        let text = try XCTUnwrap(id)
        XCTAssertEqual(doc.texts.first { $0.id == text }?.layer, .silkscreen)
        try assertShows(doc, face: .back, least: 50, "moved to the bottom, the back shows it") {
            doc.setTextSide(text, .bottom)
        }
    }
}
