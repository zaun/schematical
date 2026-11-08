import SwiftUI
import XCTest
@testable import schematical

/// plan-28-A: ink roles and the two palettes.
final class SchematicPainterTests: XCTestCase {
    /// Every role, one of each kind of associated value.
    private let roles: [Ink.Role] = [
        .primary, .secondary, .accent, .paper, .power, .ground, .switchOn, .knobOn,
        .unlit, .displayFace, .faceEdge, .lamp("Red"), .display(0.1, 0.2, 0.3), .color(.red),
    ]

    /// The export alpha of every role, as plan-28-A § 3.1 lists it.
    func testExportAlphaPerRole() {
        let expected: [(Ink.Role, Double?)] = [
            (.primary, 1), (.secondary, 0.6), (.accent, 1), (.paper, nil), (.power, 1), (.ground, 1),
            (.switchOn, 0.25), (.knobOn, 1), (.unlit, 0.15), (.displayFace, nil), (.faceEdge, 1),
            (.lamp("Red"), 1), (.lamp("White"), 1), (.display(0.1, 0.2, 0.3), nil), (.color(.red), nil),
        ]
        for (role, alpha) in expected {
            XCTAssertEqual(ExportPalette.alpha(for: role), alpha, "\(role)")
        }
    }

    /// An ink's export alpha is the role's alpha times its opacity.
    func testExportAlphaTimesOpacity() {
        XCTAssertEqual(ExportPalette.alpha(for: Ink.secondary.opacity(0.5)), 0.3)
        XCTAssertEqual(ExportPalette.alpha(for: Ink.primary.opacity(0.5)), 0.5)
        XCTAssertNil(ExportPalette.alpha(for: Ink.paper.opacity(0.5)))
    }

    func testOpacityMultiplies() {
        XCTAssertEqual(Ink.primary.opacity(0.5).opacity(0.5).opacity, 0.25)
        XCTAssertEqual(Ink.primary.opacity, 1)
        XCTAssertEqual(Ink.lamp("Red").opacity(0.4).role, .lamp("Red"))
    }

    /// The screen draws every role but the face outline; the export draws
    /// exactly the roles with an export alpha, all in black.
    func testPalettesDrawWhatTheyShould() {
        for role in roles {
            let ink = Ink(role: role)
            if role == .faceEdge {
                XCTAssertNil(ScreenPalette.screen.color(for: ink), "\(role)")
            } else {
                XCTAssertNotNil(ScreenPalette.screen.color(for: ink), "\(role)")
            }
            XCTAssertEqual(ScreenPalette.export.color(for: ink) == nil, ExportPalette.alpha(for: role) == nil, "\(role)")
        }
        XCTAssertEqual(ScreenPalette.export.color(for: .primary), Color.black)
        XCTAssertEqual(ScreenPalette.export.color(for: .secondary), Color.black.opacity(0.6))
    }
}
