import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// The Trace tool's angles (A), its Space / `/` / Esc draft, the PCB Delete
/// tool and ⌘⌫ on both pages.
@MainActor
final class TraceAngleTests: XCTestCase {
    private let a = CGPoint(x: 10, y: 10)
    private let scale = BoardScale.pointsPerMM

    private func p(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    /// The angle in degrees (0–90) between segment u–v and the axis `horizontal` names.
    private func angle(_ u: CGPoint, _ v: CGPoint, fromHorizontal horizontal: Bool) -> Double {
        let along = horizontal ? abs(v.x - u.x) : abs(v.y - u.y)
        let across = horizontal ? abs(v.y - u.y) : abs(v.x - u.x)
        return atan2(Double(across), Double(along)) * 180 / .pi
    }

    // MARK: Angles

    func testAngleCycle() {
        XCTAssertEqual(TraceRoute.defaultAngle, 45)
        var angle = TraceRoute.defaultAngle
        var seen: [Int] = []
        for _ in 0..<6 {
            angle = TraceRoute.nextAngle(after: angle)
            seen.append(angle)
        }
        XCTAssertEqual(seen, [60, 80, 90, 20, 40, 45])
        XCTAssertEqual(TraceRoute.nextAngle(after: 33), 45)
    }

    /// Every angle, every octant, run first and leg first: the route ends
    /// at b, has one axis-aligned run along the longer axis, and its other
    /// leg is at the angle (or 90° − the angle where the angle can't reach)
    /// to that axis.
    func testEveryAngleInEveryOctant() {
        let offsets: [(CGFloat, CGFloat)] = [(8, 1), (5, 2), (5, 4), (2, 5), (-2, 5), (-5, 4), (-5, -2), (-1, -8), (2, -5)]
        for degrees in TraceRoute.angles {
            for (dx, dy) in offsets {
                let b = p(a.x + dx, a.y + dy)
                let horizontal = abs(dx) >= abs(dy)
                let slope = Double(min(abs(dx), abs(dy)) / max(abs(dx), abs(dy)))
                let wanted = tan(Double(degrees) * .pi / 180) >= slope ? Double(degrees) : 90 - Double(degrees)
                for legFirst in [false, true] {
                    let route = TraceRoute.route(from: a, to: b, angle: degrees, legFirst: legFirst)
                    let label = "\(degrees)° (\(dx), \(dy)) legFirst \(legFirst): \(route)"
                    XCTAssertEqual(route.first, a, label)
                    XCTAssertTrue(TraceRoute.same(route.last!, b), label)
                    XCTAssertTrue(TraceRoute.isLegal(route), label)
                    guard route.count == 3 else {
                        // Already on the leg's line.
                        XCTAssertEqual(angle(a, b, fromHorizontal: horizontal), wanted, accuracy: 1e-6, label)
                        continue
                    }
                    let (run, leg) = legFirst ? ((route[1], route[2]), (route[0], route[1]))
                                              : ((route[0], route[1]), (route[1], route[2]))
                    XCTAssertEqual(angle(run.0, run.1, fromHorizontal: horizontal), 0, accuracy: 1e-6, label)
                    XCTAssertEqual(angle(leg.0, leg.1, fromHorizontal: horizontal), wanted, accuracy: 1e-6, label)
                }
            }
        }
    }

    /// 20° to a target 26.6° off the axis can't reach it, so the leg is 70°.
    func testSmallAngleUsesTheComplement() {
        let route = TraceRoute.route(from: p(0, 0), to: p(10, 5), angle: 20, legFirst: false)
        XCTAssertEqual(route.count, 3)
        XCTAssertEqual(route[1].y, 0)
        XCTAssertEqual(angle(route[1], route[2], fromHorizontal: true), 70, accuracy: 1e-6)
    }

    func testNinetyIsAnL() {
        XCTAssertEqual(TraceRoute.route(from: p(0, 0), to: p(10, 4), angle: 90, legFirst: false),
                       [p(0, 0), p(10, 0), p(10, 4)])
        XCTAssertEqual(TraceRoute.route(from: p(0, 0), to: p(10, 4), angle: 90, legFirst: true),
                       [p(0, 0), p(0, 4), p(10, 4)])
    }

    /// 45° is the old routing.
    func testFortyFiveIsUnchanged() {
        XCTAssertEqual(TraceRoute.route(from: p(0, 0), to: p(10, 4), angle: 45, legFirst: false),
                       [p(0, 0), p(6, 0), p(10, 4)])
        XCTAssertEqual(TraceRoute.route(from: p(0, 0), to: p(10, 4), angle: 45, legFirst: true),
                       [p(0, 0), p(4, 4), p(10, 4)])
        XCTAssertEqual(TraceRoute.route(from: p(0, 0), to: p(4, 4), angle: 45, legFirst: false), [p(0, 0), p(4, 4)])
    }

    // MARK: The draft

    func testLockFlipUnlock() {
        var draft = TraceDraft(start: p(0, 0))
        XCTAssertEqual(draft.path(to: p(10, 4), angle: 45), [p(0, 0), p(6, 0), p(10, 4)])
        draft.flip()
        XCTAssertEqual(draft.path(to: p(10, 4), angle: 45), [p(0, 0), p(4, 4), p(10, 4)])

        // A lock keeps the leg as shown and clears the flip.
        XCTAssertTrue(draft.lock(to: p(10, 4), angle: 45))
        XCTAssertEqual(draft.legStart, p(10, 4))
        XCTAssertEqual(draft.path(to: p(10, 10), angle: 45), [p(0, 0), p(4, 4), p(10, 4), p(10, 10)])
        // A lock where the leg has no length does nothing.
        XCTAssertFalse(draft.lock(to: p(10, 4), angle: 45))
        // The angle only changes the live leg; its run carries straight on
        // from the lock, so they are one segment.
        XCTAssertEqual(draft.path(to: p(20, 8), angle: 90), [p(0, 0), p(4, 4), p(20, 4), p(20, 8)])

        XCTAssertTrue(draft.unlock())
        XCTAssertFalse(draft.hasLocks)
        XCTAssertFalse(draft.unlock())
    }

    /// A leg that carries straight on past a lock is one segment.
    func testStraightOnPastALockIsOneSegment() {
        var draft = TraceDraft(start: p(0, 0))
        draft.lock(to: p(5, 0), angle: 45)
        XCTAssertEqual(draft.path(to: p(10, 0), angle: 45), [p(0, 0), p(10, 0)])
    }

    /// A locked draft at 20° is added as drawn and joins like any trace.
    func testFinishAddsTheLockedRoute() throws {
        let doc = TestSupport.newDocument()
        var draft = TraceDraft(start: TraceDrawing.anchor(doc, at: p(0, 0), layer: .top, scale: scale))
        draft.lock(to: p(12.7, 2.54), angle: 20)
        let id = try XCTUnwrap(TraceDrawing.finish(doc, draft: draft, to: p(12.7, 12.7), layer: .top, scale: scale,
                                                   angle: 90, visible: BoardVisibility(), onShowLayer: { _ in }))
        let trace = try XCTUnwrap(doc.traces.first { $0.id == id })
        XCTAssertEqual(trace.points.first, p(0, 0))
        XCTAssertEqual(trace.points.last, p(12.7, 12.7))
        XCTAssertEqual(angle(trace.points[0], trace.points[1], fromHorizontal: true), 0, accuracy: 1e-6)
        XCTAssertEqual(angle(trace.points[1], trace.points[2], fromHorizontal: true), 20, accuracy: 1e-6)

        // It reopens from the file as saved.
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.traces.first?.points, trace.points)
    }

