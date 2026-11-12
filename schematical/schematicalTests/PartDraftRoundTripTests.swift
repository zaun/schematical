import Foundation
import XCTest
@testable import schematical

/// The Part Editor's draft reproduces every shipped part of a kind exactly,
/// and the backend accepts what it writes (plan-32-C Phase 2; the kinds of
/// plan-32-F … J use `PartKindRoundTrip.check`).
@MainActor
enum PartKindRoundTrip {
    struct Count {
        var checked = 0
        var mismatched: [String] = []
        var withExtras: [String: [String]] = [:]
        var rejected: [String: String] = [:]

        var line: String {
            "checked \(checked) parts, \(mismatched.count) mismatched, \(withExtras.count) with extras, \(rejected.count) rejected"
        }
    }

    /// Copy → draft → encode for every bundled part of `kind`, and the validator on each.
    static func check(kind: String) -> Count {
        var count = Count()
        let validator = PartValidator.shared
        for name in BundledPartFile.partNames(kind: kind) {
            count.checked += 1
            guard let copy = BundledPartFile.copy(name: name, kind: kind),
                  let draft = try? PartDraft(file: copy) else {
                count.mismatched.append(name)
                continue
            }
            if draft.encoded() != copy {
                count.mismatched.append(name)
            }
            let extras = draft.extraKeys
            if !extras.isEmpty {
                count.withExtras[name] = extras
            }
            if case let .failure(failure) = validator.validate(draft, source: "\(name).schpart") {
                count.rejected[name] = failure.message
            }
        }
        return count
    }

    static func report(_ count: Count, kind: String) -> String {
        var text = count.line.replacingOccurrences(of: "parts,", with: "parts of \(kind),")
        if let name = count.mismatched.first { text += "; first mismatch \(name)" }
        if let (name, keys) = count.withExtras.first { text += "; \(name) extras \(keys)" }
        if let (name, message) = count.rejected.first { text += "; \(name) rejected: \(message)" }
        return text
    }
}

@MainActor
final class PartDraftRoundTripTests: XCTestCase {
    /// Every gate part (3438) round-trips through the draft, and the backend
    /// accepts each encoded file.
    func testEveryGatePartRoundTrips() {
        let count = PartKindRoundTrip.check(kind: "gate")
        print(count.line)
        XCTAssertEqual(count.checked, 3438)
        XCTAssertEqual(count.mismatched.count, 0, PartKindRoundTrip.report(count, kind: "gate"))
        XCTAssertEqual(count.rejected.count, 0, PartKindRoundTrip.report(count, kind: "gate"))
        XCTAssertEqual(count.withExtras.count, 0, PartKindRoundTrip.report(count, kind: "gate"))
    }

    /// A draft keeps a key it has no control for, and writes it back.
    func testUnknownKeysAreKept() throws {
        var copy = try XCTUnwrap(BundledPartFile.copy(name: "SN74HC00N", kind: "gate"))
        guard case var .object(root) = copy, case var .array(devices)? = root["devices"],
              case var .object(device)? = devices.first else { return XCTFail("shape") }
        device["futureKey"] = .string("kept")
        devices[0] = .object(device)
        root["devices"] = .array(devices)
        copy = .object(root)
        let draft = try PartDraft(file: copy)
        XCTAssertEqual(draft.extraKeys, ["device.futureKey"])
        XCTAssertEqual(draft.encoded(), copy)
    }
}
