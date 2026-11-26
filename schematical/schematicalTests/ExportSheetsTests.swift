import AppKit
import PDFKit
import SwiftUI
import XCTest
@testable import schematical

/// plan-68-Y: the options sheets of File › Export › Image…, File › Import ›
/// DXF… and File › Print… / Export › PDF…, rendered in each state and driven
/// through the accessibility tree.
@MainActor
final class ExportSheetsTests: XCTestCase {
    private var probes: [ViewProbe] = []

    override func tearDown() {
        for probe in probes { probe.close() }
        probes = []
    }

    private func probe<V: View>(_ view: V, height: CGFloat = 760) -> ViewProbe {
        let probe = ViewProbe(view, size: NSSize(width: 760, height: height))
        probes.append(probe)
        return probe
    }

    private func gate() throws -> SchematicDocument {
        let doc = TestSupport.newDocument()
        try TestSupport.place(doc, "SN74HC00N", at: CGPoint(x: 200, y: 200))
        return doc
    }

    private func checkbox(_ probe: ViewProbe, _ title: String) throws -> NSObject {
        try probe.element(title, role: "AXCheckBox")
    }

    private func press(_ element: NSObject, in probe: ViewProbe) {
        _ = element.perform(NSSelectorFromString("accessibilityPerformPress"))
        probe.settle()
    }

    // MARK: Image

