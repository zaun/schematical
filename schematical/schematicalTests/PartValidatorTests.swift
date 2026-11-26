import Foundation
import XCTest
@testable import schematical

/// The Part Editor checks a draft with the backend's loader (plan-32-C Phase 3).
@MainActor
final class PartValidatorTests: XCTestCase {
    private func gateDraft() throws -> PartDraft {
        try PartDraft(file: XCTUnwrap(BundledPartFile.copy(name: "SN74HC00N", kind: "gate")))
    }

    /// (a) A valid draft gives the part as it would load.
    func testValidDraft() throws {
        guard case let .success(part) = PartValidator.shared.validate(try gateDraft()) else {
            return XCTFail("SN74HC00N's copy was rejected")
        }
        XCTAssertEqual(part.pins.count, 14)
        XCTAssertEqual(part.name, "SN74HC00N")
    }

    /// (b) A gate on an unknown pin: the message names the pin.
    func testUnknownPinNamesThePin() throws {
        var draft = try gateDraft()
        guard case var .array(gates)? = draft.kindDevice["gates"], case var .object(first)? = gates.first else {
            return XCTFail("no gates")
        }
        first["output"] = .string("NOSUCHPIN")
        gates[0] = .object(first)
        draft.kindDevice["gates"] = .array(gates)
        guard case let .failure(failure) = PartValidator.shared.validate(draft, source: "x.schpart") else {
            return XCTFail("accepted a gate on an unknown pin")
        }
        XCTAssertTrue(failure.message.contains("NOSUCHPIN"), failure.message)
        XCTAssertTrue(failure.message.hasPrefix("x.schpart: devices[0]"), failure.message)
    }

    /// (c) A footprint whose pad count differs from the pinout: the backend's
    /// footprint message.
    func testFootprintPadCountMismatch() throws {
        var draft = try gateDraft()
        draft.footprint = "SOT23-5"
        guard case let .failure(failure) = PartValidator.shared.validate(draft) else {
            return XCTFail("accepted a 5-pad footprint on 14 pins")
        }
        XCTAssertTrue(failure.message.contains("footprint \"SOT23-5\" has 5 pads"), failure.message)
    }

    /// The scratch library is unchanged by any number of checks.
    func testChecksLeaveNothingBehind() throws {
        let draft = try gateDraft()
        for _ in 0..<5 {
            guard case .success = PartValidator.shared.validate(draft) else { return XCTFail("rejected") }
        }
    }

    // MARK: plan-68-AE

    /// A scratch library that could not be set up: every check says so, and
    /// the pickers are empty.
    func testASetupFailureMakesEveryCheckUnavailable() throws {
        struct Broken: LocalizedError { var errorDescription: String? { "no support files" } }
        let validator = PartValidator(loadSupport: { _ in throw Broken() })
        XCTAssertEqual(validator.validate(try gateDraft()), .failure(.unavailable("no support files")))
        XCTAssertEqual(validator.validate(try gateDraft()).failureMessage, "no support files")
        XCTAssertEqual(validator.footprints.count, 0)
        XCTAssertEqual(validator.datasheetIDs, [])
    }

    /// The pickers' lists are read once and kept.
    func testTheFootprintAndDatasheetListsAreKept() {
        let validator = PartValidator()
        let footprints = validator.footprints.map(\.id)
        XCTAssertGreaterThan(footprints.count, 100)
        XCTAssertEqual(validator.footprints.map(\.id), footprints)
        let ids = validator.datasheetIDs
        XCTAssertGreaterThan(ids.count, 100)
        XCTAssertEqual(Set(ids).count, ids.count, "each id once")
        XCTAssertEqual(validator.datasheetIDs, ids)
    }

    /// A draft whose file cannot be written (a number JSON has no text for).
    func testADraftThatCannotBeWritten() throws {
        var draft = try gateDraft()
        draft.kindDevice["zzBad"] = .number(.nan)
        XCTAssertEqual(PartValidator.shared.validate(draft, source: "bad.schpart"),
                       .failure(.backend("bad.schpart: the draft could not be written")))
    }

    /// A file the loader refuses: its message, prefixed with the source.
    func testTextTheLoaderRefuses() async throws {
        guard case let .failure(failure) = PartValidator.shared.validate(text: "{}", source: "empty.schpart") else {
            return XCTFail("accepted an empty object")
        }
        XCTAssertTrue(failure.message.hasPrefix("empty.schpart"), failure.message)
        let off = await PartValidator.shared.validated(try gateDraft(), source: "x.schpart")
        XCTAssertEqual(off.map(\.name), .success("SN74HC00N"))
    }
}

private extension Result where Failure == PartValidator.Failure {
    var failureMessage: String? {
        if case let .failure(failure) = self { return failure.message }
        return nil
    }
}
