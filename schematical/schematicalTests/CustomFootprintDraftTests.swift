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
        // 458 before plan-36, which added 24 (3 of them by ball: DRQFN-44/64, WLCSP-12);
        // 564 since the plans after it added theirs (plan-46-J § Measured populations);
        // 571 with the seven nixie tube footprints; 572 with the serial
        // memory kind's one ball-grid footprint (WLCSP-8-ST-M95P32); 582
        // with ten more tube footprints (VFD, numitron, more nixies); 613
        // with the 31 explicit footprints of the regulator packages (plan-42-D / E / F);
        // 638 with the five VFD tube footprints (IV-8, IV-17, IV-22, IV-25, IV-27), the four
        // dekatron base footprints and the crystal packages';
        // 932 with the card-slot footprints of plan-54-H to K (SOCK-, DIMM-, EDGE-; 294 more); 944 with the power switches of plan-55 (12 explicit).
        // 954 with the display-driver footprints of plan-56 that are explicit (10 more).
        // 1691, measured 2026-11-24 from `Parts/footprints.json` (3959 entries, 2268 of them parametric):
        // the count had not been re-measured since 954 while plan-47 (relays, buzzers, inductors, fuses),
        // plan-51 (sockets) and plan-58 (slide switches) added their explicit footprints.
        // 1696, measured 2026-11-26 (plan-68-AC): plans 78-82 (potentiometers, battery holders,
        // addressable LEDs, OLEDs, indicator LEDs) added five explicit footprints.
        XCTAssertEqual(ids.count, 1696)
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

    // MARK: The pad table's edits (plan-68-AC: moved from CustomFootprintEditor)

    func testAddPadTakesTheNextFreePinAndCopiesTheLastPad() {
        var draft = CustomFootprintDraft(id: "CUSTOM-1", description: "d", bodyWidth: 5, bodyHeight: 5, pads: [])
        draft.addPad(pinNumbers: [1, 2])
        XCTAssertEqual(draft.pads, [PadDraft(pin: 1, x: 2.54, y: 0, width: 1.6, height: 1.6, shape: "round", drill: 0.8)])
        draft.pads[0].y = 1
        draft.pads[0].width = 2
        draft.pads[0].shape = "rect"
        draft.pads[0].drill = 0
        draft.addPad(pinNumbers: [1, 2])
        XCTAssertEqual(draft.pads[1], PadDraft(pin: 2, x: 5.08, y: 1, width: 2, height: 1.6, shape: "rect", drill: 0))
        draft.addPad(pinNumbers: [1, 2])
        XCTAssertEqual(draft.pads[2].pin, 0, "every pin has a pad: a centre pad")
        draft.removePad(draft.pads[1].id)
        XCTAssertEqual(draft.pads.map(\.pin), [1, 0])
    }

    func testCornersKeepTheirOrderAndPinChoicesKeepTheOwnPin() {
        var pad = PadDraft(pin: 9, x: 0, y: 0, width: 1, height: 1)
        pad.setCorner("bl", true)
        pad.setCorner("tl", true)
        XCTAssertEqual(pad.corners, ["tl", "bl"])
        pad.setCorner("tl", false)
        XCTAssertEqual(pad.corners, ["bl"])
        XCTAssertEqual(pad.pinChoices([1, 2]), [1, 2, 9], "a pin the pinout lacks stays choosable")
        XCTAssertEqual(pad.pinChoices([1, 9]), [1, 9])
        pad.pin = 0
        XCTAssertEqual(pad.pinChoices([1, 2]), [1, 2], "0 is the centre item")
    }

    func testPadsHashByPinAndPlaceAndAnUnknownKeyIsRefused() {
        var a = PadDraft(pin: 1, x: 0, y: 0, width: 1, height: 1)
        let b = PadDraft(pin: 1, x: 0, y: 0, width: 1, height: 1)
        XCTAssertEqual(a.hashValue, b.hashValue, "ids differ, the pads are the same")
        XCTAssertEqual(Set([a, b]).count, 1)
        a.x = 2
        XCTAssertEqual(Set([a, b]).count, 2)
        let unknown: JSONValue = .object(["pin": .number(1), "x": .number(0), "y": .number(0), "width": .number(1),
                                          "height": .number(1), "colour": .string("red")])
        XCTAssertNil(PadDraft(json: unknown))
        let both: JSONValue = .object(["pin": .number(1), "ball": .string("A1"), "x": .number(0), "y": .number(0),
                                       "width": .number(1), "height": .number(1)])
        XCTAssertNil(PadDraft(json: both), "a pad is numbered or a ball, not both")
    }
}
