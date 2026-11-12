import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// The Part Editor's block preview is the schematic page's chip (plan-32-E).
@MainActor
final class PartPreviewTests: XCTestCase {
    /// The schematic page's chip for a part: placed in a document, as a drop does.
    private func placed(_ part: CatalogPart) throws -> PlacedChip {
        let doc = TestSupport.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 300, y: 300))
        return try XCTUnwrap(doc.chips.last)
    }

    /// Phase 1: the factory's layout size equals the placed chip's, for
    /// SN74HC00N and every part of gate.json's first device.
    func testFactoryLayoutMatchesThePlacedChip() throws {
        guard case let .object(root)? = BundledPartFile.file(kind: "gate"), case let .array(devices)? = root["devices"],
              case let .object(device)? = devices.first, case let .array(parts)? = device["parts"] else {
            return XCTFail("gate.json shape")
        }
        let names = ["SN74HC00N"] + parts.compactMap { part -> String? in
            guard case let .object(keys) = part, case let .string(name)? = keys["name"] else { return nil }
            return name
        }
        XCTAssertGreaterThan(names.count, 10)
        for name in names {
            let part = try TestSupport.part(name)
            let preview = PartPreview.chip(for: part)
            let page = try placed(part)
            XCTAssertEqual(preview.layout.size, page.layout.size, name)
            XCTAssertEqual(preview.symbol, page.symbol, name)
            XCTAssertEqual(preview.layout.slots.map(\.centerY), page.layout.slots.map(\.centerY), name)
        }
    }
}

/// Records every stroked path's points, so two drawings can be compared.
struct CapturePainter: SchematicPainter {
    var strokes: [[CGPoint]] = []
    var texts: [(String, CGPoint)] = []

    mutating func fill(_ path: Path, with ink: Ink) {}

    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) {
        var points: [CGPoint] = []
        path.forEach { element in
            switch element {
            case let .move(to: p), let .line(to: p): points.append(p)
            case let .quadCurve(to: p, control: _): points.append(p)
            case let .curve(to: p, control1: _, control2: _): points.append(p)
            case .closeSubpath: break
            }
        }
        strokes.append(points)
    }

    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {
        texts.append((String(describing: text), point))
    }

    mutating func translateBy(x: CGFloat, y: CGFloat) {}
    mutating func rotate(by angle: Angle) {}
}

@MainActor
final class BlockPreviewViewTests: XCTestCase {
    /// Phase 2: the preview's pin stubs end where the schematic chip's do.
    func testPinEndpointsMatchTheSchematic() throws {
        let part = try TestSupport.part("SN74HC00N")
        let doc = TestSupport.newDocument()
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 300, y: 300))
        let page = try XCTUnwrap(doc.chips.last)
        var a = CapturePainter()
        var b = CapturePainter()
        ChipBlockView.paint(&a, chip: PartPreview.chip(for: part), isSelected: false, isHovered: false)
        ChipBlockView.paint(&b, chip: page, isSelected: false, isHovered: false)
        XCTAssertEqual(a.strokes.count, b.strokes.count)
        XCTAssertGreaterThanOrEqual(a.strokes.count, 14, "a stub per pin")
        for (sa, sb) in zip(a.strokes, b.strokes) {
            XCTAssertEqual(sa.count, sb.count)
            for (pa, pb) in zip(sa, sb) {
                XCTAssertEqual(pa.x, pb.x, accuracy: 1e-9)
                XCTAssertEqual(pa.y, pb.y, accuracy: 1e-9)
            }
        }
        XCTAssertEqual(a.texts.map(\.1), b.texts.map(\.1))
    }
}
