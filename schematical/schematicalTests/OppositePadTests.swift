import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// plan-54-F: an opposite-side pad (`"opposite": true`, plan-54-E) is copper on
/// the other face of the board from its part's side, and an `edgeMount`
/// footprint is checked against the outline by its pads only.
@MainActor
final class OppositePadTests: XCTestCase {
    /// A gate with a 14-pad custom footprint, read by the backend's own loader
    /// (mm, from the body centre, y down): pad 1 on the part's own face at
    /// (-4, 0), pads 2 and 3 opposite at (-4, 2) and (-1, 2), pads 4 to 14 on
    /// the own face in a row at y = 5 from x = -4 on, 2 mm apart. Every pad is
    /// 1 × 1 mm. The body is 12 × 6 mm about the centre, where the reference
    /// text is.
    /// With `oppositePads` false pads 2 and 3 are on the own face too (a control).
    static func part(edgeMount: Bool = false, oppositePads: Bool = true) throws -> CatalogPart {
        func pad(_ pin: Int, _ x: Double, _ y: Double, opposite: Bool = false) -> JSONValue {
            var keys: [String: JSONValue] = ["pin": .number(Double(pin)), "x": .number(x), "y": .number(y),
                                             "width": .number(1), "height": .number(1), "shape": .string("rect")]
            if opposite { keys["opposite"] = .bool(true) }
            return .object(keys)
        }
        var pads = [pad(1, -4, 0), pad(2, -4, 2, opposite: oppositePads), pad(3, -1, 2, opposite: oppositePads)]
        pads += (4...14).map { pad($0, -4 + 2 * Double($0 - 4), 5) }
        var entry: [String: JSONValue] = [
            "id": .string("OPP-TEST"), "description": .string("Opposite pad test"), "style": .string("explicit"),
            "pins": .number(14), "body": .object(["width": .number(12), "height": .number(6)]),
            "pads": .array(pads),
        ]
        if edgeMount { entry["edgeMount"] = .bool(true) }
        var draft = try PartDraft(file: XCTUnwrap(BundledPartFile.copy(name: "SN74HC00N", kind: "gate")))
        draft.footprints = [.object(entry)]
        draft.footprint = "OPP-TEST"
        switch PartValidator.shared.validate(draft) {
        case let .success(part): return part
        case let .failure(failure): throw CustomFootprintDraftTests.LoadFailure(message: failure.message)
        }
    }

    /// A 2-layer document with the test part on `side`, pad 1 of it at `origin`
    /// (mm; snapped to the grid): the board the export tests (plan-54-G) use.
    static func fixture(side: BoardSide = .top, at origin: CGPoint = CGPoint(x: 30, y: 30),
                        oppositePads: Bool = true) throws -> (doc: SchematicDocument, id: UUID) {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        let part = try Self.part(oppositePads: oppositePads)
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        let id = try XCTUnwrap(doc.chips.last).id
        if side == .bottom { doc.setFootprintSide(id, .bottom) }
        doc.moveFootprintOrigin(id, to: origin)
        return (doc, id)
    }

    /// A 2-layer document with the test part placed on Top.
    private func document(edgeMount: Bool = false) throws -> (doc: SchematicDocument, id: UUID) {
        let doc = TestSupport.newDocument()
        doc.setLayerCount(2)
        let part = try Self.part(edgeMount: edgeMount)
        doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        return (doc, try XCTUnwrap(doc.chips.last).id)
    }

    private func chip(_ doc: SchematicDocument, _ id: UUID) throws -> PlacedChip {
        try XCTUnwrap(doc.chips.first { $0.id == id })
    }

    private func pad(_ chip: PlacedChip, _ designator: String) throws -> PlacedPad {
        try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
    }

    private func centre(_ pad: PlacedPad) -> CGPoint { CGPoint(x: pad.rect.midX, y: pad.rect.midY) }

