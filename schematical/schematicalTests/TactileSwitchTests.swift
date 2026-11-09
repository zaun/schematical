import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-27-J: tactile (push-button) switches in the drawer, on the canvas,
/// in the netlist, in the file and under the mouse.
@MainActor
final class TactileSwitchTests: XCTestCase {
    /// Every part the drawer lists.
    private var drawerParts: [CatalogPart] {
        TestSupport.catalog.groups(matching: DrawerFilter()).flatMap { category in
            category.functions.flatMap(\.parts) + category.subcategories.flatMap { sub in
                sub.parts + sub.functions.flatMap(\.parts)
            }
        }
    }

    private var tactileParts: [CatalogPart] {
        drawerParts.filter(\.isTactileSwitch)
    }

    private func firstDip() throws -> CatalogPart {
        try XCTUnwrap(drawerParts.filter { $0.kind == PART_SWITCH && !$0.isTactileSwitch }
            .sorted { $0.name < $1.name }.first)
    }

    /// The first tactile part (by name) matching `wanted`.
    private func firstTactile(_ what: String, _ wanted: (CatalogPart) -> Bool) throws -> CatalogPart {
        let parts = tactileParts.filter(wanted).sorted { $0.name < $1.name }
        return try XCTUnwrap(parts.first, "no tactile part: \(what)")
    }

    private func terminals(_ part: CatalogPart) -> Int { part.pins.filter { !$0.isHidden }.count }

    private func noTwo() throws -> CatalogPart {
        try firstTactile("NO 2-terminal") { !$0.isNormallyClosed && terminals($0) == 2 }
    }

    private func noFour() throws -> CatalogPart {
        try firstTactile("NO 4-terminal") { !$0.isNormallyClosed && terminals($0) == 4 }
    }

    private func nc() throws -> CatalogPart {
        try firstTactile("NC") { $0.isNormallyClosed }
    }

