import Foundation
import XCTest
@testable import schematical

/// A custom footprint made in the Part Editor goes into the part file
/// (plan-32-D Phase 3).
@MainActor
final class PartEditorFootprintTests: XCTestCase {
    private func gateWithCustomFootprint() throws -> PartDraft {
        var draft = try PartDraft(file: XCTUnwrap(BundledPartFile.copy(name: "SN74HC00N", kind: "gate")))
        var custom = CustomFootprintDraft.starter(id: CustomFootprintDraft.id(forPart: draft.uuid), pins: 14)
        for index in custom.pads.indices { custom.pads[index].pin = index + 1 }
        draft.customFootprint = custom
        return draft
    }

    /// (a) The saved file has one inline footprint, and the part names it.
    func testCustomFootprintIsSavedInThePartFile() throws {
        let draft = try gateWithCustomFootprint()
        guard case let .object(root) = draft.encoded(), case let .array(footprints)? = root["footprints"],
              case let .array(devices)? = root["devices"], case let .object(device)? = devices.first,
              case let .array(parts)? = device["parts"], case let .object(part)? = parts.first else {
            return XCTFail("shape")
        }
        XCTAssertEqual(footprints.count, 1)
        guard case let .object(entry)? = footprints.first else { return XCTFail("entry") }
        XCTAssertEqual(entry["id"], part["footprint"])
        XCTAssertEqual(entry["style"], .string("explicit"))
        guard case let .success(loaded) = PartValidator.shared.validate(draft) else { return XCTFail("rejected") }
        XCTAssertEqual(loaded.footprint?.id, CustomFootprintDraft.id(forPart: draft.uuid))
        XCTAssertEqual(loaded.footprint?.pads.count, 14)
        // The file reads back to the same draft.
        XCTAssertEqual(try PartDraft(fileText: draft.fileText()), draft)
    }

    /// (b) A pad on a pin the pinout lacks: the backend's message.
    func testPadOnAnUnknownPinIsReported() throws {
        var draft = try gateWithCustomFootprint()
        var custom = try XCTUnwrap(draft.customFootprint)
        custom.pads[13].pin = 99
        draft.customFootprint = custom
        guard case let .failure(failure) = PartValidator.shared.validate(draft, source: "b.schpart") else {
            return XCTFail("accepted a pad on pin 99")
        }
        XCTAssertTrue(failure.message.hasPrefix("b.schpart: "), failure.message)
        // The pins key counts 14 numbered pads, so pin 99 is out of range; the message names the pad.
        XCTAssertTrue(failure.message.contains("pads[13]"), failure.message)
    }

    /// (c) Removing the custom footprint removes the `footprints` key.
    func testRemovingTheCustomFootprint() throws {
        var draft = try gateWithCustomFootprint()
        draft.customFootprint = nil
        guard case let .object(root) = draft.encoded() else { return XCTFail("shape") }
        XCTAssertNil(root["footprints"])
        XCTAssertNil(draft.footprint)
    }
}