    /// The connectivity pad of a part's pad, by its label ("U1.2").
    private func connectivityPad(_ doc: SchematicDocument, _ chip: PlacedChip, _ designator: String) throws
        -> (index: Int, pad: BoardConnectivity.Pad) {
        let copper = doc.connectivity()
        let index = try XCTUnwrap(copper.pads.firstIndex { $0.label == "\(chip.reference).\(designator)" })
        return (index, copper.pads[index])
    }

    // MARK: The model

    /// The backend's `opposite` and `edge_mount` reach the Swift footprint.
    func testFootprintCarriesTheKeys() throws {
        let part = try Self.part(edgeMount: true)
        let footprint = try XCTUnwrap(part.footprint)
        XCTAssertTrue(footprint.edgeMount)
        XCTAssertEqual(footprint.pads.filter(\.opposite).compactMap(\.designator), ["2", "3"])
        XCTAssertFalse(try XCTUnwrap(Self.part().footprint).edgeMount)
        let own = try XCTUnwrap(footprint.pads.first { $0.designator == "1" })
        XCTAssertFalse(own.opposite)
    }

    func testLayerSideSwapsOnlyAnOppositePad() throws {
        let (doc, id) = try document()
        let placed = try chip(doc, id)
        XCTAssertEqual(try pad(placed, "1").layerSide(on: .top), .top)
        XCTAssertEqual(try pad(placed, "1").layerSide(on: .bottom), .bottom)
        XCTAssertEqual(try pad(placed, "2").layerSide(on: .top), .bottom)
        XCTAssertEqual(try pad(placed, "2").layerSide(on: .bottom), .top)
    }

    // MARK: Connectivity

    /// On Top, pad 1 is on Top and pad 2 on Bottom; on Bottom the other way
    /// round; `setFootprintSide` swaps them.
    func testConnectivityPutsTheOppositePadOnTheOtherLayer() throws {
        let (doc, id) = try document()
        var placed = try chip(doc, id)
        XCTAssertEqual(try connectivityPad(doc, placed, "1").pad.side, .top)
        XCTAssertEqual(try connectivityPad(doc, placed, "2").pad.side, .bottom)
        XCTAssertEqual(try connectivityPad(doc, placed, "4").pad.side, .top)
        doc.setFootprintSide(id, .bottom)
        placed = try chip(doc, id)
        XCTAssertEqual(placed.board?.side, .bottom)
        XCTAssertEqual(try connectivityPad(doc, placed, "1").pad.side, .bottom)
        XCTAssertEqual(try connectivityPad(doc, placed, "2").pad.side, .top)
        XCTAssertEqual(try connectivityPad(doc, placed, "3").pad.side, .top)
        doc.setFootprintSide(id, .top)
        placed = try chip(doc, id)
        XCTAssertEqual(try connectivityPad(doc, placed, "1").pad.side, .top)
        XCTAssertEqual(try connectivityPad(doc, placed, "2").pad.side, .bottom)
    }

    /// A trace joins the opposite pads on their layer only, for a part on
    /// either side.
    func testATraceReachesTheOppositePadOnItsLayerOnly() throws {
        for side in [BoardSide.top, .bottom] {
            let own = side.copperLayer
            let other = (side == .top ? BoardSide.bottom : .top).copperLayer
            // On the other layer, pads 2 and 3 are joined by a trace; on the
            // part's own layer, a trace over them touches nothing.
            for (layer, joins) in [(other, true), (own, false)] {
                let (doc, id) = try document()
                if side == .bottom { doc.setFootprintSide(id, .bottom) }
                let placed = try chip(doc, id)
                let a = centre(try pad(placed, "2")), b = centre(try pad(placed, "3"))
                XCTAssertNotNil(doc.addTrace(points: [a, b], layer: layer))
                let copper = doc.connectivity()
                let two = try connectivityPad(doc, placed, "2").index, three = try connectivityPad(doc, placed, "3").index
                XCTAssertEqual(copper.root[two] == copper.root[three], joins, "\(side) part, trace on \(layer)")
            }
        }
    }