    private func positions(_ doc: SchematicDocument, _ chip: UUID) -> [Int] {
        let netlist = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses)
        let part = netlist.chipIDs.firstIndex(of: chip)
        return netlist.switchesOn.filter { $0.part == part }.map(\.position).sorted()
    }

    // MARK: Phase 1 — catalog, drawer, facts

    /// Every tactile part draws as a tactile switch, has no "(n)" count and is
    /// named "Tactile switch" in the sidebar; its drawer leaf matches its contact.
    func testCatalogFacts() throws {
        XCTAssertFalse(tactileParts.isEmpty, "the catalog has no tactile switch")
        for part in tactileParts {
            XCTAssertEqual(symbol(for: part), .tactileSwitch, part.name)
            XCTAssertNil(part.count, part.name)
            XCTAssertEqual(PartFacts.kindName(part), "Tactile switch", part.name)
            XCTAssertEqual(part.drawer, ["Switch", "Tactile"], part.name)
            XCTAssertEqual(part.function, part.isNormallyClosed ? "Normal Close" : "Normal Open", part.name)
            XCTAssertNotNil(part.voltageLabel, part.name)
            XCTAssertTrue([2, 4].contains(terminals(part)), part.name)
        }
    }

    /// Switch → Tactile groups its parts by body size ("6 × 6 mm"), smallest
    /// first; Switch → DIP still lists its parts directly.
    func testDrawerLeaves() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let switchGroup = try XCTUnwrap(groups.first { $0.category == "Switch" })
        let tactile = try XCTUnwrap(switchGroup.subcategories.first { $0.subcategory == "Tactile" })
        XCTAssertTrue(tactile.parts.isEmpty)
        XCTAssertGreaterThan(tactile.functions.count, 10)
        XCTAssertTrue(tactile.functions.contains { $0.function == "6 × 6 mm" })
        XCTAssertTrue(tactile.functions.contains { $0.function == "12 × 12 mm" })
        let lengths = tactile.functions.map { Int($0.function.prefix { $0.isNumber }) ?? 0 }
        XCTAssertEqual(lengths, lengths.sorted(), "sizes smallest first")
        for function in tactile.functions {
            XCTAssertNotNil(function.function.range(of: #"^\d+ × \d+ mm$"#, options: .regularExpression),
                            function.function)
            for part in function.parts {
                XCTAssertEqual(part.drawerLeaf, function.function, part.name)
            }
        }
        let dip = try XCTUnwrap(switchGroup.subcategories.first { $0.subcategory == "DIP" })
        XCTAssertTrue(dip.functions.isEmpty)
        XCTAssertFalse(dip.parts.isEmpty)
    }

    /// The sidebar's Details rows for a tactile part.
    func testSidebarRows() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, try noTwo().name, at: CGPoint(x: 200, y: 200))
        let rows = Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
        XCTAssertEqual(rows["Kind"], "Tactile switch")
        XCTAssertEqual(rows["Contact"], "Normal open, momentary")
        XCTAssertEqual(rows["Terminals"], "2")
        XCTAssertNotNil(rows["Actuator"])
        XCTAssertNotNil(rows["Rating"])
        XCTAssertNotNil(rows["Body"])
        XCTAssertNil(rows["Positions"])
    }

    // MARK: Phase 2 — model, symbol, file, netlist

    /// (1) A two-terminal Normal Open switch: released at rest, its contact
    /// in the netlist only while pressed; the layout has A left, B right.
    func testNormalOpenTwoTerminal() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, try noTwo().name, at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(chip.symbol, .tactileSwitch)
        XCTAssertEqual(chip.switchesOn, [false])
        XCTAssertEqual(chip.layout.slots.filter { $0.side == .left }.map(\.pin.name), ["A"])
        XCTAssertEqual(chip.layout.slots.filter { $0.side == .right }.map(\.pin.name), ["B"])
        XCTAssertEqual(positions(doc, chip.id), [])
        XCTAssertEqual(doc.setButton(chip.id, pressed: true), true)
        XCTAssertEqual(doc.chips[0].switchesOn, [true])
        XCTAssertEqual(positions(doc, chip.id), [1])
        XCTAssertEqual(doc.setButton(chip.id, pressed: false), false)
        XCTAssertEqual(positions(doc, chip.id), [])
    }

    /// (2) A four-terminal Normal Open switch: its joined pairs (positions 2
    /// and 3) are always closed; position 1 only while pressed. Its unwired
    /// pins get nets of their own, so every pin has a connection.
    func testNormalOpenFourTerminal() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, try noFour().name, at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(chip.layout.slots.filter { $0.side == .left }.map(\.pin.name), ["A1", "A2"])
        XCTAssertEqual(chip.layout.slots.filter { $0.side == .right }.map(\.pin.name), ["B1", "B2"])
        XCTAssertEqual(positions(doc, chip.id), [2, 3])
        _ = doc.setButton(chip.id, pressed: true)
        XCTAssertEqual(positions(doc, chip.id), [1, 2, 3])
        _ = doc.setButton(chip.id, pressed: false)
        XCTAssertEqual(positions(doc, chip.id), [2, 3])
        let netlist = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses)
        XCTAssertEqual(netlist.connections.filter { $0.part == 0 }.count, 4)
        XCTAssertEqual(netlist.positionCounts, [1])
    }

    /// (3) A Normal Close switch: its contact is in the netlist at rest and
    /// not while pressed.
    func testNormalClose() throws {
        let doc = TestSupport.newDocument()
        let part = try nc()
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200, y: 200))
        let joined = terminals(part) == 4 ? [2, 3] : []
        XCTAssertEqual(positions(doc, chip.id), [1] + joined)
        _ = doc.setButton(chip.id, pressed: true)
        XCTAssertEqual(positions(doc, chip.id), joined)
    }

    /// (4) Save, then reopen with the switch pressed: it reopens released and
    /// the file has no `switches` key for it.
    func testFileNeverSavesPressed() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, try noTwo().name, at: CGPoint(x: 200, y: 200))
        _ = doc.setButton(chip.id, pressed: true)
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertNil(file.parts[0].switches)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.chips[0].switchesOn, [false])
        XCTAssertFalse(opened.chips[0].isPressed)
    }

    /// (5) A DIP switch is unchanged: one state per position, saved, in the
    /// netlist while ON, and no button.
    func testDipSwitchUnchanged() throws {
        let doc = TestSupport.newDocument()
        let dip = try firstDip()
        let chip = try TestSupport.place(doc, dip.name, at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(chip.symbol, PartSymbol.dipSwitch)
        XCTAssertEqual(chip.switchesOn.count, dip.count)
        XCTAssertNil(chip.buttonRect)
        XCTAssertEqual(positions(doc, chip.id), [])
        XCTAssertEqual(doc.toggleSwitch(chip.id, position: 1), true)
        XCTAssertEqual(positions(doc, chip.id), [1])
        let netlist = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses)
        XCTAssertEqual(netlist.positionCounts, [dip.count ?? 0])
        let file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.parts[0].switches?.first, true)
    }

    /// The button is the middle third of the body under the header; a point
    /// on it hits, a pin stub does not.
    func testButtonHit() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, try noTwo().name, at: CGPoint(x: 200, y: 200))
        let button = try XCTUnwrap(chip.buttonRect)
        XCTAssertTrue(chip.buttonHit(at: CGPoint(x: button.midX, y: button.midY)))
        let stub = try XCTUnwrap(chip.layout.slots.first)
        XCTAssertFalse(chip.buttonHit(at: chip.tip(of: stub)))
    }

    // MARK: Phase 3 — press and release

    /// `setButton` refuses a DIP switch; pressing then releasing a tactile
    /// switch gives [true] then [false]; `releaseAllButtons` lets go of all.
    func testPressAndRelease() throws {
        let doc = TestSupport.newDocument()
        let dip = try firstDip()
        let dipChip = try TestSupport.place(doc, dip.name, at: CGPoint(x: 100, y: 100))
        XCTAssertNil(doc.setButton(dipChip.id, pressed: true))
        let one = try TestSupport.place(doc, try noTwo().name, at: CGPoint(x: 400, y: 100))
        let two = try TestSupport.place(doc, try nc().name, at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(doc.setButton(one.id, pressed: true), true)
        XCTAssertEqual(doc.chips[1].switchesOn, [true])
        XCTAssertEqual(doc.setButton(one.id, pressed: false), false)
        XCTAssertEqual(doc.chips[1].switchesOn, [false])
        _ = doc.setButton(one.id, pressed: true)
        _ = doc.setButton(two.id, pressed: true)
        doc.releaseAllButtons()
        XCTAssertEqual(doc.chips[1].switchesOn, [false])
        XCTAssertEqual(doc.chips[2].switchesOn, [false])
    }
}
