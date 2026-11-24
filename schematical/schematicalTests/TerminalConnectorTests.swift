import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-45-H: PCB terminal blocks and JST board headers (the `wireConnector`
/// details of a `connector` part with `terminal` or `jst`). The kind name,
/// facts rows, chip labels, symbol, drawer branches (Connector › Terminal
/// block › <pitch> mm and Connector › JST › <series>) and the position order
/// of a leaf read the real catalog's parts.
@MainActor
final class TerminalConnectorTests: XCTestCase {
    static let pitches = ["3.50 mm", "3.81 mm", "5.00 mm", "5.08 mm", "7.50 mm", "7.62 mm"]

    /// Every terminal block and JST header of the library.
    static var wireConnectors: [CatalogPart] { TestSupport.catalog.allParts.filter(\.isWireConnector) }

    private func everything() -> DrawerFilter {
        var filter = DrawerFilter()
        filter.visibility = PartVisibility(showObsolete: true, showMilitary: true, showAutomotive: true)
        return filter
    }

    private func value(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    private func pitchText(_ pitchUM: UInt32) -> String { String(format: "%.2f mm", Double(pitchUM) / 1000) }

    /// A rated part of one maker with a given kind, fewest pins then name.
    private func rated(_ maker: String, kind: String) throws -> CatalogPart {
        try XCTUnwrap(Self.wireConnectors.filter {
            guard case let .wireConnector(k, _, _, mv, ma) = $0.details else { return false }
            return $0.manufacturer == maker && k == kind && mv > 0 && ma > 0
        }.min {
            $0.pins.count != $1.pins.count ? $0.pins.count < $1.pins.count : $0.name < $1.name
        }, "no rated \(kind) part of \(maker)")
    }

    /// What one terminal block or JST header shows in the app.
    private func checkShape(_ part: CatalogPart, _ chip: PlacedChip) throws {
        let name = part.name
        guard case let .wireConnector(kind, variant, pitchUM, ratedMV, ratedMA) = part.details else {
            return XCTFail("\(name): \(part.details)")
        }
        let jst = kind == "jst"
        XCTAssertTrue(["terminal", "jst"].contains(kind), name)
        XCTAssertEqual(part.kind, PART_CONNECTOR, name)
        XCTAssertGreaterThan(pitchUM, 0, name)
        XCTAssertEqual(PartFacts.kindName(part), jst ? "JST connector" : "Terminal block", name)
        XCTAssertEqual(symbol(for: part), .block, name)
        XCTAssertEqual(chip.symbol, .block, name)
        XCTAssertEqual(ReferenceCategory.of(part), .femaleConnector, name)
        XCTAssertTrue(chip.reference.hasPrefix("J"), name)
        // The facts rows.
        XCTAssertEqual(value(chip, "Kind"), jst ? "JST connector" : "Terminal block", name)
        XCTAssertEqual(value(chip, "Type"), jst ? "JST header" : "Terminal block", name)
        XCTAssertEqual(value(chip, "Pitch"), pitchText(pitchUM), name)
        XCTAssertEqual(value(chip, jst ? "Series" : "Clamp"),
                       jst ? variant : ["screw": "Screw", "spring": "Spring-cage", "push-in": "Push-in",
                                        "pluggable": "Plug-in header"][variant], name)
        XCTAssertNil(value(chip, jst ? "Clamp" : "Series"), name)
        XCTAssertEqual(value(chip, "Positions"), String(part.pins.count), name)
        XCTAssertEqual(value(chip, "Rated voltage"), ratedMV > 0 ? "\(VoltageCheck.volts(ratedMV)) V" : nil, name)
        XCTAssertEqual(value(chip, "Rated current"), ratedMA > 0 ? String(format: "%g A", Double(ratedMA) / 1000) : nil, name)
        for label in ["Centre pin", "Rating", "Sleeve contact", "Card"] {
            XCTAssertNil(value(chip, label), "\(name): \(label)")
        }
        // The chip's secondary labels.
        let current = ratedMA > 0 ? [String(format: "%g A", Double(ratedMA) / 1000)] : []
        XCTAssertEqual(part.detailLabels, jst ? [variant, pitchText(pitchUM)]
                       : [pitchText(pitchUM), PartFacts.clampName(variant)] + current, name)
        XCTAssertEqual(part.voltageLabel, ratedMV > 0 ? "\(VoltageCheck.volts(ratedMV)) V max" : nil, name)
        XCTAssertEqual(part.count, part.pins.count, name)
        XCTAssertEqual(part.trailingLabel, "(\(part.pins.count))", name)
        // The drawer path.
        XCTAssertEqual(part.drawer.first, "Connector", name)
        XCTAssertEqual(part.drawer.dropFirst().first, jst ? "JST" : "Terminal block", name)
        XCTAssertEqual(part.drawer.count, 3, name)
        if !jst { XCTAssertTrue(Self.pitches.contains(part.drawer[2]), "\(name): \(part.drawer)") }
        // The footprint has a pad for every pin.
        let footprint = try XCTUnwrap(part.footprint, name)
        let padNames = Set(footprint.pads.compactMap(\.designator))
        XCTAssertEqual(Set(part.pins.map(\.designator)).subtracting(padNames), [], "\(name): pins with no pad")
    }

    /// The four examples of the plan: details, facts, labels, symbol, drawer
    /// path and the pads of a placed part.
    func testExamples() throws {
        let generic = try TestSupport.part("GENERIC-TERMINAL-5.08-03")
        let xh = try TestSupport.part("GENERIC-JST-XH-04")
        let phoenix = try rated("Phoenix Contact", kind: "terminal")
        let wurth = try rated("Würth Elektronik", kind: "terminal")
        let doc = TestSupport.newDocument()
        for (index, part) in [generic, xh, phoenix, wurth].enumerated() {
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200 + 300 * index, y: 200))
            XCTAssertEqual(chip.reference, "J\(index + 1)", part.name)
            try checkShape(part, chip)
        }
        XCTAssertEqual(generic.drawer + [generic.function], ["Connector", "Terminal block", "5.08 mm", "Generic"])
        XCTAssertEqual(xh.drawer + [xh.function], ["Connector", "JST", "XH", "Generic"])
        XCTAssertEqual(generic.pins.count, 3)
        XCTAssertEqual(xh.pins.count, 4)
        // The generics have no rating; the maker's parts have one.
        XCTAssertNil(generic.voltageLabel)
        XCTAssertEqual(generic.detailLabels, ["5.08 mm", "Screw"])
        XCTAssertEqual(xh.detailLabels, ["XH", "2.50 mm"])
        XCTAssertNotNil(phoenix.voltageLabel)
        // The generic's pads are 5.08 mm apart.
        let doc2 = TestSupport.newDocument()
        let chip = try TestSupport.place(doc2, generic.name, at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(chip.boardPads.count, 3)
        let xs = chip.boardPads.map(\.rect.midX).sorted()
        XCTAssertEqual(xs[1] - xs[0], 5.08, accuracy: 1e-6)
        XCTAssertEqual(xs[2] - xs[1], 5.08, accuracy: 1e-6)
    }

    /// Every terminal block and JST header of the library has the shape.
    func testEveryWireConnectorPart() throws {
        let parts = Self.wireConnectors
        XCTAssertGreaterThanOrEqual(parts.count, 589)
        for (index, part) in parts.enumerated() {
            let doc = TestSupport.newDocument()
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200 + 40 * (index % 20), y: 200))
            try checkShape(part, chip)
            XCTAssertEqual(ChipLayout(part: part, symbol: .block).slots.count, part.pins.count, part.name)
        }
        // Every part with a drawer path under Terminal block or JST is one.
        let under = TestSupport.catalog.allParts.filter { ["Terminal block", "JST"].contains($0.drawer.dropFirst().first ?? "") }
        XCTAssertEqual(Set(under.map(\.name)), Set(parts.map(\.name)))
    }

    /// The drawer: Connector › Terminal block has the six pitches in numeric
    /// order; Connector › JST has the series; a leaf's parts are in position
    /// order, then by name.
    func testDrawerBranches() throws {
        let groups = TestSupport.catalog.groups(matching: everything())
        let connector = try XCTUnwrap(groups.first { $0.category == "Connector" })
        let block = try XCTUnwrap(connector.subcategories.first { $0.subcategory == "Terminal block" })
        XCTAssertEqual(block.path, ["Connector", "Terminal block"])
        XCTAssertEqual(block.subgroups.map(\.subcategory), Self.pitches)
        let jst = try XCTUnwrap(connector.subcategories.first { $0.subcategory == "JST" })
        XCTAssertEqual(jst.path, ["Connector", "JST"])
        XCTAssertEqual(Set(jst.subgroups.map(\.subcategory)), ["EH", "GH", "PH", "SH-SR", "VH", "XH", "ZH"])
        for leaf in block.subgroups + jst.subgroups {
            XCTAssertTrue(leaf.functions.isEmpty || leaf.parts.isEmpty, "\(leaf.subcategory) lists parts both ways")
            for group in leaf.functions {
                let parts = group.parts
                XCTAssertFalse(parts.isEmpty)
                XCTAssertTrue(parts.allSatisfy { $0.function == group.function && $0.drawer == leaf.path }, group.function)
                let counts = parts.map(\.pins.count)
                XCTAssertEqual(counts, counts.sorted(), "\(leaf.subcategory) › \(group.function)")
                for (a, b) in zip(parts, parts.dropFirst()) where a.pins.count == b.pins.count {
                    XCTAssertNotEqual(a.name.localizedStandardCompare(b.name), .orderedDescending, "\(a.name) before \(b.name)")
                }
            }
        }
        // The generic's leaf lists 02, 03 … 12 in order.
        let leaf = try XCTUnwrap(block.subgroups.first { $0.subcategory == "5.08 mm" })
        let generics = try XCTUnwrap(leaf.functions.first { $0.function == "Generic" }).parts
        XCTAssertEqual(generics.map(\.name), (2...12).map { String(format: "GENERIC-TERMINAL-5.08-%02d", $0) })
        XCTAssertEqual(generics.map(\.pins.count), Array(2...12))
    }

    /// A barrel jack, a USB connector and a card slot keep their details,
    /// facts, symbol and kind name.
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
        XCTAssertFalse(barrel.isWireConnector)
        XCTAssertFalse(usb.isWireConnector)
        XCTAssertEqual(PartFacts.kindName(barrel), "Barrel connector")
        XCTAssertEqual(PartFacts.kindName(usb), "USB connector")
        XCTAssertEqual(symbol(for: barrel), .barrelJack)
        XCTAssertEqual(symbol(for: usb), .block)
        let barrelChip = try TestSupport.place(doc, barrel.name, at: CGPoint(x: 100, y: 100))
        let usbChip = try TestSupport.place(doc, usb.name, at: CGPoint(x: 400, y: 100))
        XCTAssertNotNil(value(barrelChip, "Centre pin"))
        XCTAssertNil(value(barrelChip, "Clamp"))
        XCTAssertNil(value(barrelChip, "Positions"))
        XCTAssertNotNil(value(usbChip, "Type"))
        XCTAssertNil(value(usbChip, "Pitch"))
        XCTAssertNil(barrel.count)
        XCTAssertNil(usb.count)
        for part in TestSupport.catalog.allParts where part.kind == PART_CONNECTOR && !part.isWireConnector {
            XCTAssertNotEqual(part.drawer.dropFirst().first, "Terminal block", part.name)
            XCTAssertNotEqual(part.drawer.dropFirst().first, "JST", part.name)
        }
    }
}
