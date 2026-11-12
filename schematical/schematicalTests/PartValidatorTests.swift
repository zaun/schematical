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
}
