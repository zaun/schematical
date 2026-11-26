import AppKit
import CoreGraphics
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AC: the Schematic sidebar's Options section as it is shown, per
/// kind of part, and the socket picker sheet it opens.
@MainActor
final class PartOptionsSectionTests: XCTestCase {
    private var catalog: PartCatalogStore { TestSupport.catalog }
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        for probe in probes {
            if let sheet = probe.window.attachedSheet { probe.window.endSheet(sheet) }
            probe.close()
        }
        probes = []
    }

    /// The section for `name`, placed on a new document, in a grouped form.
    private func show(_ name: String, _ setUp: (SchematicDocument, PlacedChip) -> Void = { _, _ in }) throws
        -> (ViewProbe, SchematicDocument, PlacedChip) {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, name, at: CGPoint(x: 100, y: 100))
        setUp(doc, chip)
        let probe = ViewProbe(Form { PartOptionsSection(catalog: catalog, document: doc, chip: chip) }
            .formStyle(.grouped), size: NSSize(width: 420, height: 640))
        probes.append(probe)
        return (probe, doc, chip)
    }

    private func now(_ doc: SchematicDocument, _ chip: PlacedChip) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == chip.id })
    }

    func testAGateShowsTheViewAndMountingChoices() throws {
        let (probe, doc, chip) = try show("SN74HC00N")
        XCTAssertTrue(probe.has("Options"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Block"), "\(probe.labels())")
        XCTAssertTrue(probe.has("IC + Socket"), "\(probe.labels())")
        XCTAssertFalse(probe.has("Choose Socket"), "no socket yet")
        try probe.pressControl("Gate")
        XCTAssertTrue(try now(doc, chip).isSplit)
        try probe.pressControl("IC + Socket")
        XCTAssertEqual(try now(doc, chip).socket?.mode, .withSocket)
        XCTAssertTrue(probe.has("GENERIC-SOCKET-DIP-14-300"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Choose Socket…", role: "AXButton"), "\(probe.labels())")
    }

    func testAHeaderShowsItsSizeAndGender() throws {
        let (probe, doc, chip) = try show("GENERIC-HEADER-1X08")
        XCTAssertTrue(probe.has("Pins per row"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Female"), "\(probe.labels())")
        try probe.pressControl("Female")
        XCTAssertEqual(try now(doc, chip).gender, "Female")
        try probe.pressControl("2")
        XCTAssertEqual(try now(doc, chip).part.name, "GENERIC-HEADER-2X08")
    }

    func testAnLEDShowsItsColorAndFootprint() throws {
        let (probe, _, chip) = try show("GENERIC-LED")
        XCTAssertTrue(probe.has("Color"), "\(probe.labels())")
        XCTAssertTrue(probe.has(chip.part.colors[0]), "\(probe.labels())")
        XCTAssertTrue(probe.has("Footprint"), "\(probe.labels())")
        XCTAssertTrue(probe.has(try XCTUnwrap(chip.part.footprint?.description)), "\(probe.labels())")
        let swatch = ColorChoicePicker.swatch(.red)
        XCTAssertFalse(swatch.isTemplate)
        XCTAssertEqual(swatch.size, CGSize(width: 12, height: 12))
    }

    func testAResistorAndACapacitorShowTheirValue() throws {
        let (resistor, rDoc, rChip) = try show("GENERIC-RESISTOR") { doc, chip in doc.setPartOhms(chip.id, 4700) }
        XCTAssertTrue(resistor.has("Ohms"), "\(resistor.labels())")
        XCTAssertTrue(resistor.has("4.7"), "\(resistor.labels())")
        XCTAssertTrue(resistor.has("kΩ"), "\(resistor.labels())")
        // A change from outside shows at once.
        rDoc.setPartOhms(rChip.id, 2_200_000)
        resistor.settle()
        XCTAssertTrue(resistor.has("2.2"), "\(resistor.labels())")
        XCTAssertTrue(resistor.has("MΩ"), "\(resistor.labels())")

        let (capacitor, _, _) = try show("GENERIC-CAPACITOR")
        XCTAssertTrue(capacitor.has("Capacitance"), "\(capacitor.labels())")
        XCTAssertTrue(capacitor.has("nF") || capacitor.has("µF") || capacitor.has("pF"), "\(capacitor.labels())")
    }

    func testATestPointShowsSizeAndThroughHole() throws {
        let (probe, doc, chip) = try show("GENERIC-TESTPOINT")
        XCTAssertTrue(probe.has("Size"), "\(probe.labels())")
        let before = try XCTUnwrap(PartOptionsModel(catalog: catalog, document: doc, chip: chip).testPoint)
        try probe.pressControl("Through hole")
        XCTAssertEqual(PartOptionsModel(catalog: catalog, document: doc, chip: chip).testPoint?.throughHole,
                       !before.throughHole)
    }

    func testChooseSocketOpensThePickerAndUseSocketSwitchesIt() throws {
        let (probe, doc, chip) = try show("SN74HC00N") { doc, chip in
            doc.setPartSocket(chip.id, mode: .socketOnly, socket: try? TestSupport.part("GENERIC-SOCKET-DIP-14-300"))
        }
        XCTAssertTrue(probe.has("GENERIC-SOCKET-DIP-14-300"), "\(probe.labels())")
        XCTAssertTrue(probe.has(PartOptionsSection.socketCaption(try TestSupport.part("GENERIC-SOCKET-DIP-14-300"))))
        try probe.press("Choose Socket…")
        let sheet = ViewProbe(window: try probe.sheet())
        XCTAssertTrue(sheet.has("Sockets for"), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Use Socket", role: "AXButton"), "\(sheet.labels())")

        // Pick another socket in the table, then Use Socket.
        let table = try XCTUnwrap(sheet.views(NSTableView.self).first)
        let all = catalog.sockets(fitting: try XCTUnwrap(chip.part.footprint))
        let other = try XCTUnwrap(all.firstIndex { $0.manufacturer != "Generic" })
        table.selectRowIndexes([other], byExtendingSelection: false)
        sheet.settle()
        try sheet.press("Use Socket")
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil }, "the sheet closes")
        XCTAssertEqual(try now(doc, chip).socket, ChipSocket(part: all[other], mode: .socketOnly))
    }

    func testADoubleClickUsesTheSocket() throws {
        let (probe, doc, chip) = try show("SN74HC00N") { doc, chip in
            doc.setPartSocket(chip.id, mode: .withSocket, socket: try? TestSupport.part("GENERIC-SOCKET-DIP-14-300"))
        }
        try probe.press("Choose Socket…")
        let sheet = ViewProbe(window: try probe.sheet())
        let table = try XCTUnwrap(sheet.views(NSTableView.self).first)
        let all = catalog.sockets(fitting: try XCTUnwrap(chip.part.footprint))
        let other = try XCTUnwrap(all.firstIndex { $0.manufacturer != "Generic" })
        try sheet.doubleClick(table, row: other)
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil }, "the sheet closes")
        XCTAssertEqual(try now(doc, chip).socket, ChipSocket(part: all[other], mode: .withSocket))
    }

    func testCancelClosesThePickerAndChangesNothing() throws {
        let socket = try TestSupport.part("GENERIC-SOCKET-DIP-14-300")
        let (probe, doc, chip) = try show("SN74HC00N") { doc, chip in
            doc.setPartSocket(chip.id, mode: .withSocket, socket: socket)
        }
        try probe.press("Choose Socket…")
        let sheet = ViewProbe(window: try probe.sheet())
        XCTAssertTrue(sheet.has("of \(catalog.sockets(fitting: try XCTUnwrap(chip.part.footprint)).count) sockets"),
                      "\(sheet.labels())")
        try sheet.press("Cancel")
        XCTAssertTrue(probe.wait { probe.window.attachedSheet == nil }, "the sheet closes")
        XCTAssertEqual(try now(doc, chip).socket, ChipSocket(part: socket, mode: .withSocket))
    }
}
