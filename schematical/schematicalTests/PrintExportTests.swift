import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// File > Print… and File > Export > PDF…: the pages, their 1:1 scale, mirror and tiles.
@MainActor
final class PrintExportTests: XCTestCase {
    private let k = PrintExport.pointsPerMM

    private func options(_ edit: (inout PrintOptions) -> Void = { _ in }) -> PrintOptions {
        var o = PrintOptions()
        o.paper = .a4
        o.marks = false
        edit(&o)
        return o
    }

    /// Only `kinds`' pages.
    private func only(_ kinds: PrintPageKind..., edit: (inout PrintOptions) -> Void = { _ in }) -> PrintOptions {
        let every: Set<PrintPageKind> = [.schematic, .board, .copper(.top), .copper(.bottom), .copper(.inner(2)),
                                         .copper(.inner(3)), .parts(.top), .parts(.bottom), .silkscreen(.top),
                                         .silkscreen(.bottom)]
        return options {
            $0.hidden = every.subtracting(kinds)
            edit(&$0)
        }
    }

    private func pdf(_ doc: SchematicDocument, _ o: PrintOptions) throws -> PDFDocument {
        let data = try XCTUnwrap(PrintExport.pdf(for: doc, name: "t", o))
        return try XCTUnwrap(PDFDocument(data: data))
    }

    /// The bounding box, in mm from the page's top-left, of the dark pixels of `page` inside its printable
    /// area (so not the footer); nil when there are none.
    private func ink(_ page: PDFPage, paper: PrintPaper) -> CGRect? {
        let box = page.bounds(for: .mediaBox)
        let scale: CGFloat = 4
        let w = Int(box.width * scale), h = Int(box.height * scale)
        var pixels = [UInt8](repeating: 255, count: w * h)
        guard let context = CGContext(data: &pixels, width: w, height: h, bitsPerComponent: 8, bytesPerRow: w,
                                      space: CGColorSpaceCreateDeviceGray(), bitmapInfo: CGImageAlphaInfo.none.rawValue)
        else { return nil }
        context.setFillColor(gray: 1, alpha: 1)
        context.fill(CGRect(x: 0, y: 0, width: w, height: h))
        context.scaleBy(x: scale, y: scale)
        page.draw(with: .mediaBox, to: context)
        let landscape = box.width > box.height
        let area = PrintExport.printable(paper, landscape: landscape)
        var minX = Int.max, maxX = -1, minY = Int.max, maxY = -1
        // Row 0 of the bitmap's memory is the top of the image.
        for y in 0..<h {
            let mmY = CGFloat(y) / scale / k
            guard mmY >= area.minY, mmY <= area.maxY else { continue }
            for x in 0..<w where pixels[y * w + x] < 128 {
                minX = min(minX, x); maxX = max(maxX, x); minY = min(minY, y); maxY = max(maxY, y)
            }
        }
        guard maxX >= 0 else { return nil }
        let toMM = 1 / scale / k
        return CGRect(x: CGFloat(minX) * toMM, y: CGFloat(minY) * toMM,
                      width: CGFloat(maxX - minX + 1) * toMM, height: CGFloat(maxY - minY + 1) * toMM)
    }

    /// The page's horizontal middle, mm.
    private func center(_ page: PDFPage) -> CGFloat { page.bounds(for: .mediaBox).width / k / 2 }

    // MARK: Pages

    func testNewBoardHasNoSchematicPageAndSilkscreenIsOff() {
        let doc = TestSupport.newDocument()
        let kinds = PrintExport.kinds(of: doc)
        XCTAssertEqual(kinds, [.copper(.top), .copper(.bottom), .parts(.top), .parts(.bottom),
                               .silkscreen(.top), .silkscreen(.bottom), .board])
        let pages = PrintExport.pages(for: doc, PrintOptions())
        XCTAssertEqual(pages.map(\.kind), [.copper(.top), .copper(.bottom), .parts(.top), .parts(.bottom), .board])
    }