    // MARK: The canvas, hit testing and visibility

    /// `pads(on:)`: Bottom has the opposite pads of a Top part and not its own
    /// pad 1; Top the other way round; a Bottom part the reverse.
    func testPadsOnALayerFollowThePadsLayer() throws {
        let (doc, id) = try document()
        func designators(_ layer: CopperLayer) -> Set<String> {
            Set(doc.pads(on: layer).compactMap { $0.pad.designator })
        }
        XCTAssertEqual(designators(.bottom), ["2", "3"])
        XCTAssertTrue(designators(.top).isSuperset(of: ["1", "4", "14"]))
        XCTAssertTrue(designators(.top).isDisjoint(with: ["2", "3"]))
        doc.setFootprintSide(id, .bottom)
        XCTAssertTrue(designators(.top) == ["2", "3"])
        XCTAssertTrue(designators(.bottom).isSuperset(of: ["1", "4", "14"]))
        XCTAssertFalse(designators(.bottom).contains("2"))
    }

    /// `padLabel(at:)` finds an opposite pad only while its own layer's copper
    /// is visible, and an ordinary pad only while its part's side is.
    func testHitTestingFollowsThePadsLayer() throws {
        let (doc, id) = try document()
        let placed = try chip(doc, id)
        let one = centre(try pad(placed, "1")), two = centre(try pad(placed, "2"))
        func label(_ point: CGPoint, hidden: Set<BoardViewLayer> = []) -> String? {
            doc.padLabel(at: point, visible: BoardVisibility(hidden: hidden))?.label
        }
        XCTAssertEqual(label(one)?.hasPrefix("\(placed.reference).1"), true)
        XCTAssertEqual(label(two)?.hasPrefix("\(placed.reference).2"), true)
        // Bottom copper hidden: the opposite pad can't be hit, pad 1 can.
        XCTAssertNil(label(two, hidden: [.copper(.bottom)]))
        XCTAssertNotNil(label(one, hidden: [.copper(.bottom)]))
        // Top copper hidden: the reverse.
        XCTAssertNotNil(label(two, hidden: [.copper(.top)]))
        XCTAssertNil(label(one, hidden: [.copper(.top)]))
    }

    /// A footprint is drawn (and so hit) while any pad of it shows: with only
    /// Bottom copper visible, a Top part with an opposite pad is, a Top part
    /// without one is not.
    func testAFootprintShowsWhileItsOppositePadDoes() throws {
        let (doc, id) = try document()
        let plain = try TestSupport.place(doc, "SN74HC00D", at: CGPoint(x: 400, y: 400))
        XCTAssertFalse(plain.boardPads.contains { $0.opposite })
        let onlyBottom = BoardVisibility(hidden: [.copper(.top)])
        XCTAssertTrue(onlyBottom.showsFootprint(try chip(doc, id)))
        XCTAssertFalse(onlyBottom.showsFootprint(try chip(doc, plain.id)))
        let onlyTop = BoardVisibility(hidden: [.copper(.bottom)])
        XCTAssertTrue(onlyTop.showsFootprint(try chip(doc, id)))
        XCTAssertTrue(onlyTop.showsFootprint(try chip(doc, plain.id)))
    }

    /// The pixel (RGBA) at a board point of a PCB image export with some
    /// layers hidden.
    private func pixels(_ doc: SchematicDocument, hidden: Set<BoardViewLayer>, at points: [CGPoint]) throws
        -> [[CGFloat]] {
        var options = ImageExportOptions()
        options.view = .pcb
        options.format = .png
        options.scale = 1
        options.hiddenLayers = hidden
        let data = try XCTUnwrap(ImageExport.data(for: doc, options))
        let image = try XCTUnwrap(NSBitmapImageRep(data: data))
        let mm = ImageExport.pcbBounds(doc)
        let perMM = Double(image.pixelsWide) / ceil(mm.width * BoardScale.pointsPerMM) * Double(BoardScale.pointsPerMM)
        return try points.map { point in
            let x = Int((point.x - mm.minX) * perMM), y = Int((point.y - mm.minY) * perMM)
            let colour = try XCTUnwrap(image.colorAt(x: x, y: y)?.usingColorSpace(.deviceRGB))
            return [colour.redComponent, colour.greenComponent, colour.blueComponent, colour.alphaComponent]
        }
    }

