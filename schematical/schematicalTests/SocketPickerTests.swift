import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-51-I: the Mounting control appears only for a chip a socket fits, and
/// the compatible-sockets dialog's filter is a pure function of the list.
@MainActor
final class SocketPickerTests: XCTestCase {
    private var catalog: PartCatalogStore { TestSupport.catalog }

    private func gateFootprint() throws -> Footprint {
        try XCTUnwrap(try TestSupport.part("SN74HC00N").footprint)
    }

    private func sockets() throws -> [CatalogPart] { catalog.sockets(fitting: try gateFootprint()) }

    func testTheControlAppliesToAChipASocketFits() throws {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(PartOptionsSection.isSocketable(gate.part, in: catalog))
        XCTAssertTrue(PartOptionsSection.applies(to: gate, in: catalog))
        let plcc = try TestSupport.part("W65C02S6TPLG-14")
        XCTAssertTrue(PartOptionsSection.isSocketable(plcc, in: catalog))
    }

    func testTheControlIsAbsentForChipsNoSocketFits() throws {
        for name in ["GENERIC-LED", "GENERIC-HEADER-1X02", "5V"] {
            XCTAssertFalse(PartOptionsSection.isSocketable(try TestSupport.part(name), in: catalog), name)
        }
        let display = try XCTUnwrap(TestSupport.allParts.first { $0.kind == PART_7SEG })
        XCTAssertFalse(PartOptionsSection.isSocketable(display, in: catalog), display.name)
        let oscillator = try XCTUnwrap(TestSupport.allParts.first { $0.kind == PART_OSCILLATOR && $0.pins.count == 4 })
        XCTAssertFalse(PartOptionsSection.isSocketable(oscillator, in: catalog), oscillator.name)
        // A socket itself is not socketable: no socket holds a socket's footprint.
        let socket = try TestSupport.part("GENERIC-SOCKET-DIP-14-300")
        XCTAssertFalse(PartOptionsSection.isSocketable(socket, in: catalog))
        // The LED and the header still show their own options.
        let led = try TestSupport.place(TestSupport.newDocument(), "GENERIC-LED", at: .zero)
        XCTAssertTrue(PartOptionsSection.applies(to: led, in: catalog))
    }

    func testTheMountingChoicesMapToTheModes() {
        XCTAssertEqual(PartOptionsSection.MountingChoice.allCases.map(\.title), ["IC", "IC + Socket", "Socket Only"])
        XCTAssertNil(PartOptionsSection.MountingChoice.ic.mode)
        XCTAssertEqual(PartOptionsSection.MountingChoice.withSocket.mode, .withSocket)
        XCTAssertEqual(PartOptionsSection.MountingChoice.socketOnly.mode, .socketOnly)
        XCTAssertEqual(PartOptionsSection.MountingChoice(nil), .ic)
    }

