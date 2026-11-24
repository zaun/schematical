import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-51-H: a placed DIP or PLCC chip's socket mode. Plain IC, IC + Socket
/// (the chip is soldered into the socket) and Socket only (the socket is on the
/// board, the chip is not): the PCB carries the socket's footprint with the
/// same pad numbers, the file, undo, paste, the Bill of Materials, the facts,
/// the exports and `set_part` follow, and Run ignores the mode.
@MainActor
final class SocketModeTests: XCTestCase {
    /// Kept for the life of the test process (see PlaySwitchResetTests).
    private static var keptSimulations: [SimulationController] = []

    override func setUp() {
        super.setUp()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    private let pdip14 = "PDIP-14"
    private let generic14 = "GENERIC-SOCKET-DIP-14-300"

    private func gate(_ doc: SchematicDocument) throws -> PlacedChip {
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 400, y: 300))
    }

    private func socketPart(_ name: String) throws -> CatalogPart { try TestSupport.part(name) }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    /// Pad 1's centre on the board.
    private func pad1(_ chip: PlacedChip) throws -> CGPoint {
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == "1" })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    private func value(_ chip: PlacedChip, _ label: String) -> String? {
        PartFacts.rows(for: chip).first { $0.label == label }?.value
    }

    // MARK: Phase 1: the model

    func testTheGenericSocketListIsFirstForTheChipFootprints() throws {
        let part = try TestSupport.part("SN74HC00N")
        let base = try XCTUnwrap(part.footprint)
        XCTAssertEqual(base.id, pdip14)
        let list = TestSupport.catalog.sockets(fitting: base)
        XCTAssertEqual(list.first?.name, generic14)
        XCTAssertEqual(TestSupport.catalog.genericSocket(fitting: base)?.name, generic14)
        XCTAssertGreaterThan(list.count, 50)
        XCTAssertTrue(list.allSatisfy { $0.kind == PART_SOCKET && $0.fits.contains { $0.id == pdip14 } })
        XCTAssertEqual(list.filter { $0.manufacturer == "Generic" }.count, 1)
        // Generics first, then by maker.
        let makers = list.dropFirst().map(\.manufacturer)
        XCTAssertEqual(makers, makers.sorted { $0.localizedStandardCompare($1) == .orderedAscending })
        // CDIP-14 is held by the same sockets' 14-pin DIP list.
        let ceramic = try XCTUnwrap(TestSupport.catalog.part(named: "SN54HC00J")?.footprint)
        XCTAssertEqual(TestSupport.catalog.genericSocket(fitting: ceramic)?.name, generic14)
        // A chip footprint no socket holds has none.
        let led = try XCTUnwrap(TestSupport.catalog.part(named: "GENERIC-LED")?.footprint)
        XCTAssertTrue(TestSupport.catalog.sockets(fitting: led).isEmpty)
    }

    func testTheThreeModesAndThePcbFootprint() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        XCTAssertNil(placed.socket)
        XCTAssertEqual(placed.footprint?.id, pdip14)
        let before = try pad1(placed)
        let generic = try socketPart(generic14)

        doc.setPartSocket(placed.id, mode: .withSocket, socket: generic)
        var now = try chip(doc, placed.id)
        XCTAssertEqual(now.socket, ChipSocket(part: generic, mode: .withSocket))
        XCTAssertEqual(now.footprint?.id, "SKT-DIP14-300-TH-MM-110")
        XCTAssertEqual(Set(now.boardPads.compactMap(\.designator)), Set((1...14).map(String.init)))
        XCTAssertEqual(now.boardPads.count, 14)
        let inSocket = try pad1(now)
        XCTAssertEqual(inSocket.x, before.x, accuracy: 0.001)
        XCTAssertEqual(inSocket.y, before.y, accuracy: 0.001)
        // The wire-up is by designator, so the pins keep their pads: pin n on pad n.
        XCTAssertEqual(now.part.pins.map(\.designator), (1...14).map(String.init))

        doc.setPartSocket(placed.id, mode: .socketOnly, socket: nil)
        now = try chip(doc, placed.id)
        XCTAssertEqual(now.socket?.mode, .socketOnly)
        XCTAssertEqual(now.socket?.part.name, generic14, "the socket stays when only the mode changes")
        XCTAssertEqual(now.footprint?.id, "SKT-DIP14-300-TH-MM-110")
        let socketOnly = try pad1(now)
        XCTAssertEqual(socketOnly.x, before.x, accuracy: 0.001)

        doc.setPartSocket(placed.id, mode: nil, socket: nil)
        now = try chip(doc, placed.id)
        XCTAssertNil(now.socket)
        XCTAssertEqual(now.footprint?.id, pdip14)
        let back = try pad1(now)
        XCTAssertEqual(back.x, before.x, accuracy: 0.001)
        XCTAssertEqual(back.y, before.y, accuracy: 0.001)
    }

    func testASocketThatDoesNotFitIsRefused() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        let undoName = doc.undoName(on: .schematic)
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart("GENERIC-SOCKET-DIP-28-300"))
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart("GENERIC-SOCKET-PLCC-44"))
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try TestSupport.part("SN74HC00N"))
        doc.setPartSocket(placed.id, mode: .withSocket, socket: nil)
        XCTAssertNil(try chip(doc, placed.id).socket)
        XCTAssertEqual(doc.undoName(on: .schematic), undoName)
    }

    func testAChipASocketCannotTakeKeepsNoSocket() throws {
        let doc = TestSupport.newDocument()
        // A generic LED has footprint choices.
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        doc.setPartSocket(led.id, mode: .withSocket, socket: try socketPart(generic14))
        XCTAssertNil(try chip(doc, led.id).socket)
        // A 4-pin DIP oscillator is no DIP socket's chip: its footprint is in no `fits` list.
        let part = try XCTUnwrap(TestSupport.allParts.first { $0.kind == PART_OSCILLATOR && $0.pins.count == 4 })
        let osc = try TestSupport.place(doc, part.name, at: CGPoint(x: 300, y: 100))
        XCTAssertTrue(TestSupport.catalog.sockets(fitting: try XCTUnwrap(osc.part.footprint)).isEmpty)
        doc.setPartSocket(osc.id, mode: .withSocket, socket: try socketPart(generic14))
        XCTAssertNil(try chip(doc, osc.id).socket)
    }

    // MARK: Phase 2: file, undo, paste

    func testSaveThenOpenKeepsModeAndSocket() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        doc.setPartSocket(placed.id, mode: .socketOnly, socket: try socketPart("A 14-LC-TR"))
        let footprint = try chip(doc, placed.id).footprint?.id
        XCTAssertEqual(footprint, "SKT-DIP14-300-TH-AS-ASS-6449")
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let text = String(decoding: data, as: UTF8.self)
        XCTAssertTrue(text.contains("\"socket\""))
        XCTAssertTrue(text.contains("\"part\" : \"A 14-LC-TR\""))
        XCTAssertTrue(text.contains("\"mode\" : \"socketOnly\""))
        XCTAssertTrue(text.contains("\"version\" : 4"), "an optional field keeps the version")

        let opened = try BoardLayersTests.roundTrip(doc)
        let reopened = try XCTUnwrap(opened.chips.first)
        XCTAssertEqual(reopened.socket?.part.name, "A 14-LC-TR")
        XCTAssertEqual(reopened.socket?.mode, .socketOnly)
        XCTAssertEqual(reopened.footprint?.id, footprint)
        XCTAssertEqual(reopened.board, try chip(doc, placed.id).board)
    }

    func testAChipWithNoSocketWritesNoSocketKey() throws {
        let doc = TestSupport.newDocument()
        _ = try gate(doc)
        let text = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertFalse(text.contains("socket"))
    }

    private func opening(_ edit: (String) -> String) throws -> Error? {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart(generic14))
        let text = edit(String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self))
        do {
            try SchematicFile.decode(Data(text.utf8)).apply(to: TestSupport.newDocument(), catalog: TestSupport.catalog, fileURL: nil)
            return nil
        } catch {
            return error
        }
    }

    func testABadSocketEntryFailsWithItsMessage() throws {
        let notASocket = try opening { $0.replacingOccurrences(of: "\"part\" : \"\(generic14)\"", with: "\"part\" : \"SN74HC00N\"") }
        XCTAssertEqual(notASocket?.localizedDescription, "U1: \"SN74HC00N\" is not an IC socket")
        let wrongSize = try opening {
            $0.replacingOccurrences(of: "\"part\" : \"\(generic14)\"", with: "\"part\" : \"GENERIC-SOCKET-DIP-8-300\"")
        }
        XCTAssertEqual(wrongSize?.localizedDescription, "U1: \"GENERIC-SOCKET-DIP-8-300\" does not fit PDIP-14")
        let unknownMode = try opening { $0.replacingOccurrences(of: "\"mode\" : \"withSocket\"", with: "\"mode\" : \"ic\"") }
        XCTAssertEqual(unknownMode?.localizedDescription, "U1: \"ic\" is not a socket mode")
        let missing = try opening { $0.replacingOccurrences(of: "\"part\" : \"\(generic14)\"", with: "\"part\" : \"NO-SUCH-SOCKET\"") }
        XCTAssertEqual(missing?.localizedDescription, "U1: \"NO-SUCH-SOCKET\" is not an IC socket")
        let fine = try opening { $0 }
        XCTAssertNil(fine)
    }

    func testUndoAndRedoOfASocketChange() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        doc.endCoalescing()
        let boardBefore = try chip(doc, placed.id).board
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart(generic14))
        doc.endCoalescing()
        XCTAssertEqual(doc.undoName(on: .schematic), "Change Socket")
        let after = try chip(doc, placed.id)
        XCTAssertNotNil(after.socket)

        XCTAssertTrue(doc.undo(on: .schematic))
        var now = try chip(doc, placed.id)
        XCTAssertNil(now.socket)
        XCTAssertEqual(now.footprint?.id, pdip14)
        XCTAssertEqual(now.board, boardBefore)

        XCTAssertTrue(doc.redo(on: .schematic))
        now = try chip(doc, placed.id)
        XCTAssertEqual(now.socket, after.socket)
        XCTAssertEqual(now.board, after.board)

        // The mode alone is one step too.
        doc.setPartSocket(placed.id, mode: .socketOnly, socket: nil)
        doc.endCoalescing()
        XCTAssertTrue(doc.undo(on: .schematic))
        XCTAssertEqual(try chip(doc, placed.id).socket?.mode, .withSocket)
    }

    func testCopyAndPasteKeepTheSocket() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart(generic14))
        doc.select(.chip(placed.id))
        let copy = try XCTUnwrap(doc.schematicCopy())
        doc.endCoalescing()
        XCTAssertTrue(doc.pasteSchematic(copy, offset: CGSize(width: 300, height: 0)))
        let pasted = try XCTUnwrap(doc.chips.last)
        XCTAssertNotEqual(pasted.id, placed.id)
        XCTAssertEqual(pasted.socket, ChipSocket(part: try socketPart(generic14), mode: .withSocket))
        XCTAssertEqual(pasted.footprint?.id, "SKT-DIP14-300-TH-MM-110")
        XCTAssertNotNil(pasted.board)
    }

    // MARK: Phase 3: Bill of Materials, facts, automation, exports, run

    private func lines(_ doc: SchematicDocument) -> [[String]] {
        BillOfMaterials.lines(for: doc.chips).map { [$0.partNumber, $0.referenceText] }
    }

    func testTheBillOfMaterialsFollowsTheMode() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        XCTAssertEqual(lines(doc), [["SN74HC00N", "U1"]])
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart(generic14))
        XCTAssertEqual(lines(doc), [["SN74HC00N", "U1"], [generic14, "XU1"]])
        let socketLine = try XCTUnwrap(BillOfMaterials.lines(for: doc.chips).last)
        XCTAssertEqual(socketLine.package, "DIP socket")
        XCTAssertEqual(socketLine.description, "14-pin 0.3 in DIP socket")
        XCTAssertEqual(socketLine.manufacturer, "")
        doc.setPartSocket(placed.id, mode: .socketOnly, socket: nil)
        XCTAssertEqual(lines(doc), [[generic14, "XU1"]])
        doc.setPartSocket(placed.id, mode: nil, socket: nil)
        XCTAssertEqual(lines(doc), [["SN74HC00N", "U1"]])
    }

    func testTwoChipsInTheSameSocketPartShareALine() throws {
        let doc = TestSupport.newDocument()
        let one = try gate(doc)
        let two = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 700, y: 300))
        let generic = try socketPart(generic14)
        doc.setPartSocket(one.id, mode: .withSocket, socket: generic)
        doc.setPartSocket(two.id, mode: .socketOnly, socket: generic)
        XCTAssertEqual(lines(doc), [["SN74HC00N", "U1"], [generic14, "XU1, XU2"]])
        // A maker socket is its own line, with its maker.
        let maker = try socketPart("A 14-LC-TR")
        doc.setPartSocket(two.id, mode: .socketOnly, socket: maker)
        let all = BillOfMaterials.lines(for: doc.chips)
        XCTAssertEqual(all.map(\.partNumber), ["SN74HC00N", generic14, "A 14-LC-TR"])
        XCTAssertEqual(all.last?.manufacturer, "Assmann WSW Components")
    }

    func testTheFactsRows() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        XCTAssertNil(value(placed, "Socket"))
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart(generic14))
        var now = try chip(doc, placed.id)
        XCTAssertEqual(value(now, "Socket"), "\(generic14) (IC + Socket)")
        XCTAssertTrue(value(now, "Footprint")?.hasPrefix("SKT-DIP14-300-TH-MM-110") == true)
        XCTAssertEqual(value(now, "Kind"), "Logic gate")
        doc.setPartSocket(placed.id, mode: .socketOnly, socket: nil)
        now = try chip(doc, placed.id)
        XCTAssertEqual(value(now, "Socket"), "\(generic14) (Socket only)")
    }

    func testKiCadAndPickAndPlaceFollowTheSocket() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        var board = KiCadExport.files(for: doc, base: "x")[2].text
        XCTAssertTrue(board.contains(pdip14))
        doc.setPartSocket(placed.id, mode: .withSocket, socket: try socketPart(generic14))
        board = KiCadExport.files(for: doc, base: "x")[2].text
        XCTAssertTrue(board.contains("SKT-DIP14-300-TH-MM-110"), "the KiCad board names the socket footprint")
        XCTAssertFalse(board.contains("\"\(pdip14)\""))
        // A surface-mount socket is placed by the fab under the socket's reference.
        let plcc = try TestSupport.part("W65C02S6TPLG-14")
        let chip44 = try TestSupport.place(doc, plcc.name, at: CGPoint(x: 800, y: 300))
        let smd = try XCTUnwrap(TestSupport.catalog.genericSocket(fitting: try XCTUnwrap(chip44.part.footprint)))
        doc.setPartSocket(chip44.id, mode: .withSocket, socket: smd)
        let rows = PickAndPlaceExport.rows(for: doc.chips, top: 100)
        XCTAssertEqual(rows.map(\.designator), ["X\(chip44.reference)"])
    }

    func testSetPartSocketModesThroughTheDocumentAreTheSameAsTheAutomationOnes() throws {
        // The automation path is in AutomationTests; here the pure checks of the mode names.
        XCTAssertEqual(ChipSocket.Mode(rawValue: "withSocket"), .withSocket)
        XCTAssertEqual(ChipSocket.Mode(rawValue: "socketOnly"), .socketOnly)
        XCTAssertNil(ChipSocket.Mode(rawValue: "ic"))
    }

    func testRunBuildsTheSameCircuitInAllThreeModes() throws {
        let doc = TestSupport.newDocument()
        let placed = try gate(doc)
        var results: [Bool] = []
        for mode in [nil, ChipSocket.Mode.withSocket, .socketOnly] {
            doc.setPartSocket(placed.id, mode: mode, socket: mode == nil ? nil : try socketPart(generic14))
            XCTAssertTrue(SimulationAvailability.canStart(doc))
            let simulation = SimulationController(library: TestSupport.catalog.library)
            Self.keptSimulations.append(simulation)
            simulation.start(document: doc)
            results.append(simulation.isRunning)
            XCTAssertTrue(simulation.isRunning, "\(String(describing: mode)): \(simulation.errorMessage ?? "")")
            simulation.stop()
        }
        XCTAssertEqual(results, [true, true, true])
    }
}
