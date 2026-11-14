import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Per-net layer rules and net groups (plan-38-B): resolution, edits, the
/// file and the DRC's layer-rule line.
@MainActor
final class NetRulesTests: XCTestCase {
    /// Two LEDs, K of the first wired to A of the second: one net, named by
    /// `.pin(first, K)`.
    private func wiredPair() throws -> (doc: SchematicDocument, member: PlaneNet, chips: [PlacedChip]) {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        let k = try TestSupport.pin(one, "K"), a = try TestSupport.pin(two, "A")
        XCTAssertNil(doc.connect(.pin(k), .pin(a)))
        return (doc, .pin(chip: one.id, designator: one.part.pins[k.index].designator), [one, two])
    }

    private func net(_ doc: SchematicDocument, _ member: PlaneNet) throws -> (Int, BoardConnectivity) {
        let copper = doc.connectivity()
        return (try XCTUnwrap(copper.net(of: member)), copper)
    }

    private func group(_ doc: SchematicDocument, _ id: UUID) throws -> NetGroup {
        try XCTUnwrap(doc.board.netGroups.first { $0.id == id })
    }

    // MARK: Phase 1 — resolution and edits

    func testOverrideBeatsClassRuleBeatsNone() throws {
        let (doc, member, _) = try wiredPair()
        var (n, copper) = try net(doc, member)
        XCTAssertNil(doc.layerRule(ofNet: n, in: copper))
        XCTAssertEqual(doc.effectiveLayers(ofNet: n, in: copper), EffectiveLayers(allowed: [.top, .bottom], preferred: []))

        let power = doc.addNetClass(named: "Power")
        doc.assignNet(member, to: power)
        let classRule = LayerRule(mode: .only, layers: [.bottom])
        doc.setNetClassLayerRule(power, classRule)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Layer Rule")
        (n, copper) = try net(doc, member)
        XCTAssertEqual(doc.layerRule(ofNet: n, in: copper), classRule)
        XCTAssertEqual(doc.effectiveLayers(ofNet: n, in: copper).allowed, [.bottom])

        let own = LayerRule(mode: .only, layers: [.top])
        doc.setNetLayerRule(member, own)
        XCTAssertEqual(doc.layerRule(ofNet: n, in: copper), own)
        XCTAssertEqual(doc.effectiveLayers(ofNet: n, in: copper).allowed, [.top])
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Layer Rule")

        doc.setNetLayerRule(member, nil)
        XCTAssertTrue(doc.board.netOverrides.isEmpty)
        XCTAssertEqual(doc.layerRule(ofNet: n, in: copper), classRule)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.layerRule(ofNet: n, in: copper), own)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.layerRule(ofNet: n, in: copper), classRule)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertNil(doc.layerRule(ofNet: n, in: copper))
    }

    func testOnlyAndPreferLayers() throws {
        let (doc, member, _) = try wiredPair()
        let (n, copper) = try net(doc, member)
        doc.setNetLayerRule(member, LayerRule(mode: .only, layers: [.top]))
        XCTAssertEqual(doc.effectiveLayers(ofNet: n, in: copper).allowed, [.top])

        doc.setNetLayerRule(member, LayerRule(mode: .prefer, layers: [.bottom]))
        let prefer = doc.effectiveLayers(ofNet: n, in: copper)
        XCTAssertEqual(prefer.allowed, [.top, .bottom], "Prefer leaves every trace layer allowed")
        XCTAssertEqual(prefer.preferred, [.bottom])

        // A Plane layer is no trace layer: Only on it alone leaves nothing.
        doc.setLayerCount(4)
        XCTAssertEqual(doc.board.style(of: .inner(2)).type, .plane)
        doc.setNetLayerRule(member, LayerRule(mode: .only, layers: [.inner(2), .bottom]))
        XCTAssertEqual(doc.effectiveLayers(ofNet: n, in: doc.connectivity()).allowed, [.bottom])
        doc.setNetLayerRule(member, LayerRule(mode: .only, layers: [.inner(2)]))
        XCTAssertEqual(doc.effectiveLayers(ofNet: n, in: doc.connectivity()).allowed, [])

        // An empty rule is ignored.
        doc.setNetLayerRule(member, LayerRule(mode: .only, layers: []))
        XCTAssertEqual(doc.board.netOverrides.first?.layerRule?.layers, [.inner(2)])
    }

    func testPairRefusesAThirdMemberAndAModeSwitchWithThree() throws {
        let doc = TestSupport.newDocument()
        let nets = (0..<3).map { _ in PlaneNet.pin(chip: UUID(), designator: "1") }
        let pair = doc.addNetGroup(named: "USB", mode: .diffPair)
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Net Group")
        XCTAssertTrue(doc.assignNet(nets[0], toGroup: pair))
        XCTAssertTrue(doc.assignNet(nets[1], toGroup: pair))
        XCTAssertFalse(doc.assignNet(nets[2], toGroup: pair))
        XCTAssertNotNil(doc.editorMessage)
        XCTAssertEqual(try group(doc, pair).members, [nets[0], nets[1]])

        let bus = doc.addNetGroup(named: "Bus", mode: .lengthMatch)
        for member in nets { XCTAssertTrue(doc.assignNet(member, toGroup: bus)) }
        XCTAssertTrue(try group(doc, pair).members.isEmpty, "assigning moved them out of the pair")
        doc.editorMessage = nil
        XCTAssertFalse(doc.setNetGroupMode(bus, .diffPair))
        XCTAssertNotNil(doc.editorMessage)
        XCTAssertEqual(try group(doc, bus).mode, .lengthMatch)
        doc.unassignNet(nets[2])
        XCTAssertTrue(doc.setNetGroupMode(bus, .diffPair))
        XCTAssertEqual(try group(doc, bus).mode, .diffPair)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Group")
    }

    func testAssigningToASecondGroupRemovesItFromTheFirst() throws {
        let (doc, member, _) = try wiredPair()
        let one = doc.addNetGroup(named: "One", mode: .lengthMatch)
        let two = doc.addNetGroup(named: "Two", mode: .lengthMatch)
        XCTAssertTrue(doc.assignNet(member, toGroup: one))
        let (n, copper) = try net(doc, member)
        XCTAssertEqual(doc.group(ofNet: n, in: copper)?.id, one)
        XCTAssertTrue(doc.assignNet(member, toGroup: two))
        XCTAssertTrue(try group(doc, one).members.isEmpty)
        XCTAssertEqual(try group(doc, two).members, [member])
        XCTAssertEqual(doc.group(ofNet: n, in: copper)?.id, two)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try group(doc, one).members, [member])
        XCTAssertTrue(try group(doc, two).members.isEmpty)
    }

    func testEachGroupEditIsOneUndoStepAndClamps() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetGroup(named: "  Bus  ", mode: .lengthMatch)
        XCTAssertEqual(try group(doc, id).name, "Bus")
        XCTAssertEqual(try group(doc, id).tolerance, 0.25)
        XCTAssertEqual(try group(doc, id).gap, 0.2)
        XCTAssertEqual(try group(doc, id).maxUncoupled, 5)
        XCTAssertEqual(try group(doc, id).maxMeander, 4)

        let edits: [(String, () -> Void, (NetGroup) -> Bool)] = [
            ("Rename Net Group", { doc.renameNetGroup(id, to: "Data") }, { $0.name == "Data" }),
            ("Change Net Group", { doc.setNetGroupTolerance(id, 0) }, { $0.tolerance == 0.01 }),
            ("Change Net Group", { doc.setNetGroupGap(id, 99) }, { $0.gap == 10 }),
            ("Change Net Group", { doc.setNetGroupMaxUncoupled(id, -1) }, { $0.maxUncoupled == 0 }),
            ("Change Net Group", { doc.setNetGroupMaxMeander(id, 0.1) }, { $0.maxMeander == 0.5 }),
        ]
        for (name, edit, holds) in edits {
            let before = try group(doc, id)
            edit()
            XCTAssertTrue(holds(try group(doc, id)), name)
            XCTAssertEqual(doc.undoName(on: .pcb), name)
            XCTAssertTrue(doc.undo(on: .pcb))
            XCTAssertEqual(try group(doc, id), before, name)
            XCTAssertTrue(doc.redo(on: .pcb))
        }
        doc.removeNetGroup(id)
        XCTAssertTrue(doc.board.netGroups.isEmpty)
        XCTAssertEqual(doc.undoName(on: .pcb), "Delete Net Group")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.netGroups.map(\.id), [id])
    }

    func testGroupProblem() {
        func problem(_ mode: NetGroupMode = .lengthMatch, members: Int = 2, tolerance: Double = 0.25,
                     name: String = "G") -> String? {
            NetGroup.problem(mode: mode, members: members, tolerance: tolerance, gap: 0.2, maxUncoupled: 5,
                             maxMeander: 4, name: name)
        }
        XCTAssertNil(problem())
        XCTAssertNil(problem(.diffPair, members: 0), "a pair being built")
        XCTAssertNotNil(problem(.diffPair, members: 3))
        XCTAssertNil(problem(members: 5))
        XCTAssertNotNil(problem(tolerance: 0))
        XCTAssertNotNil(problem(name: " "))
    }
}
