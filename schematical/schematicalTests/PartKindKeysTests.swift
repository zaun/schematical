import Foundation
import XCTest
@testable import schematical

/// The Part Editor's form has a control for every key the backend accepts
/// for a kind (plan-32-F … J, the drift test): the `KEYS[] = { … }` arrays of
/// the kind's model sources, against `PartDraft.formKeys(kind)`.
final class PartKindKeysTests: XCTestCase {
    /// Each kind's model sources under backend/src/models: its own files, the
    /// shared helpers its loader calls (seq.c for the sequential kinds; the
    /// converter loaders for rtc).
    static let files: [String: [String]] = [
        "gate": ["gate.c"],
        "dff": ["dff.c"], "jkff": ["jkff.c"],
        "mux": ["mux.c", "mux_stored.c", "seq.c"], "decoder": ["decoder.c", "seq.c"],
    ]

    static var models: URL {
        URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("backend/src/models")
    }

    /// The string keys of every `KEYS[] = { … }` array in a source file.
    static func keyArrays(in file: String) throws -> [[String]] {
        let text = try String(contentsOf: models.appendingPathComponent(file), encoding: .utf8)
        let array = try NSRegularExpression(pattern: #"KEYS\[\]\s*=\s*\{(.*?)\}"#, options: [.dotMatchesLineSeparators])
        let literal = try NSRegularExpression(pattern: #""([^"]*)""#)
        return array.matches(in: text, range: NSRange(text.startIndex..., in: text)).map { match in
            let body = String(text[Range(match.range(at: 1), in: text)!])
            return literal.matches(in: body, range: NSRange(body.startIndex..., in: body)).map {
                String(body[Range($0.range(at: 1), in: body)!])
            }
        }
    }

    /// Keys of a shared helper in seq.c that only that helper reads: a kind
    /// whose own sources never call the helper (in that mode) does not accept
    /// them, so the form needs no control for them. Derived from the sources.
    static func helperOnlyKeys(kind: String) throws -> Set<String> {
        let own = try Self.files[kind]!.filter { $0 != "seq.c" }.map {
            try String(contentsOf: models.appendingPathComponent($0), encoding: .utf8)
        }.joined(separator: "\n")
        guard Self.files[kind]!.contains("seq.c") else { return [] }
        var exempt = Set<String>()
        if !own.contains("seq_gate_name(") { exempt.insert("op") }
        if !own.contains("seq_clock_name(") { exempt.insert("edge") }
        // "when" only with allow_when set: seq_control_name(ctx, json, key, required, 1, …)
        // or seq_control_object(ctx, item, 1, …).
        let when = try NSRegularExpression(
            pattern: #"seq_control_name\([^;]*?"[A-Za-z]+",\s*[01],\s*1\s*,|seq_control_object\(\s*ctx,\s*\w+,\s*1\s*,"#)
        if when.firstMatch(in: own, range: NSRange(own.startIndex..., in: own)) == nil { exempt.insert("when") }
        return exempt
    }

    func testFormKeysCoverTheBackendKeys() throws {
        var missing: [String: [String]] = [:]
        for kind in Self.files.keys.sorted() {
            var keys = Set<String>()
            var arrays = 0
            for file in Self.files[kind]! {
                let found = try Self.keyArrays(in: file)
                arrays += found.count
                keys.formUnion(found.flatMap { $0 })
            }
            let exempt = try Self.helperOnlyKeys(kind: kind)
            print("\(kind): \(arrays) KEYS arrays, \(keys.count) keys, seq.c helper keys not read: \(exempt.sorted())")
            let lacking = keys.subtracting(PartDraft.formKeys(kind)).subtracting(exempt).sorted()
            if !lacking.isEmpty { missing[kind] = lacking }
        }
        XCTAssertEqual(missing, [:], "keys the form has no control for: \(missing)")
    }
}
