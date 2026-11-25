import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-58-G: slide switches in the drawer, on the canvas, in the file, in the
/// netlist and under the mouse.
@MainActor
final class SlideSwitchTests: XCTestCase {
    /// Kept for the life of the test process, like `TestSupport`'s documents:
    /// freeing one inside a test method aborts the host.
    private static var keptSimulations: [SimulationController] = []

    private var slideParts: [CatalogPart] {
        TestSupport.allParts.filter(\.isSlideSwitch)
    }

    private func place(_ doc: SchematicDocument, _ name: String, at point: CGPoint = CGPoint(x: 200, y: 200)) throws -> PlacedChip {
        try TestSupport.place(doc, name, at: point)
    }

    // MARK: Phase 1 — catalog, drawer, chip state, file

    /// Every slide part draws as a slide switch, is named "Slide switch", has
    /// no "(n)" count and sits under Switches & Relays → Slide by its function.
    func testCatalogFacts() throws {
        XCTAssertGreaterThan(slideParts.count, 100)
        for part in slideParts {
            XCTAssertEqual(symbol(for: part), .slideSwitch, part.name)
            XCTAssertFalse(part.isTactileSwitch, part.name)
            XCTAssertNil(part.count, part.name)
            XCTAssertNil(part.trailingLabel, part.name)
            XCTAssertEqual(PartFacts.kindName(part), "Slide switch", part.name)
            XCTAssertEqual(part.drawer, ["Switches & Relays", "Slide"], part.name)
            XCTAssertEqual(part.drawerLeaf, part.function, part.name)
            XCTAssertEqual(part.slideStates?.circuit, part.function.split(separator: " ").first.map(String.init), part.name)
        }
    }

    /// The drawer lists one leaf per circuit and action under Switches &
    /// Relays → Slide, DIP switches stay listed directly.
    func testDrawerLeaves() throws {
        let groups = TestSupport.catalog.groups(matching: DrawerFilter())
        let switchGroup = try XCTUnwrap(groups.first { $0.category == "Switches & Relays" })
        let slide = try XCTUnwrap(switchGroup.subcategories.first { $0.subcategory == "Slide" })
        XCTAssertTrue(slide.parts.isEmpty)
        XCTAssertEqual(Set(slide.functions.map(\.function)),
                       ["SPST On-Off", "SPDT On-On", "SPDT On-Off-On", "DPST On-Off", "DPDT On-On", "DPDT On-Off-On"])
        XCTAssertTrue(slide.functions.contains { $0.function == "SPDT On-On" && $0.parts.contains { $0.name == "1101M2S3CQE2" } })
        XCTAssertEqual(slide.functions.flatMap(\.parts).count, slideParts.count)
        for function in slide.functions {
            for part in function.parts { XCTAssertEqual(part.function, function.function, part.name) }
        }
        let dip = try XCTUnwrap(switchGroup.subcategories.first { $0.subcategory == "DIP" })
        XCTAssertTrue(dip.functions.isEmpty)
        XCTAssertFalse(dip.parts.isEmpty)
    }

    /// A part row reads package, actuator, then `N V max`.
    func testDrawerRowDetail() throws {
        let part = try TestSupport.part("1101M2S3CQE2")
        XCTAssertEqual(part.package, "THT")
        XCTAssertEqual(part.detailLabels, ["Standard"])
        XCTAssertEqual(part.voltageLabel, "28 V max")
    }

