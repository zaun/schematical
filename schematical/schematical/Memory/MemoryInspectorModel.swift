//
//  MemoryInspectorModel.swift
//  schematical
//
//  What a memory window shows and does, apart from its layout (plan-68-W):
//  the page, the words read from the running simulation (or, for a stopped
//  EEPROM, the contents it will start with), which words just changed, the
//  address field, and the EEPROM's load file.
//

import AppKit
import Foundation
import Observation

/// What a memory window reads: the running simulation's memory parts.
@MainActor
protocol MemorySource: AnyObject {
    var isRunning: Bool { get }
    /// Goes up after every simulation step.
    var stateVersion: Int { get }
    func memorySize(of chipID: UUID) -> MemorySize?
    func readMemory(of chipID: UUID, start: Int, count: Int) -> [MemoryWord]
}

extension SimulationController: MemorySource {}

@MainActor
@Observable
final class MemoryInspectorModel {
    /// Words per page.
    static let pageWords = 256
    /// Simulator steps a changed word stays highlighted (about half a second).
    static let highlightSteps = 30

    let chipID: UUID
    let document: SchematicDocument
    @ObservationIgnored let source: MemorySource

    var page = 0
    private(set) var size: MemorySize?
    private(set) var words: [MemoryWord] = []
    /// Simulator state version at which each word on the page last changed.
    private(set) var changedAt: [Int: Int] = [:]
    var addressText = ""
    /// EEPROM, stopped: the contents the chip will start with, or why the
    /// load file can't be used.
    private(set) var startImage: [UInt32] = []
    private(set) var loadError: String?

    /// Asks for a load file (the Open panel; a test answers instead).
    @ObservationIgnored var chooseFile: @MainActor (NSWindow?) async -> MemoryFile? = MemoryFileAccess.choose(attachedTo:)
    /// A typed address that isn't one.
    @ObservationIgnored var beep: @MainActor () -> Void = NSSound.beep

    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime.
    nonisolated deinit {}

    init(chipID: UUID, document: SchematicDocument, source: MemorySource) {
        self.chipID = chipID
        self.document = document
        self.source = source
    }

    var chip: PlacedChip? {
        document.chips.first { $0.id == chipID }
    }

    var isEEPROM: Bool { chip?.part.kind == PART_EEPROM }

    /// An EEPROM while the simulation is stopped: the window previews its
    /// start contents instead of live ones.
    var showsStartContents: Bool {
        isEEPROM && !source.isRunning
    }

    static func pageCount(_ size: MemorySize) -> Int {
        max(1, (size.words + pageWords - 1) / pageWords)
    }

    /// The page's first word address.
    var start: Int { page * Self.pageWords }

    /// The page's addresses in hex ("000–0FF").
    func rangeText(_ size: MemorySize) -> String {
        let last = min(start + Self.pageWords, size.words) - 1
        let digits = MemoryFormat.addressDigits(size)
        return "\(MemoryFormat.hex(start, digits: digits))–\(MemoryFormat.hex(last, digits: digits))"
    }

    /// The footer's two halves.
    func footer(_ size: MemorySize, function: String) -> (left: String, right: String) {
        let known = words.filter { $0.known == MemoryFormat.wordMask(size.bits) }.count
        let prefix = showsStartContents ? "Contents at start · " : ""
        return ("\(prefix)\(size.words) words × \(size.bits) bits (\(function))",
                "\(known) of \(words.count) words on this page defined")
    }

    var highlighted: Set<Int> {
        Set(changedAt.compactMap { index, version in
            source.stateVersion - version < Self.highlightSteps ? index : nil
        })
    }

    // MARK: Events

    func appeared() {
        refreshStartImage()
        reload()
    }

    /// The simulation ran a step.
    func stepped() { reload() }

    func runningChanged() {
        if !source.isRunning { refreshStartImage() }
        changedAt = [:]
        reload()
    }

    func fileChanged() {
        refreshStartImage()
        reload()
    }

    /// The page was set: kept in range, then read afresh.
    func pageChanged() {
        if let size, page < 0 || page >= Self.pageCount(size) {
            page = min(max(page, 0), Self.pageCount(size) - 1)
            return
        }
        changedAt = [:]
        words = []
        reload()
    }

    func previousPage() { page = max(0, page - 1) }

    func nextPage(_ size: MemorySize) { page = min(Self.pageCount(size) - 1, page + 1) }

    /// Goes to the page of the typed word address (hex, "0x" optional);
    /// beeps when it isn't one of this memory's.
    func goToAddress() {
        guard let size else { return beep() }
        var text = addressText.trimmingCharacters(in: .whitespaces)
        if text.lowercased().hasPrefix("0x") { text.removeFirst(2) }
        guard let address = Int(text, radix: 16), address >= 0, address < size.words else { return beep() }
        page = address / Self.pageWords
    }

