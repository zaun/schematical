import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// Every generic body the board preview gives a catalog part (plan-68-Z): each
/// look the catalog's parts produce, with its body along x and along y, is
/// built and drawn top-down, angled and turned on the bottom side, and
/// draws something (a test point draws nothing: its pad is the board's).
@MainActor
final class PartLookTests: XCTestCase {
    nonisolated deinit {}

    /// The look's case, with its flags and the body's long axis.
    static func name(of look: PartLook) -> String {
        let axis = look.footprint.body.width >= look.footprint.body.height ? "x" : "y"
        let kind: String
        switch look.kind {
        case let .dip(ceramic): kind = "dip" + (ceramic ? " ceramic" : "")
        case let .leadedIC(ceramic): kind = "leadedIC" + (ceramic ? " ceramic" : "")
        case let .leadlessIC(ceramic): kind = "leadlessIC" + (ceramic ? " ceramic" : "")
        case .chipResistor: kind = "chipResistor"
        case .chipCapacitor: kind = "chipCapacitor"
        case .chipLED: kind = "chipLED"
        case .moldedTantalum: kind = "moldedTantalum"
        case .axialResistor: kind = "axialResistor"
        case let .axialDiode(glass): kind = "axialDiode" + (glass ? " glass" : "")
        case .smdDiode: kind = "smdDiode"
        case let .electrolytic(smd): kind = "electrolytic" + (smd ? " smd" : "")
        case .tantalumDrop: kind = "tantalumDrop"
        case .upright: kind = "upright"
        case .radialLED: kind = "radialLED"
        case .squareLED: kind = "squareLED"
        case .sevenSegment: kind = "sevenSegment"
        case let .nixie(endView, _): kind = "nixie" + (endView ? " end" : "") + (look.footprint.bodyRound ? " round" : "")
        case .lcd: kind = "lcd"
        case .dipSwitch: kind = "dipSwitch"
        case .tactile: kind = "tactile"
        case let .header(female): kind = "header" + (female ? " female" : "")
        case .canOscillator: kind = "canOscillator"
        case .smdOscillator: kind = "smdOscillator"
        case let .usb(type): kind = "usb " + (type ?? "-")
        case .cardSlot: kind = "cardSlot"
        case .terminalBlock: kind = "terminalBlock"
        case .jstHeader: kind = "jstHeader"
        case .testPad: kind = "testPad"
        case .barrel: kind = "barrel"
        case .toPackage: kind = "toPackage"
        case .block: kind = "block"
        }
        return kind + " " + axis
    }

    /// A placed chip of `part` on the board's top, centred at the origin,
    /// with `footprintID` (one of its choices) and its options.
    static func chip(_ part: CatalogPart, footprintID: String? = nil, gender: String? = nil,
                     side: BoardSide = .top, rotation: Int = 0) -> PlacedChip {
        var placement = BoardPlacement(centre: .zero, rotation: rotation)
        placement.side = side
        return PlacedChip(reference: "U1", part: part, symbol: symbol(for: part), origin: .zero,
                          switchesOn: part.kind == PART_SWITCH ? [true, false, true, false] : [],
                          board: placement, color: part.colors.first, footprintID: footprintID, gender: gender)
    }

    /// One look of each name, from the catalog: every part, every footprint
    /// choice, both header genders.
    static func survey() -> [String: PartLook] {
        var found: [String: PartLook] = [:]
        for part in TestSupport.catalog.allParts {
            var chips: [PlacedChip] = []
            if part.footprint != nil { chips.append(chip(part)) }
            for choice in part.footprintChoices { chips.append(chip(part, footprintID: choice.footprint.id)) }
            for gender in part.genders where part.footprint != nil { chips.append(chip(part, gender: gender)) }
            for chip in chips where chip.footprint != nil {
                let look = PartLook.of(chip)
                let key = name(of: look)
                if found[key] == nil { found[key] = look }
            }
        }
        return found
    }

