import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-53's end-to-end run (plan-53-F § 4.3), walked through the same code
/// the app's menus, sheets and windows call, in the test host. The sheets and
/// the windows themselves are the thin layer over these calls.
@MainActor
final class UserLibraryEndToEndTests: XCTestCase {
    private var directory: URL!
    private var scratch: URL!
    private static var keptStores: [PartCatalogStore] = []
    private static var keptModels: [PartEditorModel] = []

    override func setUp() async throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("plan53-\(UUID().uuidString)")
        directory = root.appendingPathComponent("UserLibrary")
        scratch = root.appendingPathComponent("scratch")
        try FileManager.default.createDirectory(at: scratch, withIntermediateDirectories: true)
    }

    override func tearDown() async throws {
        try? FileManager.default.removeItem(at: directory.deletingLastPathComponent())
    }

    /// A launch: a fresh catalog with the bundled parts and the library folder attached.
    private func launch() -> PartCatalogStore {
        let store = PartCatalogStore()
        store.load()
        Self.keptStores.append(store)
        store.attachUserLibrary(UserPartLibrary(directory: directory))
        return store
    }

    private func libraryFiles() -> [String] {
        ((try? FileManager.default.contentsOfDirectory(atPath: directory.path)) ?? []).sorted()
    }

    func testTheWholeRun() async throws {
        let store = launch()
        XCTAssertEqual(libraryFiles(), [])

        // 1. Copy to Editor… a drawer part, name it PROOF-NAND, Save Part… to a file.
        let editor = PartEditorModel(target: PartEditorTarget(source: .copy(name: "SN74HC00N")), catalog: store)
        Self.keptModels.append(editor)
        XCTAssertNil(editor.openError)
        editor.draft.name = "PROOF-NAND"
        let partFile = scratch.appendingPathComponent("proof-nand.schpart")
        try editor.saveAsNewVersion(to: partFile)
        let uuid = editor.draft.uuid

        // 2. Load Part… → Use Once: a File Library part, the library folder unchanged.
        let text = try PartFileReader.read(url: partFile)
        var asked: [String] = []
        let once = try await PartFileLoading.load(text: text, source: "proof-nand.schpart", catalog: store) {
            asked.append($0)
            return .useOnce
        }
        guard case let .usedOnce(filePart) = once else { return XCTFail("expected usedOnce, got \(once)") }
        XCTAssertEqual(asked, ["PROOF-NAND"])
        XCTAssertEqual(filePart.drawer.first, "File Library")
        XCTAssertTrue(store.categories.contains("File Library"))
        XCTAssertEqual(libraryFiles(), [])

        // 3. Right-click → Add to User Library: listed under User Library, one file.
        XCTAssertTrue(UserLibraryPrompts.add([filePart], catalog: store).isEmpty)
        let kept = try XCTUnwrap(store.part(uuid: uuid))
        XCTAssertEqual(kept.drawer.first, "User Library")
        XCTAssertEqual(libraryFiles(), ["\(uuid).schpart"])

        // 4. File › User Library… shows it with manufacturer, kind, package, date.
        let rows = UserLibraryRow.rows(from: store.userLibraryItems)
        XCTAssertEqual(rows.map(\.name), ["PROOF-NAND"])
        XCTAssertEqual(rows.first?.manufacturer, "Texas Instruments")
        XCTAssertFalse(rows.first?.kind.isEmpty ?? true)
        XCTAssertFalse(rows.first?.package.isEmpty ?? true)

        // 5. Place it on a schematic and save the schematic.
        let doc = TestSupport.newDocument()
        TestSupport.openSystemDocument(of: doc, catalog: store, until: self)
        doc.placeChip(part: kept, symbol: symbol(for: kept), centeredAt: CGPoint(x: 200, y: 200))
        let chipID = try XCTUnwrap(doc.chips.last?.id)
        let schematic = try SchematicFile(document: doc, fileURL: nil, catalog: store).encoded()

        // 6. Library window → Edit…: Save updates the part, an open schematic follows at once.
        let library = model(store, uuid)
        XCTAssertNil(library.openError)
        XCTAssertEqual(library.libraryUUID, uuid)
        library.draft.description = "Edited in the library"
        let updated = try library.saveToLibrary()
        XCTAssertEqual(updated.uuid, uuid)
        XCTAssertEqual(library.status, "Updated PROOF-NAND in your User Library")
        XCTAssertEqual(doc.chips.first { $0.id == chipID }?.part.description, "Edited in the library")
        XCTAssertEqual(libraryFiles(), ["\(uuid).schpart"])
        XCTAssertTrue(try String(contentsOf: directory.appendingPathComponent("\(uuid).schpart"),
                                 encoding: .utf8).contains("Edited in the library"))

        // 7. Save As…: a second library part, the first untouched.
        let second = try library.saveAsNewLibraryPart()
        XCTAssertEqual(second.name, "PROOF-NAND.2")
        XCTAssertNotEqual(second.uuid, uuid)
        XCTAssertEqual(library.draft.name, "PROOF-NAND.2")
        XCTAssertEqual(store.part(uuid: uuid)?.description, "Edited in the library")
        XCTAssertEqual(libraryFiles().count, 2)

        // 8. Quit and relaunch: both are listed under User Library with no action.
        let relaunched = launch()
        XCTAssertEqual(UserLibraryRow.rows(from: relaunched.userLibraryItems).map(\.name),
                       ["PROOF-NAND", "PROOF-NAND.2"])
        XCTAssertEqual(relaunched.part(uuid: uuid)?.drawer.first, "User Library")
        XCTAssertTrue(relaunched.userLibraryProblems.isEmpty)

        // 9. Open the schematic saved at step 5 (it embeds the old version under the uuid the
        //    library now holds with other content): it opens, the offer lists its part, "Not Now"
        //    leaves it a File Library part; opening again offers again; Add to Library keeps it.
        let opened = TestSupport.newDocument()
        let embedded = try SchematicFile.decode(schematic).apply(to: opened, catalog: relaunched, fileURL: nil)
        XCTAssertEqual(opened.chips.count, 1)
        XCTAssertEqual(embedded.count, 1)
        let embeddedPart = try XCTUnwrap(embedded.first)
        XCTAssertNotEqual(embeddedPart.uuid, uuid)
        XCTAssertEqual(opened.chips.first?.part.description, "Quad 2-input NAND gate")
        XCTAssertEqual(UserLibraryPrompts.offerable(embedded, catalog: relaunched).map(\.name), [embeddedPart.name])
        XCTAssertEqual(relaunched.listing(of: embeddedPart), .file)
        let again = TestSupport.newDocument()
        let embeddedAgain = try SchematicFile.decode(schematic).apply(to: again, catalog: relaunched, fileURL: nil)
        XCTAssertEqual(embeddedAgain.map(\.uuid), embedded.map(\.uuid))
        XCTAssertEqual(embeddedAgain.map(\.name), embedded.map(\.name))
        XCTAssertEqual(UserLibraryPrompts.offerable(embeddedAgain, catalog: relaunched).count, 1)
        XCTAssertTrue(UserLibraryPrompts.add(embeddedAgain, catalog: relaunched).isEmpty)
        XCTAssertTrue(UserLibraryPrompts.offerable(embeddedAgain, catalog: relaunched).isEmpty)
        XCTAssertEqual(UserLibraryRow.rows(from: relaunched.userLibraryItems).count, 3)

        // 10. Select all three in the window → Remove: none listed, the folder as it began.
        for row in UserLibraryRow.rows(from: relaunched.userLibraryItems) {
            try relaunched.removeFromUserLibrary(uuid: row.id)
        }
        XCTAssertTrue(relaunched.userLibraryItems.isEmpty)
        XCTAssertEqual(libraryFiles(), [])
        XCTAssertFalse(relaunched.allParts.contains { $0.drawer.first == "User Library" })
        XCTAssertTrue(launch().userLibraryItems.isEmpty)
    }

    private func model(_ store: PartCatalogStore, _ uuid: String) -> PartEditorModel {
        let model = PartEditorModel(target: PartEditorTarget(source: .libraryPart(uuid: uuid)), catalog: store)
        Self.keptModels.append(model)
        return model
    }
}
