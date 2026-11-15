import Foundation
import XCTest
@testable import schematical

/// The example boards plan-38's router is measured on (AFU, LFU, RegFile),
/// exact copies of `planning/plan-38-data/fixtures/`.
@MainActor
enum Fixture {
    static let names = ["AFU", "LFU", "RegFile"]

    /// `schematicalTests/Fixtures/`.
    static var directory: URL {
        URL(fileURLWithPath: #filePath).deletingLastPathComponent().appendingPathComponent("Fixtures")
    }

    static func url(_ name: String) -> URL {
        directory.appendingPathComponent("\(name).schematical")
    }

    /// A fresh document with fixture `name` opened in it.
    static func open(_ name: String) throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        try SchematicFile.decode(Data(contentsOf: url(name))).apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        return doc
    }
}