    /// How many pixels the look's solids draw on a clear picture.
    static func drawn(_ look: PartLook, angle: BoardRenderAngle) throws -> Int {
        let span = max(look.footprint.body.width, look.footprint.body.height) * 0.9 + 1
        let outline = [CGPoint(x: -span, y: -span), CGPoint(x: span, y: -span), CGPoint(x: span, y: span),
                       CGPoint(x: -span, y: span)]
        let canvas = Canvas { context, size in
            let projection = RenderProjection.fitting(outline, face: look.placement.side == .bottom ? .back : .front,
                                                      angle: angle, size: size, height: look.height, below: 0)
            let solids = look.solids(painter: SolidPainter(projection: projection)).sorted { $0.depth < $1.depth }
            for solid in solids { solid.draw(&context) }
        }
        .frame(width: 160, height: 160)
        let image = try XCTUnwrap(ImageRenderer(content: canvas).cgImage)
        let data = try XCTUnwrap(image.dataProvider?.data as Data?)
        let bytes = image.bitsPerPixel / 8
        let alpha = image.alphaInfo == .premultipliedFirst || image.alphaInfo == .first ? 0 : bytes - 1
        var count = 0
        for row in 0..<image.height {
            for column in 0..<image.width where data[row * image.bytesPerRow + column * bytes + alpha] > 0 {
                count += 1
            }
        }
        return count
    }

    func testEveryLookDraws() throws {
        let looks = Self.survey()
        print("PartLookTests: \(looks.count) looks: \(looks.keys.sorted())")
        // Each family the preview knows is in the catalog.
        for family in ["dip x", "leadedIC", "leadlessIC", "chipResistor", "chipCapacitor", "chipLED", "axialResistor",
                       "axialDiode", "smdDiode", "electrolytic", "upright", "radialLED", "sevenSegment", "nixie", "lcd",
                       "dipSwitch", "tactile", "header", "header female", "canOscillator", "smdOscillator", "usb",
                       "terminalBlock", "jstHeader", "testPad", "barrel", "toPackage", "block"] {
            XCTAssertTrue(looks.keys.contains { $0.hasPrefix(family) }, "no catalog part looks like \(family)")
        }
        for (key, look) in looks.sorted(by: { $0.key < $1.key }) {
            let flat = try Self.drawn(look, angle: .top)
            let angled = try Self.drawn(look, angle: .angled)
            var turned = look.chip
            turned.board?.side = .bottom
            turned.board?.rotation = 90
            let under = try Self.drawn(PartLook.of(turned), angle: .angled)
            if case .testPad = look.kind {
                XCTAssertEqual(look.height, 0, key)
                XCTAssertEqual([flat, angled, under], [0, 0, 0], key)
            } else {
                XCTAssertGreaterThan(look.height, 0, key)
                XCTAssertGreaterThan(flat, 20, "\(key) (\(look.chip.part.name)) top-down")
                XCTAssertGreaterThan(angled, 20, "\(key) (\(look.chip.part.name)) angled")
                XCTAssertGreaterThan(under, 20, "\(key) (\(look.chip.part.name)) on the bottom")
            }
        }
    }

    /// Heights by family, as the look gives them.
    func testHeights() throws {
        let looks = Self.survey()
        func height(_ prefix: String) throws -> CGFloat {
            try XCTUnwrap(looks.first { $0.key.hasPrefix(prefix) }?.value.height, prefix)
        }
        XCTAssertEqual(try height("radialLED"), 8.6)
        XCTAssertEqual(try height("sevenSegment"), 8)
        XCTAssertEqual(try height("lcd"), 12)
        XCTAssertEqual(try height("dipSwitch"), 3.8)
        XCTAssertEqual(try height("tactile"), 5)
        XCTAssertEqual(try height("header"), 8.5)
        XCTAssertEqual(try height("canOscillator"), 5.5)
        XCTAssertEqual(try height("barrel"), 11)
        XCTAssertEqual(try height("toPackage"), 15)
        if let a = looks["usb A x"] ?? looks["usb A y"] { XCTAssertEqual(a.height, 7) }
        if let c = looks["usb C x"] ?? looks["usb C y"] { XCTAssertEqual(c.height, 3.2) }
    }

    func testLEDColours() {
        XCTAssertEqual(PartLook.ledTint("Green"), Tint(0.18, 0.82, 0.28))
        XCTAssertEqual(PartLook.ledTint("blue"), Tint(0.18, 0.38, 0.98))
        XCTAssertEqual(PartLook.ledTint("Amber"), PartLook.ledTint("orange"))
        XCTAssertEqual(PartLook.ledTint("Yellow"), Tint(1.0, 0.86, 0.12))
        XCTAssertEqual(PartLook.ledTint("RGB"), PartLook.ledTint("white"))
        XCTAssertEqual(PartLook.ledTint("Warm White"), Tint(1.0, 0.82, 0.5))
        XCTAssertEqual(PartLook.ledTint("Blue-Green"), Tint(0.25, 0.9, 0.8))
        XCTAssertEqual(PartLook.ledTint("Infrared"), Tint(0.92, 0.14, 0.10))
    }
}
