import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Switch flips made while the simulation runs are play-mode input, not
/// edits: stopping puts every switch back the way it was when the run started.
@MainActor
final class PlaySwitchResetTests: XCTestCase {
    /// Kept for the life of the test process, like `TestSupport`'s documents:
    /// freeing one inside a test method aborts the host.
    private static var keptSimulations: [SimulationController] = []

    /// The first DIP switch (by name) with at least two positions.
    private func firstDip() throws -> CatalogPart {
        let parts = TestSupport.catalog.groups(matching: DrawerFilter()).flatMap { category in
            category.functions.flatMap(\.parts) + category.subcategories.flatMap { sub in
                sub.parts + sub.functions.flatMap(\.parts)
            }
        }
        return try XCTUnwrap(parts.filter { $0.kind == PART_SWITCH && !$0.isTactileSwitch && ($0.count ?? 0) >= 2 }
            .sorted { $0.name < $1.name }.first, "no DIP switch with two positions")
    }

    func testStopRestoresSwitchesFromRunStart() throws {
        let doc = TestSupport.newDocument()
        let dip = try firstDip()
        let chip = try TestSupport.place(doc, dip.name, at: CGPoint(x: 200, y: 200))
        // Flipped while editing: kept.
        XCTAssertEqual(doc.toggleSwitch(chip.id, position: 1), true)

        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        // Flipped in play mode: undone by Stop.
        XCTAssertEqual(doc.toggleSwitch(chip.id, position: 1), false)
        XCTAssertEqual(doc.toggleSwitch(chip.id, position: 2), true)
        simulation.stop()

        let after = try XCTUnwrap(doc.chips.first { $0.id == chip.id })
        XCTAssertEqual(Array(after.switchesOn.prefix(2)), [true, false])
    }
}