    func testTheImageSheetShowsTheSchematicsChoicesAndExportsThem() throws {
        let doc = try gate()
        var exported: ImageExportOptions?
        var cancelled = false
        let sheet = probe(ImageExportDialog(document: doc, options: ImageExportOptions(),
                                            onCancel: { cancelled = true }, onExport: { exported = $0 }))
        XCTAssertTrue(sheet.has("Export Image"), "\(sheet.labels())")
        let size = try XCTUnwrap(ImageExport.pixelSize(of: doc, ImageExportOptions()))
        XCTAssertTrue(sheet.has("\(Int(size.width)) × \(Int(size.height)) px"), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Format:"))
        XCTAssertTrue(sheet.has("Size:"))
        XCTAssertFalse(sheet.has("Layers:"), "the PCB's only")
        XCTAssertFalse(sheet.has("View:"), "a render's only")
        try sheet.press("Export…")
        XCTAssertEqual(exported, ImageExportOptions().normalized())
        try sheet.press("Cancel")
        XCTAssertTrue(cancelled)
    }

    func testAnSVGHasNoSizeAndIsMeasuredInPoints() throws {
        let doc = try gate()
        var options = ImageExportOptions()
        options.format = .svg
        let sheet = probe(ImageExportDialog(document: doc, options: options, onCancel: {}, onExport: { _ in }))
        XCTAssertFalse(sheet.has("Size:"), "\(sheet.labels())")
        XCTAssertTrue(sheet.labels().contains { $0.hasSuffix(" pt") }, "\(sheet.labels())")
    }

    func testThePCBPageHasALayerBoxEachAndAHiddenLayerIsLeftOut() throws {
        let doc = try gate()
        var options = ImageExportOptions()
        options.view = .pcb
        var exported: ImageExportOptions?
        let sheet = probe(ImageExportDialog(document: doc, options: options, onCancel: {}, onExport: { exported = $0 }))
        XCTAssertTrue(sheet.has("Layers:"), "\(sheet.labels())")
        let layers: [BoardViewLayer] = [.board] + doc.board.copperLayers.map { .copper($0) } + [.silkscreen, .document, .ratsnest]
        for layer in layers { XCTAssertNoThrow(try checkbox(sheet, layer.title), layer.title) }
        press(try checkbox(sheet, "Silkscreen"), in: sheet)
        press(try checkbox(sheet, "Ratsnest"), in: sheet)
        try sheet.press("Export…")
        XCTAssertEqual(exported?.hiddenLayers, [.silkscreen], "Silkscreen hidden, Ratsnest shown")
    }

    func testARenderHasItsViewAndAnEmptySchematicHasNothingToExport() throws {
        var options = ImageExportOptions()
        options.view = .front
        let render = probe(ImageExportDialog(document: try gate(), options: options, onCancel: {}, onExport: { _ in }))
        XCTAssertTrue(render.has("View:"), "\(render.labels())")

        let empty = probe(ImageExportDialog(document: TestSupport.newDocument(), options: ImageExportOptions(),
                                            onCancel: {}, onExport: { _ in }))
        XCTAssertTrue(empty.has("Nothing to export: add a part, wire or bus first."), "\(empty.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try empty.element("Export…", role: "AXButton")))
    }

    func testChoosingThePCBPageTurnsAnSVGIntoAPNG() throws {
        var options = ImageExportOptions()
        options.format = .svg
        var exported: ImageExportOptions?
        let sheet = probe(ImageExportDialog(document: try gate(), options: options, onCancel: {}, onExport: { exported = $0 }))
        let views = try XCTUnwrap(sheet.views(NSSegmentedControl.self).first)
        views.selectedSegment = try XCTUnwrap(ImageExportView.allCases.firstIndex(of: .pcb))
        _ = views.sendAction(views.action, to: views.target)
        sheet.settle()
        XCTAssertTrue(sheet.has("Layers:"), "\(sheet.labels())")
        try sheet.press("Export…")
        XCTAssertEqual(exported?.view, .pcb)
        XCTAssertEqual(exported?.format, .png)
    }

    func testTheOptionsTitles() {
        XCTAssertEqual(ImageExportView.allCases.map(\.fileSuffix), ["Schematic", "PCB", "Front", "Back"])
        XCTAssertEqual(ImageExportView.allCases.map(\.id), ImageExportView.allCases)
        XCTAssertEqual(ImageExportFormat.allCases.map(\.title), ["PNG", "JPG", "SVG"])
        XCTAssertEqual(ImageExportFormat.allCases.map(\.id), ImageExportFormat.allCases)
        XCTAssertEqual(ImageExportBackground.allCases.map(\.id), ImageExportBackground.allCases)
        XCTAssertEqual(ImageExportBackground.allCases.map { $0.title(for: .pcb) }, ["Dark", "White", "Transparent"])
        XCTAssertEqual(ImageExportBackground.canvas.title(for: .front), "Backdrop")
    }

    func testThePCBPicturesBoundsTakeInATraceOffTheBoard() throws {
        let doc = try ExportSamples.routed()
        let bounds = ImageExport.pcbBounds(doc)
        for trace in doc.traces {
            for point in trace.points {
                XCTAssertTrue(bounds.insetBy(dx: ImageExport.pcbPadding - 1e-6, dy: ImageExport.pcbPadding - 1e-6)
                    .contains(point), "\(point)")
            }
        }
    }

    // MARK: DXF

    /// A board edge on one layer and a hole on another.
    static let twoLayers = "  0\nSECTION\n  2\nENTITIES\n"
        + "  0\nLWPOLYLINE\n  8\nEdge\n 90\n4\n 70\n1\n 10\n0\n 20\n0\n 10\n40\n 20\n0\n 10\n40\n 20\n30\n 10\n0\n 20\n30\n"
        + "  0\nCIRCLE\n  8\nHoles\n 10\n10\n 20\n10\n 40\n3\n  0\nENDSEC\n  0\nEOF\n"

    private func state(_ text: String, board: BoardSettings? = nil) throws -> DXFImportState {
        DXFImportState(fileName: "board.dxf", drawing: try DXFReader.parse(text), board: board ?? TestSupport.newDocument().board)
    }

    func testTheDXFSheetShowsTheLayersAndWhatItMakes() throws {
        let state = try state(Self.twoLayers)
        XCTAssertEqual(state.drawing.layers, ["Edge", "Holes"])
        var imported = false, cancelled = false
        let sheet = probe(DXFImportSheet(state: state, onCancel: { cancelled = true }, onImport: { imported = true }))
        XCTAssertTrue(sheet.has("Import “board.dxf”"), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Layers:"))
        XCTAssertTrue(sheet.has(state.summary), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Replaces the board's edge and its cutouts."))
        for note in state.notes { XCTAssertTrue(sheet.has("Left out: " + note), note) }
        try sheet.press("Import")
        XCTAssertTrue(imported)
        try sheet.press("Cancel")
        XCTAssertTrue(cancelled)

        // Turning a layer off: it is no longer read.
        let layer = try XCTUnwrap(state.layers.sorted().first)
        press(try checkbox(sheet, layer), in: sheet)
        XCTAssertFalse(state.layers.contains(layer))
        press(try checkbox(sheet, layer), in: sheet)
        XCTAssertTrue(state.layers.contains(layer))
    }

    func testCutoutsOnlyAsksWhereTheOriginGoesAndAProblemStopsTheImport() throws {
        let state = try state(DXFFixtures.roundedBoard)
        state.mode = .cutoutsOnly
        let sheet = probe(DXFImportSheet(state: state, onCancel: {}, onImport: {}))
        XCTAssertTrue(sheet.has("Origin on the board:"), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("mm"))
        XCTAssertFalse(sheet.has("Replaces the board's edge"))

        // No loops on the chosen layers: the problem shows and Import is off.
        state.layers = []
        sheet.settle()
        let problem = try XCTUnwrap(state.problem)
        XCTAssertTrue(sheet.has(problem), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Nothing to import") || sheet.has("0 cutouts"), "\(sheet.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try sheet.element("Import", role: "AXButton")))
    }

    func testADrawingThatDoesNotSayItsUnitsSaysSo() throws {
        let text = "  0\nSECTION\n  2\nENTITIES\n  0\nCIRCLE\n  8\nEdge\n 10\n5\n 20\n5\n 40\n3\n  0\nENDSEC\n  0\nEOF\n"
        let state = try state(text)
        XCTAssertNil(state.drawing.units)
        let sheet = probe(DXFImportSheet(state: state, onCancel: {}, onImport: {}))
        XCTAssertFalse(sheet.has("Layers:"), "one layer: no choice")
        XCTAssertTrue(sheet.has("Import “board.dxf”"))
    }

    // MARK: Print and PDF

    func testThePrintSheetListsThePagesAndPrintsThem() throws {
        let doc = try gate()
        var done: PrintOptions?
        var cancelled = false
        let sheet = probe(PrintDialog(document: doc, name: "Clock", action: .print, options: PrintOptions(),
                                      onCancel: { cancelled = true }, onDone: { done = $0 }))
        XCTAssertTrue(sheet.has("Print"), "\(sheet.labels())")
        for kind in PrintExport.kinds(of: doc) { XCTAssertNoThrow(try checkbox(sheet, kind.title), kind.title) }
        XCTAssertTrue(sheet.has("1 page"), "\(sheet.labels())")
        // The preview is made after a short wait.
        let end = Date().addingTimeInterval(3)
        while sheet.views(PDFView.self).first?.document == nil, Date() < end { sheet.settle(0.1) }
        XCTAssertEqual(sheet.views(PDFView.self).first?.document?.pageCount, PrintExport.pages(for: doc, PrintOptions()).count,
                       "the preview is the pages")
        try sheet.press("Cancel")
        XCTAssertTrue(cancelled)

        // Every page off: none chosen, Print is off; one back on, it prints.
        for kind in PrintExport.kinds(of: doc) where !PrintOptions().hidden.contains(kind) {
            press(try checkbox(sheet, kind.title), in: sheet)
        }
        XCTAssertTrue(sheet.has("No pages chosen."), "\(sheet.labels())")
        XCTAssertFalse(ViewProbe.isEnabled(try sheet.element("Print…", role: "AXButton")))
        let schematic = PrintPageKind.schematic
        press(try checkbox(sheet, schematic.title), in: sheet)
        try sheet.press("Print…")
        XCTAssertEqual(done?.hidden.contains(schematic), false)
    }

    func testABigBoardIsTiledAndThePDFSheetSavesAPDF() throws {
        let doc = try gate()
        doc.setBoardSize(width: 400, height: 300)
        let sheet = probe(PrintDialog(document: doc, name: "Big", action: .savePDF, options: PrintOptions(),
                                      onCancel: {}, onDone: { _ in }))
        XCTAssertTrue(sheet.has("Export PDF"), "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Save PDF…"))
        XCTAssertTrue(sheet.labels().contains { $0.contains("tiles)") }, "\(sheet.labels())")
        XCTAssertTrue(sheet.has("Mirror (toner transfer, film face down)"), "a board has copper pages")
    }
}
