import AppKit
import Foundation
import XCTest
@testable import schematical

/// plan-68-W: an EEPROM's load file — the Open panel (answered by the test),
/// reading it, its start contents, bookmarks, and how the schematic names it.
@MainActor
final class MemoryFileAccessTests: XCTestCase {
    private var folder: URL!
    private var windows: [NSWindow] = []

    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
        folder = FileManager.default.temporaryDirectory.appendingPathComponent("MemoryFileAccessTests-\(UUID().uuidString)")
        try? FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
    }

    override func tearDown() {
        for window in windows {
            if let sheet = window.attachedSheet { window.endSheet(sheet) }
            window.close()
        }
        windows = []
        try? FileManager.default.removeItem(at: folder)
        super.tearDown()
    }

    private func write(_ name: String, _ bytes: [UInt8]) throws -> URL {
        let url = folder.appendingPathComponent(name)
        try Data(bytes).write(to: url)
        return url
    }

    // MARK: Choosing

    func testThePanelTakesOneFile() {
        let panel = MemoryFileAccess.openPanel()
        XCTAssertFalse(panel.allowsMultipleSelection)
        XCTAssertFalse(panel.canChooseDirectories)
        XCTAssertTrue(panel.canChooseFiles)
        XCTAssertEqual(panel.prompt, "Choose")
        XCTAssertTrue(panel.message.contains("Intel HEX"))
    }

    func testAChosenFileComesBackAndACancelGivesNothing() async throws {
        let rom = try write("rom.bin", [1, 2, 3])
        var asked: [NSWindow?] = []
        let chosen = await MemoryFileAccess.choose(attachedTo: nil) { panel, window in
            XCTAssertTrue(panel is NSOpenPanel)
            asked.append(window)
            return rom
        }
        XCTAssertEqual(chosen?.url, rom)
        XCTAssertEqual(chosen?.name, "rom.bin")
        XCTAssertEqual(asked.count, 1)
        let cancelled = await MemoryFileAccess.choose(attachedTo: nil) { _, _ in nil }
        XCTAssertNil(cancelled)
    }

    /// The real panel runs as a sheet on the window; cancelled, it gives nothing.
    func testThePanelIsASheetOnTheWindow() throws {
        let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 480, height: 320), styleMask: [.titled],
                              backing: .buffered, defer: false)
        window.isReleasedWhenClosed = false
        windows.append(window)
        window.orderFront(nil)
        var result: MemoryFile??
        Task { result = .some(await MemoryFileAccess.choose(attachedTo: window)) }
        var sheet: NSWindow?
        for _ in 0..<100 where sheet == nil {
            RunLoop.main.run(until: Date().addingTimeInterval(0.05))
            sheet = window.attachedSheet
        }
        let panel = try XCTUnwrap(sheet as? NSOpenPanel, "the Open panel is up as a sheet")
        panel.cancel(nil)
        for _ in 0..<100 where result == nil { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
        XCTAssertEqual(result, .some(nil), "cancelled: no file")
        XCTAssertNil(window.attachedSheet)
    }

    // MARK: Reading

    func testReadingAFileAndAMissingOne() throws {
        let rom = try write("rom.bin", [0xEA, 0x00])
        XCTAssertEqual(try MemoryFileAccess.read(MemoryFile(url: rom, bookmark: nil)), Data([0xEA, 0x00]))
        let gone = MemoryFile(url: folder.appendingPathComponent("gone.bin"), bookmark: nil)
        XCTAssertThrowsError(try MemoryFileAccess.read(gone)) { error in
            guard case let .unreadable(name, _) = error as? MemoryFileError else { return XCTFail("\(error)") }
            XCTAssertEqual(name, "gone.bin")
            XCTAssertTrue(error.localizedDescription.hasPrefix("can't read gone.bin: "), error.localizedDescription)
        }
    }

    func testStartContents() throws {
        let size = MemorySize(words: 4, bits: 8)
        XCTAssertEqual(try MemoryFileAccess.image(nil, size: size), [0xFF, 0xFF, 0xFF, 0xFF], "no file: erased")
        let rom = try write("rom.bin", [1, 2])
        XCTAssertEqual(try MemoryFileAccess.image(MemoryFile(url: rom, bookmark: nil), size: size), [1, 2, 0xFF, 0xFF])
        let bad = try write("bad.hex", Array("not intel hex at all\n".utf8))
        XCTAssertThrowsError(try MemoryFileAccess.image(MemoryFile(url: bad, bookmark: nil), size: size)) { error in
            guard case let .invalid(name, message) = error as? MemoryFileError else { return XCTFail("\(error)") }
            XCTAssertEqual(name, "bad.hex")
            XCTAssertFalse(message.isEmpty)
            XCTAssertEqual(error.localizedDescription, "bad.hex: \(message)")
        }
    }

    // MARK: Bookmarks

    func testABookmarkResolvesToItsFileAndJunkToNothing() throws {
        let rom = try write("rom.bin", [7])
        XCTAssertNil(MemoryFileAccess.resolve(Data("not a bookmark".utf8)))
        let file = MemoryFileAccess.file(path: "elsewhere/rom.bin", bookmark: Data("junk".utf8),
                                         schematic: folder.appendingPathComponent("a.schematical"))
        XCTAssertNil(file.bookmark, "a bookmark that does not resolve falls back to the path")
        XCTAssertEqual(file.url, folder.appendingPathComponent("elsewhere/rom.bin").standardizedFileURL)
        if let bookmark = MemoryFileAccess.bookmark(for: rom) {
            let resolved = try XCTUnwrap(MemoryFileAccess.resolve(bookmark))
            XCTAssertEqual(resolved.url.resolvingSymlinksInPath(), rom.resolvingSymlinksInPath())
            let kept = MemoryFileAccess.file(path: "ignored.bin", bookmark: bookmark, schematic: nil)
            XCTAssertEqual(kept.url.resolvingSymlinksInPath(), rom.resolvingSymlinksInPath())
            XCTAssertNotNil(kept.bookmark)
            XCTAssertEqual(try MemoryFileAccess.read(kept), Data([7]), "read through its bookmark")
        }
    }

    /// A stale bookmark is made afresh while the file can be read; one that
    /// can't be made again keeps the old.
    func testAStaleBookmarkIsMadeAgain() throws {
        let rom = try write("rom.bin", [7])
        let old = Data("old".utf8)
        XCTAssertEqual(MemoryFileAccess.kept(rom, bookmark: old, stale: false).bookmark, old)
        let fresh = MemoryFileAccess.kept(rom, bookmark: old, stale: true)
        XCTAssertEqual(fresh.url, rom)
        if MemoryFileAccess.bookmark(for: rom) != nil {
            XCTAssertNotEqual(fresh.bookmark, old, "a fresh bookmark")
        } else {
            XCTAssertEqual(fresh.bookmark, old)
        }
        let gone = folder.appendingPathComponent("gone.bin")
        XCTAssertEqual(MemoryFileAccess.kept(gone, bookmark: old, stale: true).bookmark, old, "no file: the old one stays")
    }

    // MARK: Paths

    func testPathsRelativeToTheSchematic() {
        let schematic = URL(fileURLWithPath: "/tmp/designs/cpu/board.schematical")
        let rom = URL(fileURLWithPath: "/tmp/designs/roms/boot.bin")
        XCTAssertEqual(MemoryFileAccess.path(of: rom, schematic: nil), "/tmp/designs/roms/boot.bin")
        XCTAssertEqual(MemoryFileAccess.path(of: rom, schematic: schematic), "../roms/boot.bin")
        XCTAssertEqual(MemoryFileAccess.url(forPath: "../roms/boot.bin", schematic: schematic).path, "/tmp/designs/roms/boot.bin")
        XCTAssertEqual(MemoryFileAccess.url(forPath: "/abs/boot.bin", schematic: schematic).path, "/abs/boot.bin")
        XCTAssertEqual(MemoryFileAccess.url(forPath: "boot.bin", schematic: nil).lastPathComponent, "boot.bin")
        XCTAssertEqual(MemoryFileAccess.file(path: "../roms/boot.bin", bookmark: nil, schematic: schematic).url.path,
                       "/tmp/designs/roms/boot.bin")
    }
}
