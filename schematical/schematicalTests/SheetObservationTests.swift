import CoreGraphics
import Foundation
import Observation
import XCTest
@testable import schematical

/// The sheet keeps each array observed on its own (plan-68-D § 3.4): a view
/// reading one is not invalidated by an edit to another, as when the
/// document stored them itself.
@MainActor
final class SheetObservationTests: XCTestCase {
    /// Whether `change` invalidates a reader of `read`.
    private func invalidates(_ read: @escaping () -> Void, by change: () -> Void) -> Bool {
        var fired = false
        withObservationTracking(read) { fired = true }
        change()
        return fired
    }

    private func document() -> SchematicDocument {
        let doc = TestSupport.newDocument()
        doc.addNote(at: .zero)
        doc.addNetLabel(at: CGPoint(x: 400, y: 400))
        return doc
    }

    func testEditsInvalidateOnlyTheirOwnArrays() {
        var doc = document()
        let from = doc.netLabels[0].id, to = doc.addNetLabel(at: CGPoint(x: 400, y: 600))
        XCTAssertFalse(invalidates({ _ = doc.notes }, by: { _ = doc.connect(.label(from), .label(to)) }))
        XCTAssertEqual(doc.wires.count, 1)
        XCTAssertFalse(invalidates({ _ = doc.notes }, by: { doc.addBus(from: CGPoint(x: 0, y: 200), to: CGPoint(x: 200, y: 200)) }))
        doc = document()
        XCTAssertFalse(invalidates({ _ = doc.buses }, by: { doc.addNote(at: CGPoint(x: 100, y: 100)) }))
        XCTAssertFalse(invalidates({ _ = doc.wires }, by: { doc.addNetLabel(at: CGPoint(x: 600, y: 600)) }))
        XCTAssertTrue(invalidates({ _ = doc.notes }, by: { doc.addNote(at: CGPoint(x: 300, y: 300)) }))
        XCTAssertTrue(invalidates({ _ = doc.netLabels }, by: { doc.addNetLabel(at: CGPoint(x: 800, y: 800)) }))
    }

    /// Port edits on nothing change nothing and record no step.
    func testPortEditsOnNothing() {
        let doc = TestSupport.newDocument()
        XCTAssertNil(doc.addPort(at: CGPoint(x: 500, y: 500)), "no bus there")
        XCTAssertNil(doc.setPortWires(UUID(), []))
        XCTAssertNil(doc.setPortSide(UUID(), .top))
        XCTAssertNil(doc.undoName)
    }

    func testAnEditWritesBackOnlyWhatChanged() {
        let sheet = SchematicSheet()
        let doc = TestSupport.newDocument()
        doc.edit(.schematic, "Test") { scope in
            XCTAssertFalse(invalidates({ _ = sheet.junctions }, by: {
                sheet.updateWiring(scope) { $0.wires.append(Wire(start: .junction(UUID()), end: .junction(UUID()), interior: [])) }
            }))
            XCTAssertFalse(invalidates({ _ = sheet.wires }, by: { sheet.updateWiring(scope) { _ in } }))
            XCTAssertTrue(invalidates({ _ = sheet.junctions }, by: {
                sheet.updateWiring(scope) { $0.junctions.append(Junction(point: .zero)) }
            }))
            let before = sheet.copperRevision
            sheet.updateBuses(scope) { _ in }
            sheet.updateLabels(scope) { _ in }
            sheet.updateNotes(scope) { _ in }
            XCTAssertEqual(sheet.copperRevision, before, "nothing changed, nothing written")
            sheet.restore(wires: sheet.wires, junctions: [], buses: [], netLabels: [], notes: [], scope)
            XCTAssertTrue(sheet.junctions.isEmpty)
            sheet.restore(wires: sheet.wires, junctions: [], buses: [], netLabels: [], notes: [], scope)
        }
    }
}