    // MARK: Deleting

    func testDeleteToolDeletesTextAndTraceButNotFootprints() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let centre = try XCTUnwrap(chip.board).centre
        let tolerance = TraceDrawing.tolerance / scale
        XCTAssertFalse(doc.deleteBoardItem(at: centre, tolerance: tolerance))
        XCTAssertNotNil(doc.chips.first { $0.id == chip.id }?.board)

        let trace = try XCTUnwrap(doc.addTrace(from: p(0, 50), to: p(10, 50), layer: .top, diagonalFirst: false))
        XCTAssertTrue(doc.deleteBoardItem(at: p(5, 50), tolerance: tolerance))
        XCTAssertFalse(doc.traces.contains { $0.id == trace })
        XCTAssertEqual(doc.undoName(on: .pcb), "Delete Trace")

        let text = doc.addText(at: p(0, 80))
        let rect = try XCTUnwrap(doc.texts.first { $0.id == text }).boardRect
        XCTAssertTrue(doc.deleteBoardItem(at: CGPoint(x: rect.midX, y: rect.midY), tolerance: tolerance))
        XCTAssertTrue(doc.texts.isEmpty)
        XCTAssertFalse(doc.deleteBoardItem(at: p(300, 300), tolerance: tolerance), "empty board")
    }

    func testCommandDeleteOnThePCBKeepsFootprints() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        let trace = try XCTUnwrap(doc.addTrace(from: p(0, 50), to: p(10, 50), layer: .top, diagonalFirst: false))
        let text = doc.addText(at: p(0, 80))
        doc.select(.footprint(chip.id))
        XCTAssertFalse(doc.deleteBoardSelection(), "a footprint alone: nothing to delete")
        doc.toggleSelection(.trace(trace))
        doc.toggleSelection(.text(text))

        XCTAssertTrue(doc.deleteBoardSelection())
        XCTAssertTrue(doc.traces.isEmpty)
        XCTAssertTrue(doc.texts.isEmpty)
        XCTAssertNotNil(doc.chips.first { $0.id == chip.id }?.board)
        XCTAssertEqual(doc.boardSelection.items, [.footprint(chip.id)])

        // One step brings both back.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.traces.map(\.id), [trace])
        XCTAssertEqual(doc.texts.map(\.id), [text])
    }

    func testCommandDeleteOnTheSchematic() throws {
        let doc = TestSupport.newDocument()
        let left = try TestSupport.place(doc, "SN74HC00N", at: p(200, 200))
        let right = try TestSupport.place(doc, "SN74HC00N", at: p(500, 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(left, "1Y")), .pin(try TestSupport.pin(right, "1A"))))
        let wire = try XCTUnwrap(doc.wires.last)
        let note = doc.addNote(at: p(200, 600))
        doc.clearAllSelections()
        XCTAssertFalse(doc.deleteSchematicSelection(), "nothing selected")

        // The part goes; its wire stays, with a loose end.
        doc.select(.chip(right.id))
        doc.toggleSelection(.note(note))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertFalse(doc.chips.contains { $0.id == right.id })
        XCTAssertTrue(doc.notes.isEmpty)
        let kept = try XCTUnwrap(doc.wires.first { $0.id == wire.id })
        guard case let .junction(id) = kept.end else { return XCTFail("end \(kept.end) is not loose") }
        XCTAssertEqual(doc.junctions.first { $0.id == id }?.isLoose, true)

        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(doc.chips.count, 2)
        XCTAssertEqual(doc.notes.count, 1)

        // A selected wire goes, and the junctions only it used with it.
        doc.select(.wire(wire.id))
        XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertTrue(doc.wires.isEmpty)
        XCTAssertEqual(doc.chips.count, 2)
    }

    // MARK: Copy and paste

    func testPasteCopiesSelectedTracesOffsetAndSelectsThem() throws {
        let doc = TestSupport.newDocument()
        let one = try XCTUnwrap(doc.addTrace(from: p(0, 0), to: p(10, 4), layer: .top, diagonalFirst: false))
        let two = try XCTUnwrap(doc.addTrace(from: p(10, 4), to: p(10, 10), layer: .top, diagonalFirst: false))
        doc.setTraceWidth(two, 25)
        doc.select(.trace(one))
        doc.toggleSelection(.trace(two))
        let copied = doc.selectedTraces()
        XCTAssertEqual(copied.map(\.id), [one, two])

        let ids = doc.pasteTraces(copied, offset: CGSize(width: 25.4, height: 25.4))
        XCTAssertEqual(ids.count, 2)
        XCTAssertEqual(doc.traces.count, 4)
        XCTAssertEqual(doc.boardSelection.items, Set(ids.map(BoardItem.trace)))
        XCTAssertEqual(doc.undoName(on: .pcb), "Paste")
        for (original, id) in zip(copied, ids) {
            let copy = try XCTUnwrap(doc.traces.first { $0.id == id })
            XCTAssertEqual(copy.width, original.width)
            XCTAssertEqual(copy.layer, original.layer)
            XCTAssertEqual(copy.points.first, p(original.points[0].x + 25.4, original.points[0].y + 25.4))
        }
        // The copies stay joined to each other.
        XCTAssertEqual(Set(doc.connectedRun(of: ids[0])), Set(ids))

        // One undo takes the whole paste back.
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.traces.map(\.id), [one, two])
    }

    func testPasteSkipsLayersNotOnTheBoard() throws {
        let doc = TestSupport.newDocument()
        // Copied from a 4-layer board, pasted on a 2-layer one.
        let inner = BoardTrace(id: UUID(), layer: .inner(3), width: 10, points: [p(0, 0), p(5, 0)])
        XCTAssertEqual(doc.pasteTraces([inner], offset: .zero), [])
        XCTAssertTrue(doc.traces.isEmpty)
    }

    /// A paste centres the copy's box on the pointer, in whole grid steps.
    func testPasteOffsetCentresOnThePointer() {
        let trace = BoardTrace(id: UUID(), layer: .top, width: 10, points: [p(0, 0), p(5.08, 0), p(5.08, 2.54)])
        // Box centre (2.54, 1.27); pointer near (50.8, 25.4).
        let offset = TraceClipboard.offset(centring: [trace], on: p(50.9, 25.3), grid: 0.635)
        XCTAssertEqual(offset.width, 48.26, accuracy: 1e-9)
        XCTAssertEqual(offset.height, 24.13, accuracy: 1e-9)
        for value in [offset.width, offset.height] {
            let steps = value / 0.635
            XCTAssertEqual(steps, steps.rounded(), accuracy: 1e-9, "a whole number of grid steps")
        }
    }

    func testClipboardOffsetsStepAndResetOnCopy() {
        TraceClipboard.copy([])
        XCTAssertEqual(TraceClipboard.nextOffset(), CGSize(width: 2.54, height: 2.54))
        XCTAssertEqual(TraceClipboard.nextOffset(), CGSize(width: 5.08, height: 5.08))
        TraceClipboard.copy([])
        XCTAssertEqual(TraceClipboard.nextOffset(), CGSize(width: 2.54, height: 2.54))
    }
}
