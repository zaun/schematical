import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-51-F: the IC sockets are catalog parts, listed under Connector › IC
/// Socket. A socket is a passive block of pins with the footprint of its
/// series; its sidebar shows Family, Pins, Row spacing, Mounting and Finish,
/// its reference is `XS<n>`, and Run still works with one on the sheet.
@MainActor
final class SocketPartsTests: XCTestCase {
    /// Kept for the life of the test process (see PlaySwitchResetTests).
    private static var keptSimulations: [SimulationController] = []

    override func setUp() {
        super.setUp()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    private func value(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    private func pads(_ footprint: Footprint?) -> Int {
        footprint?.pads.filter { $0.designator != nil }.count ?? 0
    }

    func testTheCatalogHasTheFifteenGenericSockets() {
        let generics = TestSupport.allParts.filter { $0.kind == PART_SOCKET && $0.manufacturer == "Generic" }
        XCTAssertEqual(generics.count, 15)
        XCTAssertEqual(generics.filter { $0.drawer == ["Connector", "IC Socket", "DIP"] }.count, 11)
        XCTAssertEqual(generics.filter { $0.drawer == ["Connector", "IC Socket", "PLCC"] }.count, 4)
        XCTAssertTrue(generics.allSatisfy { $0.datasheetURL == nil && $0.fits.count >= 1 })
    }

    func testEverySocketIsListedUnderIcSocketWithAFootprintOfItsPins() {
        let sockets = TestSupport.allParts.filter { $0.kind == PART_SOCKET }
        XCTAssertGreaterThanOrEqual(sockets.count, 575)
        for socket in sockets {
            XCTAssertEqual(Array(socket.drawer.prefix(2)), ["Connector", "IC Socket"], socket.name)
            XCTAssertEqual(pads(socket.footprint), socket.pins.count, socket.name)
            XCTAssertFalse(socket.fits.isEmpty, socket.name)
            XCTAssertTrue(socket.fits.allSatisfy { pads($0) == socket.pins.count }, socket.name)
            XCTAssertTrue(socket.isSimulated, socket.name)
        }
    }

    func testTheGenericFourteenPinDipSocket() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-SOCKET-DIP-14-300", at: CGPoint(x: 100, y: 100))
        let part = chip.part
        XCTAssertEqual(part.kind, PART_SOCKET)
        XCTAssertEqual(part.pins.count, 14)
        XCTAssertEqual(pads(part.footprint), 14)
        XCTAssertEqual(part.footprint?.id, "SKT-DIP14-300-TH-MM-110")
        XCTAssertEqual(part.fits.map(\.id), ["PDIP-14", "CDIP-14"])
        guard case let .socket(family, rowUM, mounting, finish) = part.details else {
            return XCTFail("not a socket: \(part.details)")
        }
        XCTAssertEqual(family, "DIP")
        XCTAssertEqual(rowUM, 7620)
        XCTAssertEqual(mounting, "through-hole")
        XCTAssertNil(finish)
        XCTAssertEqual(part.drawer, ["Connector", "IC Socket", "DIP"])
        XCTAssertEqual(part.package, "DIP socket")

        XCTAssertEqual(chip.reference, "XS1")
        XCTAssertEqual(ReferenceCategory.of(part), .socket)
        XCTAssertEqual(ReferenceCategory.socket.standardPrefix, "XS")
        XCTAssertEqual(PartFacts.kindName(PART_SOCKET), "IC socket")
        XCTAssertEqual(PartKinds.name(of: PART_SOCKET), "socket")
        XCTAssertEqual(value(chip, "Kind"), "IC socket")
        XCTAssertEqual(value(chip, "Family"), "DIP")
        XCTAssertEqual(value(chip, "Pins"), "14")
        XCTAssertEqual(value(chip, "Row spacing"), "0.3 in")
        XCTAssertEqual(value(chip, "Mounting"), "through-hole")
        XCTAssertNil(value(chip, "Finish"))
    }

    func testAGenericPlccSocketIsSurfaceMountWithNoRowSpacing() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "GENERIC-SOCKET-PLCC-44", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(chip.part.pins.count, 44)
        XCTAssertEqual(Set(chip.part.fits.map(\.id)), ["PLCC-44", "PLCC-44-MC-44J"])
        XCTAssertEqual(value(chip, "Family"), "PLCC")
        XCTAssertEqual(value(chip, "Mounting"), "surface-mount")
        XCTAssertNil(value(chip, "Row spacing"))
        XCTAssertEqual(chip.part.drawer, ["Connector", "IC Socket", "PLCC"])
    }

    func testAMakerSocketReadsItsFinishDatasheetAndFootprint() throws {
        let part = try TestSupport.part("A 14-LC-TR")
        XCTAssertEqual(part.manufacturer, "Assmann WSW Components")
        XCTAssertEqual(part.datasheetURL?.absoluteString, "https://www.assmann-wsw.com/uploads/datasheets/ASS_6449_CO.pdf")
        XCTAssertEqual(part.footprint?.id, "SKT-DIP14-300-TH-AS-ASS-6449")
        guard case let .socket(_, _, _, finish) = part.details else { return XCTFail("not a socket") }
        XCTAssertEqual(finish, "tin")
        XCTAssertEqual(part.detailLabels, ["through hole", "tin"])
        let chip = try TestSupport.place(TestSupport.newDocument(), part.name, at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(value(chip, "Finish"), "tin")
        XCTAssertEqual(value(chip, "Footprint")?.hasPrefix("SKT-DIP14-300-TH-AS-ASS-6449"), true)
    }

    func testSocketsCountXsAndAGateIsStillU() throws {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "GENERIC-SOCKET-DIP-14-300", at: CGPoint(x: 100, y: 100))
        let second = try TestSupport.place(doc, "GENERIC-SOCKET-DIP-8-300", at: CGPoint(x: 400, y: 100))
        let gate = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 700, y: 100))
        XCTAssertEqual(second.reference, "XS2")
        XCTAssertEqual(gate.reference, "U1")
    }

    func testRunStartsWithASocketOnTheSheet() throws {
        let doc = TestSupport.newDocument()
        let socket = try TestSupport.place(doc, "GENERIC-SOCKET-DIP-14-300", at: CGPoint(x: 100, y: 100))
        XCTAssertTrue(SimulationAvailability.unsimulated(doc.chips).isEmpty)
        XCTAssertTrue(SimulationAvailability.canStart(doc))
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        simulation.stop()
        XCTAssertEqual(doc.chips.map(\.id), [socket.id])
    }
}