    /// An opposite pad of a Top part is drawn in the Bottom layer's pass: hiding
    /// Bottom copper leaves it out and hiding Top copper does not; pad 1 the
    /// other way round.
    func testAnOppositePadIsDrawnInTheBottomLayersPass() throws {
        let (doc, id) = try document()
        doc.moveFootprintOrigin(id, to: CGPoint(x: 30, y: 30))
        let placed = try chip(doc, id)
        // Off-centre in each 1 mm pad, clear of the reference text and the body outline.
        func probe(_ designator: String) throws -> CGPoint {
            let rect = try pad(placed, designator).rect
            return CGPoint(x: rect.midX + 0.3, y: rect.midY + 0.3)
        }
        let points = [try probe("1"), try probe("2")]
        let neither = try pixels(doc, hidden: [.ratsnest, .copper(.top), .copper(.bottom)], at: points)
        let noBottom = try pixels(doc, hidden: [.ratsnest, .copper(.bottom)], at: points)
        let noTop = try pixels(doc, hidden: [.ratsnest, .copper(.top)], at: points)
        // Pad 1 (Top): drawn with Top copper, absent without it.
        XCTAssertNotEqual(noBottom[0], neither[0])
        XCTAssertEqual(noTop[0], neither[0])
        // Pad 2 (opposite, so Bottom): drawn with Bottom copper, absent without it.
        XCTAssertNotEqual(noTop[1], neither[1])
        XCTAssertEqual(noBottom[1], neither[1])
    }

    // MARK: The design-rule check

    private let rules = DesignRules.pcbway

    /// The part's lines of one kind.
    private func issues(_ doc: SchematicDocument, _ kind: DesignRuleIssue.Kind, of id: UUID) throws
        -> [DesignRuleIssue] {
        let reference = try chip(doc, id).reference
        return doc.designRuleCheck(rules: rules).filter { $0.kind == kind && $0.reference == reference }
    }

