import AppKit
import SwiftUI
import XCTest
@testable import schematical

/// Board Preview: each placed part gets a generic body for its package
/// family, and the preview draws front and back, straight down and angled.
@MainActor
final class BoardPreviewTests: XCTestCase {
    /// Places the first drawer part whose footprint id starts with `prefix`.
    private func place(_ doc: SchematicDocument, footprint prefix: String,
                       where test: (CatalogPart) -> Bool = { _ in true }) throws -> PlacedChip {
        let part = try XCTUnwrap(TestSupport.allParts.first { ($0.footprint?.id.hasPrefix(prefix) ?? false) && test($0) },
                                 "no part with a \(prefix) footprint")
        return try TestSupport.place(doc, part.name, at: CGPoint(x: 200, y: 200))
    }

    func testPackagesGetTheirGenericBody() throws {
        let doc = TestSupport.newDocument()
        let dip = try place(doc, footprint: "PDIP-")
        guard case .dip(ceramic: false) = PartLook.of(dip).kind else { return XCTFail("PDIP is \(PartLook.of(dip).kind)") }
        let soic = try place(doc, footprint: "SOIC-")
        guard case .leadedIC = PartLook.of(soic).kind else { return XCTFail("SOIC is \(PartLook.of(soic).kind)") }
        let header = try place(doc, footprint: "PINHDR-")
        guard case .header(female: false) = PartLook.of(header).kind else { return XCTFail("header is \(PartLook.of(header).kind)") }
        let can = try place(doc, footprint: "CAPE-RAD")
        guard case .electrolytic(smd: false) = PartLook.of(can).kind else { return XCTFail("can is \(PartLook.of(can).kind)") }
        let led = try place(doc, footprint: "LED-RADIAL")
        guard case .radialLED = PartLook.of(led).kind else { return XCTFail("LED is \(PartLook.of(led).kind)") }
        XCTAssertGreaterThan(PartLook.of(can).height, PartLook.of(dip).height)
    }

    /// The preview draws every face and angle of a populated board.
    func testRendersEveryView() throws {
        let doc = TestSupport.newDocument()
        for prefix in ["PDIP-", "SOIC-", "PINHDR-", "CAPE-RAD", "LED-RADIAL"] {
            _ = try place(doc, footprint: prefix)
        }
        doc.setStackupFinish(.enig)
        for face in BoardRenderFace.allCases {
            for angle in BoardRenderAngle.allCases {
                let view = BoardPreviewView.canvas(document: doc, face: face, angle: angle)
                    .frame(width: 400, height: 300)
                let image = try XCTUnwrap(ImageRenderer(content: view).cgImage, "\(face) \(angle)")
                XCTAssertEqual(image.width, 400)
            }
        }
    }

    /// The picture's pixels, for comparing two renders.
    private func pixels(_ doc: SchematicDocument) throws -> Data {
        let view = BoardPreviewView.canvas(document: doc, face: .front, angle: .top).frame(width: 300, height: 220)
        let image = try XCTUnwrap(ImageRenderer(content: view).cgImage)
        return try XCTUnwrap(image.dataProvider?.data as Data?)
    }

    /// Bare copper is drawn in the board's finish: ENIG, OSP and HASL differ,
    /// leaded and lead-free HASL look alike.
    func testFinishFollowsTheBoard() throws {
        let doc = TestSupport.newDocument()
        _ = try place(doc, footprint: "PDIP-")
        func render(_ finish: SurfaceFinish) throws -> Data {
            doc.setStackupFinish(finish)
            return try pixels(doc)
        }
        let hasl = try render(.hasl), lead = try render(.leadFreeHasl), enig = try render(.enig), osp = try render(.osp)
        XCTAssertEqual(hasl, lead)
        XCTAssertNotEqual(hasl, enig)
        XCTAssertNotEqual(hasl, osp)
        XCTAssertNotEqual(enig, osp)
    }

    /// The back view mirrors left to right; the angled view lifts parts.
    func testProjection() {
        let outline = [CGPoint.zero, CGPoint(x: 100, y: 0), CGPoint(x: 100, y: 50), CGPoint(x: 0, y: 50)]
        let size = CGSize(width: 800, height: 600)
        let front = RenderProjection.fitting(outline, face: .front, angle: .top, size: size, height: 0, below: 0)
        let back = RenderProjection.fitting(outline, face: .back, angle: .top, size: size, height: 0, below: 0)
        XCTAssertLessThan(front.point(.zero).x, front.point(CGPoint(x: 100, y: 0)).x)
        XCTAssertGreaterThan(back.point(.zero).x, back.point(CGPoint(x: 100, y: 0)).x)
        XCTAssertEqual(front.lift, 0)
        let angled = RenderProjection.fitting(outline, face: .front, angle: .angled, size: size, height: 10, below: 2)
        XCTAssertLessThan(angled.point(CGPoint(x: 50, y: 25), 5).y, angled.point(CGPoint(x: 50, y: 25)).y)
    }
}