    /// The DIP "(n) positions" menu keeps listing DIP switches' counts only, and
    /// picking one leaves the slide leaves in the drawer.
    func testNoDipCountMenuForSlide() throws {
        let catalog = TestSupport.catalog
        let dipCounts = Set(TestSupport.allParts.filter {
            $0.kind == PART_SWITCH && !$0.isTactileSwitch && !$0.isSlideSwitch
        }.compactMap(\.count))
        XCTAssertFalse(dipCounts.isEmpty)
        XCTAssertEqual(Set(catalog.switchPositionCounts(in: "Switches & Relays")), dipCounts)
        var filter = DrawerFilter()
        filter.switchPositions = dipCounts.max()
        let switchGroup = try XCTUnwrap(catalog.groups(matching: filter).first { $0.category == "Switches & Relays" })
        let slide = try XCTUnwrap(switchGroup.subcategories.first { $0.subcategory == "Slide" })
        XCTAssertEqual(slide.functions.flatMap(\.parts).count, slideParts.count)
        let dip = try XCTUnwrap(switchGroup.subcategories.first { $0.subcategory == "DIP" })
        XCTAssertTrue(dip.parts.allSatisfy { $0.count == dipCounts.max() })
    }

    /// The voltage menu hides a slide switch rated below the rail.
    func testVoltageFilter() throws {
        var filter = DrawerFilter()
        filter.voltageMV = 12_000
        let groups = TestSupport.catalog.groups(matching: filter)
        let names = Set(groups.flatMap(\.allParts).filter(\.isSlideSwitch).map(\.name))
        XCTAssertTrue(names.contains("1101M2S3CQE2"))
        XCTAssertFalse(names.contains("JS102000SAQN"), "rated 5 V")
    }

    /// States, rest and masks of an SPDT On-On, an SPDT On-Off-On and a DPDT On-On.
    func testDetails() throws {
        func slide(_ name: String) throws -> (circuit: String, positions: Int, rest: Int, masks: [UInt32]) {
            try XCTUnwrap(try TestSupport.part(name).slideStates, name)
        }
        let spdt = try slide("1101M2S3CQE2")
        XCTAssertEqual(spdt.circuit, "SPDT")
        XCTAssertEqual(spdt.positions, 2)
        XCTAssertEqual(spdt.rest, 1)
        XCTAssertEqual(spdt.masks, [0b01, 0b10])
        let center = try slide("1103M2S3CBE2")
        XCTAssertEqual(center.positions, 3)
        XCTAssertEqual(center.rest, 2)
        XCTAssertEqual(center.masks, [0b01, 0, 0b10])
        let dpdt = try slide("1201M2S3CQE2")
        XCTAssertEqual(dpdt.circuit, "DPDT")
        XCTAssertEqual(dpdt.positions, 2)
        XCTAssertEqual(dpdt.rest, 1)
        XCTAssertEqual(dpdt.masks, [0b0101, 0b1010])
        XCTAssertEqual(try TestSupport.part("1201M2S3CQE2").slideContacts, 4)
        XCTAssertEqual(try TestSupport.part("1101M2S3CQE2").slideContacts, 2)
        if case let .slideSwitch(_, _, _, _, actuator, ratedMV) = try TestSupport.part("1101M2S3CQE2").details {
            XCTAssertEqual(actuator, "Standard")
            XCTAssertEqual(ratedMV, 28_000)
        } else {
            XCTFail("1101M2S3CQE2 is not a slide switch")
        }
    }

