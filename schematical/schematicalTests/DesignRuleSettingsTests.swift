import Foundation
import XCTest
@testable import schematical

/// Settings ▸ DRC Defaults: the manufacturer picks the rules; Custom's are
/// stored and kept while another manufacturer is picked.
final class DesignRuleSettingsTests: XCTestCase {
    func testCurrentRules() throws {
        let suite = "DesignRuleSettingsTests.\(UUID().uuidString)"
        let defaults = try XCTUnwrap(UserDefaults(suiteName: suite))
        defer { defaults.removePersistentDomain(forName: suite) }

        // Never set: Custom, starting from PCBWay's values.
        XCTAssertEqual(DesignRules.current(defaults: defaults), .customDefault)

        var custom = DesignRules.customDefault
        custom.minTraceWidth = 0.2
        defaults.set(custom.encoded, forKey: PreferenceKey.drcCustomRules)
        XCTAssertEqual(DesignRules.current(defaults: defaults), custom)

        // PCBWay ignores the stored Custom rules.
        defaults.set(DRCManufacturer.pcbway.rawValue, forKey: PreferenceKey.drcManufacturer)
        XCTAssertEqual(DesignRules.current(defaults: defaults), .pcbway)
        XCTAssertEqual(DesignRules.pcbway.minTraceWidth, 0.1)
        defaults.set(DRCManufacturer.jlcpcb.rawValue, forKey: PreferenceKey.drcManufacturer)
        XCTAssertEqual(DesignRules.current(defaults: defaults), .jlcpcb)

        // Back to Custom: the edit is still there.
        defaults.set(DRCManufacturer.custom.rawValue, forKey: PreferenceKey.drcManufacturer)
        XCTAssertEqual(DesignRules.current(defaults: defaults), custom)

        // Rules that don't decode fall back to the default.
        defaults.set(Data("x".utf8), forKey: PreferenceKey.drcCustomRules)
        XCTAssertEqual(DesignRules.current(defaults: defaults), .customDefault)
    }

    /// Every manufacturer has rules, and they hang together.
    func testManufacturerRules() {
        for manufacturer in DRCManufacturer.allCases where manufacturer != .custom {
            let name = manufacturer.title
            guard let rules = manufacturer.fixedRules else {
                return XCTFail("\(name) has no rules")
            }
            for group in DesignRules.groups {
                for rule in group.rules {
                    let value = rules[keyPath: rule.field]
                    if rule.field == \DesignRules.solderMaskExpansion {
                        XCTAssertGreaterThanOrEqual(value, 0, "\(name) \(rule.title)")
                    } else {
                        XCTAssertGreaterThan(value, 0, "\(name) \(rule.title)")
                    }
                }
            }
            XCTAssertGreaterThan(rules.minViaDiameter, rules.minViaDrill, name)
        }
        XCTAssertNil(DRCManufacturer.custom.fixedRules)
    }
}