    func testOneLayerBoardHasNoBottomPages() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(1)
        XCTAssertEqual(PrintExport.kinds(of: doc), [.copper(.top), .parts(.top), .silkscreen(.top), .board])
    }

    func testFourLayersHaveAPagePerCopperLayerInStackOrder() {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(4)
        XCTAssertEqual(PrintExport.kinds(of: doc).prefix(4), [.copper(.top), .copper(.inner(2)), .copper(.inner(3)), .copper(.bottom)])
    }

    func testPlacedPartAddsTheSchematicPageFirst() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        XCTAssertEqual(PrintExport.kinds(of: doc).first, .schematic)
    }

    func testHiddenKindsAreLeftOut() {
        let doc = TestSupport.newDocument()
        var o = PrintOptions()
        o.hidden = [.copper(.top), .copper(.bottom), .parts(.bottom), .silkscreen(.top), .silkscreen(.bottom), .board]
        XCTAssertEqual(PrintExport.pages(for: doc, o).map(\.kind), [.parts(.top)])
    }

    // MARK: Tiles

    func testBoardThatFitsIsOnePageCentred() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        let pages = PrintExport.pages(for: doc, only(.board) { $0.paper = .a4 })
        XCTAssertEqual(pages.count, 1)
        XCTAssertFalse(pages[0].isTile)
        // Centred: the printable area's middle is the board's.
        let area = PrintExport.printable(.a4, landscape: pages[0].landscape)
        let board = PrintExport.boardBounds(doc)
        XCTAssertEqual(pages[0].origin.x + area.width / 2, board.midX, accuracy: 1e-6)
        XCTAssertEqual(pages[0].origin.y + area.height / 2, board.midY, accuracy: 1e-6)
    }

    func testBigBoardIsTiledWithOverlapCoveringTheBoard() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 400, height: 150)
        let o = only(.board) { $0.paper = .a4 }
        let pages = PrintExport.pages(for: doc, o)
        XCTAssertGreaterThan(pages.count, 1)
        let first = try XCTUnwrap(pages.first)
        XCTAssertEqual(pages.count, first.columns * first.rows)
        let area = PrintExport.printable(.a4, landscape: first.landscape)
        let content = PrintExport.boardBounds(doc).insetBy(dx: -2, dy: -2)
        let xs = Set(pages.map(\.origin.x)).sorted()
        XCTAssertEqual(xs.first!, content.minX, accuracy: 1e-6)
        XCTAssertEqual(xs.last! + area.width, content.maxX, accuracy: 1e-6)
        for (a, b) in zip(xs, xs.dropFirst()) {
            XCTAssertLessThanOrEqual(b - a, area.width - PrintExport.overlap + 1e-6)
        }
        // Landscape needs fewer sheets for a long board.
        XCTAssertTrue(first.landscape)
    }

    func testTileCountsWithRegistrationMarksUseTheirRoom() {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 270, height: 100)
        var plain = only(.copper(.top)) { $0.paper = .a4; $0.marks = false }
        XCTAssertEqual(PrintExport.pages(for: doc, plain).count, 1)
        plain.marks = true
        XCTAssertGreaterThan(PrintExport.pages(for: doc, plain).count, 1)
    }

    // MARK: PDF

    func testPDFPagesAreTheChosenPaperInPoints() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        for paper in PrintPaper.allCases {
            let pdf = try pdf(doc, options { $0.paper = paper })
            XCTAssertEqual(pdf.pageCount, 5)
            let box = try XCTUnwrap(pdf.page(at: 0)).bounds(for: .mediaBox)
            // Either way up.
            XCTAssertEqual(min(box.width, box.height), paper.size.width * k, accuracy: 0.01, paper.title)
            XCTAssertEqual(max(box.width, box.height), paper.size.height * k, accuracy: 0.01, paper.title)
        }
    }

    func testBoardOutlineIsDrawnOneToOne() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        let pdf = try pdf(doc, only(.board))
        let page = try XCTUnwrap(pdf.page(at: 0))
        let rect = try XCTUnwrap(ink(page, paper: .a4))
        // The 0.2 mm outline pen is centred on the 50 × 30 mm edge.
        XCTAssertEqual(rect.width, 50.2, accuracy: 0.3)
        XCTAssertEqual(rect.height, 30.2, accuracy: 0.3)
    }

    func testBoardPageHasAFiftyMillimetreRuler() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        let pdf = try pdf(doc, only(.board))
        let page = try XCTUnwrap(pdf.page(at: 0))
        let text = try XCTUnwrap(page.string)
        XCTAssertTrue(text.contains("1:1"))
        XCTAssertTrue(text.contains("50 mm"))
        XCTAssertTrue(text.contains("Board outline and drills"))
    }

    func testTraceIsOneToOneAndBottomIsMirroredOnItsOwnPage() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        _ = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 15), CGPoint(x: 15, y: 15)], layer: .top, width: 40))
        _ = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 15), CGPoint(x: 15, y: 15)], layer: .bottom, width: 40))
        let pdf = try pdf(doc, only(.copper(.top), .copper(.bottom)))
        let center = center(try XCTUnwrap(pdf.page(at: 0)))
        let top = try XCTUnwrap(ink(try XCTUnwrap(pdf.page(at: 0)), paper: .a4))
        let bottom = try XCTUnwrap(ink(try XCTUnwrap(pdf.page(at: 1)), paper: .a4))
        // 40 mil = 1.016 mm wide, 10 mm long, plus the round ends.
        XCTAssertEqual(top.width, 10 + 1.016, accuracy: 0.3)
        XCTAssertEqual(top.height, 1.016, accuracy: 0.3)
        XCTAssertLessThan(top.midX, center, "top copper is seen from above: the trace is on the left")
        XCTAssertGreaterThan(bottom.midX, center, "bottom copper is seen from below: on the right")
    }

    func testMirrorFlipsCopperPagesOnly() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        _ = try XCTUnwrap(doc.addTrace(points: [CGPoint(x: 5, y: 15), CGPoint(x: 15, y: 15)], layer: .top, width: 40))
        let plainPage = try XCTUnwrap(pdf(doc, only(.copper(.top))).page(at: 0))
        let center = center(plainPage)
        let plain = try XCTUnwrap(ink(plainPage, paper: .a4))
        let flipped = try XCTUnwrap(ink(try XCTUnwrap(pdf(doc, only(.copper(.top)) { $0.mirror = true }).page(at: 0)), paper: .a4))
        XCTAssertLessThan(plain.midX, center)
        XCTAssertGreaterThan(flipped.midX, center)
        XCTAssertEqual(plain.midX + flipped.midX, 2 * center, accuracy: 0.3)
        // The outline page does not mirror.
        let board = try XCTUnwrap(ink(try XCTUnwrap(pdf(doc, only(.board) { $0.mirror = true }).page(at: 0)), paper: .a4))
        XCTAssertEqual(board.midX, center, accuracy: 0.3)
    }

    func testNegativeFillsTheBoardAreaBlack() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        XCTAssertNil(ink(try XCTUnwrap(pdf(doc, only(.copper(.top))).page(at: 0)), paper: .a4), "no copper, no ink")
        let negative = try XCTUnwrap(ink(try XCTUnwrap(pdf(doc, only(.copper(.top)) { $0.negative = true }).page(at: 0)), paper: .a4))
        // The board, grown by the ink margin each side.
        XCTAssertEqual(negative.width, 50 + 2 * PrintExport.inkMargin, accuracy: 0.3)
        XCTAssertEqual(negative.height, 30 + 2 * PrintExport.inkMargin, accuracy: 0.3)
    }

    func testRegistrationMarksAreFourCrossesOutsideTheBoard() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 50, height: 30)
        let rect = try XCTUnwrap(ink(try XCTUnwrap(pdf(doc, only(.copper(.top)) { $0.marks = true }).page(at: 0)), paper: .a4))
        let reach = PrintExport.markOffset + PrintExport.markArm
        XCTAssertEqual(rect.width, 50 + 2 * reach, accuracy: 0.3)
        XCTAssertEqual(rect.height, 30 + 2 * reach, accuracy: 0.3)
    }

    func testSchematicPageIsVectorAndFitsThePage() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let data = try XCTUnwrap(PrintExport.pdf(for: doc, name: "t", only(.schematic)))
        let pdf = try XCTUnwrap(PDFDocument(data: data))
        XCTAssertEqual(pdf.pageCount, 1)
        let rect = try XCTUnwrap(ink(try XCTUnwrap(pdf.page(at: 0)), paper: .a4))
        let area = PrintExport.printable(.a4, landscape: false)
        XCTAssertGreaterThan(rect.width, 5)
        XCTAssertLessThanOrEqual(rect.width, area.width + 0.1)
        XCTAssertFalse(String(decoding: data, as: UTF8.self).contains("/Subtype /Image"), "line art, not a picture")
    }

    /// plan-54-G: the printed copper pages follow the Gerber artwork, so a
    /// Top part's opposite pads are on the Bottom page and its own pads on the
    /// Top page, and the other way round for a Bottom part.
    func testOppositePadsArePrintedOnTheOtherSidesCopperPage() throws {
        for side in [BoardSide.top, .bottom] {
            let (doc, _) = try OppositePadTests.fixture(side: side)
            let artwork = PrintExport.artwork(for: doc)
            func pads(_ layer: CopperLayer) -> Int {
                (artwork.copper[layer] ?? []).filter { $0.dark && $0.function == "SMDPad,CuDef" }.count
            }
            let own = side.copperLayer, other = (side == .top ? BoardSide.bottom : .top).copperLayer
            XCTAssertEqual(pads(own), 12, "\(side) part: its own pads (1 and 4 to 14)")
            XCTAssertEqual(pads(other), 2, "\(side) part: its opposite pads (2 and 3)")
        }
        // The Bottom copper page of a Top part is inked only where pads 2 and 3 are: a 4 × 1 mm box
        // (the pads are 1 mm square, 3 mm apart); its Top page by pads 1 and 4 to 14: 21 × 1 mm.
        let (doc, _) = try OppositePadTests.fixture()
        let pdf = try pdf(doc, only(.copper(.top), .copper(.bottom)))
        let top = try XCTUnwrap(ink(try XCTUnwrap(pdf.page(at: 0)), paper: .a4))
        let bottom = try XCTUnwrap(ink(try XCTUnwrap(pdf.page(at: 1)), paper: .a4))
        XCTAssertEqual(bottom.width, 4, accuracy: 0.3)
        XCTAssertEqual(bottom.height, 1, accuracy: 0.3)
        XCTAssertEqual(top.width, 21, accuracy: 0.3)
    }

    func testPartsPageShowsPadsAndTrueDrills() throws {
        let doc = TestSupport.newDocument()
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        let pdf = try pdf(doc, only(.parts(.top)))
        let rect = try XCTUnwrap(ink(try XCTUnwrap(pdf.page(at: 0)), paper: .a4))
        XCTAssertGreaterThan(rect.width, 5)
        XCTAssertTrue(try XCTUnwrap(pdf.page(at: 0)?.string).contains("Top parts"))
    }

    func testPrintOperationPrintsAtActualSize() throws {
        let doc = TestSupport.newDocument()
        let pdf = try pdf(doc, only(.board))
        let operation = try XCTUnwrap(PrintExport.printOperation(for: pdf, paper: .a4))
        let info = operation.printInfo
        XCTAssertEqual(info.paperSize.width, 210 * k, accuracy: 0.5)
        XCTAssertEqual(info.paperSize.height, 297 * k, accuracy: 0.5)
        XCTAssertEqual(info.leftMargin, 0)
    }

    func testNoPagesGivesNoPDF() {
        let doc = TestSupport.newDocument()
        var o = PrintOptions()
        o.hidden = Set(PrintExport.kinds(of: doc))
        XCTAssertNil(PrintExport.pdf(for: doc, name: "t", o))
    }

    func testTiledPageNamesItsTile() throws {
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 400, height: 150)
        let pdf = try pdf(doc, only(.board))
        XCTAssertGreaterThan(pdf.pageCount, 1)
        XCTAssertTrue(try XCTUnwrap(pdf.page(at: 1)?.string).contains("tile"))
    }

    // MARK: Options

    func testOptionsAreRemembered() throws {
        let defaults = try XCTUnwrap(UserDefaults(suiteName: "PrintExportTests.\(UUID().uuidString)"))
        var o = PrintOptions.saved(defaults)
        XCTAssertTrue(o.marks)
        o.paper = .a3
        o.mirror = true
        o.negative = true
        o.marks = false
        o.save(defaults)
        let back = PrintOptions.saved(defaults)
        XCTAssertEqual(back.paper, .a3)
        XCTAssertTrue(back.mirror)
        XCTAssertTrue(back.negative)
        XCTAssertFalse(back.marks)
    }

    /// The options sheet lays out at a sensible size.
    func testDialogLaysOut() throws {
        let doc = try Fixture.open("AFU")
        let host = NSHostingController(rootView: PrintDialog(document: doc, name: "AFU", action: .print,
                                                             options: PrintOptions(), onCancel: {}, onDone: { _ in }))
        host.sizingOptions = .preferredContentSize
        let size = host.view.fittingSize
        XCTAssertGreaterThan(size.width, 500)
        XCTAssertLessThan(size.width, 900)
        XCTAssertGreaterThan(size.height, 400)
        XCTAssertLessThan(size.height, 800)
        guard ProcessInfo.processInfo.environment["PRINT_OUT"] != nil else { return }
        host.view.frame = CGRect(origin: .zero, size: size)
        host.view.layoutSubtreeIfNeeded()
        let rep = try XCTUnwrap(host.view.bitmapImageRepForCachingDisplay(in: host.view.bounds))
        host.view.cacheDisplay(in: host.view.bounds, to: rep)
        let png = try XCTUnwrap(rep.representation(using: .png, properties: [:]))
        print("PRINT_FILE DIALOG \(png.base64EncodedString())")
    }

    /// With `TEST_RUNNER_PRINT_OUT=1`, prints the PDFs of the fixtures as `PRINT_FILE <name> <base64>` lines, for
    /// looking at (the host is sandboxed).
    func testPrintSamplesWhenAsked() throws {
        guard ProcessInfo.processInfo.environment["PRINT_OUT"] != nil else { return }
        for name in Fixture.names {
            let doc = try Fixture.open(name)
            var o = PrintOptions()
            o.hidden = []
            o.paper = .a4
            let data = try XCTUnwrap(PrintExport.pdf(for: doc, name: name, o))
            print("PRINT_FILE \(name) \(data.base64EncodedString())")
        }
        let doc = TestSupport.newDocument()
        doc.setBoardSize(width: 60, height: 40)
        _ = try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        _ = doc.addTrace(points: [CGPoint(x: 10, y: 35), CGPoint(x: 40, y: 35)], layer: .bottom, width: 20)
        var o = PrintOptions()
        o.hidden = []
        o.paper = .a4
        o.marks = false
        print("PRINT_FILE THT \(try XCTUnwrap(PrintExport.pdf(for: doc, name: "THT", o)).base64EncodedString())")
    }
}