    /// The layouts: every pin of each circuit has a place, common pins on the
    /// left, throws on the right, throw A above throw B.
    func testLayouts() throws {
        let doc = TestSupport.newDocument()
        let spdt = try place(doc, "1101M2S3CQE2", at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(spdt.symbol, .slideSwitch)
        func slot(_ chip: PlacedChip, _ name: String) throws -> ChipLayout.PinSlot {
            try XCTUnwrap(chip.layout.slots.first { $0.pin.name == name }, name)
        }
        XCTAssertEqual(spdt.layout.slots.count, 3)
        XCTAssertEqual(try slot(spdt, "C").side, .left)
        XCTAssertEqual(try slot(spdt, "A").side, .right)
        XCTAssertEqual(try slot(spdt, "B").side, .right)
        XCTAssertLessThan(try slot(spdt, "A").centerY, try slot(spdt, "B").centerY)
        XCTAssertEqual(try slot(spdt, "C").centerY, (try slot(spdt, "A").centerY + (try slot(spdt, "B").centerY)) / 2)
        let dpdt = try place(doc, "1201M2S3CQE2", at: CGPoint(x: 500, y: 200))
        XCTAssertEqual(dpdt.layout.slots.count, 6)
        XCTAssertLessThan(try slot(dpdt, "B1").centerY, try slot(dpdt, "A2").centerY)
        XCTAssertEqual(try slot(dpdt, "C2").side, .left)
        // Every pin tip is on the grid.
        for chip in doc.chips {
            for item in chip.layout.slots {
                let tip = chip.tip(of: item)
                XCTAssertEqual(tip.x.truncatingRemainder(dividingBy: 10), 0, "\(chip.partNumber) \(item.pin.name)")
                XCTAssertEqual(tip.y.truncatingRemainder(dividingBy: 10), 0, "\(chip.partNumber) \(item.pin.name)")
            }
        }
        // A DPST and an SPST place their poles in a row each, A left, B right.
        let dpst = try XCTUnwrap(slideParts.first { $0.function == "DPST On-Off" })
        let dpstChip = try place(doc, dpst.name, at: CGPoint(x: 800, y: 200))
        XCTAssertEqual(try slot(dpstChip, "A1").side, .left)
        XCTAssertEqual(try slot(dpstChip, "B2").side, .right)
        XCTAssertEqual(try slot(dpstChip, "A1").centerY, try slot(dpstChip, "B1").centerY)
        XCTAssertLessThan(try slot(dpstChip, "B1").centerY, try slot(dpstChip, "B2").centerY)
        let spst = try XCTUnwrap(slideParts.first { $0.function == "SPST On-Off" })
        let spstChip = try place(doc, spst.name, at: CGPoint(x: 1100, y: 200))
        XCTAssertEqual(try slot(spstChip, "A").side, .left)
        XCTAssertEqual(try slot(spstChip, "B").side, .right)
    }

    /// The sidebar's Details rows for a slide part.
    func testSidebarRows() throws {
        let doc = TestSupport.newDocument()
        let chip = try place(doc, "1101M2S3CQE2")
        let rows = Dictionary(PartFacts.rows(for: chip).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
        XCTAssertEqual(rows["Kind"], "Slide switch")
        XCTAssertEqual(rows["Circuit"], "SPDT")
        XCTAssertEqual(rows["Action"], "On-On")
        XCTAssertEqual(rows["Position"], "1 of 2")
        XCTAssertEqual(rows["Actuator"], "Standard")
        XCTAssertEqual(rows["Rating"], "28 V max")
        XCTAssertNil(rows["Positions"])
        let center = try place(doc, "1103M2S3CBE2", at: CGPoint(x: 500, y: 200))
        let centerRows = Dictionary(PartFacts.rows(for: center).map { ($0.label, $0.value) }, uniquingKeysWith: { a, _ in a })
        XCTAssertEqual(centerRows["Action"], "On-Off-On")
        XCTAssertEqual(centerRows["Position"], "2 of 3")
    }

    /// A new chip of any slide function starts in its rest position, its
    /// contacts exactly that position's.
    func testNewChipStartsInRest() throws {
        var seen: Set<String> = []
        for part in slideParts where seen.insert(part.function).inserted {
            let slide = try XCTUnwrap(part.slideStates, part.name)
            let chip = PlacedChip(reference: "SW1", part: part, symbol: symbol(for: part), origin: .zero)
            XCTAssertEqual(chip.slidePosition, slide.rest, part.name)
            XCTAssertEqual(chip.switchesOn, PlacedChip.contacts(mask: slide.masks[slide.rest - 1], count: part.slideContacts),
                           part.name)
            XCTAssertEqual(chip.switchesOn.count, part.slideContacts, part.name)
        }
        XCTAssertEqual(seen.count, 6)
        // A switch that has an On and an Off position rests Off.
        let spst = PlacedChip(reference: "SW1", part: try XCTUnwrap(slideParts.first { $0.function == "SPST On-Off" }),
                              symbol: .slideSwitch, origin: .zero)
        XCTAssertEqual(spst.switchesOn, [false])
        XCTAssertEqual(spst.slidePosition, 2)
        XCTAssertNil(PlacedChip(reference: "U1", part: try TestSupport.part("SN74HC00N"), symbol: .block, origin: .zero).slidePosition)
    }

    /// The chip takes the position its contacts are in; anything else is the
    /// rest position.
    func testChipInitFromContacts() throws {
        let part = try TestSupport.part("1103M2S3CBE2")
        func chip(_ switches: [Bool]) -> PlacedChip {
            PlacedChip(reference: "SW1", part: part, symbol: .slideSwitch, origin: .zero, switchesOn: switches)
        }
        XCTAssertEqual(chip([true, false]).slidePosition, 1)
        XCTAssertEqual(chip([false, false]).slidePosition, 2)
        XCTAssertEqual(chip([false, true]).slidePosition, 3)
        // Both contacts, a short or a long array, or none: the rest position (2).
        XCTAssertEqual(chip([true, true]).slidePosition, 2)
        XCTAssertEqual(chip([true]).slidePosition, 2)
        XCTAssertEqual(chip([true, false, false]).slidePosition, 2)
        XCTAssertEqual(chip([]).slidePosition, 2)
        XCTAssertEqual(chip([true, true]).switchesOn, [false, false])
    }

    /// The position is saved as `switches` (per contact, as for a DIP switch)
    /// and reopens; a file whose `switches` matches no position opens in rest.
    func testFileRoundTrip() throws {
        let doc = TestSupport.newDocument()
        let chip = try place(doc, "1101M2S3CQE2")
        XCTAssertEqual(chip.switchesOn, [true, false])
        var file = try SchematicFile.decode(SchematicFile(document: doc, fileURL: nil).encoded())
        XCTAssertEqual(file.parts[0].switches, [true, false])
        file.parts[0].switches = [false, true]
        let opened = TestSupport.newDocument()
        _ = try file.apply(to: opened, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(opened.chips[0].switchesOn, [false, true])
        XCTAssertEqual(opened.chips[0].slidePosition, 2)
        // Saved again, the same.
        let again = try SchematicFile.decode(SchematicFile(document: opened, fileURL: nil).encoded())
        XCTAssertEqual(again.parts[0].switches, [false, true])

        // A file whose switches match no position: the rest position.
        for bad: [Bool]? in [[true, true], [false, false], [true], [true, false, true], nil] {
            var broken = file
            broken.parts[0].switches = bad
            let reopened = TestSupport.newDocument()
            _ = try broken.apply(to: reopened, catalog: TestSupport.catalog, fileURL: nil)
            XCTAssertEqual(reopened.chips[0].switchesOn, [true, false], "\(String(describing: bad))")
            XCTAssertEqual(reopened.chips[0].slidePosition, 1)
        }
    }

    /// DIP and tactile switches are unchanged next to a slide switch.
    func testDipAndTactileUnchanged() throws {
        let doc = TestSupport.newDocument()
        let dip = try TestSupport.firstDip()
        let dipChip = try place(doc, dip.name, at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(dipChip.symbol, .dipSwitch)
        XCTAssertNil(dipChip.slidePosition)
        XCTAssertNil(dipChip.nextSlidePosition)
        XCTAssertEqual(dipChip.switchesOn.count, dip.count)
        let tactile = try XCTUnwrap(TestSupport.allParts.first { $0.isTactileSwitch })
        let tactileChip = try place(doc, tactile.name, at: CGPoint(x: 500, y: 200))
        XCTAssertEqual(tactileChip.symbol, .tactileSwitch)
        XCTAssertNil(tactileChip.slidePosition)
        XCTAssertEqual(tactileChip.switchesOn, [false])
    }
}
