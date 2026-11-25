import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-36-H: Simulation › Run is off while a part that isn't simulated (a
/// microcontroller) is placed, with the reason under it; ⌘P's path does
/// not start the simulation either.
@MainActor
final class SimulationAvailabilityTests: XCTestCase {
    /// Kept for the life of the test process (see PlaySwitchResetTests).
    private static var keptSimulations: [SimulationController] = []

    override func setUp() {
        super.setUp()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    private func target(_ doc: SchematicDocument, page: EditorPage = .schematic) -> SimulationTarget {
        SimulationAvailability.target(page: page, chips: doc.chips, isRunning: false, toggle: {})
    }

    func testNoUnsimulatedPartIsAvailable() throws {
        let doc = TestSupport.newDocument()
        XCTAssertTrue(target(doc).isAvailable)
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let t = target(doc)
        XCTAssertTrue(t.isAvailable)
        XCTAssertNil(t.blockedReason)
        XCTAssertTrue(SimulationAvailability.canStart(doc))
    }

    func testOneMicrocontrollerBlocksRun() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let mcu = try TestSupport.place(doc, "ATTINY13A-PU", at: CGPoint(x: 400, y: 100))
        XCTAssertEqual(mcu.reference, "U2")
        let t = target(doc)
        XCTAssertFalse(t.isAvailable)
        XCTAssertEqual(t.blockedReason, "U2 (ATTINY13A-PU) is not simulated")
        XCTAssertFalse(SimulationAvailability.canStart(doc))
    }

    func testThreeMicrocontrollersNameTheFirst() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "ATTINY13A-PU", at: CGPoint(x: 100, y: 100))
        try TestSupport.place(doc, "ESP32-WROOM-32E-N4", at: CGPoint(x: 400, y: 100))
        try TestSupport.place(doc, "SC0914(13)", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(target(doc).blockedReason, "U1 (ATTINY13A-PU) and 2 more are not simulated")
    }

    /// The first in reference order, not placement order: U2 before U10.
    func testReasonFollowsReferenceOrder() throws {
        let doc = TestSupport.newDocument()
        let late = try TestSupport.place(doc, "ATTINY13A-PU", at: CGPoint(x: 100, y: 100))
        let early = try TestSupport.place(doc, "SC0914(13)", at: CGPoint(x: 400, y: 100))
        var chips = doc.chips
        chips[chips.firstIndex { $0.id == late.id }!].reference = "U10"
        chips[chips.firstIndex { $0.id == early.id }!].reference = "U2"
        XCTAssertEqual(SimulationAvailability.reason(chips), "U2 (SC0914(13)) and 1 more are not simulated")
    }

    /// ⇧⌘R's path: toggling with a microcontroller placed does not start.
    func testToggleDoesNotStart() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "ATTINY13A-PU", at: CGPoint(x: 100, y: 100))
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        SimulationAvailability.toggle(simulation, document: doc)
        XCTAssertFalse(simulation.isRunning)
        XCTAssertNil(simulation.errorMessage)
    }

    /// Without one, the same path starts and stops.
    func testToggleStartsWithoutOne() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        SimulationAvailability.toggle(simulation, document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        SimulationAvailability.toggle(simulation, document: doc)
        XCTAssertFalse(simulation.isRunning)
    }

    /// The PCB page: not available, as before plan-36, and no reason line.
    func testPcbPageIsNotAvailable() throws {
        let doc = TestSupport.newDocument()
        XCTAssertFalse(target(doc, page: .pcb).isAvailable)
        try TestSupport.place(doc, "ATTINY13A-PU", at: CGPoint(x: 100, y: 100))
        let t = target(doc, page: .pcb)
        XCTAssertFalse(t.isAvailable)
        XCTAssertNil(t.blockedReason)
    }
}
