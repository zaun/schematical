import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-56-G phase 1: the LED and LCD display drivers in the app. They are
/// modeled as `rtc` is (converter engine, registers in the register window),
/// draw as a block, take a U, and the sidebar names their kind.
@MainActor
final class DisplayDriverTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []

    private static let kinds: [(kind: PartKind, name: String, file: String, example: String, drawer: String)] = [
        (PART_LEDDRIVER, "LED driver", "leddriver", "MAX6951CEE+T", "LED drivers"),
        (PART_LCDDRIVER, "LCD driver", "lcddriver", "PCF85176T/1Y", "LCD drivers"),
    ]

    func testNamesAndFacts() throws {
        for (kind, name, file, example, drawer) in Self.kinds {
            XCTAssertEqual(PartFacts.kindName(kind), name)
            XCTAssertEqual(PartKinds.name(of: kind), file)
            let part = try TestSupport.part(example)
            XCTAssertEqual(part.kind, kind, example)
            XCTAssertEqual(part.drawer, ["Displays & Indicators", drawer], example)
            XCTAssertTrue(part.hasRegisters, example)
            XCTAssertEqual(symbol(for: part), .block, example)
            XCTAssertEqual(ReferenceCategory.of(part), .integratedCircuit, example)
            XCTAssertEqual(ReferencePrefixes.prefix(for: .of(part), in: Data()), "U", example)
            let doc = TestSupport.newDocument()
            let chip = try TestSupport.place(doc, example, at: CGPoint(x: 100, y: 100))
            XCTAssertEqual(chip.reference, "U1", example)
            XCTAssertEqual(PartFacts.kindName(chip.part), name, example)
        }
    }

    /// Every shipped part of both kinds has the register window and no
    /// kind-specific facts rows beyond the common ones.
    func testEveryDriverHasRegisters() {
        let drivers = TestSupport.catalog.allParts.filter { $0.kind == PART_LEDDRIVER || $0.kind == PART_LCDDRIVER }
        XCTAssertEqual(drivers.count, 51)
        XCTAssertTrue(drivers.allSatisfy(\.hasRegisters))
        XCTAssertTrue(drivers.allSatisfy { part in
            if case .displayDriver = part.details { return true }
            return false
        })
    }

    /// A running driver gives the register window its registers.
    func testRunningDriverReportsRegisters() throws {
        for (_, _, _, example, _) in Self.kinds {
            let doc = TestSupport.newDocument()
            let chip = try TestSupport.place(doc, example, at: CGPoint(x: 200, y: 200))
            let simulation = SimulationController(library: TestSupport.catalog.library)
            Self.keptSimulations.append(simulation)
            simulation.start(document: doc)
            XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? example)
            let snapshot = try XCTUnwrap(simulation.registerState(of: chip), example)
            XCTAssertFalse(snapshot.registers.isEmpty, example)
            XCTAssertNil(snapshot.clock, "\(example): only an RTC has a clock")
            simulation.stop()
        }
    }
}
