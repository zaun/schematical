import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-X: the register window (RegisterWindow.swift): its snapshot of a
/// converter's or RTC's state, and what each state of the window shows.
@MainActor
final class RegisterWindowTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []
    private var probes: [ViewProbe] = []

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
        try await super.tearDown()
    }

    private func probe<V: View>(_ view: V) -> ViewProbe {
        let probe = ViewProbe(view)
        probes.append(probe)
        return probe
    }

    // MARK: The snapshot

    func testSnapshotCopiesTheBackendLists() {
        let name = strdup("CTRL")
        defer { free(name) }
        var named = ConverterRegister()
        named.name = UnsafePointer(name)
        named.address = 0x1F
        named.space = 2
        named.width = 8
        named.value = 0xA5
        var internalRegister = ConverterRegister()
        internalRegister.address = -1
        internalRegister.width = 12
        internalRegister.value = 0x7
        var time = RtcTime()
        time.year = 2024
        time.month = 2
        time.day = 29
        time.weekday = 4
        time.hour = 23
        time.minute = 59
        time.second = 58
        time.hundredth = 7
        time.hundredths = 1
        time.running = 0
        let snapshot = RegisterSnapshot([named, internalRegister], clock: time)
        XCTAssertEqual(snapshot.registers, [
            .init(name: "CTRL", address: 0x1F, space: 2, bits: 8, value: 0xA5),
            .init(name: "", address: nil, space: 0, bits: 12, value: 7),
        ])
        XCTAssertEqual(snapshot.clock, .init(year: 2024, month: 2, day: 29, weekday: 4, hour: 23, minute: 59,
                                             second: 58, hundredth: 7, hundredths: true, running: false))
        XCTAssertTrue(snapshot.hasSpaces)
        XCTAssertNil(RegisterSnapshot([internalRegister], clock: nil).clock)
        XCTAssertFalse(RegisterSnapshot([internalRegister], clock: nil).hasSpaces)
    }

    func testTextForms() {
        XCTAssertEqual(RegisterSnapshot.hex(3, bits: 1), "1")
        XCTAssertEqual(RegisterSnapshot.hex(0xA, bits: 12), "00A")
        XCTAssertEqual(RegisterSnapshot.address(.init(name: "", address: nil, space: 0, bits: 8, value: 0)), "—")
        XCTAssertEqual(RegisterSnapshot.address(.init(name: "", address: 0x2, space: 0, bits: 8, value: 0)), "0x02")
        XCTAssertEqual(RegisterSnapshot.address(.init(name: "", address: 0x123, space: 0, bits: 8, value: 0)), "0x0123")
        let clock = RegisterSnapshot.Clock(year: 2000, month: 1, day: 1, weekday: 6, hour: 0, minute: 0, second: 5,
                                           hundredth: 25, hundredths: false, running: true)
        XCTAssertEqual(RegisterSnapshot.clockText(clock), "2000-01-01 00:00:05 Saturday")
        let fine = RegisterSnapshot.Clock(year: 2000, month: 1, day: 1, weekday: 9, hour: 0, minute: 0, second: 5,
                                          hundredth: 25, hundredths: true, running: true)
        XCTAssertEqual(RegisterSnapshot.clockText(fine), "2000-01-01 00:00:05.25", "no weekday name out of range")
    }

    // MARK: The views

    func testStateViewWithoutASnapshotAsksForARun() {
        let probe = probe(RegisterStateView(kindName: "ADC", snapshot: nil))
        XCTAssertTrue(probe.has("ADC"))
        XCTAssertTrue(probe.has("Run the simulation to see the part's registers."))
        XCTAssertFalse(probe.has("Registers"))
    }

    func testStateViewShowsTheClockAndRegisters() {
        let clock = RegisterSnapshot.Clock(year: 2001, month: 2, day: 3, weekday: 0, hour: 4, minute: 5, second: 6,
                                           hundredth: 0, hundredths: false, running: false)
        let snapshot = RegisterSnapshot(registers: [
            .init(name: "SECONDS", address: 0, space: 0, bits: 8, value: 0x59),
            .init(name: "RAM0", address: 0x08, space: 1, bits: 8, value: 0x3C),
        ], clock: clock)
        let probe = probe(RegisterStateView(kindName: "RTC", snapshot: snapshot))
        XCTAssertTrue(probe.has("Time"), "\(probe.labels())")
        XCTAssertTrue(probe.has("2001-02-03 04:05:06 Sunday"))
        XCTAssertTrue(probe.has("Stopped"))
        XCTAssertTrue(probe.has("Space"), "a register outside space 0 shows the column")
        XCTAssertTrue(probe.has("SECONDS"))
        XCTAssertTrue(probe.has("59"))
        XCTAssertTrue(probe.has("0x08"))
        XCTAssertTrue(probe.has("3C"))
    }

    func testStateViewOfARunningClockInSpaceZero() {
        let clock = RegisterSnapshot.Clock(year: 2001, month: 2, day: 3, weekday: 0, hour: 4, minute: 5, second: 6,
                                           hundredth: 0, hundredths: false, running: true)
        let snapshot = RegisterSnapshot(registers: [.init(name: "CTRL", address: 1, space: 0, bits: 8, value: 1)],
                                        clock: clock)
        let probe = probe(RegisterStateView(kindName: "RTC", snapshot: snapshot))
        XCTAssertTrue(probe.has("CTRL"))
        XCTAssertFalse(probe.has("Stopped"))
        XCTAssertFalse(probe.has("Space"))
    }

    func testWindowOfAClosedSchematic() {
        let probe = probe(RegisterWindowView(target: RegisterWindowTarget(documentID: UUID(), chipID: UUID())))
        XCTAssertTrue(probe.has("Schematic Closed"), "\(probe.labels())")
    }

    /// An open schematic: the part stopped, running, then removed.
    func testWindowFollowsThePartAndTheRun() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let open = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let chip = try TestSupport.place(doc, "BQ32000D", at: CGPoint(x: 200, y: 200))
        let target = RegisterWindowTarget(documentID: open.model.id, chipID: chip.id)
        let probe = probe(RegisterWindowView(target: target))
        XCTAssertTrue(probe.has(PartFacts.kindName(chip.part)), "\(probe.labels())")
        XCTAssertTrue(probe.has("Run the simulation to see the part's registers."))

        let simulation = open.model.simulation
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        probe.settle()
        XCTAssertTrue(probe.has("Registers"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Time"), "an RTC shows its clock")
        simulation.stop()
        probe.settle()

        XCTAssertTrue(doc.undo(), "undo the placement")
        probe.settle()
        XCTAssertTrue(probe.has("Part Removed"), "\(probe.labels())")
    }

    /// The inspector on its own, for a converter-engine part that is not an RTC.
    func testInspectorOfADriver() throws {
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let chip = try TestSupport.place(doc, "MAX6951CEE+T", at: CGPoint(x: 200, y: 200))
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        let probe = probe(RegisterInspector(chipID: chip.id, document: doc, simulation: simulation))
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        probe.settle()
        XCTAssertTrue(probe.has("Registers"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Time"), "only an RTC has a clock")
        simulation.stop()
    }
}