    /// On a 1-layer board a part with an opposite pad is told it has pads on
    /// Bottom; on a 2-layer board it is not, on either side.
    func testAnOppositePadOnAOneLayerBoardIsReported() throws {
        let one = TestSupport.newDocument()
        one.setLayerCount(1)
        let part = try Self.part()
        one.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100, y: 100))
        let id = try XCTUnwrap(one.chips.last).id
        one.moveFootprintOrigin(id, to: CGPoint(x: 30, y: 30))
        let found = try issues(one, .layer, of: id)
        XCTAssertEqual(found.map(\.issue), ["Has pads on Bottom, but a 1-layer board has only Top"], "\(found)")
        XCTAssertEqual(found.first?.items, [.footprint(id)])

        let (two, twoID) = try document()
        two.moveFootprintOrigin(twoID, to: CGPoint(x: 30, y: 30))
        XCTAssertEqual(try issues(two, .layer, of: twoID), [])
        two.setFootprintSide(twoID, .bottom)
        XCTAssertEqual(try issues(two, .layer, of: twoID), [])

        // A part with no opposite pad on a 1-layer board has no line.
        let plain = TestSupport.newDocument()
        plain.setLayerCount(1)
        let chip = try TestSupport.place(plain, "SN74HC00D", at: CGPoint(x: 100, y: 100))
        XCTAssertEqual(try issues(plain, .layer, of: chip.id), [])
    }

    /// An edge-mount footprint whose body hangs over the outline, with its pads
    /// on the board, is not reported; the same footprint without `edgeMount`
    /// is; a pad of an edge-mount footprint off the board still is.
    func testAnEdgeMountFootprintIsCheckedByItsPads() throws {
        // Pad 1 at 1.27 mm from the top edge: the body (3 mm above the
        // body centre, which is level with pad 1) hangs over it.
        let hanging = CGPoint(x: 30, y: 1)
        let plain = try document(edgeMount: false)
        plain.doc.moveFootprintOrigin(plain.id, to: hanging)
        let hangs = try issues(plain.doc, .placement, of: plain.id)
        XCTAssertEqual(hangs.map(\.issue), ["Hangs off the board's edge"], "\(hangs)")

        let edge = try document(edgeMount: true)
        edge.doc.moveFootprintOrigin(edge.id, to: hanging)
        let placed = try chip(edge.doc, edge.id)
        let bounds = try XCTUnwrap(placed.boardBounds), padBounds = try XCTUnwrap(placed.boardPadBounds)
        XCTAssertLessThan(bounds.minY, 0, "the body hangs over the edge")
        XCTAssertGreaterThan(padBounds.minY, 0, "the pads are on the board")
        XCTAssertEqual(try issues(edge.doc, .placement, of: edge.id), [])

        // Pad 1 off the board: reported, though edge-mount.
        edge.doc.moveFootprintOrigin(edge.id, to: CGPoint(x: 30, y: -1))
        let off = try issues(edge.doc, .placement, of: edge.id)
        XCTAssertEqual(off.count, 1, "\(off)")
        XCTAssertEqual(off.first?.issue, "Hangs off the board's edge")
        // Wholly off the board.
        edge.doc.moveFootprintOrigin(edge.id, to: CGPoint(x: 30, y: -50))
        XCTAssertEqual(try issues(edge.doc, .placement, of: edge.id).map(\.issue), ["Off the board"])
    }

    // MARK: The router

    /// The router puts an opposite pad on Bottom: a net between the opposite
    /// pads of two Top parts is routed on Bottom (and never on Top); a net
    /// between their own-face pads is routed on Top.
    func testTheRouterPutsAnOppositePadOnBottom() throws {
        func routed(pin: String) throws -> (problem: RoutingProblem, result: RoutingResult) {
            let doc = TestSupport.newDocument()
            doc.setLayerCount(2)
            let part = try Self.part()
            var ids: [UUID] = []
            for x in [10.0, 50.0] {
                doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 100 + x * 20, y: 100))
                let id = try XCTUnwrap(doc.chips.last).id
                doc.moveFootprintOrigin(id, to: CGPoint(x: x, y: 20))
                ids.append(id)
            }
            let first = try TestSupport.pin(try chip(doc, ids[0]), pin), second = try TestSupport.pin(try chip(doc, ids[1]), pin)
            XCTAssertNil(doc.connect(.pin(first), .pin(second)))
            let problem = doc.routingProblem(options: .fixture)
            return (problem, Router.route(problem))
        }
        // Pin "1B" is pad 2 (opposite), pin "1A" is pad 1.
        let opposite = try routed(pin: "1B")
        let pads = opposite.problem.nets.flatMap(\.components).flatMap(\.pads)
        XCTAssertEqual(pads.count, 2)
        XCTAssertTrue(pads.allSatisfy { $0.layers == [.bottom] }, "\(pads.map(\.layers))")
        XCTAssertEqual(opposite.result.nets.map(\.status), [.complete])
        XCTAssertFalse(opposite.result.traces.isEmpty)
        XCTAssertTrue(opposite.result.traces.allSatisfy { $0.layer == .bottom }, "\(opposite.result.traces.map(\.layer))")

        let own = try routed(pin: "1A")
        XCTAssertTrue(own.problem.nets.flatMap(\.components).flatMap(\.pads).allSatisfy { $0.layers == [.top] })
        XCTAssertEqual(own.result.nets.map(\.status), [.complete])
        XCTAssertTrue(own.result.traces.allSatisfy { $0.layer == .top })
    }
}
