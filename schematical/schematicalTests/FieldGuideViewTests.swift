import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AF: the guidance under a Guided-mode field as it is shown — its
/// help, where to find it with the Find button, its other names and its
/// example — and the one-line "Usually under" text.
@MainActor
final class FieldGuideViewTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    private func show(_ guide: FieldGuide, examplePlaced: Bool = false,
                      find: (([String]) -> Void)? = nil) -> ViewProbe {
        let probe = ViewProbe(FieldGuideView(guide: guide, examplePlaced: examplePlaced)
            .environment(\.datasheetFind, find).padding(), size: NSSize(width: 520, height: 200))
        probes.append(probe)
        return probe
    }

    private let full = FieldGuide(label: "Supply", help: "The supply range.",
                                  whereToFind: ["Recommended Operating Conditions", "Ratings"],
                                  alsoCalled: ["VCC", "VDD"], example: "2 to 6 V")

    func testEverythingAGuideSaysIsShown() throws {
        var asked: [[String]] = []
        let probe = show(full) { asked.append($0) }
        XCTAssertTrue(probe.has("The supply range."), "\(probe.labels())")
        XCTAssertTrue(probe.has("Usually under: Recommended Operating Conditions (also: Ratings)"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Also called: VCC, VDD"), "\(probe.labels())")
        XCTAssertTrue(probe.has("For example: 2 to 6 V"), "\(probe.labels())")
        try probe.press("Find Recommended Operating Conditions in the datasheet")
        XCTAssertEqual(asked, [["Recommended Operating Conditions", "Ratings"]])
    }

    func testOutsideTheWizardThereIsNoFindButton() {
        let probe = show(full)
        XCTAssertTrue(probe.has("Usually under: Recommended Operating Conditions"), "\(probe.labels())")
        XCTAssertFalse(probe.has("in the datasheet", role: "AXButton"), "\(probe.labels())")
    }

    func testAPlacedExampleAndMissingPartsAreLeftOut() {
        let probe = show(full, examplePlaced: true)
        XCTAssertFalse(probe.has("For example"), "the placeholder shows it: \(probe.labels())")
        let bare = show(FieldGuide(help: "Only help."))
        XCTAssertTrue(bare.has("Only help."), "\(bare.labels())")
        XCTAssertFalse(bare.has("Usually under"), "\(bare.labels())")
        XCTAssertFalse(bare.has("Also called"), "\(bare.labels())")
        XCTAssertFalse(bare.has("For example"), "\(bare.labels())")
    }

    func testTheExampleShowsUnlessPlaced() {
        XCTAssertFalse(FieldGuideView(guide: full).examplePlaced, "shown by default")
    }

    func testALaterGuideOverridesAnEarlierOne() {
        let base = ["a": FieldGuide(help: "old"), "b": FieldGuide(help: "kept")]
        let merged = base.overridden(by: ["a": FieldGuide(help: "new"), "c": FieldGuide(help: "added")])
        XCTAssertEqual(merged.mapValues(\.help), ["a": "new", "b": "kept", "c": "added"])
        // The flip-flop guides add their own keys to the shared timing ones.
        XCTAssertNotNil(DffGuide.guide.fields["device.clockEdge"])
        XCTAssertNotNil(DffGuide.guide.fields["device.timing"])
    }

    func testAListOfPinsIsPinsOnly() {
        XCTAssertTrue(GuideLayout.isPinsOnly(.listOf(.pin)))
        XCTAssertFalse(GuideLayout.isPinsOnly(.listOf(.text)))
    }

    func testTheWhereLineNamesTheOthersOnlyWhenThereAreSome() {
        XCTAssertEqual(Guidance.whereLine(FieldGuide(help: "", whereToFind: ["Pin Configuration"])),
                       "Usually under: Pin Configuration")
        XCTAssertEqual(Guidance.whereLine(full), "Usually under: Recommended Operating Conditions (also: Ratings)")
        XCTAssertNil(Guidance.whereLine(FieldGuide(help: "")))
    }

    func testOnlySingleTextControlsTakeAPlaceholder() {
        XCTAssertEqual([FieldType.text, .note, .number(unit: "V"), .integer, .suggested(["a"])].map(\.takesPlaceholder),
                       [true, true, true, true, true])
        XCTAssertEqual([FieldType.bool, .choice(["a"]), .pin, .pins, .names].map(\.takesPlaceholder),
                       [false, false, false, false, false])
    }

    func testAControlKeyFallsBackToTheSharedControlGuide() {
        XCTAssertEqual(Guidance.guide(kind: "nosuchkind", path: "device.clear.pin")?.label, "Pin")
        XCTAssertEqual(Guidance.guide(kind: "nosuchkind", path: "vcc")?.label, "Supply voltage range")
        XCTAssertNil(Guidance.guide(kind: "nosuchkind", path: "device.zzz"))
        XCTAssertFalse(StepGuides.footprint.isEmpty)
    }
}
