import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-46-B: a `generic` part (a symbol and a footprint, no simulation)
/// registers like any custom part, lists as unsimulated, numbers as an IC and
/// keeps Simulation › Run off while it is placed.
@MainActor
final class GenericPartTests: XCTestCase {
    /// plan-46-B § 4 (a): three pins, "simulated": false, no supply range.
    static func fileText(uuid: String = UUID().uuidString, name: String) -> String {
        """
        {"format":"schematical-part","version":1,"uuid":"\(uuid)","kind":"generic",
         "devices":[{"id":"custom","function":"Sensor","description":"A sensor","drawer":["Custom"],
           "simulated":false,
           "pinouts":{"default":[{"number":1,"name":"VCC","role":"power"},
                                  {"number":2,"name":"GND","role":"ground"},
                                  {"number":3,"name":"OUT","role":"output"}]},
           "parts":[{"name":"\(name)","manufacturer":"Generic","package":"SOT-23","pinout":"default",
                     "footprint":"SOT23-3"}]}]}
        """
    }

    override func setUp() {
        super.setUp()
        UserDefaults.standard.removeObject(forKey: PreferenceKey.referencePrefixes)
    }

    func testAGenericPartRegistersAsUnsimulated() throws {
        let name = "GENERIC-\(UUID().uuidString.prefix(8))"
        let part = try TestSupport.catalog.register(fileText: Self.fileText(name: name), source: "\(name).schpart")
        XCTAssertEqual(TestSupport.catalog.part(named: name), part)
        XCTAssertEqual(part.kind, PART_GENERIC)
        XCTAssertFalse(part.isSimulated)
        XCTAssertEqual(part.pins.count, 3)
        XCTAssertEqual(ReferenceCategory.of(part), .integratedCircuit)
        XCTAssertEqual(PartKinds.name(of: part.kind), "generic")
        XCTAssertEqual(PartDraft.displayName(of: "generic"), "Other part (no simulation)")

        // It is in the drawer's list.
        XCTAssertTrue(TestSupport.allParts.contains { $0.name == name })

        // Placed, it keeps Run off and is named in the reason.
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, name, at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(SimulationAvailability.unsimulated(doc.chips).map(\.id), [chip.id])
        XCTAssertEqual(SimulationAvailability.reason(doc.chips), "U1 (\(name)) is not simulated")
        XCTAssertFalse(SimulationAvailability.canStart(doc))
    }
}
