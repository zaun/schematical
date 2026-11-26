import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-AC: the custom footprint's form as it is shown: its fields, the
/// pad table, Add Pad, a pad's remove button and corner toggles, and Remove
/// Custom Footprint.
@MainActor
final class CustomFootprintEditorTests: XCTestCase {
    @MainActor @Observable
    final class Box {
        var draft: CustomFootprintDraft
        var removed = 0
        init(_ draft: CustomFootprintDraft) { self.draft = draft }
        nonisolated deinit {}
    }

    private var probes: [ViewProbe] = []

    override func setUp() async throws {
        // Closing a test window must not quit the test host.
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    override func tearDown() async throws {
        probes.forEach { $0.close() }
        probes = []
    }

    private func show(_ box: Box, pins: [Int]) -> ViewProbe {
        let probe = ViewProbe(Form {
            CustomFootprintEditor(footprint: Binding(get: { box.draft }, set: { box.draft = $0 }), pinNumbers: pins,
                                  onRemove: { box.removed += 1 })
        }.formStyle(.grouped), size: NSSize(width: 1100, height: 600))
        probes.append(probe)
        return probe
    }

    func testTheFormShowsTheFootprintAndItsPads() {
        var draft = CustomFootprintDraft.starter(id: "CUSTOM-TEST0001", pins: 2)
        draft.pads[1].pin = 7
        let probe = show(Box(draft), pins: [1, 2])
        XCTAssertTrue(probe.has("CUSTOM-TEST0001"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Custom footprint"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Edge mount"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Chamfer"), "\(probe.labels())")
        XCTAssertEqual(probe.find("Remove Pad", role: "AXButton").count, 2, "\(probe.labels())")
    }

    func testAddAndRemovePadsAndCutACorner() throws {
        let box = Box(CustomFootprintDraft.starter(id: "CUSTOM-TEST0002", pins: 2))
        let probe = show(box, pins: [1, 2, 3])
        try probe.press("Add Pad")
        XCTAssertEqual(box.draft.pads.map(\.pin), [1, 2, 3])
        XCTAssertEqual(probe.find("Remove Pad", role: "AXButton").count, 3, "\(probe.labels())")

        let first = box.draft.pads[0].id
        let remove = try XCTUnwrap(probe.find("Remove Pad", role: "AXButton").first)
        _ = remove.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(box.draft.pads.count, 2)
        XCTAssertFalse(box.draft.pads.contains { $0.id == first })

        let corner = try XCTUnwrap(probe.find("br").first { ["AXCheckBox", "AXButton", "AXToggle"].contains(ViewProbe.role($0)) },
                                   "\(probe.labels())")
        _ = corner.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
        XCTAssertEqual(box.draft.pads[0].corners, ["br"])

        try probe.press("Remove Custom Footprint")
        XCTAssertEqual(box.removed, 1)
    }

    func testTheBodySizeFieldsWriteTheDraft() throws {
        let box = Box(CustomFootprintDraft.starter(id: "CUSTOM-TEST0003", pins: 1))
        let probe = show(box, pins: [1])
        try probe.pressControl("Round")
        XCTAssertTrue(box.draft.bodyRound)
        try probe.pressControl("Edge mount")
        XCTAssertTrue(box.draft.edgeMount)
    }
}
