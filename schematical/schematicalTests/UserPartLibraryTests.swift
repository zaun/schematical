import Foundation
import XCTest
@testable import schematical

/// The User Library's folder (plan-53-B).
@MainActor
final class UserPartLibraryTests: XCTestCase {
    private var directory: URL!

    override func setUp() async throws {
        directory = FileManager.default.temporaryDirectory.appendingPathComponent("userlib-\(UUID().uuidString)")
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory)
    }

    private func text(_ uuid: String, _ name: String = "LIB-A") -> String {
        PartRegistrationTests.fileText(uuid: uuid, name: name)
    }

    func testDefaultDirectoryIsInTheTemporaryDirectoryUnderXCTest() {
        let path = UserPartLibrary.defaultDirectory.standardizedFileURL.path
        let temp = URL(fileURLWithPath: NSTemporaryDirectory()).standardizedFileURL.path
        XCTAssertTrue(path.hasPrefix(temp), "\(path) is not under \(temp)")
        XCTAssertFalse(path.contains("Application Support"))
    }

    func testWriteThenFilesReturnsTheFile() throws {
        let library = UserPartLibrary(directory: directory)
        let uuid = "A1000000-0000-4000-8000-000000000001"
        let before = Date().addingTimeInterval(-5)
        try library.write(uuid: uuid, text: text(uuid))
        let listed = library.files()
        XCTAssertEqual(listed.problems, [])
        XCTAssertEqual(listed.files.count, 1)
        XCTAssertEqual(listed.files.first?.uuid, uuid)
        XCTAssertEqual(listed.files.first?.text, text(uuid))
        XCTAssertEqual(listed.files.first?.url, library.url(forUUID: uuid))
        XCTAssertGreaterThan(try XCTUnwrap(listed.files.first?.added), before)
    }

    func testRemoveDeletesAndASecondRemoveDoesNotThrow() throws {
        let library = UserPartLibrary(directory: directory)
        let uuid = "A2000000-0000-4000-8000-000000000001"
        try library.write(uuid: uuid, text: text(uuid))
        try library.remove(uuid: uuid)
        XCTAssertEqual(library.files().files.count, 0)
        XCTAssertNoThrow(try library.remove(uuid: uuid))
    }

    func testOnlySchpartFilesAreListed() throws {
        let library = UserPartLibrary(directory: directory)
        let uuid = "A3000000-0000-4000-8000-000000000001"
        try library.write(uuid: uuid, text: text(uuid))
        try Data("hello".utf8).write(to: directory.appendingPathComponent("notes.txt"))
        let listed = library.files()
        XCTAssertEqual(listed.files.count, 1)
        XCTAssertEqual(listed.problems, [])
    }

    func testFilesAreInFileNameOrder() throws {
        let library = UserPartLibrary(directory: directory)
        let b = "B4000000-0000-4000-8000-000000000001"
        let a = "A4000000-0000-4000-8000-000000000001"
        try library.write(uuid: b, text: text(b, "LIB-B"))
        try library.write(uuid: a, text: text(a, "LIB-A"))
        XCTAssertEqual(library.files().files.map(\.uuid), [a, b])
    }

    func testUnreadableFilesAreProblems() throws {
        let library = UserPartLibrary(directory: directory)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        // No uuid: JSON, but not a part file.
        let noUUID = directory.appendingPathComponent("a-nouuid.schpart")
        try Data("{\"format\":\"schematical-part\"}".utf8).write(to: noUUID)
        // Not UTF-8.
        let binary = directory.appendingPathComponent("b-binary.schpart")
        try Data([0xFF, 0xFE, 0xFD]).write(to: binary)
        let uuid = "A5000000-0000-4000-8000-000000000001"
        try library.write(uuid: uuid, text: text(uuid))
        let listed = library.files()
        XCTAssertEqual(listed.files.map(\.uuid), [uuid])
        XCTAssertEqual(listed.problems.map(\.url.lastPathComponent), ["a-nouuid.schpart", "b-binary.schpart"])
        XCTAssertFalse(listed.problems.contains { $0.reason.isEmpty })
        // A problem's file can be deleted.
        try library.removeFile(at: noUUID)
        XCTAssertEqual(library.files().problems.count, 1)
    }

    func testAMissingFolderIsEmptyAndWriteCreatesIt() throws {
        let library = UserPartLibrary(directory: directory)
        XCTAssertFalse(FileManager.default.fileExists(atPath: directory.path))
        let listed = library.files()
        XCTAssertEqual(listed.files.count, 0)
        XCTAssertEqual(listed.problems.count, 0)
        let uuid = "A6000000-0000-4000-8000-000000000001"
        try library.write(uuid: uuid, text: text(uuid))
        XCTAssertTrue(FileManager.default.fileExists(atPath: directory.path))
    }
}
