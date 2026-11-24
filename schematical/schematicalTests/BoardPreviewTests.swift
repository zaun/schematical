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

    /// How many bytes of two renders differ. Two renders of one picture can differ by a few pixels of antialiasing
    /// (seen once in a full run of the suite), so tests compare amounts, not equality.
    private func differing(_ a: Data, _ b: Data) -> Int {
        zip(a, b).reduce(0) { $0 + ($1.0 == $1.1 ? 0 : 1) } + abs(a.count - b.count)
    }

    /// Bare copper is drawn in the board's finish: ENIG, OSP and HASL differ a lot,
    /// leaded and lead-free HASL look alike.
    func testFinishFollowsTheBoard() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 30, height: 16)
        _ = try place(doc, footprint: "PDIP-")
        func render(_ finish: SurfaceFinish) throws -> Data {
            doc.setStackupFinish(finish)
            return try pixels(doc)
        }
        // The first render in a process differs a little from later ones (caches): warm up, discard.
        _ = try render(.hasl)
        let hasl = try render(.hasl)
        let lead = try render(.leadFreeHasl), enig = try render(.enig), osp = try render(.osp)
        let apart = min(differing(hasl, enig), differing(hasl, osp), differing(enig, osp))
        XCTAssertGreaterThan(apart, 100, "the finishes look different")
        XCTAssertLessThan(differing(hasl, lead), apart / 4, "leaded and lead-free HASL look alike")
    }

    /// An open via shows as bare finish; a tented one is under the mask.
    func testOpenViaChangesTheRender() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 10, height: 8)
        let id = try XCTUnwrap(doc.addVia(at: CGPoint(x: 5, y: 4)))
        doc.setViaDiameter(id, 3)
        let tented = try pixels(doc)
        doc.setViaTenting(id, .top, false)
        let open = try pixels(doc)
        let change = differing(tented, open)
        XCTAssertGreaterThan(change, 1000, "an open via shows")
        doc.setViaTenting(id, .top, true)
        XCTAssertLessThan(differing(try pixels(doc), tented), change / 10, "tented again looks as before")
        // The back view of a via open only on Top is unchanged.
        func backPixels() throws -> Data {
            let view = BoardPreviewView.canvas(document: doc, face: .back, angle: .top).frame(width: 300, height: 220)
            return try XCTUnwrap(ImageRenderer(content: view).cgImage?.dataProvider?.data as Data?)
        }
        let backTented = try backPixels()
        doc.setViaTenting(id, .top, false)
        XCTAssertLessThan(differing(try backPixels(), backTented), change / 10)
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

    /// plan-54-G: a Top part's opposite pads are bare copper (solder) on the back face, and a
    /// Top part's own pads are not there: the back render of a board with the opposite-pad part
    /// differs from a bare board's, that of the same part without opposite pads does not.
    func testOppositePadsAreDrawnOnTheOtherFace() throws {
        func back(_ doc: SchematicDocument) throws -> Data {
            let view = BoardPreviewView.canvas(document: doc, face: .back, angle: .top).frame(width: 300, height: 220)
            return try XCTUnwrap(ImageRenderer(content: view).cgImage?.dataProvider?.data as Data?)
        }
        // A small board, so the pads (1 mm) are some pixels across.
        let bare = TestSupport.newDocument()
        bare.setLayerCount(2)
        let withOpposite = try OppositePadTests.fixture(at: CGPoint(x: 10, y: 6)).doc
        let control = try OppositePadTests.fixture(at: CGPoint(x: 10, y: 6), oppositePads: false).doc
        for doc in [bare, withOpposite, control] { doc.setBoardSize(width: 40, height: 20) }
        // The first render in a process differs a little from later ones (caches): warm up, discard.
        _ = try back(bare)
        let bareBack = try back(bare)
        let change = differing(bareBack, try back(withOpposite))
        XCTAssertGreaterThan(change, 100, "the opposite pads show on the back")
        XCTAssertLessThan(differing(bareBack, try back(control)), change / 10, "a Top part's own pads do not")
    }

    /// plan-54-O: a placed card slot gets the `cardSlot` look, a dark housing over its pads, with the height of
    /// its drawer leaf (3.0 mm a memory / SIM socket, 5.2 mm a memory module, 6.0 mm a card-edge socket); a USB
    /// connector is still `.usb`. Covers the `examples.tsv` parts, then one library part for each other leaf.
    func testCardSlotLook() throws {
        let doc = TestSupport.newDocument()
        var checked: [String] = [], missing: [String] = []
        for (index, entry) in try CardSlotTests.representatives().enumerated() {
            guard let part = entry.part else { missing.append(entry.leaf); continue }
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 200 + 300 * index, y: 200))
            let look = PartLook.of(chip)
            let height: CGFloat = entry.leaf == "Memory module" ? 5.2 : entry.leaf == "Card edge" ? 6.0 : 3.0
            guard case let .cardSlot(kindHeight) = look.kind else { XCTFail("\(part.name) is \(look.kind)"); continue }
            XCTAssertEqual(kindHeight, height, part.name)
            XCTAssertEqual(look.height, height, part.name)
            // One dark box, on the pads' bounds, as tall as the leaf's height.
            let outline = [CGPoint.zero, CGPoint(x: 100, y: 0), CGPoint(x: 100, y: 50), CGPoint(x: 0, y: 50)]
            let projection = RenderProjection.fitting(outline, face: .front, angle: .top, size: CGSize(width: 800, height: 600),
                                                      height: height, below: 0)
            let solids = look.solids(painter: SolidPainter(projection: projection))
            XCTAssertEqual(solids.count, 1, part.name)
            let solid = try XCTUnwrap(solids.first)
            XCTAssertEqual(solid.height, height, part.name)
            let pads = try XCTUnwrap(chip.boardPadBounds)
            let box = try XCTUnwrap(solid.shadow).boundingRect
            XCTAssertEqual(box.minX, pads.minX, accuracy: 1e-6, part.name)
            XCTAssertEqual(box.minY, pads.minY, accuracy: 1e-6, part.name)
            XCTAssertEqual(box.width, pads.width, accuracy: 1e-6, part.name)
            XCTAssertEqual(box.height, pads.height, accuracy: 1e-6, part.name)
            checked.append(entry.leaf)
        }
        print("BoardPreviewTests: card slot looks checked for \(checked), none in the library yet: \(missing)")
        XCTAssertFalse(checked.isEmpty, "the library has no card slot")
        let usb = try XCTUnwrap(TestSupport.catalog.allParts.first {
            if case .connector(_, _, _, _, _?) = $0.details { return true }
            return false
        })
        let usbChip = try TestSupport.place(doc, usb.name, at: CGPoint(x: 200, y: 700))
        guard case .usb = PartLook.of(usbChip).kind else { return XCTFail("USB is \(PartLook.of(usbChip).kind)") }
        if !missing.isEmpty {
            throw XCTSkip("no library part in the drawer leaf(s) \(missing.joined(separator: ", ")) yet; the other \(checked.count) were checked")
        }
    }

    /// A card slot on a board shows in the render: the front view differs from the bare board's.
    func testCardSlotShowsInTheRender() throws {
        let bare = TestSupport.newDocument()
        let doc = TestSupport.newDocument()
        let part = try XCTUnwrap(try CardSlotTests.representatives().compactMap(\.part).first)
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 100, y: 100))
        for document in [bare, doc] { document.setBoardSize(width: 120, height: 80) }
        doc.moveFootprintOrigin(chip.id, to: CGPoint(x: 30, y: 30))
        _ = try pixels(bare)
        XCTAssertGreaterThan(differing(try pixels(bare), try pixels(doc)), 100, part.name)
    }

    /// plan-52-F: a pour on Top shows in the front render, under the mask, and not in the back one.
    func testAPourShowsInTheFrontRenderOnly() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 30, height: 20)
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let ground = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 300))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
        doc.moveFootprint(led.id, to: CGPoint(x: 15, y: 10))
        func back() throws -> Data {
            let view = BoardPreviewView.canvas(document: doc, face: .back, angle: .top).frame(width: 300, height: 220)
            return try XCTUnwrap(ImageRenderer(content: view).cgImage?.dataProvider?.data as Data?)
        }
        let before = try pixels(doc), backBefore = try back()
        let id = try XCTUnwrap(doc.addPour(rect: CGRect(x: 0, y: 0, width: 30, height: 20), layer: .top))
        let change = differing(before, try pixels(doc))
        XCTAssertGreaterThan(change, 1000, "the pour shows on the front")
        XCTAssertLessThan(differing(backBefore, try back()), change / 10, "and not on the back")
        doc.deletePour(id)
        XCTAssertLessThan(differing(try pixels(doc), before), change / 10, "deleted: as before")
    }
}
