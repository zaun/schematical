import Foundation
import XCTest
@testable import schematical

/// A custom footprint's draft writes the `explicit` style's keys as the
/// backend reads them (plan-32-D Phase 1): every shipped explicit footprint,
/// read into a draft and written back, loads to the same pads.
@MainActor
final class CustomFootprintDraftTests: XCTestCase {
    struct LoadFailure: Error { let message: String }

    /// Ids of the shipped footprints of the `explicit` style.
    private func explicitIDs() throws -> [String] {
        let url = try XCTUnwrap(Bundle.main.url(forResource: "footprints", withExtension: "json"))
        guard case let .object(root) = try JSONDecoder().decode(JSONValue.self, from: Data(contentsOf: url)),
              case let .array(entries)? = root["footprints"] else { throw XCTSkip("no footprints") }
        return entries.compactMap { entry in
            guard case let .object(keys) = entry, keys["style"] == .string("explicit"),
                  case let .string(id)? = keys["id"] else { return nil }
            return id
        }
    }

    /// Loads one entry on its own; the expanded footprint, or the loader's message.
    static func load(_ entry: JSONValue) -> Result<Footprint, LoadFailure> {
        let library = PartLibraryHandle()
        let file: JSONValue = .object(["format": .string("schematical-footprints"), "version": .number(1),
                                       "footprints": .array([entry])])
        guard let text = try? file.text() else { return .failure(LoadFailure(message: "could not write")) }
        var error: UnsafeMutablePointer<CChar>?
        if load_footprints(library.pointer, text, "custom", &error) != 0 {
            let message = error.map { String(cString: $0) } ?? "?"
            free(error)
            return .failure(LoadFailure(message: message))
        }
        guard let footprint = part_library_footprint_at(library.pointer, 0) else { return .failure(LoadFailure(message: "none")) }
        return .success(Footprint(footprint.pointee))
    }

    func testEveryExplicitFootprintRoundTrips() throws {
        let ids = try explicitIDs()
        let bundled = Dictionary(PartValidator.shared.footprints.map { ($0.id, $0) }, uniquingKeysWith: { a, _ in a })
        var mismatched: [String] = []
        var rejected: [String: String] = [:]
        for id in ids {
            guard let original = bundled[id] else {
                mismatched.append(id)
                continue
            }
            let draft = CustomFootprintDraft(footprint: original)
            switch Self.load(draft.encoded()) {
            case let .failure(failure):
                rejected[id] = failure.message
            case let .success(loaded):
                if loaded.pads != original.pads || loaded.body != original.body
                    || loaded.bodyRound != original.bodyRound || loaded.description != original.description {
                    mismatched.append(id)
                }
            }
            // The entry read back from JSON is the same draft.
            if CustomFootprintDraft(json: draft.encoded()) != draft {
                mismatched.append("\(id) (json)")
            }
        }
        print("checked \(ids.count) explicit footprints, \(mismatched.count + rejected.count) mismatched")
        XCTAssertEqual(ids.count, 458)
        XCTAssertEqual(mismatched, [])
        XCTAssertEqual(rejected, [:])
    }

    /// The starter footprint loads, with one pad per pin.
    func testStarterFootprintLoads() throws {
        let draft = CustomFootprintDraft.starter(id: "CUSTOM-ABCDEF12", pins: 14)
        guard case let .success(footprint) = Self.load(draft.encoded()) else { return XCTFail("rejected") }
        XCTAssertEqual(footprint.pads.count, 14)
        XCTAssertEqual(CustomFootprintDraft.id(forPart: "6f1c2e8a-3b7d"), "CUSTOM-6F1C2E8A")
    }
}
