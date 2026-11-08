import SwiftUI
import XCTest
@testable import schematical

/// plan-28-C: the SVG element mapping, on exact strings.
final class SVGPainterTests: XCTestCase {
    private let line: Path = {
        var path = Path()
        path.move(to: .zero)
        path.addLine(to: CGPoint(x: 1, y: 0))
        return path
    }()

    private func element(_ draw: (inout SVGPainter) -> Void) -> String? {
        var painter = SVGPainter()
        draw(&painter)
        XCTAssertLessThanOrEqual(painter.output.elements.count, 1)
        return painter.output.elements.first
    }

    func testPathData() {
        var path = Path()
        path.move(to: CGPoint(x: 10, y: 20))
        path.addLine(to: CGPoint(x: 40, y: 20))
        path.addQuadCurve(to: CGPoint(x: 60, y: 40), control: CGPoint(x: 60, y: 20))
        path.addCurve(to: CGPoint(x: 10, y: 60), control1: CGPoint(x: 40, y: 60), control2: CGPoint(x: 10, y: 40))
        path.closeSubpath()
        XCTAssertEqual(SVGPainter.d(for: path), "M10 20 L40 20 Q60 20 60 40 C40 60 10 40 10 60 Z")
    }

    func testNumbers() {
        XCTAssertEqual(SVGPainter.number(1.5), "1.5")
        XCTAssertEqual(SVGPainter.number(2.0), "2")
        XCTAssertEqual(SVGPainter.number(0.333333), "0.33")
        XCTAssertEqual(SVGPainter.number(-0.001), "0")
        XCTAssertEqual(SVGPainter.number(100), "100")
    }

    func testStrokeAttributes() throws {
        let style = StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round, dash: [4, 3])
        let dashed = try XCTUnwrap(element { $0.stroke(line, with: .primary, style: style) })
        for attribute in ["stroke=\"#000\"", "stroke-width=\"1.5\"", "stroke-linecap=\"round\"",
                          "stroke-linejoin=\"round\"", "stroke-dasharray=\"4 3\"", "fill=\"none\""] {
            XCTAssertTrue(dashed.contains(attribute), "\(attribute) missing in \(dashed)")
        }
        XCTAssertFalse(dashed.contains("stroke-opacity"))

        let plain = try XCTUnwrap(element { $0.stroke(line, with: .primary, lineWidth: 1) })
        XCTAssertTrue(plain.contains("stroke-miterlimit=\"10\""), plain)
        for attribute in ["stroke-linecap", "stroke-linejoin", "stroke-dasharray", "stroke-dashoffset"] {
            XCTAssertFalse(plain.contains(attribute), "\(attribute) in \(plain)")
        }

        let phased = try XCTUnwrap(element {
            $0.stroke(line, with: .primary, style: StrokeStyle(lineWidth: 1, lineCap: .square, lineJoin: .bevel,
                                                               dash: [8, 6], dashPhase: 2))
        })
        for attribute in ["stroke-linecap=\"square\"", "stroke-linejoin=\"bevel\"", "stroke-dasharray=\"8 6\"",
                          "stroke-dashoffset=\"2\""] {
            XCTAssertTrue(phased.contains(attribute), "\(attribute) missing in \(phased)")
        }
    }

    func testOpacity() throws {
        let half = try XCTUnwrap(element { $0.stroke(line, with: Ink.primary.opacity(0.5), lineWidth: 1) })
        XCTAssertTrue(half.contains("stroke-opacity=\"0.5\""), half)
        let secondary = try XCTUnwrap(element { $0.stroke(line, with: .secondary, lineWidth: 1) })
        XCTAssertTrue(secondary.contains("stroke-opacity=\"0.6\""), secondary)
        let lighter = try XCTUnwrap(element { $0.stroke(line, with: Ink.secondary.opacity(0.5), lineWidth: 1) })
        XCTAssertTrue(lighter.contains("stroke-opacity=\"0.3\""), lighter)
        let filled = try XCTUnwrap(element { $0.fill(line, with: .unlit) })
        XCTAssertTrue(filled.contains("fill=\"#000\" fill-opacity=\"0.15\""), filled)
        XCTAssertNil(element { $0.fill(line, with: .paper) })
        XCTAssertNil(element { $0.stroke(line, with: .color(.red), lineWidth: 1) })
        XCTAssertNil(element { $0.draw(PaintedText("x", size: 9, ink: .color(.red)), at: .zero, anchor: .center) })
    }

    func testText() throws {
        func text(_ text: PaintedText, _ anchor: UnitPoint) throws -> String {
            try XCTUnwrap(element { $0.draw(text, at: CGPoint(x: 3, y: 4), anchor: anchor) })
        }
        let plain = PaintedText("A&B<", size: 9)
        XCTAssertTrue(try text(plain, .leading).contains("text-anchor=\"start\""))
        XCTAssertTrue(try text(plain, .center).contains("text-anchor=\"middle\""))
        XCTAssertTrue(try text(plain, .trailing).contains("text-anchor=\"end\""))
        let centred = try text(plain, .center)
        XCTAssertTrue(centred.hasSuffix(">A&amp;B&lt;</text>"), centred)
        XCTAssertTrue(centred.contains("x=\"3\" y=\"4\""), centred)
        XCTAssertTrue(centred.contains("dominant-baseline=\"central\""), centred)
        XCTAssertTrue(centred.contains("font-weight=\"400\""), centred)
        XCTAssertFalse(centred.contains("transform"), centred)
        XCTAssertTrue(try text(PaintedText("x", size: 11, weight: .semibold), .center).contains("font-weight=\"600\""))
        XCTAssertTrue(try text(PaintedText("x", size: 11, monospaced: true), .center).contains("font-family=\"Menlo"))
        XCTAssertTrue(try text(PaintedText("x", size: 11), .center).contains("font-family=\"'Helvetica Neue'"))
        XCTAssertTrue(try text(PaintedText("x", size: 9, ink: .secondary), .center).contains("fill-opacity=\"0.6\""))
        XCTAssertEqual(SVGPainter.escaped("\"'>"), "&quot;&apos;&gt;")
    }

    func testTransforms() throws {
        let shifted = try XCTUnwrap(element {
            $0.translateBy(x: 5, y: 5)
            $0.stroke(line, with: .primary, lineWidth: 1)
        })
        XCTAssertTrue(shifted.hasPrefix("<path d=\"M5 5 L6 5\""), shifted)

        let turned = try XCTUnwrap(element {
            $0.translateBy(x: 5, y: 7)
            $0.rotate(by: .degrees(-90))
            $0.draw(PaintedText("B0", size: 10), at: .zero, anchor: .center)
        })
        XCTAssertTrue(turned.contains("transform=\"matrix(0 -1 1 0 5 7)\""), turned)
        XCTAssertTrue(turned.contains("x=\"0\" y=\"0\""), turned)
    }

    /// A copy that draws writes into the original's output.
    func testCopiesShareOutput() {
        let painter = SVGPainter()
        var copy = painter
        copy.translateBy(x: 1, y: 1)
        copy.stroke(line, with: .primary, lineWidth: 1)
        XCTAssertEqual(painter.output.elements.count, 1)
        XCTAssertEqual(painter.transform, .identity)
    }
}
