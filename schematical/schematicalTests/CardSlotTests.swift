import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-54-O: card slots (the `cardSlot` details of a `connector` part with a
/// `card`): a memory / SIM socket, a memory-module socket or a card-edge
/// socket. The kind name, facts rows, chip labels, symbol, drawer branch
/// (Connector › Card slot › leaf) and order read the real catalog's parts: the
/// examples of `planning/plan-54-data/examples.tsv` (every row present), one
/// library part for each drawer leaf the examples do not cover, and the whole
/// library.
@MainActor
final class CardSlotTests: XCTestCase {
    /// The drawer leaves of Connector › Card slot (plan-54-L § 3.2).
    static let leaves = ["SD", "microSD", "SIM", "Smart card", "Memory module", "Card edge"]

    /// One row of `planning/plan-54-data/examples.tsv`.
    struct Example {
        let mpn: String
        let family: String
        let positions: Int
        let drawer: [String]
        let footprint: String
    }

    /// The rows of `examples.tsv` as the repository has them now: the file
    /// grows (the card-edge and straddle-mount examples come later), so no
    /// test counts its rows.
    static func examples() throws -> [Example] {
        let url = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("planning/plan-54-data/examples.tsv")
        let text = try String(contentsOf: url, encoding: .utf8)
        let rows = text.split(separator: "\n").map { $0.split(separator: "\t", omittingEmptySubsequences: false).map(String.init) }
        let header = try XCTUnwrap(rows.first)
        XCTAssertEqual(header, ["mpn", "family", "positions", "drawer", "footprint"])
        return try rows.dropFirst().filter { !$0.isEmpty }.map { row in
            XCTAssertEqual(row.count, 5, "\(row)")
            return Example(mpn: row[0], family: row[1], positions: try XCTUnwrap(Int(row[2])),
                           drawer: row[3].split(separator: "/").map(String.init), footprint: row[4])
        }
    }

    /// Every card slot of the library, drawer-visible or not.
    static var cardSlots: [CatalogPart] { TestSupport.catalog.allParts.filter(\.isCardSlot) }

    /// The drawer leaf of a card slot (the third level), "" for another part.
    static func leaf(_ part: CatalogPart) -> String { part.drawer.count == 3 ? part.drawer[2] : "" }

    /// The parts to assert on: every example of `examples.tsv`, and for each
    /// drawer leaf with no example the library's first card slot in it (fewest
    /// pins, then by name), nil for a leaf the library has no part in.
    static func representatives() throws -> [(leaf: String, part: CatalogPart?, example: Example?)] {
        let examples = try examples()
        var found: [(leaf: String, part: CatalogPart?, example: Example?)] = try examples.map {
            ($0.drawer.last ?? "", try TestSupport.part($0.mpn), $0)
        }
        for leaf in leaves where !examples.contains(where: { $0.drawer.last == leaf }) {
            let part = cardSlots.filter { Self.leaf($0) == leaf }.min {
                $0.pins.count != $1.pins.count ? $0.pins.count < $1.pins.count : $0.name < $1.name
            }
            found.append((leaf, part, nil))
        }
        return found
    }

    private func everything() -> DrawerFilter {
        var filter = DrawerFilter()
        filter.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
        return filter
    }

    private func value(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    private func pitchText(_ pitchUM: UInt32) -> String { String(format: "%.2f mm", Double(pitchUM) / 1000) }

    /// What one card slot shows in the app: details, kind name, facts rows,
    /// chip labels, symbol, count and drawer leaf. `chip` is the part placed.
    private func checkShape(_ part: CatalogPart, _ chip: PlacedChip) throws {
        let name = part.name
        guard case let .cardSlot(card, pitchUM) = part.details else { return XCTFail("\(name): \(part.details)") }
        XCTAssertEqual(part.kind, PART_CONNECTOR, name)
        XCTAssertFalse(card.isEmpty, name)
        XCTAssertEqual(PartFacts.kindName(part), "Card slot", name)
        XCTAssertEqual(symbol(for: part), .block, name)
        XCTAssertEqual(chip.symbol, .block, name)
        XCTAssertEqual(ReferenceCategory.of(part), .femaleConnector, name)
        // The facts: Card, Pitch when above 0, Pins; none of a barrel jack's or a USB connector's.
        XCTAssertEqual(value(chip, "Kind"), "Card slot", name)
        XCTAssertEqual(value(chip, "Card"), card, name)
        XCTAssertEqual(value(chip, "Pitch"), pitchUM > 0 ? pitchText(pitchUM) : nil, name)
        XCTAssertEqual(value(chip, "Pins"), String(part.pins.count), name)
        for label in ["Type", "Centre pin", "Rating", "Sleeve contact", "Rated voltage", "Rated current"] {
            XCTAssertNil(value(chip, label), "\(name): \(label)")
        }
        // The chip's secondary labels: the card text, then the pitch when above 0.
        XCTAssertEqual(part.detailLabels, [card] + (pitchUM > 0 ? [pitchText(pitchUM)] : []), name)
        XCTAssertEqual(part.count, part.pins.count, name)
        XCTAssertEqual(part.trailingLabel, "(\(part.pins.count))", name)
        XCTAssertNil(part.voltageLabel, name)
        // The drawer path: Connector › Card slot › one of the six leaves; the function is the card text
        // (or "<pitch> mm <card>" for a card edge).
        XCTAssertEqual(Array(part.drawer.prefix(2)), ["Connector", "Card slot"], name)
        XCTAssertEqual(part.drawer.count, 3, name)
        XCTAssertTrue(Self.leaves.contains(Self.leaf(part)), "\(name): \(part.drawer)")
        if ["Memory module", "Card edge"].contains(Self.leaf(part)) {
            XCTAssertGreaterThan(pitchUM, 0, "\(name) is a module or card-edge socket")
        }
        if Self.leaf(part) == "Card edge" {
            XCTAssertTrue(part.function.hasSuffix(card), "\(name): \(part.function)")
        } else {
            XCTAssertEqual(part.function, card, name)
        }
        // The footprint has a pad for every pin.
        let footprint = try XCTUnwrap(part.footprint, name)
        XCTAssertGreaterThanOrEqual(footprint.pads.count, part.pins.count, name)
        let padNames = Set(footprint.pads.compactMap(\.designator))
        XCTAssertEqual(Set(part.pins.map(\.designator)).subtracting(padNames), [], "\(name): pins with no pad")
    }

    /// The examples of `examples.tsv`: each is a card slot with the facts of its
    /// family, listed where the file says, placed as `J1` on its footprint's pads.
    func testExamples() throws {
        let examples = try Self.examples()
        XCTAssertFalse(examples.isEmpty)
        for example in examples {
            let part = try TestSupport.part(example.mpn)
            let doc = TestSupport.newDocument()
            let chip = try TestSupport.place(doc, example.mpn, at: CGPoint(x: 200, y: 200))
            try checkShape(part, chip)
            XCTAssertEqual(part.drawer, example.drawer, example.mpn)
            XCTAssertEqual(part.footprint?.id, example.footprint, example.mpn)
            XCTAssertEqual(chip.reference, "J1", example.mpn)
            XCTAssertEqual(chip.footprint?.id, example.footprint, example.mpn)
            // The PCB pads are the footprint's, at its positions.
            let footprint = try XCTUnwrap(part.footprint)
            let placement = try XCTUnwrap(chip.board)
            XCTAssertEqual(chip.boardPads.count, footprint.pads.count, example.mpn)
            for (placed, pad) in zip(chip.boardPads, footprint.pads) {
                XCTAssertEqual(placed.designator, pad.designator, example.mpn)
                let at = placement.board(pad.centre)
                XCTAssertEqual(placed.rect.midX, at.x, accuracy: 1e-6, "\(example.mpn) pad \(pad.designator ?? "-")")
                XCTAssertEqual(placed.rect.midY, at.y, accuracy: 1e-6, "\(example.mpn) pad \(pad.designator ?? "-")")
            }
            // A family's positions fit its pins and pads.
            XCTAssertLessThanOrEqual(example.positions, footprint.pads.count, example.mpn)
            switch example.family {
            case "module", "edge":
                XCTAssertGreaterThan(try XCTUnwrap(value(chip, "Pitch")).count, 0, example.mpn)
            default:
                break
            }
        }
    }

    /// Every library part with a drawer leaf (the examples' leaves and each
    /// other leaf the library has a part in) has the shape of a card slot.
    /// A leaf with no part in the library yet is reported, not failed.
    func testEveryLeafHasARepresentative() throws {
        var checked = 0, missing: [String] = []
        let doc = TestSupport.newDocument()
        for (index, entry) in try Self.representatives().enumerated() {
            guard let part = entry.part else { missing.append(entry.leaf); continue }
            XCTAssertEqual(Self.leaf(part), entry.leaf)
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200 + 300 * index, y: 200))
            try checkShape(part, chip)
            checked += 1
        }
        print("CardSlotTests: leaves checked \(checked), none in the library yet: \(missing)")
        XCTAssertGreaterThan(checked, 0, "the library has no card slot at all")
        if !missing.isEmpty {
            throw XCTSkip("no library part in the drawer leaf(s) \(missing.joined(separator: ", ")) yet; the other \(checked) were checked")
        }
    }

    /// Every card slot of the library: details, facts, labels, symbol, drawer
    /// path, a footprint with a pad for every pin, a block symbol with every
    /// pin and two columns above 40 pins.
    func testEveryCardSlotPart() throws {
        let parts = Self.cardSlots
        XCTAssertFalse(parts.isEmpty)
        let doc = TestSupport.newDocument()
        for (index, part) in parts.enumerated() {
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200 + 40 * (index % 20), y: 200 + 40 * (index / 20)))
            try checkShape(part, chip)
            let layout = ChipLayout(part: part, symbol: .block)
            XCTAssertEqual(layout.slots.count, part.pins.count, part.name)
            if part.pins.count > 40 {
                let left = layout.slots.filter { $0.side == .left }.count
                let right = layout.slots.filter { $0.side == .right }.count
                XCTAssertGreaterThan(left, 0, "\(part.name) has \(part.pins.count) pins in one column")
                XCTAssertGreaterThan(right, 0, "\(part.name) has \(part.pins.count) pins in one column")
            }
        }
        // Every part with a drawer path under Card slot is one.
        let underCardSlot = TestSupport.catalog.allParts.filter { Array($0.drawer.prefix(2)) == ["Connector", "Card slot"] }
        XCTAssertEqual(Set(underCardSlot.map(\.name)), Set(parts.map(\.name)))
    }

    /// The drawer: Connector › Card slot has a leaf for each drawer leaf the
    /// library has parts in (and no other), each leaf's function groups hold
    /// the parts of that function, and a leaf's parts are in pin-count order,
    /// then by name. A leaf whose parts all use its own name as the function
    /// (SD, microSD, Smart card) lists them directly: no repeated title.
    func testDrawerBranch() throws {
        let groups = TestSupport.catalog.groups(matching: everything())
        let connector = try XCTUnwrap(groups.first { $0.category == "Connector" })
        let slot = try XCTUnwrap(connector.subcategories.first { $0.subcategory == "Card slot" })
        XCTAssertEqual(slot.path, ["Connector", "Card slot"])
        XCTAssertTrue(slot.functions.isEmpty, "every card slot has a leaf")
        let inLibrary = Set(Self.cardSlots.map(Self.leaf))
        XCTAssertEqual(Set(slot.subgroups.map(\.subcategory)), inLibrary)
        XCTAssertTrue(inLibrary.isSubset(of: Self.leaves), "\(inLibrary)")
        XCTAssertEqual(slot.allParts.count, Self.cardSlots.count)
        for leaf in slot.subgroups {
            XCTAssertEqual(leaf.path.prefix(2), ["Connector", "Card slot"])
            XCTAssertFalse(leaf.functions.contains { $0.function.caseInsensitiveCompare(leaf.subcategory) == .orderedSame }
                           && leaf.functions.count == 1, "\(leaf.subcategory) repeats its own name")
            // Direct parts sort like a function group's: pin count, then name.
            let direct = [(function: leaf.subcategory, parts: leaf.parts)].filter { !$0.parts.isEmpty }
            XCTAssertTrue(direct.isEmpty || leaf.functions.isEmpty, "\(leaf.subcategory) lists parts both ways")
            for group in leaf.functions.map({ (function: $0.function, parts: $0.parts) }) + direct {
                let parts = group.parts
                XCTAssertFalse(parts.isEmpty)
                XCTAssertTrue(parts.allSatisfy { $0.function == group.function && $0.drawer == leaf.path }, group.function)
                let counts = parts.map(\.pins.count)
                XCTAssertEqual(counts, counts.sorted(), "\(leaf.subcategory) › \(group.function)")
                for (a, b) in zip(parts, parts.dropFirst()) where a.pins.count == b.pins.count {
                    XCTAssertNotEqual(a.name.localizedStandardCompare(b.name), .orderedDescending,
                                      "\(a.name) before \(b.name)")
                }
            }
            // The leaves of the groups sort in natural order ("0.50 mm" before "1.00 mm").
            let names = leaf.functions.map(\.function)
            XCTAssertEqual(names, names.sorted { $0.localizedStandardCompare($1) == .orderedAscending }, leaf.subcategory)
        }
        // Each example is in the leaf and function group its row says.
        for example in try Self.examples() {
            let part = try TestSupport.part(example.mpn)
            let leaf = try XCTUnwrap(slot.subgroups.first { $0.subcategory == example.drawer.last }, example.mpn)
            let listed = leaf.functions.first { $0.function == part.function }?.parts ?? leaf.parts
            XCTAssertTrue(listed.contains(part), example.mpn)
            XCTAssertEqual(example.drawer.prefix(2), ["Connector", "Card slot"])
        }
    }

    /// The microSD example is listed at Connector › Card slot › microSD › microSD
    /// when `examples.tsv` has a microSD row.
    func testMicroSDPath() throws {
        let example = try XCTUnwrap(Self.examples().first { $0.drawer.last == "microSD" })
        let part = try TestSupport.part(example.mpn)
        XCTAssertEqual(part.drawer + [part.function], ["Connector", "Card slot", "microSD", "microSD"])
    }

    /// A barrel jack and a USB connector keep their details, facts, symbol and
    /// kind name.
    func testOtherConnectorsAreUnchanged() throws {
        let doc = TestSupport.newDocument()
        let barrel = try XCTUnwrap(TestSupport.catalog.allParts.first {
            if case .connector(_, _, _, _, nil) = $0.details { return true }
            return false
        })
        let usb = try XCTUnwrap(TestSupport.catalog.allParts.first {
            if case .connector(_, _, _, _, _?) = $0.details { return true }
            return false
        })
        XCTAssertFalse(barrel.isCardSlot)
        XCTAssertFalse(usb.isCardSlot)
        XCTAssertEqual(PartFacts.kindName(barrel), "Barrel connector")
        XCTAssertEqual(PartFacts.kindName(usb), "USB connector")
        XCTAssertEqual(symbol(for: barrel), .barrelJack)
        XCTAssertEqual(symbol(for: usb), .block)
        let barrelChip = try TestSupport.place(doc, barrel.name, at: CGPoint(x: 100, y: 100))
        let usbChip = try TestSupport.place(doc, usb.name, at: CGPoint(x: 400, y: 100))
        XCTAssertNotNil(value(barrelChip, "Centre pin"))
        XCTAssertNil(value(barrelChip, "Card"))
        XCTAssertNil(value(barrelChip, "Pitch"))
        XCTAssertNotNil(value(usbChip, "Type"))
        XCTAssertNil(value(usbChip, "Card"))
        XCTAssertEqual(barrel.count, nil)
        XCTAssertEqual(usb.count, nil)
        // Neither is a card slot anywhere in the library.
        for part in TestSupport.catalog.allParts where part.kind == PART_CONNECTOR && !part.isCardSlot {
            XCTAssertNotEqual(part.drawer.dropFirst().first, "Card slot", part.name)
        }
    }
}
