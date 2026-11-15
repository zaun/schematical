import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// Net classes (plan-38-A): their edits and clamps, the file, net
/// resolution and names, and the `auto` flag on traces and vias.
@MainActor
final class NetClassTests: XCTestCase {
    private func netClass(_ doc: SchematicDocument, _ id: UUID) throws -> NetClass {
        try XCTUnwrap(doc.board.netClasses.first { $0.id == id })
    }

    // MARK: The Default class

    /// The Default class is the smallest legal copper of every manufacturer's rules.
    func testDefaultClassIsLegalAndMinimalForEveryManufacturer() {
        let all = DRCManufacturer.allCases.compactMap(\.fixedRules) + [DesignRules.customDefault]
        XCTAssertGreaterThan(all.count, 5)
        for rules in all {
            let standard = NetRules.defaults(for: rules)
            XCTAssertNil(NetClass.problem(width: standard.width, clearance: standard.clearance, viaDrill: standard.viaDrill,
                                          viaDiameter: standard.viaDiameter, name: "Default"))
            let mm = Double(standard.width) * 0.0254
            XCTAssertGreaterThanOrEqual(mm, rules.minTraceWidth - 1e-9)
            // No wider than it must be: the next mil down is under the rule, or under a trace's least.
            XCTAssertTrue(mm - 0.0254 < rules.minTraceWidth || standard.width == BoardTrace.widthRange.lowerBound)
            XCTAssertNil(standard.clearance)
            XCTAssertEqual(standard.viaDrill, rules.minViaDrill, accuracy: 1e-9)
            XCTAssertGreaterThanOrEqual(standard.viaDiameter, rules.minViaDiameter - 1e-9)
            XCTAssertGreaterThanOrEqual((standard.viaDiameter - standard.viaDrill) / 2, rules.minAnnularRing - 1e-9)
            let least = max(rules.minViaDiameter, rules.minViaDrill + 2 * rules.minAnnularRing)
            XCTAssertEqual(standard.viaDiameter, least, accuracy: 0.001)
        }
    }

    func testDefaultClassUnderPCBWay() {
        let standard = NetRules.defaults(for: .pcbway)
        XCTAssertEqual(standard.width, 5)
        XCTAssertEqual(standard.viaDrill, 0.2)
        XCTAssertEqual(standard.viaDiameter, 0.5)
    }

    /// Rules outside what a trace or via can be still give a legal class.
    func testDefaultClassClampsToWhatCopperCanBe() {
        var rules = DesignRules.pcbway
        rules.minTraceWidth = 5
        rules.minViaDrill = 0.01
        rules.minViaDiameter = 0.02
        rules.minAnnularRing = 0
        let standard = NetRules.defaults(for: rules)
        XCTAssertEqual(standard.width, 100)
        XCTAssertNil(BoardVia.problem(drill: standard.viaDrill, diameter: standard.viaDiameter))
    }

    // MARK: Phase 1 — edits

    func testAddRenameRemoveAreEachOneUndoStep() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetClass(named: "  Power  ")
        XCTAssertEqual(doc.undoName(on: .pcb), "Add Net Class")
        let added = try netClass(doc, id)
        XCTAssertEqual(added.name, "Power")
        // A new class starts as the Default class: the design rules' minimums.
        let standard = NetRules.defaults(for: .current())
        XCTAssertEqual(added.width, standard.width)
        XCTAssertNil(added.clearance)
        XCTAssertEqual(added.viaDrill, standard.viaDrill)
        XCTAssertEqual(added.viaDiameter, standard.viaDiameter)
        XCTAssertTrue(added.members.isEmpty)

