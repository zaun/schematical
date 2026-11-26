import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Movable chains (plan-66-B): which existing copper push and shove may move.
@MainActor
final class RoutingChainTests: XCTestCase {
    /// Three through-hole LEDs on a board. Net X joins K1 and A2 (and, with
    /// `tied`, A1); net Y joins A1 and K2 when not `tied`.
    private struct Rig {
        let doc: SchematicDocument
        let k1: CGPoint, a1: CGPoint, k2: CGPoint, a2: CGPoint
    }

    private func centre(_ doc: SchematicDocument, _ ref: PinRef) throws -> CGPoint {
        let chip = try XCTUnwrap(doc.chips.first { $0.id == ref.chipID })
        let designator = chip.part.pins[ref.index].designator
        let pad = try XCTUnwrap(chip.boardPads.first { $0.designator == designator })
        return CGPoint(x: pad.rect.midX, y: pad.rect.midY)
    }

    private func rig(tied: Bool = false) throws -> Rig {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        doc.moveFootprint(one.id, to: CGPoint(x: 30, y: 40))
        doc.moveFootprint(two.id, to: CGPoint(x: 60, y: 40))
        let k1 = try TestSupport.pin(one, "K"), a1 = try TestSupport.pin(one, "A")
        let k2 = try TestSupport.pin(two, "K"), a2 = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k1), .pin(a2)))
        if tied { XCTAssertNil(doc.connect(.pin(k1), .pin(a1))) } else { XCTAssertNil(doc.connect(.pin(a1), .pin(k2))) }
        return Rig(doc: doc, k1: try centre(doc, k1), a1: try centre(doc, a1), k2: try centre(doc, k2), a2: try centre(doc, a2))
    }

    private func options(_ change: (inout RoutingOptions) -> Void = { _ in }) -> RoutingOptions {
        var options = RoutingOptions()
        options.pushAndShove = true
        change(&options)
        return options
    }

    private func problem(_ doc: SchematicDocument, push: Bool = true,
                         _ change: (inout RoutingOptions) -> Void = { _ in }) -> RoutingProblem {
        var o = options(change)
        o.pushAndShove = push
        return doc.routingProblem(options: o, rules: .customDefault)
    }

    /// A zig-zag from K1 down, across and up to A2: two bends, three segments.
    private func zigzag(_ rig: Rig, y: Double = 8) -> [CGPoint] {
        [rig.k1, CGPoint(x: rig.k1.x, y: rig.k1.y + y), CGPoint(x: rig.a2.x, y: rig.k1.y + y), rig.a2]
    }

    /// Whether the copper is left alone: no chain, the obstacles as with the option off.
    private func assertFixed(_ doc: SchematicDocument, _ message: String, file: StaticString = #filePath, line: UInt = #line,
                             _ change: (inout RoutingOptions) -> Void = { _ in }) {
        let on = problem(doc, push: true, change), off = problem(doc, push: false, change)
        XCTAssertTrue(on.movable.isEmpty, message, file: file, line: line)
        XCTAssertTrue(on.bystanders.isEmpty, message, file: file, line: line)
        XCTAssertEqual(on.obstacles, off.obstacles, message, file: file, line: line)
    }

    func testAPadToPadTraceWithBendsIsOneChain() throws {
        let rig = try rig()
        let id = try XCTUnwrap(rig.doc.addTrace(points: zigzag(rig), layer: .top, width: 10))
        let on = problem(rig.doc), off = problem(rig.doc, push: false)
        XCTAssertEqual(on.movable.count, 1)
        let chain = try XCTUnwrap(on.movable.first)
        XCTAssertEqual(chain.id, 0)
        XCTAssertEqual(chain.runs.count, 1)
        XCTAssertTrue(chain.vias.isEmpty)
        XCTAssertEqual(chain.runs[0].trace, id)
        XCTAssertEqual(chain.runs[0].layer, .top)
        XCTAssertEqual(chain.runs[0].points.count, 4, "the bends stay inside one run")
        XCTAssertEqual(chain.runs[0].first, 0)
        XCTAssertEqual(chain.runs[0].last, 3)
        XCTAssertFalse(chain.runs[0].reversed)
        XCTAssertEqual(chain.width, 10 * 0.0254, accuracy: 1e-9)
        XCTAssertEqual(chain.ends.count, 2)
        XCTAssertEqual(chain.ends.map(\.anchor.pads.count), [1, 1])
        XCTAssertEqual(chain.ends[0].point, rig.k1)
        XCTAssertEqual(chain.ends[1].point, rig.a2)
        XCTAssertTrue(chain.ends.allSatisfy { !$0.anchor.padLabels[0].isEmpty && $0.anchor.pads[0].node >= 0 })
        // The chain's three segments are out of the obstacles, nothing else.
        XCTAssertEqual(off.obstacles.count - on.obstacles.count, 3)
        XCTAssertEqual(Set(on.obstacles).subtracting(off.obstacles), [])
        // Its net has a bystander with the chain's width, and no copper to route.
        XCTAssertEqual(on.bystanders.map(\.id), [chain.net])
        XCTAssertEqual(on.bystanders[0].width, chain.width, accuracy: 1e-9)
        XCTAssertTrue(on.bystanders[0].components.isEmpty)
        XCTAssertEqual(on.bystanders[0].connections, 0)
        XCTAssertFalse(on.nets.contains { $0.id == chain.net }, "a net that is not routed")
    }

    func testATrunkWithABranchIsThreeChainsMeetingAtAJunction() throws {
        let rig = try rig()
        let trunk = zigzag(rig)
        XCTAssertNotNil(rig.doc.addTrace(points: trunk, layer: .top, width: 10))
        // A branch leaves the second bend and ends free.
        let bend = trunk[2]
        XCTAssertNotNil(rig.doc.addTrace(points: [bend, CGPoint(x: bend.x, y: bend.y + 6)], layer: .top, width: 10))
        let on = problem(rig.doc)
        XCTAssertEqual(on.movable.count, 3)
        XCTAssertEqual(on.movable.map(\.id), [0, 1, 2])
        // Each chain ends at two anchors; the junction is shared by all three.
        let atBend = on.movable.flatMap(\.ends).filter { $0.point == bend }
        XCTAssertEqual(atBend.count, 3)
        XCTAssertTrue(atBend.allSatisfy { $0.anchor.pads.count == 1 && $0.anchor.pads[0].node == -2 }, "a junction is a pad-like shape")
        XCTAssertTrue(atBend.allSatisfy { $0.anchor.pads[0].radius == RoutingOptions().grid })
        let free = on.movable.flatMap(\.ends).filter { $0.point == CGPoint(x: bend.x, y: bend.y + 6) }
        XCTAssertEqual(free.count, 1, "the branch's free end")
        XCTAssertEqual(on.movable.map(\.runs.count), [1, 1, 1])
        let covered = on.movable.reduce(0) { $0 + $1.runs.reduce(0) { $0 + $1.last - $1.first } }
        XCTAssertEqual(covered, 4, "three trunk segments and the branch")
    }

    func testATopViaBottomPathIsOneChain() throws {
        let rig = try rig()
        let p = CGPoint(x: rig.k1.x, y: rig.k1.y + 8)
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.k1, p], layer: .top, width: 10))
        XCTAssertNotNil(rig.doc.addTrace(points: [p, CGPoint(x: rig.a2.x, y: p.y), rig.a2], layer: .bottom, width: 10))
        let viaID = try XCTUnwrap(rig.doc.addVia(at: p))
        let on = problem(rig.doc), off = problem(rig.doc, push: false)
        XCTAssertEqual(on.movable.count, 1)
        let chain = try XCTUnwrap(on.movable.first)
        XCTAssertEqual(chain.vias.map(\.id), [viaID])
        XCTAssertEqual(chain.vias.map(\.centre), [p])
        XCTAssertEqual(chain.runs.count, 2)
        XCTAssertNotEqual(chain.runs[0].layer, chain.runs[1].layer)
        XCTAssertEqual(chain.runs[0].points, [rig.k1, p])
        XCTAssertEqual(chain.runs[1].points, [p, CGPoint(x: rig.a2.x, y: p.y), rig.a2])
        XCTAssertGreaterThan(chain.viaDiameter, chain.viaDrill)
        XCTAssertEqual(on.bystanders[0].viaDrill, chain.viaDrill)
        XCTAssertEqual(on.bystanders[0].viaDiameter, chain.viaDiameter)
        // Three segments and the via are out of the obstacles.
        XCTAssertEqual(off.obstacles.count - on.obstacles.count, 4)
        XCTAssertFalse(on.obstacles.contains { $0.kind == .via })
    }

    func testAChainRunningTheOtherWayIsReversed() throws {
        let rig = try rig()
        // Drawn from A2 to K1, then the chain still reads from its lowest segment.
        XCTAssertNotNil(rig.doc.addTrace(points: Array(zigzag(rig).reversed()), layer: .top, width: 10))
        let chain = try XCTUnwrap(problem(rig.doc).movable.first)
        XCTAssertEqual(chain.runs[0].first, 0)
        XCTAssertEqual(chain.runs[0].last, 3)
        XCTAssertEqual(chain.ends[0].point, rig.a2, "the trace's first point is end A")
        XCTAssertEqual(chain.runs[0].points, Array(zigzag(rig).reversed()))
    }

    // MARK: The chains that stay put

    func testALockedTraceStays() throws {
        let rig = try rig()
        let id = try XCTUnwrap(rig.doc.addTrace(points: zigzag(rig), layer: .top, width: 10))
        rig.doc.setLocked(.trace(id), true)
        assertFixed(rig.doc, "locked trace")
    }

    func testALockedViaKeepsItsChain() throws {
        let rig = try rig()
        let p = CGPoint(x: rig.k1.x, y: rig.k1.y + 8)
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.k1, p], layer: .top, width: 10))
        XCTAssertNotNil(rig.doc.addTrace(points: [p, CGPoint(x: rig.a2.x, y: p.y), rig.a2], layer: .bottom, width: 10))
        let viaID = try XCTUnwrap(rig.doc.addVia(at: p))
        rig.doc.setLocked(.via(viaID), true)
        assertFixed(rig.doc, "locked via")
    }

    func testANetTheRunRoutesIsNotChained() throws {
        let rig = try rig()
        // Only half of net X: it has two components, so the run routes it and keeps its copper as it is.
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.k1, CGPoint(x: rig.k1.x, y: rig.k1.y + 8)], layer: .top, width: 10))
        assertFixed(rig.doc, "net in the run")
        let connected = problem(rig.doc, push: false)
        XCTAssertTrue(connected.nets.contains { $0.components.count == 2 })
    }

    func testTwoWidthsInOneChainStay() throws {
        let rig = try rig()
        let p = CGPoint(x: rig.k1.x, y: rig.k1.y + 8)
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.k1, p], layer: .top, width: 10))
        XCTAssertNotNil(rig.doc.addTrace(points: [p, CGPoint(x: rig.a2.x, y: p.y), rig.a2], layer: .bottom, width: 20))
        XCTAssertNotNil(rig.doc.addVia(at: p))
        assertFixed(rig.doc, "two widths")
    }

    func testAPadTouchingAMiddleSegmentKeepsItsChain() throws {
        let rig = try rig(tied: true)
        // K1 to a point past A1 on the same line: the trace runs over A1's copper with no vertex there.
        let past = CGPoint(x: rig.a1.x + (rig.a1.x - rig.k1.x), y: rig.a1.y + (rig.a1.y - rig.k1.y))
        let over = try XCTUnwrap(rig.doc.addTrace(points: [rig.k1, past], layer: .top, width: 10))
        // From its end to A2, so net X (K1, A1, A2) is one piece and not routed.
        let onward = try XCTUnwrap(rig.doc.addTrace(points: [past, CGPoint(x: past.x, y: past.y + 8),
                                                             CGPoint(x: rig.a2.x, y: past.y + 8), rig.a2],
                                                    layer: .top, width: 10))
        XCTAssertEqual(rig.doc.traces.first { $0.id == over }?.points, [rig.k1, past], "no vertex was added over A1")
        let on = problem(rig.doc)
        XCTAssertTrue(on.nets.isEmpty, "net X is whole; net Y has no copper and one pad")
        // The trace over A1 stays; the one from its end moves.
        XCTAssertEqual(on.movable.count, 1)
        XCTAssertEqual(on.movable.first?.runs.map(\.trace), [onward])
        XCTAssertEqual(on.movable.first?.ends.first?.point, past)
        XCTAssertEqual(on.movable.first?.ends.first?.anchor.pads.first?.node, -2, "a junction of two traces")
    }

    func testARunOnADisabledLayerStays() throws {
        let rig = try rig()
        let p = CGPoint(x: rig.k1.x, y: rig.k1.y + 8)
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.k1, p, CGPoint(x: rig.a2.x, y: p.y), rig.a2], layer: .bottom, width: 10))
        XCTAssertEqual(problem(rig.doc).movable.count, 1, "movable while the bottom layer is on")
        assertFixed(rig.doc, "disabled layer") { $0.layers[.bottom] = .init(enabled: false, direction: .auto) }
    }

    func testAClosedLoopStays() throws {
        let rig = try rig()
        let a = CGPoint(x: rig.k1.x, y: rig.k1.y + 5), b = CGPoint(x: rig.k1.x + 5, y: rig.k1.y + 5)
        // Out of K1 and back to it: one anchor, two chain ends.
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.k1, a, b, rig.k1], layer: .top, width: 10))
        XCTAssertTrue(problem(rig.doc).movable.isEmpty)
    }

    func testANetsFirstChainSetsItsWidth() throws {
        let rig = try rig(tied: true)
        // Net X = K1, A1, A2, whole: a 10 mil trace K1 to A2 on top and a 20 mil one A1 to A2 below.
        let low = rig.k1.y + 8, high = rig.k1.y + 12
        let first = try XCTUnwrap(rig.doc.addTrace(points: [rig.k1, CGPoint(x: rig.k1.x, y: low), CGPoint(x: rig.a2.x, y: low), rig.a2],
                                                   layer: .top, width: 10))
        XCTAssertNotNil(rig.doc.addTrace(points: [rig.a1, CGPoint(x: rig.a1.x, y: high), CGPoint(x: rig.a2.x, y: high), rig.a2],
                                         layer: .bottom, width: 20))
        let on = problem(rig.doc)
        XCTAssertTrue(on.nets.isEmpty, "one piece, not routed")
        // One routing profile per net: the first chain's 10 mil; the 20 mil chain stays an obstacle.
        XCTAssertEqual(on.movable.count, 1)
        XCTAssertEqual(on.movable.first?.runs.map(\.trace), [first])
        XCTAssertEqual(on.movable.first?.width ?? 0, 10 * 0.0254, accuracy: 1e-9)
        XCTAssertEqual(on.bystanders.count, 1)
        let off = problem(rig.doc, push: false)
        XCTAssertEqual(off.obstacles.count - on.obstacles.count, 3, "only the first chain's three segments left the obstacles")
    }

    // MARK: Over a routed board

    /// The routed RegFile with `scope` one of its nets, the option on and off.
    private func routedRegFile() throws -> (doc: SchematicDocument, scope: PlaneNet) {
        let doc = try Fixture.open("RegFile")
        let first = doc.routingProblem(options: .fixture, rules: .customDefault)
        let result = Router.route(first)
        doc.applyRouting(result, problem: first)
        return (doc, try XCTUnwrap(first.nets.first?.key))
    }

    func testEveryExistingSegmentIsInExactlyOneChainOrOneObstacle() throws {
        let (doc, scope) = try routedRegFile()
        var o = RoutingOptions.fixture
        o.pushAndShove = true
        o.scope = .nets([scope])
        let on = doc.routingProblem(options: o, rules: .customDefault)
        let copper = doc.connectivity()
        let netOfRoot = copper.netOfRoot()
        XCTAssertGreaterThan(copper.traces.count, 0)
        XCTAssertFalse(on.movable.isEmpty, "a routed board has chains")
        var fromCopper: [Int: Int] = [:], fromProblem: [Int: Int] = [:]
        for (t, trace) in copper.traces.enumerated() {
            guard let net = netOfRoot[copper.root[copper.traceNode(t)]] else { continue }
            fromCopper[net, default: 0] += trace.segments.count
        }
        for v in copper.vias.indices {
            guard let net = netOfRoot[copper.root[copper.viaNode(v)]] else { continue }
            fromCopper[net, default: 0] += 1
        }
        for obstacle in on.obstacles where obstacle.kind == .trace || obstacle.kind == .via {
            if let net = obstacle.net { fromProblem[net, default: 0] += 1 }
        }
        for chain in on.movable {
            fromProblem[chain.net, default: 0] += chain.runs.reduce(0) { $0 + $1.last - $1.first } + chain.vias.count
        }
        XCTAssertEqual(fromProblem, fromCopper)
        XCTAssertEqual(Set(on.bystanders.map(\.id)), Set(on.movable.map(\.net)))
        // No segment in two chains.
        var seen = Set<String>()
        for chain in on.movable {
            for run in chain.runs {
                for s in run.first..<run.last { XCTAssertTrue(seen.insert("\(run.trace)/\(s)").inserted) }
            }
        }
    }

    func testTheOptionOffGivesTheSameObstaclesAsBefore() throws {
        let (doc, scope) = try routedRegFile()
        var o = RoutingOptions.fixture
        o.scope = .nets([scope])
        let off = doc.routingProblem(options: o, rules: .customDefault)
        XCTAssertTrue(off.movable.isEmpty)
        XCTAssertTrue(off.bystanders.isEmpty)
        o.pushAndShove = true
        let on = doc.routingProblem(options: o, rules: .customDefault)
        // With the option on, the obstacles plus the chains' own copper are the obstacles with it off.
        let copper = doc.connectivity()
        let netOfRoot = copper.netOfRoot()
        var rebuilt = Set(on.obstacles)
        for chain in on.movable {
            for run in chain.runs {
                let t = try XCTUnwrap(copper.traces.firstIndex { $0.id == run.trace })
                let trace = copper.traces[t]
                let node = copper.traceNode(t)
                for s in run.first..<run.last {
                    rebuilt.insert(RoutingObstacle(shape: CopperShape(core: .segment(trace.points[s], trace.points[s + 1]),
                                                                      radius: trace.widthMM / 2, layers: [trace.layer], node: node),
                                                   net: netOfRoot[copper.root[node]], kind: .trace))
                }
            }
            for via in chain.vias {
                let v = try XCTUnwrap(copper.vias.firstIndex { $0.id == via.id })
                let node = copper.viaNode(v), real = copper.vias[v]
                rebuilt.insert(RoutingObstacle(shape: CopperShape(core: .segment(real.centre, real.centre),
                                                                  radius: real.diameter / 2, layers: nil, node: node),
                                               net: netOfRoot[copper.root[node]], kind: .via,
                                               drill: CopperShape(core: .segment(real.centre, real.centre),
                                                                  radius: real.drill / 2, layers: nil, node: node)))
            }
        }
        XCTAssertEqual(rebuilt, Set(off.obstacles))
        XCTAssertEqual(on.obstacles.count + on.movable.reduce(0) { $0 + $1.runs.reduce(0) { $0 + $1.last - $1.first } + $1.vias.count },
                       off.obstacles.count)
        // Everything but the obstacles is as it is with the option off.
        XCTAssertEqual(on.nets, off.nets)
        XCTAssertEqual(on.groups, off.groups)
    }
}