    func testChoosingAMountingGoesThroughTheDocument() throws {
        let doc = TestSupport.newDocument()
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 100, y: 100))
        PartOptionsSection.choose(.withSocket, for: gate, in: doc, catalog: catalog)
        var now = try XCTUnwrap(doc.chips.first)
        XCTAssertEqual(now.socket?.part.name, "GENERIC-SOCKET-DIP-14-300")
        XCTAssertEqual(PartOptionsSection.MountingChoice(now.socket), .withSocket)
        // Between the socket modes the chip keeps its socket.
        let maker = try TestSupport.part("A 14-LC-TR")
        doc.setPartSocket(gate.id, mode: .withSocket, socket: maker)
        now = try XCTUnwrap(doc.chips.first)
        PartOptionsSection.choose(.socketOnly, for: now, in: doc, catalog: catalog)
        XCTAssertEqual(doc.chips.first?.socket, ChipSocket(part: maker, mode: .socketOnly))
        PartOptionsSection.choose(.ic, for: try XCTUnwrap(doc.chips.first), in: doc, catalog: catalog)
        XCTAssertNil(doc.chips.first?.socket)
    }

    func testTheRowsFilterByTextMountingAndMaker() throws {
        let all = try sockets()
        let rows = SocketPickerSheet.rows
        XCTAssertEqual(rows(all, "", .all, nil), all)
        XCTAssertEqual(all.first?.name, "GENERIC-SOCKET-DIP-14-300", "generics first")

        let mill = rows(all, "mill", .all, nil)
        XCTAssertFalse(mill.isEmpty)
        XCTAssertLessThan(mill.count, all.count)
        XCTAssertTrue(mill.allSatisfy { $0.manufacturer.localizedCaseInsensitiveContains("mill") })
        XCTAssertEqual(rows(all, "  MILL ", .all, nil).count, mill.count, "case and edge spaces do not matter")

        let surface = rows(all, "", .surfaceMount, nil)
        XCTAssertFalse(surface.isEmpty)
        XCTAssertTrue(surface.allSatisfy { SocketPickerSheet.mountingText($0) == "surface-mount" })
        let through = rows(all, "", .throughHole, nil)
        XCTAssertEqual(surface.count + through.count, all.count)

        let makers = SocketPickerSheet.makers(all)
        XCTAssertEqual(makers.first, "Generic")
        XCTAssertTrue(makers.contains("Preci-Dip"))
        let precidip = rows(all, "", .all, "Preci-Dip")
        XCTAssertFalse(precidip.isEmpty)
        XCTAssertTrue(precidip.allSatisfy { $0.manufacturer == "Preci-Dip" })

        let together = rows(all, "110", .surfaceMount, "Preci-Dip")
        XCTAssertTrue(together.allSatisfy {
            $0.manufacturer == "Preci-Dip" && SocketPickerSheet.mountingText($0) == "surface-mount" && $0.name.contains("110")
        })
        XCTAssertFalse(together.isEmpty)
        XCTAssertTrue(rows(all, "no-such-socket", .all, nil).isEmpty)
        // The description is searched too.
        XCTAssertFalse(rows(all, "14-pin 0.3 in DIP socket", .all, nil).isEmpty)
    }

    func testEveryRowHoldsTheChipsFootprintAndTheTextsAreRight() throws {
        let all = try sockets()
        XCTAssertGreaterThan(all.count, 50)
        for socket in all {
            XCTAssertTrue(socket.fits.contains { $0.id == "PDIP-14" }, socket.name)
            XCTAssertEqual(socket.pins.count, 14, socket.name)
            XCTAssertEqual(SocketPickerSheet.rowSpacingText(socket).hasSuffix(" in"), true, socket.name)
        }
        let generic = try XCTUnwrap(all.first)
        XCTAssertEqual(SocketPickerSheet.rowSpacingText(generic), "0.3 in")
        XCTAssertEqual(SocketPickerSheet.mountingText(generic), "through-hole")
        XCTAssertEqual(SocketPickerSheet.finishText(generic), "")
        XCTAssertEqual(PartOptionsSection.socketCaption(generic), "DIP socket, 0.3 in, through hole")
        let tin = try TestSupport.part("A 14-LC-TR")
        XCTAssertEqual(PartOptionsSection.socketCaption(tin), "Assmann WSW Components · DIP socket, 0.3 in, through hole, tin")
        // A PLCC socket has no row spacing.
        let plcc = try XCTUnwrap(catalog.genericSocket(fitting: try XCTUnwrap(try TestSupport.part("W65C02S6TPLG-14").footprint)))
        XCTAssertEqual(SocketPickerSheet.rowSpacingText(plcc), "")
        XCTAssertEqual(PartOptionsSection.socketCaption(plcc), "PLCC socket, surface mount")
    }

    /// plan-68-AC: a part that is not a socket has no socket texts.
    func testANonSocketHasNoSocketTexts() throws {
        let led = try TestSupport.part("GENERIC-LED")
        XCTAssertEqual(SocketPickerSheet.mountingText(led), "")
        XCTAssertEqual(SocketPickerSheet.finishText(led), "")
        XCTAssertEqual(SocketPickerSheet.rowSpacingText(led), "")
        XCTAssertNil(SocketPickerSheet.socket(named: nil, in: [led]))
        XCTAssertEqual(SocketPickerSheet.socket(named: "GENERIC-LED", in: [led]), led)
    }
}
