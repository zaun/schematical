import AppKit
import Foundation
import Observation
import SwiftUI
import XCTest
@testable import schematical

/// A memory part's contents as a test sets them (`MemorySource`).
@MainActor
@Observable
final class FakeMemory: MemorySource {
    var isRunning = true
    var stateVersion = 0
    var size: MemorySize?
    var values: [MemoryWord]

    nonisolated deinit {}

    init(words: Int, bits: Int, known: Bool = true) {
        size = MemorySize(words: words, bits: bits)
        let mask: UInt32 = bits >= 32 ? .max : (1 << UInt32(bits)) - 1
        values = (0..<words).map { MemoryWord(value: UInt32($0) & mask, known: known ? mask : 0) }
    }

    func memorySize(of chipID: UUID) -> MemorySize? { isRunning ? size : nil }

    func readMemory(of chipID: UUID, start: Int, count: Int) -> [MemoryWord] {
        guard isRunning, start < values.count else { return [] }
        return Array(values[start..<min(start + count, values.count)])
    }
}

/// plan-68-W: the memory window — its model (pages, highlights, the
/// address field, an EEPROM's start contents and load file), the grid's
/// formatting, and the window drawn in each state.
@MainActor
final class MemoryWindowTests: XCTestCase {
    private var folder: URL!

    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
        folder = FileManager.default.temporaryDirectory.appendingPathComponent("MemoryWindowTests-\(UUID().uuidString)")
        try? FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
    }

    override func tearDown() {
        try? FileManager.default.removeItem(at: folder)
        super.tearDown()
    }

    private func chip(_ name: String) throws -> (SchematicDocument, PlacedChip) {
        let doc = TestSupport.newDocument()
        return (doc, try TestSupport.place(doc, name, at: CGPoint(x: 200, y: 200)))
    }

    private let sram = "AS6C6264-55PCN"
    private let eeprom = "AT28C256-15PU"

    // MARK: Model

    func testPagesAreReadAndKeptInRange() throws {
        let (doc, ram) = try chip(sram)
        let memory = FakeMemory(words: 600, bits: 8)
        let model = MemoryInspectorModel(chipID: ram.id, document: doc, source: memory)
        model.appeared()
        let size = try XCTUnwrap(model.size)
        XCTAssertEqual(MemoryInspectorModel.pageCount(size), 3)
        XCTAssertEqual(model.words.count, 256)
        XCTAssertEqual(model.rangeText(size), "000–0FF")
        model.nextPage(size)
        model.pageChanged()
        XCTAssertEqual(model.page, 1)
        XCTAssertEqual(model.words.first?.value, 0, "word 256, low byte")
        model.nextPage(size)
        model.pageChanged()
        XCTAssertEqual(model.words.count, 88, "the last page is short")
        XCTAssertEqual(model.rangeText(size), "200–257")
        model.nextPage(size)
        XCTAssertEqual(model.page, 2, "no page past the last")
        model.page = 9
        model.pageChanged()
        XCTAssertEqual(model.page, 2, "a typed page past the end is the last")
        model.page = -1
        model.pageChanged()
        XCTAssertEqual(model.page, 0)
        XCTAssertEqual(model.words.count, 88, "clamping alone reads nothing")
        model.pageChanged()
        XCTAssertEqual(model.words.count, 256, "the clamped page, read (the view's next change)")
        model.previousPage()
        XCTAssertEqual(model.page, 0)
        XCTAssertEqual(model.footer(size, function: "8 KB").left, "600 words × 8 bits (8 KB)")
        XCTAssertEqual(model.footer(size, function: "8 KB").right, "256 of 256 words on this page defined")

        // Shrunk below the page: back to page 0.
        model.page = 2
        memory.size = MemorySize(words: 100, bits: 8)
        model.reload()
        XCTAssertEqual(model.page, 0)
    }

    func testChangedWordsAreHighlightedThenExpire() throws {
        let (doc, ram) = try chip(sram)
        let memory = FakeMemory(words: 256, bits: 8)
        let model = MemoryInspectorModel(chipID: ram.id, document: doc, source: memory)
        model.appeared()
        XCTAssertEqual(model.highlighted, [])
        memory.values[3] = MemoryWord(value: 0x55, known: 0xFF)
        memory.stateVersion = 1
        model.stepped()
        XCTAssertEqual(model.highlighted, [3])
        memory.stateVersion = 5
        model.stepped()
        XCTAssertEqual(model.highlighted, [3], "still within half a second")
        memory.stateVersion = 1 + MemoryInspectorModel.highlightSteps
        model.stepped()
        XCTAssertEqual(model.highlighted, [])
        XCTAssertTrue(model.changedAt.isEmpty)

        memory.values[4] = MemoryWord(value: 0x56, known: 0xFF)
        model.stepped()
        XCTAssertEqual(model.changedAt.keys.sorted(), [4])
        memory.isRunning = false
        model.runningChanged()
        XCTAssertNil(model.size)
        XCTAssertEqual(model.words, [])
        XCTAssertEqual(model.changedAt, [:])
    }

    func testTheAddressFieldGoesToItsPageOrBeeps() throws {
        let (doc, ram) = try chip(sram)
        let model = MemoryInspectorModel(chipID: ram.id, document: doc, source: FakeMemory(words: 1024, bits: 8))
        var beeps = 0
        model.beep = { beeps += 1 }
        model.addressText = "1F0"
        model.goToAddress()
        XCTAssertEqual([model.page, beeps], [0, 1], "no size yet: not a page of anything")
        model.appeared()
        model.goToAddress()
        XCTAssertEqual(model.page, 1)
        model.addressText = " 0x3ff "
        model.goToAddress()
        XCTAssertEqual(model.page, 3)
        for bad in ["400", "zz", "-1", ""] {
            model.addressText = bad
            model.goToAddress()
        }
        XCTAssertEqual([model.page, beeps], [3, 5])
    }

    func testAStoppedEEPROMShowsWhatItWillStartWith() async throws {
        let (doc, rom) = try chip(eeprom)
        let memory = FakeMemory(words: 32768, bits: 8)
        memory.isRunning = false
        let model = MemoryInspectorModel(chipID: rom.id, document: doc, source: memory)
        XCTAssertTrue(model.isEEPROM)
        XCTAssertTrue(model.showsStartContents)
        model.appeared()
        let size = try XCTUnwrap(model.size)
        XCTAssertEqual(size, MemorySize(words: 32768, bits: 8))
        XCTAssertEqual(model.words.count, 256)
        XCTAssertTrue(model.words.allSatisfy { $0 == MemoryWord(value: 0xFF, known: 0xFF) })
        XCTAssertTrue(model.footer(size, function: "").left.hasPrefix("Contents at start · "))

        let file = folder.appendingPathComponent("boot.bin")
        try Data([0xA9, 0x01]).write(to: file)
        var asked = 0
        model.chooseFile = { _ in asked += 1; return MemoryFile(url: file, bookmark: nil) }
        await model.chooseLoadFile(window: nil)
        XCTAssertEqual(doc.chips.first?.memoryFile?.url, file)
        model.fileChanged()
        XCTAssertEqual(model.words.prefix(3).map(\.value), [0xA9, 0x01, 0xFF])
        model.chooseFile = { _ in asked += 1; return nil }
        await model.chooseLoadFile(window: nil)
        XCTAssertEqual(asked, 2)
        XCTAssertEqual(doc.chips.first?.memoryFile?.url, file, "a cancelled panel changes nothing")

        let bad = folder.appendingPathComponent("bad.hex")
        try Data("junk".utf8).write(to: bad)
        doc.setMemoryFile(rom.id, MemoryFile(url: bad, bookmark: nil))
        model.fileChanged()
        XCTAssertNotNil(model.loadError)
        XCTAssertEqual(model.words, [])
        model.clearLoadFile()
        XCTAssertNil(doc.chips.first?.memoryFile)
        model.fileChanged()
        XCTAssertNil(model.loadError)
        model.page = 200
        model.reload()
        XCTAssertEqual(model.page, 0, "past the end of the contents: the first page")

        memory.isRunning = true
        model.runningChanged()
        XCTAssertFalse(model.showsStartContents)
        XCTAssertEqual(model.words.first?.value, 0, "live contents now")
    }

    func testARemovedPartHasNothingToRead() throws {
        let (doc, ram) = try chip(sram)
        let model = MemoryInspectorModel(chipID: ram.id, document: doc, source: FakeMemory(words: 16, bits: 8))
        doc.selectAllSchematic(); XCTAssertTrue(doc.deleteSchematicSelection())
        XCTAssertNil(model.chip)
        XCTAssertFalse(model.isEEPROM)
        model.refreshStartImage()
        XCTAssertEqual(model.startImage, [])
    }

    // MARK: Format

    func testWordsAddressesAndText() {
        XCTAssertEqual(MemoryFormat.addressDigits(MemorySize(words: 1, bits: 8)), 1)
        XCTAssertEqual(MemoryFormat.addressDigits(MemorySize(words: 0, bits: 8)), 1)
        XCTAssertEqual(MemoryFormat.addressDigits(MemorySize(words: 0x8000, bits: 8)), 4)
        XCTAssertEqual(MemoryFormat.hex(0x1F, digits: 4), "001F")
        XCTAssertEqual(MemoryFormat.hex(0x12345, digits: 2), "12345")
        XCTAssertEqual(MemoryFormat.wordMask(8), 0xFF)
        XCTAssertEqual(MemoryFormat.wordMask(32), .max)
        XCTAssertEqual([4, 8, 12, 16, 32].map { MemoryFormat.wordsPerRow(bits: $0) }, [16, 16, 8, 8, 4])
        XCTAssertEqual(MemoryFormat.wordText(MemoryWord(value: 0xA5, known: 0xF0), bits: 8), "AX")
        XCTAssertEqual(MemoryFormat.wordText(MemoryWord(value: 0x123, known: 0xFFF), bits: 12), "123")
        XCTAssertEqual(MemoryFormat.binary(MemoryWord(value: 0b101, known: 0b011), bits: 3), "X01")
        let bytes = [MemoryWord(value: 0x41, known: 0xFF), MemoryWord(value: 0x0A, known: 0xFF),
                     MemoryWord(value: 0x42, known: 0x0F), MemoryWord(value: 0x7E, known: 0xFF)]
        XCTAssertEqual(MemoryFormat.ascii(bytes[...]), "A..~")
    }

    // MARK: Drawn

    private func probe(_ model: MemoryInspectorModel) -> ViewProbe {
        ViewProbe(MemoryInspector(model: model), size: NSSize(width: 720, height: 520))
    }

    func testARunningSRAMWindow() throws {
        let (doc, ram) = try chip(sram)
        let memory = FakeMemory(words: 1024, bits: 8)
        let model = MemoryInspectorModel(chipID: ram.id, document: doc, source: memory)
        var beeps = 0
        model.beep = { beeps += 1 }
        let probe = probe(model)
        defer { probe.close() }
        XCTAssertTrue(probe.has("of 4"), "\(probe.labels())")
        XCTAssertTrue(probe.has("000–0FF"))
        XCTAssertTrue(probe.has("1024 words × 8 bits"))
        XCTAssertTrue(probe.has("256 of 256 words on this page defined"))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Previous Page", role: "AXButton")))
        try probe.press("Next Page")
        XCTAssertEqual(model.page, 1)
        XCTAssertTrue(probe.has("100–1FF"), "\(probe.labels())")
        try probe.press("Previous Page")
        XCTAssertEqual(model.page, 0)

        memory.values[2] = MemoryWord(value: 0x99, known: 0xFF)
        memory.stateVersion += 1
        probe.settle()
        XCTAssertEqual(model.highlighted, [2], "a step read the page again")
        XCTAssertTrue(probe.has("99"))

        try probe.type("2A0", intoField: "Go to address (hex)")
        XCTAssertEqual(model.page, 2)
        try probe.type("nope", intoField: "Go to address (hex)")
        XCTAssertEqual(beeps, 1)

        memory.isRunning = false
        probe.settle()
        XCTAssertTrue(probe.has("Simulation Not Running"), "\(probe.labels())")
    }

    func testWideWordsAndUnknownBits() throws {
        let (doc, ram) = try chip(sram)
        let probe16 = probe(MemoryInspectorModel(chipID: ram.id, document: doc, source: FakeMemory(words: 16, bits: 16)))
        XCTAssertTrue(probe16.has("000F"), "\(probe16.labels())")
        probe16.close()
        let probe32 = probe(MemoryInspectorModel(chipID: ram.id, document: doc,
                                                 source: FakeMemory(words: 8, bits: 32, known: false)))
        defer { probe32.close() }
        XCTAssertTrue(probe32.has("XXXXXXXX"), "\(probe32.labels())")
        XCTAssertTrue(probe32.has("0 of 8 words on this page defined"))
    }

    func testAStoppedEEPROMWindowAndItsLoadFile() throws {
        let (doc, rom) = try chip(eeprom)
        let memory = FakeMemory(words: 32768, bits: 8)
        memory.isRunning = false
        let model = MemoryInspectorModel(chipID: rom.id, document: doc, source: memory)
        let file = folder.appendingPathComponent("boot.bin")
        try Data([0x41, 0x42]).write(to: file)
        model.chooseFile = { _ in MemoryFile(url: file, bookmark: nil) }
        let probe = probe(model)
        defer { probe.close() }
        XCTAssertTrue(probe.has("Load file on start:"), "\(probe.labels())")
        XCTAssertTrue(probe.has("None — starts erased (FF)"))
        XCTAssertTrue(probe.has("Contents at start · 32768 words × 8 bits"))
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Clear", role: "AXButton")))
        try probe.press("Choose…")
        probe.settle()
        XCTAssertEqual(doc.chips.first?.memoryFile?.url, file)
        XCTAssertTrue(probe.has("boot.bin"), "\(probe.labels())")
        XCTAssertEqual(model.words.prefix(2).map(\.value), [0x41, 0x42], "the file changed: its contents shown")
        XCTAssertTrue(probe.has("AB"))

        let bad = folder.appendingPathComponent("bad.hex")
        try Data("junk".utf8).write(to: bad)
        doc.setMemoryFile(rom.id, MemoryFile(url: bad, bookmark: nil))
        probe.settle()
        XCTAssertTrue(probe.has("Can't Load File"), "\(probe.labels())")
        try probe.press("Clear")
        XCTAssertNil(doc.chips.first?.memoryFile)

        memory.isRunning = true
        probe.settle()
        XCTAssertFalse(probe.has("Contents at start"), "\(probe.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try probe.element("Choose…", role: "AXButton")))

        doc.selectAllSchematic(); XCTAssertTrue(doc.deleteSchematicSelection())
        probe.settle()
        XCTAssertTrue(probe.has("Part Removed"), "\(probe.labels())")
    }

    func testTheWindowFindsItsSchematic() throws {
        let doc = TestSupport.newDocument()
        let ram = try TestSupport.place(doc, sram, at: CGPoint(x: 200, y: 200))
        let system = TestSupport.openSystemDocument(of: doc, catalog: TestSupport.catalog, until: self)
        let open = ViewProbe(MemoryWindowView(target: MemoryWindowTarget(documentID: system.model.id, chipID: ram.id)))
        defer { open.close() }
        XCTAssertTrue(open.has("Simulation Not Running"), "\(open.labels())")
        let closed = ViewProbe(MemoryWindowView(target: MemoryWindowTarget(documentID: UUID(), chipID: ram.id)))
        defer { closed.close() }
        XCTAssertTrue(closed.has("Schematic Closed"), "\(closed.labels())")
    }
}
