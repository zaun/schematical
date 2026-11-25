import AppKit
import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// What a person would look at (plan-65-J Phase 3): the PCB page, the board preview, the DXF sheet and an autorouted
/// board, drawn by the app's own views. The File menu is checked for its item here; the pictures are printed as
/// base64 when `CUTOUT_PICS` is set (`TEST_RUNNER_CUTOUT_PICS=1` through xcodebuild: the test host is sandboxed, so this is how they leave it).
@MainActor
final class CutoutPicturesTests: XCTestCase {
    private func png(_ document: SchematicDocument, view: ImageExportView, angle: BoardRenderAngle = .top) throws -> Data {
        var options = ImageExportOptions()
        options.view = view
        options.format = .png
        options.scale = 2
        options.angle = angle
        return try XCTUnwrap(ImageExport.data(for: document, options))
    }

    private func print(_ name: String, _ data: Data) {
        Swift.print("PIC \(name) \(data.base64EncodedString())")
    }

    func testPrintPicturesWhenAsked() throws {
        guard ProcessInfo.processInfo.environment["CUTOUT_PICS"] != nil else { return }
        // The cutouts board with a ground pour on Top.
        let board = try ExportSamples.cutouts()
        let pour = try XCTUnwrap(board.addPour(rect: CGRect(x: 0, y: 0, width: board.board.width, height: board.board.height), layer: .top))
        board.setPourNet(pour, .ground)
        print("pcb", try png(board, view: .pcb))
        print("front", try png(board, view: .front))
        print("angled", try png(board, view: .front, angle: .angled))

        // The import sheet on KiCad's own board.
        let blank = TestSupport.newDocument()
        let state = DXFImportState(fileName: "kicad_edge_probe.dxf", drawing: try DXFReader.parse(DXFFixtures.kicadEdgeProbe), board: blank.board)
        let sheet = DXFImportSheet(state: state, onCancel: {}, onImport: {}).background(Color(nsColor: .windowBackgroundColor))
        let renderer = ImageRenderer(content: sheet)
        renderer.scale = 2
        if let image = renderer.cgImage, let data = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:]) {
            print("sheet", data)
        }
        state.mode = .cutoutsOnly
        let renderer2 = ImageRenderer(content: DXFImportSheet(state: state, onCancel: {}, onImport: {}).background(Color(nsColor: .windowBackgroundColor)))
        renderer2.scale = 2
        if let image = renderer2.cgImage, let data = NSBitmapImageRep(cgImage: image).representation(using: .png, properties: [:]) {
            print("sheet-cutouts-only", data)
        }

        // A ground pour with a round and a rectangular cutout in it.
        let poured = TestSupport.newDocument()
        poured.setBoardSize(width: 60, height: 40)
        for n in 0..<2 {
            let led = try TestSupport.place(poured, "GENERIC-LED", at: CGPoint(x: 100 + 300 * n, y: 100))
            let ground = try TestSupport.place(poured, "GND", at: CGPoint(x: 100 + 300 * n, y: 400))
            XCTAssertNil(poured.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: ground.id, index: 0))))
            poured.moveFootprint(led.id, to: CGPoint(x: 12 + 36 * CGFloat(n), y: 20))
        }
        let groundPour = try XCTUnwrap(poured.addPour(rect: CGRect(x: 0, y: 0, width: 60, height: 40), layer: .top))
        poured.setPourNet(groundPour, .ground)
        _ = try XCTUnwrap(poured.addCutout(.circle(centre: CGPoint(x: 30, y: 12), diameter: 8)))
        _ = try XCTUnwrap(poured.addCutout(.rounded(rect: CGRect(x: 24, y: 24, width: 12, height: 8), radius: 1)))
        print("pour", try png(poured, view: .pcb))

        // An autoroute round a cutout.
        let routed = TestSupport.newDocument()
        let one = try TestSupport.place(routed, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(routed, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        routed.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        routed.moveFootprint(two.id, to: CGPoint(x: 70, y: 40))
        XCTAssertNil(routed.connect(.pin(try TestSupport.pin(one, "K")), .pin(try TestSupport.pin(two, "A"))))
        _ = try XCTUnwrap(routed.addCutout(.rounded(rect: CGRect(x: 45, y: 30, width: 10, height: 20), radius: 2)))
        var options = RoutingOptions.standard
        options.strategy = .fast
        let problem = routed.routingProblem(options: options, rules: routed.board.rules)
        let result = Router.route(problem)
        routed.applyRouting(result, problem: problem)
        Swift.print("ROUTED \(result.stats.routed) of \(result.stats.connections), traces \(routed.traces.count)")
        let issues = routed.designRuleCheck(rules: routed.board.rules)
        Swift.print("ROUTED_DRC \(issues.map { "\($0.kind) \($0.reference): \($0.issue)" })")
        let cutout = try XCTUnwrap(routed.cutouts.first).shape.loop
        var closest = Double.infinity
        for trace in routed.traces {
            for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                for (c, d) in zip(cutout, cutout.dropFirst() + cutout.prefix(1)) {
                    let gap = CopperGeometry.distance(.segment(a, b), .segment(c, d)) - trace.widthMM / 2
                    closest = min(closest, gap)
                }
            }
        }
        Swift.print("ROUTED_GAP closest copper-to-cutout \(closest) mm; minCopperToEdge \(routed.board.rules.minCopperToEdge)")
        print("routed", try png(routed, view: .pcb))
    }

    func testTheFileMenuHasImportDXF() throws {
        // The test host is the app: its main menu is the one a person sees.
        guard let file = NSApp.mainMenu?.items.first(where: { $0.title == "File" })?.submenu else {
            throw XCTSkip("the test host has no main menu")
        }
        let importMenu = try XCTUnwrap(file.items.first { $0.title == "Import" }?.submenu, "File ▸ Import: \(file.items.map(\.title))")
        XCTAssertEqual(importMenu.items.map(\.title), ["DXF…"])
        let titles = file.items.map(\.title)
        XCTAssertLessThan(try XCTUnwrap(titles.firstIndex(of: "Import")), try XCTUnwrap(titles.firstIndex(of: "Export")), "Import is before Export")
    }
}
