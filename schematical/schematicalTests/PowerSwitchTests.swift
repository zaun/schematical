import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-55-J: power-switch parts in the app. A power switch takes the next U,
/// draws as a block, and the sidebar names its kind ("Load switch", "Dual
/// current-limited switch", "Ideal diode" …) and its facts; the voltage check
/// refuses an input above the part's maximum and names the clash of a
/// switch's output with a supply. The parts are picked from the loaded
/// catalog by kind and facts, not by name, so the tests run on the real
/// library (plan-55-H and I's parts).
@MainActor
final class PowerSwitchTests: XCTestCase {
    private func row(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    private func place(_ doc: SchematicDocument, _ part: CatalogPart, _ x: CGFloat, _ y: CGFloat) throws -> PlacedChip {
        try TestSupport.place(doc, part.name, at: CGPoint(x: x, y: y))
    }

    /// The first part (by name) of the power switch kind that satisfies `want`.
    private func pick(_ what: String, _ want: (PowerSwitchInfo, CatalogPart) -> Bool) throws -> CatalogPart {
        try XCTUnwrap(TestSupport.allParts
            .filter { $0.kind == PART_POWERSWITCH }
            .sorted { $0.name < $1.name }
            .first { part in part.powerswitch.map { want($0, part) } ?? false }, "no power switch: \(what)")
    }

    /// A single-switch part with one input, one output and one control pin, no flag: the switch's pins by role.
    private func simple(_ part: CatalogPart) -> (input: String, output: String)? {
        let power = Set(part.pins.filter { $0.role == .power }.map(\.name))
        let outputs = Set(part.pins.filter { $0.role == .output }.map(\.name))
        guard power.count == 1, outputs.count == 1, let input = power.first, let output = outputs.first else { return nil }
        return (input, output)
    }

    // MARK: - Facts, kind names, labels

    /// A load switch: "Load switch", its input range, current, on-resistance.
    func testLoadSwitchFacts() throws {
        let part = try pick("load, one switch, with input range, current and on-resistance") { info, _ in
            info.type == "load" && info.switches == 1 && info.vinMinMV > 0 && info.vinMaxMV > 0 && info.ioutMA > 0
                && info.ronMOhm > 0
        }
        let info = try XCTUnwrap(part.powerswitch)
        let doc = TestSupport.newDocument()
        let u = try place(doc, part, 100, 100)
        XCTAssertEqual(u.reference, "U1")
        XCTAssertEqual(u.symbol, .block)
        XCTAssertEqual(symbol(for: u.part), .block)
        XCTAssertEqual(PartFacts.kindName(PART_POWERSWITCH), "Power switch")
        XCTAssertEqual(PartFacts.kindName(u.part), "Load switch")
        XCTAssertEqual(row(u, "Type"), "Load switch")
        XCTAssertNil(row(u, "Channels"))
        XCTAssertEqual(row(u, "Input"), "\(PartFacts.milli(info.vinMinMV))–\(PartFacts.milli(info.vinMaxMV)) V")
        XCTAssertEqual(row(u, "Current"), "\(PartFacts.milli(info.ioutMA)) A")
        XCTAssertEqual(row(u, "On-resistance"), PartFacts.milliohmsText(UInt64(info.ronMOhm)))
        XCTAssertNil(row(u, "Current limit"), "a load switch has no current limit row")
        XCTAssertEqual(u.part.detailLabels, ["≤ \(PartFacts.milli(info.vinMaxMV)) V", "\(PartFacts.milli(info.ioutMA)) A"])
        XCTAssertEqual(ReferenceCategory.of(u.part), .integratedCircuit)
    }

    /// A current-limited switch shows its current limit; a dual one is named by its count.
    func testCurrentLimitedFacts() throws {
        let single = try pick("current-limited, one switch, with a fixed limit") { info, _ in
            info.type == "limited" && info.switches == 1 && info.ilimitMA > 0 && info.vinMaxMV > 0
        }
        let info = try XCTUnwrap(single.powerswitch)
        let doc = TestSupport.newDocument()
        let u = try place(doc, single, 100, 100)
        XCTAssertEqual(PartFacts.kindName(u.part), "Current-limited switch")
        XCTAssertEqual(row(u, "Type"), "Current-limited switch")
        XCTAssertEqual(row(u, "Current limit"), "\(PartFacts.milli(info.ilimitMA)) A")

        let dual = try pick("current-limited, two switches") { info, _ in info.type == "limited" && info.switches == 2 }
        XCTAssertEqual(PartFacts.kindName(dual), "Dual current-limited switch")
        let chip = try place(doc, dual, 300, 100)
        XCTAssertEqual(row(chip, "Channels"), "2")

        for (count, name) in [(3, "Triple current-limited switch"), (4, "Quad current-limited switch")] {
            if let part = TestSupport.allParts.first(where: { $0.powerswitch?.type == "limited" && $0.powerswitch?.switches == count }) {
                XCTAssertEqual(PartFacts.kindName(part), name)
            }
        }
    }

    /// An ideal diode, and a package with two inputs ("Dual ideal diode", an "Inputs" row).
    func testIdealDiodeFacts() throws {
        let one = try pick("ideal diode, one input") { info, _ in info.type == "ideal-diode" && info.switches == 1 }
        XCTAssertEqual(PartFacts.kindName(one), "Ideal diode")
        let two = try pick("ideal diode, two inputs") { info, _ in info.type == "ideal-diode" && info.switches == 2 }
        XCTAssertEqual(PartFacts.kindName(two), "Dual ideal diode")
        let doc = TestSupport.newDocument()
        let chip = try place(doc, two, 100, 100)
        XCTAssertEqual(row(chip, "Type"), "Ideal diode")
        XCTAssertEqual(row(chip, "Inputs"), "2")
        XCTAssertNil(row(chip, "Channels"))
    }

    // MARK: - The messages

    /// A supply above a part's input maximum on its input pin is refused, naming the part and the limit.
    func testInputOverMessage() throws {
        let part = try pick("a load switch with a simple pinout and an input maximum below 12 V") { info, part in
            info.type == "load" && info.switches == 1 && info.vinMaxMV > 0 && info.vinMaxMV <= 6000 && self.simple(part) != nil
        }
        let info = try XCTUnwrap(part.powerswitch)
        let pins = try XCTUnwrap(simple(part))
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let sw = try place(doc, part, 300, 300)
        let supply = try TestSupport.part("12V")
        doc.placeChip(part: supply, symbol: symbol(for: supply), centeredAt: CGPoint(x: 100, y: 300))
        let source = try XCTUnwrap(doc.chips.last)
        let inPin = try XCTUnwrap(sw.part.pins.first { $0.name == pins.input })
        let message = try XCTUnwrap(doc.connect(.pin(try TestSupport.pin(source, "VCC")),
                                                .pin(try TestSupport.pin(sw, pins.input))))
        XCTAssertTrue(message.hasPrefix("Can't connect: U1 pin \(inPin.designator) (\(pins.input)) gets 12 V;"), message)
        XCTAssertTrue(message.hasSuffix("\(part.name) takes at most \(VoltageCheck.volts(info.vinMaxMV)) V in."), message)
    }

    /// A part with input range 3.3–5 V, one input and one output, no flag.
    private func rail(_ doc: SchematicDocument, _ name: String, _ x: CGFloat, _ y: CGFloat) throws -> PlacedChip {
        let part = try TestSupport.part(name)
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return try XCTUnwrap(doc.chips.last)
    }

    private func wideSwitch() throws -> CatalogPart {
        try pick("a load switch with a simple pinout that takes 3.3 V and 5 V") { info, part in
            info.type == "load" && info.switches == 1 && info.vinMaxMV >= 5000 && info.vinMaxMV <= 6000
                && info.vinMinMV <= 3300 && self.simple(part) != nil
        }
    }

    /// A 5 V switch output on the 3V3 rail: the lower voltage first, the supply named first.
    func testSupplyClashFiveVoltSwitchOnTheLowerRail() throws {
        let part = try wideSwitch()
        let pins = try XCTUnwrap(simple(part))
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let sw = try place(doc, part, 400, 300)
        let five = try rail(doc, "5V", 100, 300)
        let three = try rail(doc, "3V3", 700, 300)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(five, "VCC")), .pin(try TestSupport.pin(sw, pins.input))))
        let message = try XCTUnwrap(doc.connect(.pin(try TestSupport.pin(three, "VCC")),
                                                .pin(try TestSupport.pin(sw, pins.output))))
        XCTAssertEqual(message, "Can't connect: 3.3 V (\(three.reference)) and 5 V (\(sw.reference)) supplies on one net.")
    }

    /// A 3.3 V switch output on the 5V rail: the switch is the lower voltage.
    func testSupplyClashThreeVoltSwitchOnTheHigherRail() throws {
        let part = try wideSwitch()
        let pins = try XCTUnwrap(simple(part))
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let sw = try place(doc, part, 400, 300)
        let three = try rail(doc, "3V3", 100, 300)
        let five = try rail(doc, "5V", 700, 300)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(three, "VCC")), .pin(try TestSupport.pin(sw, pins.input))))
        let message = try XCTUnwrap(doc.connect(.pin(try TestSupport.pin(five, "VCC")),
                                                .pin(try TestSupport.pin(sw, pins.output))))
        XCTAssertEqual(message, "Can't connect: 3.3 V (\(sw.reference)) and 5 V (\(five.reference)) supplies on one net.")
    }

    /// Two switches of 5 V and 3.3 V on one net give no issue: the net reads the higher level (ORing).
    func testTwoSwitchesOnOneNetDoNotClash() throws {
        let part = try wideSwitch()
        let pins = try XCTUnwrap(simple(part))
        let doc = TestSupport.newDocument(library: TestSupport.catalog.library)
        let a = try place(doc, part, 400, 200)
        let b = try place(doc, part, 400, 500)
        let five = try rail(doc, "5V", 100, 200)
        let three = try rail(doc, "3V3", 100, 500)
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(five, "VCC")), .pin(try TestSupport.pin(a, pins.input))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(three, "VCC")), .pin(try TestSupport.pin(b, pins.input))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, pins.output)), .pin(try TestSupport.pin(b, pins.output))))
    }
}
