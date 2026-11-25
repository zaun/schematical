import Foundation
import XCTest
@testable import schematical

/// plan-51-G: the `socket` kind in the Part Editor and the Part Wizard. The
/// form edits the common part key `fits` (the chip footprints a socket holds)
/// beside the footprint; a draft round-trips with it, and a socket without it
/// is refused by the loader with a message the wizard places on its Footprint
/// page.
@MainActor
final class SocketEditorTests: XCTestCase {
    private func copy(_ name: String) throws -> JSONValue {
        try XCTUnwrap(BundledPartFile.copy(name: name, kind: "socket"), name)
    }

    private func draft(_ name: String = "GENERIC-SOCKET-DIP-14-300") throws -> PartDraft {
        try PartDraft(file: copy(name))
    }

    /// The `fits` list of the first part of an encoded file.
    private func fits(of file: JSONValue) -> [String]? {
        guard case let .object(root) = file, case let .array(devices)? = root["devices"],
              case let .object(device)? = devices.first, case let .array(parts)? = device["parts"],
              case let .object(part)? = parts.first, case let .array(items)? = part["fits"] else { return nil }
        return items.compactMap { item in
            if case let .string(id) = item { return id }
            return nil
        }
    }

    func testTheKindIsOfferedAndHasAnInfoCard() {
        XCTAssertTrue(PartDraft.enabledKinds.contains("socket"))
        let info = PartKinds.info["socket"]
        XCTAssertEqual(info?.name, "IC Socket")
        XCTAssertEqual(info?.group, .connectors)
        XCTAssertEqual(info?.examples, ["GENERIC-SOCKET-DIP-14-300", "GENERIC-SOCKET-PLCC-44"])
        XCTAssertTrue(PartKinds.kinds(in: .connectors).contains { $0.kind == "socket" })
        XCTAssertEqual(PartDraft(newOf: "socket").kind, "socket")
    }

    func testAGenericSocketDraftEncodesToTheShippedFile() throws {
        let file = try copy("GENERIC-SOCKET-DIP-14-300")
        let draft = try PartDraft(file: file)
        XCTAssertEqual(draft.kind, "socket")
        XCTAssertEqual(draft.fits, ["PDIP-14", "CDIP-14"])
        XCTAssertEqual(draft.encoded(), file)
        XCTAssertTrue(draft.extraKeys.isEmpty, "\(draft.extraKeys)")
        if case let .failure(failure) = PartValidator.shared.validate(draft, source: "socket.schpart") {
            XCTFail(failure.message)
        }
    }

    func testAPlccSocketKeepsItsFamilyAndNoRowSpacing() throws {
        let file = try copy("GENERIC-SOCKET-PLCC-44")
        let draft = try PartDraft(file: file)
        XCTAssertEqual(draft.kindDevice["family"], .string("PLCC"))
        XCTAssertNil(draft.kindDevice["rowSpacing"])
        XCTAssertEqual(draft.kindPart["mounting"], .string("surface-mount"))
        XCTAssertEqual(draft.encoded(), file)
    }

    func testUncheckingAFitSavesTheOthers() throws {
        var draft = try draft()
        draft.fits = FitsEditor.toggled(draft.fits, "CDIP-14", on: false)
        XCTAssertEqual(draft.fits, ["PDIP-14"])
        XCTAssertEqual(fits(of: draft.encoded()), ["PDIP-14"])
        if case let .failure(failure) = PartValidator.shared.validate(draft, source: "socket.schpart") {
            XCTFail(failure.message)
        }
        draft.fits = FitsEditor.toggled(draft.fits, "CDIP-14", on: true)
        XCTAssertEqual(draft.fits, ["PDIP-14", "CDIP-14"])
        // Checking one that is on does not list it twice.
        XCTAssertEqual(FitsEditor.toggled(draft.fits, "PDIP-14", on: true), ["PDIP-14", "CDIP-14"])
    }

    func testASocketWithNoFitsIsRefusedAndPlacedOnTheFootprintPage() throws {
        var draft = try draft()
        draft.fits = FitsEditor.toggled(FitsEditor.toggled(draft.fits, "PDIP-14", on: false), "CDIP-14", on: false)
        XCTAssertNil(draft.fits, "nothing left means the key is absent")
        XCTAssertNil(fits(of: draft.encoded()))
        guard case let .failure(failure) = PartValidator.shared.validate(draft, source: "socket.schpart") else {
            return XCTFail("the backend accepted a socket with no fits")
        }
        XCTAssertTrue(failure.message.contains("missing \"fits\""), failure.message)
        let steps = WizardPlan.steps(for: draft)
        let step = WizardIssueLocator.step(for: failure.message, steps: steps, draft: draft)
        XCTAssertNotNil(step)
        XCTAssertEqual(step, steps.first { $0.page == .footprint }?.id)
    }

    func testAFitsFootprintOfAnotherSizeIsRefusedWithTheLoadersMessage() throws {
        var draft = try draft()
        draft.fits = ["PDIP-8"]
        guard case let .failure(failure) = PartValidator.shared.validate(draft, source: "socket.schpart") else {
            return XCTFail("the backend accepted an 8-pin footprint on a 14-pin socket")
        }
        XCTAssertTrue(failure.message.contains("fits footprint \"PDIP-8\" has 8 pins; the part has 14"), failure.message)
    }

    func testTheFitsListOffersOnlyFootprintsWithThePartsPads() throws {
        let draft = try draft()
        let offered = FitsEditor.choices(PartValidator.shared.footprints, pins: draft.pinCount, own: draft.footprint)
        let ids = offered.map(\.id)
        XCTAssertTrue(offered.allSatisfy { Int($0.pins) == 14 })
        XCTAssertTrue(ids.contains("PDIP-14"))
        XCTAssertTrue(ids.contains("CDIP-14"))
        XCTAssertFalse(ids.contains { $0.hasPrefix("SKT-") }, "a socket does not hold a socket")
        XCTAssertFalse(ids.contains(draft.footprint ?? ""), "the part's own footprint is not offered")
    }

    func testEveryShippedSocketRoundTripsWithItsFits() {
        let count = PartKindRoundTrip.check(kind: "socket")
        XCTAssertEqual(count.checked, 575)
        XCTAssertEqual(count.mismatched.count, 0, PartKindRoundTrip.report(count, kind: "socket"))
        XCTAssertEqual(count.withExtras.count, 0, PartKindRoundTrip.report(count, kind: "socket"))
        XCTAssertEqual(count.rejected.count, 0, PartKindRoundTrip.report(count, kind: "socket"))
    }
}