        doc.renameNetClass(id, to: "Supply")
        XCTAssertEqual(try netClass(doc, id).name, "Supply")
        XCTAssertEqual(doc.undoName(on: .pcb), "Rename Net Class")
        doc.renameNetClass(id, to: "   ")
        XCTAssertEqual(try netClass(doc, id).name, "Supply", "a blank name is ignored")
        doc.renameNetClass(id, to: String(repeating: "x", count: 50))
        XCTAssertEqual(try netClass(doc, id).name.count, 40)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).name, "Power")

        doc.removeNetClass(id)
        XCTAssertTrue(doc.board.netClasses.isEmpty)
        XCTAssertEqual(doc.undoName(on: .pcb), "Delete Net Class")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.netClasses.map(\.id), [id])
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertTrue(doc.board.netClasses.isEmpty)
    }

    func testWidthClearanceAndViaClamp() throws {
        let doc = TestSupport.newDocument()
        let id = doc.addNetClass(named: "A")
        doc.setNetClassWidth(id, 4)
        XCTAssertEqual(try netClass(doc, id).width, 5)
        doc.setNetClassWidth(id, 101)
        XCTAssertEqual(try netClass(doc, id).width, 100)
        doc.setNetClassWidth(id, 20)
        XCTAssertEqual(try netClass(doc, id).width, 20)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Class Width")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).width, 100)

        doc.setNetClassClearance(id, 0.01)
        XCTAssertEqual(try netClass(doc, id).clearance, 0.05)
        doc.setNetClassClearance(id, 20)
        XCTAssertEqual(try netClass(doc, id).clearance, 10)
        doc.setNetClassClearance(id, nil)
        XCTAssertNil(try netClass(doc, id).clearance)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).clearance, 10)

        doc.setNetClassVia(id, drill: 0.6, diameter: 0.6)
        var via = try netClass(doc, id)
        XCTAssertEqual(via.viaDrill, 0.6)
        XCTAssertEqual(via.viaDiameter, 0.7, accuracy: 1e-9, "the metal keeps the thinnest ring")
        doc.setNetClassVia(id, drill: 0.01, diameter: 50)
        via = try netClass(doc, id)
        XCTAssertEqual(via.viaDrill, 0.1)
        XCTAssertEqual(via.viaDiameter, 10)
        XCTAssertNil(via.problem)
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Class Via")
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, id).viaDrill, 0.6)
    }

    func testAssignNetMovesBetweenClassesAndBackToDefault() throws {
        let doc = TestSupport.newDocument()
        let a = doc.addNetClass(named: "A")
        let b = doc.addNetClass(named: "B")
        let pin = PlaneNet.pin(chip: UUID(), designator: "3")
        doc.assignNet(.ground, to: a)
        doc.assignNet(pin, to: a)
        XCTAssertEqual(try netClass(doc, a).members, [.ground, pin])
        doc.assignNet(.ground, to: b)
        XCTAssertEqual(try netClass(doc, a).members, [pin])
        XCTAssertEqual(try netClass(doc, b).members, [.ground])
        XCTAssertEqual(doc.undoName(on: .pcb), "Change Net Class")
        doc.assignNet(.ground, to: nil)
        XCTAssertTrue(try netClass(doc, b).members.isEmpty)
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, b).members, [.ground])
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(try netClass(doc, a).members, [.ground, pin])
        XCTAssertTrue(try netClass(doc, b).members.isEmpty)
    }

    func testMoveNetClassChangesPriority() throws {
        let doc = TestSupport.newDocument()
        let a = doc.addNetClass(named: "A")
        let b = doc.addNetClass(named: "B")
        let c = doc.addNetClass(named: "C")
        doc.moveNetClass(c, to: 0)
        XCTAssertEqual(doc.board.netClasses.map(\.id), [c, a, b])
        XCTAssertEqual(doc.undoName(on: .pcb), "Move Net Class")
        doc.moveNetClass(c, to: 99)
        XCTAssertEqual(doc.board.netClasses.map(\.id), [a, b, c])
        XCTAssertTrue(doc.undo(on: .pcb))
        XCTAssertEqual(doc.board.netClasses.map(\.id), [c, a, b])
    }

    // MARK: Phase 2 — file

    func testTwoClassesRoundTrip() throws {
        let doc = TestSupport.newDocument()
        let power = doc.addNetClass(named: "Power")
        doc.setNetClassWidth(power, 24)
        doc.setNetClassClearance(power, 0.3)
        doc.setNetClassVia(power, drill: 0.5, diameter: 1)
        doc.assignNet(.ground, to: power)
        doc.assignNet(.rail(mv: 5000), to: power)
        let signal = doc.addNetClass(named: "Signal")
        doc.assignNet(.pin(chip: UUID(), designator: "A1"), to: signal)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.board.netClasses, doc.board.netClasses)
        XCTAssertEqual(opened.board.netClasses.map(\.name), ["Power", "Signal"])
    }

    /// A document with no classes and no `auto` copper, as the build before
    /// plan-38 saved it (a trace, a via; captured with that build's
    /// encoder, commit 945720a).
    static let plainGolden = #"""
{
  "board" : {
    "color" : "green",
    "grid" : 0.635,
    "height" : 80,
    "layerStyles" : [
      {
        "color" : "#C83737",
        "layer" : "top",
        "type" : "component"
      },
      {
        "color" : "#3A6FD8",
        "layer" : "bottom",
        "type" : "component"
      }
    ],
    "layers" : 2,
    "texts" : [

    ],
    "traces" : [
      {
        "id" : "38A00000-0000-4000-8000-000000000001",
        "layer" : "top",
        "locked" : true,
        "points" : [
          {
            "x" : 1,
            "y" : 2
          },
          {
            "x" : 5,
            "y" : 2
          }
        ],
        "width" : 12
      }
    ],
    "units" : "mm",
    "vias" : [
      {
        "diameter" : 0.8,
        "drill" : 0.4,
        "id" : "38A00000-0000-4000-8000-000000000002",
        "x" : 5,
        "y" : 2
      }
    ],
    "width" : 100
  },
  "buses" : [

  ],
  "format" : "schematical",
  "junctions" : [

  ],
  "nextReference" : 1,
  "notes" : [

  ],
  "parts" : [

  ],
  "version" : 4,
  "wires" : [

  ]
}
"""#

    func testNoClassesWritesNoKeyAndTheSameBytes() throws {
        let doc = TestSupport.newDocument()
        try SchematicFile.decode(Data(Self.plainGolden.utf8)).apply(to: doc, catalog: TestSupport.catalog, fileURL: nil)
        XCTAssertEqual(doc.traces.count, 1)
        XCTAssertEqual(doc.vias.count, 1)
        let data = try SchematicFile(document: doc, fileURL: nil).encoded()
        let text = String(decoding: data, as: UTF8.self)
        XCTAssertFalse(text.contains("netClasses"))
        XCTAssertFalse(text.contains("auto"))
        XCTAssertEqual(text, Self.plainGolden)
    }

    func testClassFileErrorsNameTheClass() {
        let id = UUID()
        let name = "net class \(id.uuidString)"
        func board(_ entry: String, twice: Bool = false) -> String {
            let one = #"{ "id": "\#(id)", "name": "A", "viaDrill": 0.4, "viaDiameter": 0.8\#(entry) }"#
            return #"{ "netClasses": [ \#(twice ? one + ", " + one : one) ] }"#
        }
        let cases: [(String, String)] = [
            (board(#", "width": 4, "members": []"#), "\(name) width 4 must be 5 to 100 mil"),
            (board(#", "width": 10, "members": []"#, twice: true), "\(name) is listed twice"),
            (board(#", "width": 10, "members": ["pin:nope"]"#),
             "\(name) member \"pin:nope\" must be ground, power:<mV> or pin:<part id>/<pin>"),
        ]
        for (json, message) in cases {
            XCTAssertEqual(BoardLayersTests.pcbError { _ = try BoardLayersTests.open(board: json) }, message, json)
        }
    }

    // MARK: Phase 3 — net resolution and names

    /// The net of a pin's pad; `pin` is the pin's name or designator.
    private func net(_ doc: SchematicDocument, _ copper: BoardConnectivity, _ chip: PlacedChip,
                     _ pin: String) throws -> Int {
        let index = try XCTUnwrap(doc.chips.firstIndex { $0.id == chip.id })
        let designator = chip.part.pins.first { $0.name == pin }?.designator ?? pin
        let pad = try XCTUnwrap(copper.pads.firstIndex { $0.chip == index && $0.pad.designator == designator })
        return try XCTUnwrap(copper.netOfPad[pad])
    }

    func testGroundNetIsNamedAndResolves() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 100, y: 400))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "K")), .pin(PinRef(chipID: gnd.id, index: 0))))
        let other = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .pin(try TestSupport.pin(other, "K"))))
        let copper = doc.connectivity()
        let ground = try net(doc, copper, led, "K")
        XCTAssertEqual(copper.netNames[ground], "GND")
        XCTAssertEqual(copper.net(of: .ground), ground)
        let cathode = try XCTUnwrap(led.part.pins.first { $0.name == "K" }).designator
        XCTAssertEqual(copper.net(of: .pin(chip: led.id, designator: cathode)), ground)
        XCTAssertNil(copper.net(of: .rail(mv: 5000)))

        let power = doc.addNetClass(named: "Power")
        doc.assignNet(.ground, to: power)
        XCTAssertEqual(doc.netClass(ofNet: ground, in: copper)?.id, power)
        let anode = try net(doc, copper, led, "A")
        XCTAssertNil(doc.netClass(ofNet: anode, in: copper), "an unassigned net is in Default")
    }

    func testTwoFiveVoltSymbolsAreOneNetNamed5V() throws {
        let doc = TestSupport.newDocument()
        var leds: [PlacedChip] = []
        for n in 0..<2 {
            let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100 + 300 * n, y: 100))
            let rail = try TestSupport.place(doc, "5V", at: CGPoint(x: 100 + 300 * n, y: 400))
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .pin(PinRef(chipID: rail.id, index: 0))))
            leds.append(led)
        }
        let copper = doc.connectivity()
        let first = try net(doc, copper, leds[0], "A"), second = try net(doc, copper, leds[1], "A")
        XCTAssertEqual(first, second)
        XCTAssertEqual(copper.netNames[first], "5V")
        XCTAssertEqual(copper.net(of: .rail(mv: 5000)), first)
        XCTAssertEqual(Set(copper.netNames.values).count, copper.netNames.count, "names are unique")
    }

    func testLabelledNetTakesTheLabel() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        let other = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 700, y: 100))
        let label = doc.addNetLabel(at: CGPoint(x: 200, y: 100))
        XCTAssertTrue(doc.setNetLabelName(label, "CLK"))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .label(label)))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(led, "A")), .pin(try TestSupport.pin(other, "K"))))
        let copper = doc.connectivity()
        XCTAssertEqual(copper.netNames[try net(doc, copper, led, "A")], "CLK")
    }

    func testPlainNetIsNamedByItsFirstPad() throws {
        let doc = TestSupport.newDocument()
        let one = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let two = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(two, "A")), .pin(try TestSupport.pin(one, "K"))))
        let copper = doc.connectivity()
        let pad = try XCTUnwrap(one.boardPads.first { pad in
            one.part.pins.first { $0.name == "K" }.map { $0.designator == pad.designator } ?? false
        })
        let name = "\(one.reference).\(try XCTUnwrap(pad.designator))"
        XCTAssertEqual(copper.netNames[try net(doc, copper, one, try XCTUnwrap(pad.designator))], name)
        XCTAssertTrue(name.hasPrefix("D1.") || name.hasPrefix("LED1."), name)
    }

    func testNetInTwoClassesIsInTheFirst() throws {
        let doc = TestSupport.newDocument()
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 100, y: 100))
        let other = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 400, y: 100))
        let k = try TestSupport.pin(led, "K")
        XCTAssertNil(doc.connect(.pin(k), .pin(try TestSupport.pin(other, "A"))))
        let designator = led.part.pins[k.index].designator
        let a = doc.addNetClass(named: "A")
        let b = doc.addNetClass(named: "B")
        doc.assignNet(.pin(chip: led.id, designator: designator), to: b)
        let otherDesignator = other.part.pins[try TestSupport.pin(other, "A").index].designator
        doc.assignNet(.pin(chip: other.id, designator: otherDesignator), to: a)
        let copper = doc.connectivity()
        let n = try net(doc, copper, led, designator)
        XCTAssertEqual(doc.netClass(ofNet: n, in: copper)?.id, a)
        doc.moveNetClass(b, to: 0)
        XCTAssertEqual(doc.netClass(ofNet: n, in: copper)?.id, b)
    }

    // MARK: Phase 4 — auto

    func testAutoTraceAndViaRoundTrip() throws {
        let traceID = UUID(), viaID = UUID()
        let board = #"""
        { "traces": [ { "id": "\#(traceID)", "layer": "top", "points": [ { "x": 1, "y": 1 }, { "x": 4, "y": 1 } ],
                        "auto": true } ],
          "vias": [ { "id": "\#(viaID)", "x": 4, "y": 1, "auto": true } ] }
        """#
        let doc = try BoardLayersTests.open(board: board)
        XCTAssertEqual(doc.traces.map(\.auto), [true])
        XCTAssertEqual(doc.vias.map(\.auto), [true])
        let text = String(decoding: try SchematicFile(document: doc, fileURL: nil).encoded(), as: UTF8.self)
        XCTAssertEqual(text.components(separatedBy: "\"auto\" : true").count - 1, 2)
        let opened = try BoardLayersTests.roundTrip(doc)
        XCTAssertEqual(opened.traces, doc.traces)
        XCTAssertEqual(opened.vias, doc.vias)
    }

    func testMissingAutoReadsFalse() throws {
        let board = #"""
        { "traces": [ { "id": "\#(UUID())", "layer": "top", "points": [ { "x": 1, "y": 1 }, { "x": 4, "y": 1 } ] } ],
          "vias": [ { "id": "\#(UUID())", "x": 4, "y": 1 } ] }
        """#
        let doc = try BoardLayersTests.open(board: board)
        XCTAssertEqual(doc.traces.map(\.auto), [false])
        XCTAssertEqual(doc.vias.map(\.auto), [false])
    }

    func testPastedAutoTraceIsNotAuto() throws {
        let doc = TestSupport.newDocument()
        let trace = BoardTrace(id: UUID(), layer: .top, width: 10, points: [CGPoint(x: 1, y: 1), CGPoint(x: 4, y: 1)],
                               auto: true)
        let ids = doc.pasteTraces([trace], offset: CGSize(width: 0, height: 5))
        XCTAssertEqual(ids.count, 1)
        let pasted = try XCTUnwrap(doc.traces.first { $0.id == ids.first })
        XCTAssertFalse(pasted.auto)
    }

    func testProblem() {
        func problem(width: Int = 10, clearance: Double? = nil, drill: Double = 0.4, diameter: Double = 0.8,
                     name: String = "A") -> String? {
            NetClass.problem(width: width, clearance: clearance, viaDrill: drill, viaDiameter: diameter, name: name)
        }
        XCTAssertNil(problem())
        XCTAssertNil(problem(width: 5, clearance: 0.05))
        XCTAssertNotNil(problem(width: 4))
        XCTAssertNotNil(problem(width: 101))
        XCTAssertNotNil(problem(clearance: 0.04))
        XCTAssertNotNil(problem(drill: 0.4, diameter: 0.49))
        XCTAssertNotNil(problem(name: "   "))
        XCTAssertNotNil(problem(name: ""))
        XCTAssertNotNil(problem(name: String(repeating: "x", count: 41)))
    }
}