    /// Choose… : the EEPROM's new load file, unless the panel was cancelled.
    func chooseLoadFile(window: NSWindow?) async {
        if let file = await chooseFile(window) { document.setMemoryFile(chipID, file) }
    }

    func clearLoadFile() { document.setMemoryFile(chipID, nil) }

    // MARK: Reading

    /// EEPROM: parses the load file again (or all FF without one).
    func refreshStartImage() {
        guard let chip, isEEPROM, let partSize = chip.part.memorySize else { return }
        do {
            startImage = try MemoryFileAccess.image(chip.memoryFile, size: partSize)
            loadError = nil
        } catch {
            startImage = []
            loadError = error.localizedDescription
        }
    }

    /// Reads the page again. State is only assigned when it changed, so the
    /// grid redraws only when the memory does.
    func reload() {
        if showsStartContents { return reloadStartContents() }
        let newSize = source.memorySize(of: chipID)
        if newSize != size { size = newSize }
        guard let newSize else {
            if !words.isEmpty { words = [] }
            if !changedAt.isEmpty { changedAt = [:] }
            return
        }
        if page >= Self.pageCount(newSize) { page = 0 }
        let fresh = source.readMemory(of: chipID, start: start, count: Self.pageWords)
        guard fresh != words else { return expireHighlights() }
        if words.count == fresh.count {
            var updated = changedAt
            for index in fresh.indices where fresh[index] != words[index] { updated[index] = source.stateVersion }
            changedAt = updated
        }
        words = fresh
        expireHighlights()
    }

    /// EEPROM, stopped: the page of the start contents. No highlights.
    private func reloadStartContents() {
        let newSize = chip?.part.memorySize
        if newSize != size { size = newSize }
        if !changedAt.isEmpty { changedAt = [:] }
        guard let newSize else { return }
        if page >= Self.pageCount(newSize) { page = 0 }
        let last = min(start + Self.pageWords, startImage.count)
        let fresh = start < last
            ? startImage[start..<last].map { MemoryWord(value: $0, known: MemoryFormat.wordMask(newSize.bits)) }
            : []
        if fresh != words { words = fresh }
    }

    private func expireHighlights() {
        let live = changedAt.filter { source.stateVersion - $0.value < Self.highlightSteps }
        if live.count != changedAt.count { changedAt = live }
    }
}

/// How a memory window writes addresses and words.
nonisolated enum MemoryFormat {
    /// Hex digits needed for any word address of this memory.
    static func addressDigits(_ size: MemorySize) -> Int {
        max(1, String(max(size.words - 1, 0), radix: 16).count)
    }

    static func hex(_ value: Int, digits: Int) -> String {
        let text = String(value, radix: 16, uppercase: true)
        return String(repeating: "0", count: max(0, digits - text.count)) + text
    }

    /// Bits that exist in a word of the given width.
    static func wordMask(_ bits: Int) -> UInt32 {
        bits >= 32 ? .max : (UInt32(1) << UInt32(bits)) - 1
    }

    /// Words per row of the grid: 16 bytes, 8 16-bit words, else 4.
    static func wordsPerRow(bits: Int) -> Int {
        switch bits {
        case ...8: 16
        case ...16: 8
        default: 4
        }
    }

    /// The word in hex, X for a digit with any unknown bit.
    static func wordText(_ word: MemoryWord, bits: Int) -> String {
        let digits = (bits + 3) / 4
        let present = wordMask(bits)
        var text = ""
        for nibble in (0..<digits).reversed() {
            let shift = UInt32(nibble * 4)
            let mask: UInt32 = (0xF << shift) & present
            text += word.known & mask != mask ? "X" : String((word.value >> shift) & 0xF, radix: 16, uppercase: true)
        }
        return text
    }

    /// The word in binary, X for an unknown bit.
    static func binary(_ word: MemoryWord, bits: Int) -> String {
        String((0..<bits).reversed().map { bit in
            let b = UInt32(1) << UInt32(bit)
            return word.known & b == 0 ? "X" : (word.value & b != 0 ? "1" : "0")
        })
    }

    /// Bytes as text: printable ASCII, else ".".
    static func ascii(_ words: ArraySlice<MemoryWord>) -> String {
        String(words.map { word in
            guard word.known & 0xFF == 0xFF, (0x20..<0x7F).contains(word.value & 0xFF),
                  let scalar = Unicode.Scalar(word.value & 0xFF) else { return "." }
            return Character(scalar)
        })
    }
}
