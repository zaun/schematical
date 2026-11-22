import AppKit
import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// Pours and keep-outs on the PCB page (plan-52-E): hit-testing, selection,
/// drawing and the two tools.
@MainActor
final class PourCanvasTests: XCTestCase {
    private func r(_ x: CGFloat, _ y: CGFloat, _ w: CGFloat, _ h: CGFloat) -> CGRect { CGRect(x: x, y: y, width: w, height: h) }
    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// Two LEDs on a 4-layer board, cathodes on ground, anodes together; the
    /// footprints are on the board.
    private func board() throws -> (doc: SchematicDocument, one: PlacedChip) {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        var leds: [PlacedChip] = []
        for n in 0..<2 {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: p(100 + 300 * CGFloat(n), 100))
            let ground = try TestSupport.place(doc, "GND", at: p(100 + 300 * CGFloat(n), 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
            doc.moveFootprint(led.id, to: p(30 + 30 * CGFloat(n), 40))
            leds.append(led)
        }
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(leds[0], "A")), .pin(try TestSupport.pin(leds[1], "A"))))
        return (doc, leds[0])
    }

    // MARK: Phase 1 — hit-testing, selection

    func testPressOnAPoursEdgeHitsIt() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 40, 30), layer: .top))
        let edge = doc.pours[0].rect
        XCTAssertEqual(doc.pourItem(at: p(edge.minX + 0.1, 25), tolerance: 0.5), .pour(id))
        XCTAssertEqual(doc.pourItem(at: p(30, edge.maxY - 0.2), tolerance: 0.5), .pour(id))
        XCTAssertNil(doc.pourItem(at: p(edge.minX + 2, 25), tolerance: 0.5), "2 mm inside is not the edge")
        XCTAssertNil(doc.pourItem(at: p(edge.minX - 2, 25), tolerance: 0.5))
        var hidden = BoardVisibility()
        hidden.hidden = [.copper(.top)]
        XCTAssertNil(doc.pourItem(at: p(edge.minX, 25), tolerance: 0.5, visible: hidden), "a hidden layer has no hit")
        XCTAssertEqual(doc.boardItem(at: p(edge.minX, 25), tolerance: 0.5), .pour(id))
    }

    func testAFootprintIsHitBeforeAPoursEdge() throws {
        let (doc, one) = try board()
        let chip = try XCTUnwrap(doc.chips.first { $0.id == one.id })
        let bounds = try XCTUnwrap(chip.boardBounds)
        let pad = try XCTUnwrap(chip.boardPads.first)
        let at = p(pad.rect.midX, pad.rect.midY)
        _ = doc.addPour(rect: r(at.x, at.y - 10, 20, 20), layer: .top)
        XCTAssertNotNil(doc.pourItem(at: at, tolerance: 0.6))
        XCTAssertEqual(doc.boardItem(at: at, tolerance: 0.6), .footprint(one.id))
        _ = bounds
    }

    func testBoxSelectTakesAPourWhollyInsideTheBox() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 20), layer: .top))
        let keepout = doc.addKeepout(rect: r(50, 10, 5, 5))
        doc.clearBoardSelection()
        doc.boxSelectBoard(r(5, 5, 30, 30), base: [])
        XCTAssertEqual(doc.boardSelection.items, [.pour(id)])
        doc.boxSelectBoard(r(15, 5, 30, 30), base: [])
        XCTAssertTrue(doc.boardSelection.items.isEmpty, "a box that cuts the pour does not take it")
        doc.boxSelectBoard(r(0, 0, 100, 100), base: [])
        XCTAssertEqual(doc.boardSelection.items, [.pour(id), .keepout(keepout)])
    }

    func testSelectAllTakesShownPoursAndKeepouts() throws {
        let doc = TestSupport.newDocument()
        let top = try XCTUnwrap(doc.addPour(rect: r(10, 10, 20, 20), layer: .top))
        let bottom = try XCTUnwrap(doc.addPour(rect: r(40, 10, 20, 20), layer: .bottom))
        let keepout = doc.addKeepout(rect: r(70, 10, 5, 5), layers: [.bottom])
        doc.selectAllBoard()
        XCTAssertEqual(doc.boardSelection.items, [.pour(top), .pour(bottom), .keepout(keepout)])
        var visible = BoardVisibility()
        visible.hidden = [.copper(.bottom)]
        doc.selectAllBoard(visible: visible)
        XCTAssertEqual(doc.boardSelection.items, [.pour(top)])
    }

    func testDeleteToolOnAnEdgeDeletesItAsOneStep() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 40, 30), layer: .top))
        let edge = doc.pours[0].rect
        XCTAssertTrue(doc.deleteBoardItem(at: p(edge.minX, 25), tolerance: 0.5))
        XCTAssertTrue(doc.pours.isEmpty)
        XCTAssertEqual(doc.undoName(on: .pcb), "Delete")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.pours.map(\.id), [id])
        let keepout = doc.addKeepout(rect: r(60, 10, 5, 5))
        XCTAssertTrue(doc.deleteBoardItem(at: p(60, 12), tolerance: 0.5))
        XCTAssertTrue(doc.keepouts.isEmpty)
        _ = keepout
    }

    func testAKeepoutOnBottomIsNotHitWithBottomHidden() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addKeepout(rect: r(10, 10, 20, 20), layers: [.bottom])
        XCTAssertEqual(doc.pourItem(at: p(10, 20), tolerance: 0.5), .keepout(id))
        var visible = BoardVisibility()
        visible.hidden = [.copper(.bottom)]
        XCTAssertNil(doc.pourItem(at: p(10, 20), tolerance: 0.5, visible: visible))
        XCTAssertEqual(doc.pourItem(at: p(10, 20), tolerance: 0.5), .keepout(id))
    }

    // MARK: Phase 2 — drawing

    func testPourPathsCoverOtherNetsClearancesOnly() throws {
        let (doc, one) = try board()
        guard doc.chips.first?.boardPads.allSatisfy({ $0.drill > 0 }) == true else {
            throw XCTSkip("the LED footprint is not through-hole")
        }
        let pourID = try XCTUnwrap(doc.addPour(rect: r(0, 0, doc.board.width, doc.board.height), layer: .top))
        doc.setPourClearance(pourID, 1)
        let copper = doc.connectivity()
        let draws = PCBCanvasView.pourPaths(pours: doc.pours, visible: BoardVisibility(), board: doc.board, copper: copper)
        let fill = try XCTUnwrap(draws.first?.fill)
        let ground = try XCTUnwrap(copper.net(of: .ground))
        let anode = try XCTUnwrap(copper.pads.indices.first { copper.netOfPad[$0] != nil && copper.netOfPad[$0] != ground })
        let pad = copper.pads[anode].pad.rect
        XCTAssertFalse(fill.cgPath.contains(p(pad.midX, pad.midY), using: .winding), "inside an other-net pad")
        XCTAssertFalse(fill.cgPath.contains(p(pad.maxX + 0.05, pad.midY), using: .winding), "within the clearance")
        XCTAssertTrue(fill.cgPath.contains(p(5, 5), using: .winding), "open board")
        // Drawn: the pad's hole and its clearance stay empty, the open board is filled.
        let scale: CGFloat = 4
        let view = Canvas { context, _ in
            context.scaleBy(x: scale, y: scale)
            context.fill(fill, with: .color(.red), style: FillStyle(eoFill: true))
        }.frame(width: 100 * scale, height: 80 * scale)
        let renderer = ImageRenderer(content: view)
        renderer.scale = 1
        let image = try XCTUnwrap(renderer.cgImage)
        let rep = NSBitmapImageRep(cgImage: image)
        func alpha(_ point: CGPoint) -> CGFloat { rep.colorAt(x: Int(point.x * scale), y: Int(point.y * scale))?.alphaComponent ?? -1 }
        XCTAssertEqual(alpha(p(pad.midX, pad.midY)), 0, "the hole is not drawn")
        XCTAssertEqual(alpha(p(pad.maxX + 0.5, pad.midY)), 0, "nor the clearance round it")
        XCTAssertEqual(alpha(p(pad.maxX + 1.6, pad.midY)), 1, accuracy: 0.01, "beyond it the pour is back")
        XCTAssertEqual(alpha(p(5, 5)), 1, accuracy: 0.01, "the open board is")
        _ = one
    }

    func testPourPathsAreEmptyForAHiddenLayerAndForNoNet() throws {
        let (doc, _) = try board()
        let id = try XCTUnwrap(doc.addPour(rect: r(0, 0, doc.board.width, doc.board.height), layer: .top))
        var hidden = BoardVisibility()
        hidden.hidden = [.copper(.top)]
        XCTAssertTrue(PCBCanvasView.pourPaths(pours: doc.pours, visible: hidden, board: doc.board, copper: doc.connectivity()).isEmpty)
        doc.setPourNet(id, nil)
        let draws = PCBCanvasView.pourPaths(pours: doc.pours, visible: BoardVisibility(), board: doc.board, copper: doc.connectivity())
        XCTAssertEqual(draws.count, 1)
        XCTAssertTrue(try XCTUnwrap(draws.first).fill.isEmpty, "no net: only its outline is drawn")
    }

    func testKeepoutHatch() {
        let rect = r(10, 10, 8, 5)
        let hatch = PCBCanvasView.keepoutHatch(rect, spacing: 1)
        XCTAssertFalse(hatch.isEmpty)
        let box = hatch.boundingRect
        XCTAssertGreaterThanOrEqual(box.minX, rect.minX - 1e-6)
        XCTAssertGreaterThanOrEqual(box.minY, rect.minY - 1e-6)
        XCTAssertLessThanOrEqual(box.maxX, rect.maxX + 1e-6)
        XCTAssertLessThanOrEqual(box.maxY, rect.maxY + 1e-6)
    }

    func testImageExportShowsPoursButNotKeepouts() throws {
        let (doc, _) = try board()
        func image() throws -> Data {
            var options = ImageExportOptions()
            options.view = .pcb
            options.format = .png
            options.scale = 1
            return try XCTUnwrap(ImageExport.data(for: doc, options))
        }
        _ = try image()
        let bare = try image()
        XCTAssertEqual(try image(), bare, "rendering is deterministic")
        let keepout = doc.addKeepout(rect: r(5, 5, 20, 20))
        XCTAssertEqual(try image(), bare, "keep-outs are design marks, not exported")
        doc.deleteKeepout(keepout)
        let id = try XCTUnwrap(doc.addPour(rect: r(0, 0, doc.board.width, doc.board.height), layer: .top))
        XCTAssertNotEqual(try image(), bare, "a pour is copper")
        doc.deletePour(id)
        XCTAssertEqual(try image(), bare, "added then deleted: nothing left behind")
    }

    // MARK: Phase 3 — the tools

    func testToolStringsAndSymbols() {
        for tool in [PCBTool.pour, .keepout] {
            XCTAssertFalse(tool.title.isEmpty)
            XCTAssertFalse(tool.tooltip.isEmpty)
            XCTAssertNotNil(NSImage(systemSymbolName: tool.symbol, accessibilityDescription: nil), tool.symbol)
        }
        XCTAssertEqual(PCBTool.pour.title, "Pour")
        XCTAssertEqual(PCBTool.keepout.title, "Keep-out")
        XCTAssertTrue(PCBTool.allCases.contains(.pour) && PCBTool.allCases.contains(.keepout))
    }

    func testAPourDragMakesAPourOnTheTraceLayer() throws {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .pour, from: p(10, 10), to: p(50, 40), moved: 800, layer: .bottom))
        XCTAssertEqual(doc.pours.count, 1)
        XCTAssertEqual(doc.pours[0].layer, .bottom)
        XCTAssertEqual(doc.pours[0].rect.width, 40, accuracy: doc.board.traceGrid)
        XCTAssertEqual(doc.pours[0].rect.height, 30, accuracy: doc.board.traceGrid)
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Pour")
        // Dragged up and to the left: the same rectangle.
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .pour, from: p(50, 40), to: p(10, 10), moved: 800, layer: .top))
        XCTAssertEqual(doc.pours[1].rect.origin.x, doc.pours[0].rect.origin.x, accuracy: 1e-9)
    }

    func testAClickOrATinyDragMakesNothing() {
        let doc = TestSupport.newDocument()
        XCTAssertFalse(PCBCanvasView.finishRect(doc, kind: .pour, from: p(10, 10), to: p(10, 10), moved: 1, layer: .top))
        XCTAssertFalse(PCBCanvasView.finishRect(doc, kind: .pour, from: p(10, 10), to: p(10.2, 30), moved: 40, layer: .top))
        XCTAssertFalse(PCBCanvasView.finishRect(doc, kind: .keepout, from: p(10, 10), to: p(30, 10.1), moved: 40, layer: .top))
        XCTAssertTrue(doc.pours.isEmpty && doc.keepouts.isEmpty)
    }

    func testAKeepoutDragMakesAKeepoutOnEveryLayer() {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(PCBCanvasView.finishRect(doc, kind: .keepout, from: p(10, 10), to: p(30, 25), moved: 500, layer: .top))
        XCTAssertEqual(doc.keepouts.count, 1)
        XCTAssertNil(doc.keepouts[0].layers)
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Keep-out")
    }

    // MARK: plan-52-F: resize handles

    func testEachHandleMovesOnlyItsEdges() {
        let rect = r(10, 10, 40, 30)
        let cases: [(PourHandle, CGPoint, CGRect)] = [
            (.topLeft, p(5, 6), r(5, 6, 45, 34)),
            (.top, p(99, 6), r(10, 6, 40, 34)),
            (.topRight, p(55, 6), r(10, 6, 45, 34)),
            (.right, p(55, 99), r(10, 10, 45, 30)),
            (.bottomRight, p(55, 45), r(10, 10, 45, 35)),
            (.bottom, p(99, 45), r(10, 10, 40, 35)),
            (.bottomLeft, p(5, 45), r(5, 10, 45, 35)),
            (.left, p(5, 99), r(5, 10, 45, 30)),
        ]
        for (handle, point, expected) in cases {
            XCTAssertEqual(handle.resized(rect, to: point), expected, "\(handle)")
        }
        XCTAssertEqual(PourHandle.topLeft.position(in: rect), p(10, 10))
        XCTAssertEqual(PourHandle.right.position(in: rect), p(50, 25))
        XCTAssertEqual(PourHandle.bottom.position(in: rect), p(30, 40))
    }

    func testADragPastTheOppositeEdgeClampsAtTheMinimumSide() {
        let rect = r(10, 10, 40, 30)
        XCTAssertEqual(PourHandle.bottomRight.resized(rect, to: p(0, 0)), r(10, 10, 0.5, 0.5))
        let flipped = PourHandle.left.resized(rect, to: p(80, 25))
        XCTAssertEqual(flipped.width, 0.5, accuracy: 1e-9)
        XCTAssertEqual(flipped.maxX, 50, accuracy: 1e-9)
    }

    func testHandlesOnlyForOneUnlockedItem() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 40, 30), layer: .top))
        let rect = doc.pours[0].rect
        let corner = PourHandle.bottomRight.position(in: rect)
        XCTAssertEqual(doc.pourHandle(at: corner, tolerance: 0.5)?.handle, .bottomRight)
        XCTAssertNil(doc.pourHandle(at: p(corner.x + 5, corner.y), tolerance: 0.5))
        doc.setLocked(.pour(id), true)
        XCTAssertNil(doc.pourHandle(at: corner, tolerance: 0.5), "a locked item has no handles")
        doc.setLocked(.pour(id), false)
        let keepout = doc.addKeepout(rect: r(60, 10, 10, 10))
        doc.selectBoardItems([.pour(id), .keepout(keepout)])
        XCTAssertNil(doc.pourHandle(at: corner, tolerance: 0.5), "two selected: none")
    }

    func testAResizeDragIsOneUndoStep() throws {
        let doc = TestSupport.newDocument()
        let id = try XCTUnwrap(doc.addPour(rect: r(10, 10, 40, 30), layer: .top))
        let original = doc.pours[0].rect
        let item = BoardItem.pour(id)
        doc.beginGesture(.pcb)
        for x in [52, 55, 58, 60] as [CGFloat] { doc.resizePourItem(item, handle: .right, to: p(x, 25)) }
        doc.endGesture()
        XCTAssertEqual(doc.undoName(on: .pcb), "Resize")
        XCTAssertEqual(doc.pours[0].rect.maxX, BoardGrid.snap(60, spacing: doc.board.traceGrid), accuracy: 1e-9)
        XCTAssertEqual(doc.pours[0].rect.minX, original.minX)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.pours[0].rect, original)
        let keepout = doc.addKeepout(rect: r(70, 10, 10, 10))
        doc.resizePourItem(.keepout(keepout), handle: .bottom, to: p(75, 30))
        XCTAssertEqual(doc.keepouts[0].rect.maxY, BoardGrid.snap(30, spacing: doc.board.traceGrid), accuracy: 1e-9)
    }

    /// With `POUR_OUT` set, prints `POUR_IMAGE <name> <base64 PNG>` for the PCB page and the front render of
    /// `examples/slow PCB.schematical` with a ground pour on each side and a keep-out, for looking at
    /// (the test host is sandboxed, so the lines are the way out; `TEST_RUNNER_POUR_OUT=1` passes the variable).
    func testPrintImagesWhenAsked() throws {
        guard ProcessInfo.processInfo.environment["POUR_OUT"] != nil else { return }
        let url = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("examples/slow PCB.schematical")
        let doc = TestSupport.newDocument()
        try SchematicFile.decode(Data(contentsOf: url)).apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        let board = CGRect(x: 0, y: 0, width: doc.board.width, height: doc.board.height)
        for layer in [CopperLayer.top, .bottom] {
            let id = try XCTUnwrap(doc.addPour(rect: board, layer: layer))
            doc.setPourNet(id, .ground)
        }
        doc.addKeepout(rect: CGRect(x: 40, y: 30, width: 25, height: 15), layers: [.top])
        doc.clearBoardSelection()
        print("POUR_INFO pieces \(doc.connectivity().pourFills.values.map(\.pieces.count)) ratsnest \(doc.ratsnest().count)")
        for view in [ImageExportView.pcb, .front] {
            var options = ImageExportOptions()
            options.view = view
            options.format = .png
            options.scale = view == .pcb ? 1 : 2
            let data = try XCTUnwrap(ImageExport.data(for: doc, options))
            print("POUR_IMAGE \(view.rawValue) \(data.base64EncodedString())")
        }
    }
}
