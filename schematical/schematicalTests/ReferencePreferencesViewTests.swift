import AppKit
import XCTest
@testable import schematical

/// Settings ▸ Reference Defaults as shown (plan-68-AB): a field per
/// category, a typed prefix stored, a bad one put back, and the reset.
@MainActor
final class ReferencePreferencesViewTests: XCTestCase {
    private var probe: ViewProbe?

    override func setUp() {
        super.setUp()
        // The test host stays up when the test closes its last window.
        AppDelegate.quitsAfterLastWindowCloses = false
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    override func tearDown() {
        probe?.close()
        probe = nil
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
        super.tearDown()
    }

    private var stored: [String: String] {
        ReferencePrefixes.stored(UserDefaults.standard.data(forKey: PreferenceKey.referencePrefixes) ?? Data())
    }

    func testTypingAPrefixStoresItAndResetClearsIt() throws {
        let probe = ViewProbe(ReferencePreferencesView(), size: NSSize(width: 480, height: 1400))
        self.probe = probe
        XCTAssertTrue(probe.has("Reference Prefixes"), "\(probe.labels())")
        XCTAssertTrue(probe.has("Battery"), "\(probe.labels())")
        XCTAssertFalse(probe.textFields().isEmpty)
        XCTAssertFalse(probe.has("Reset to Standard Prefixes"), "nothing stored")

        let battery = try probe.textField(showing: "BT")
        probe.type("bat", into: battery)
        probe.submit(battery)
        XCTAssertEqual(stored, ["battery": "BAT"])
        XCTAssertTrue(probe.wait { battery.stringValue == "BAT" }, battery.stringValue)

        // Not 1 to 8 letters: put back when focus leaves.
        probe.type("S1", into: battery)
        probe.blur()
        XCTAssertEqual(stored, ["battery": "BAT"])
        XCTAssertTrue(probe.wait { battery.stringValue == "BAT" }, battery.stringValue)

        XCTAssertTrue(probe.wait { probe.has("Reset to Standard Prefixes") }, "\(probe.labels())")
        try probe.press("Reset to Standard Prefixes")
        XCTAssertEqual(stored, [:])
        XCTAssertTrue(probe.wait { battery.stringValue == "BT" }, "the field follows the stored prefix")
    }
}
