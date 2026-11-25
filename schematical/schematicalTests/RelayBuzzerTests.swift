import AVFoundation
import CoreGraphics
import Foundation
import SwiftUI
import XCTest
@testable import schematical

/// plan-47-N: relays and buzzers in the app. Every relay draws as a relay (a
/// coil or LED on the left, the poles' `NO`, `COM`, `NC` on the right) and
/// every buzzer as the half disc; a relay's arms swing while its light says
/// it is operated; a buzzer's voice (`BuzzerTone`) is the part's rated
/// frequency for an indicator and the drive's for a transducer; and the
/// audio renderer makes the square wave. No test opens the audio device
/// (`BuzzerSoundSetting.isUnderTest`), and the part counts are never
/// hard-coded: the part files are regenerated. Each part is chosen by
/// property from the catalog, the first by name.
@MainActor
final class RelayBuzzerTests: XCTestCase {
    private static var keptSimulations: [SimulationController] = []

    private func parts(_ kind: PartKind) -> [CatalogPart] {
        TestSupport.catalog.allParts.filter { $0.kind == kind }
    }

    private func relay(_ test: (RelayInfo, CatalogPart) -> Bool) throws -> CatalogPart {
        try XCTUnwrap(parts(PART_RELAY).sorted { $0.name < $1.name }.first { part in
            part.relay.map { test($0, part) } ?? false
        })
    }

    private func buzzer(_ test: (BuzzerInfo, CatalogPart) -> Bool) throws -> CatalogPart {
        try XCTUnwrap(parts(PART_BUZZER).sorted { $0.name < $1.name }.first { part in
            part.buzzer.map { test($0, part) } ?? false
        })
    }

    /// The names of a side's pins, top to bottom.
    private func names(_ layout: ChipLayout, _ side: ChipLayout.Side) -> [String] {
        layout.slots.filter { $0.side == side }.sorted { $0.centerY < $1.centerY }.map(\.pin.name)
    }

    private func layout(_ part: CatalogPart) -> ChipLayout {
        ChipLayout(part: part, symbol: symbol(for: part))
    }

    private func y(_ layout: ChipLayout, _ name: String) throws -> CGFloat {
        try XCTUnwrap(layout.slots.first { $0.pin.name == name }, name).centerY
    }

    // MARK: - Phase 1: the relay symbol

    func testEveryRelayDrawsAsARelay() throws {
        let all = parts(PART_RELAY)
        XCTAssertFalse(all.isEmpty)
        for part in all {
            XCTAssertEqual(symbol(for: part), .relay, part.name)
            let info = try XCTUnwrap(part.relay, part.name)
            let shape = try XCTUnwrap(RelayShape(part: part), part.name)
            let layout = layout(part)
            // The pins on the left are coil pins, those on the right contacts;
            // no no-connect pin and no second pin of a name is drawn.
            for slot in layout.slots {
                XCTAssertEqual(slot.pin.role, slot.side == .left ? .input : .inOut, "\(part.name) \(slot.pin.name)")
            }
            XCTAssertEqual(Set(layout.slots.map(\.pin.name)).count, layout.slots.count, part.name)
            for side in [ChipLayout.Side.left, .right] {
                let ys = layout.slots.filter { $0.side == side }.map(\.centerY)
                XCTAssertEqual(Set(ys).count, ys.count, "\(part.name): two pins on a row")
            }
            // Every coil and contact pin of the part is drawn (by name).
            let wanted = Set(part.pins.filter { $0.role == .input || $0.role == .inOut }.map(\.name))
            XCTAssertEqual(Set(layout.slots.map(\.pin.name)), wanted, part.name)
            // The shape agrees with the backend's facts.
            XCTAssertEqual(shape.poles.count, info.poles, part.name)
            XCTAssertEqual(shape.coils.count, info.coils, part.name)
            XCTAssertEqual(shape.isLatching, info.latching, part.name)
            XCTAssertLessThanOrEqual(shape.rowCount, RelayShape.maxRows, part.name)
            let channels = Set(shape.poles.map(\.channel))
            XCTAssertEqual(channels, Set(0..<info.channels), "\(part.name): a channel without a pole")
            // Rows are on the grid; the body holds the lowest.
            let lowest = layout.slots.map(\.centerY).max() ?? 0
            XCTAssertLessThanOrEqual(lowest + ChipLayout.pitch / 2, layout.size.height, part.name)
            for slot in layout.slots { XCTAssertEqual(slot.centerY.truncatingRemainder(dividingBy: 10), 0, part.name) }
            XCTAssertEqual(layout.size.width, ChipLayout.relayBodyWidth, part.name)
        }
    }

    /// An SPDT: the coil `A1`, `A2` on the left (two rows apart), `NO`, `COM`,
    /// `NC` on the right, one row each.
    func testSPDTLayout() throws {
        let part = try relay { info, part in
            info.type == "electromechanical" && !info.latching && info.poles == 1 && info.coils == 1
                && part.function == "SPDT"
        }
        let layout = layout(part)
        XCTAssertEqual(names(layout, .left), ["A1", "A2"], part.name)
        XCTAssertEqual(names(layout, .right).map { $0.filter { !$0.isNumber } }, ["NO", "COM", "NC"], part.name)
        let right = layout.slots.filter { $0.side == .right }.sorted { $0.centerY < $1.centerY }
        XCTAssertEqual(right.map(\.centerY), [40, 60, 80], part.name)
        XCTAssertEqual(layout.slots.filter { $0.side == .left }.map(\.centerY).sorted(), [40, 80], part.name)
        XCTAssertEqual(layout.size, CGSize(width: 140, height: 100), part.name)
    }

    /// A DPDT: two poles of NO, COM, NC with a row between them.
    func testDPDTLayout() throws {
        let part = try relay { info, part in
            info.type == "electromechanical" && !info.latching && info.poles == 2 && info.coils == 1
                && part.function == "DPDT"
        }
        let layout = layout(part)
        XCTAssertEqual(names(layout, .left), ["A1", "A2"], part.name)
        XCTAssertEqual(names(layout, .right), ["NO1", "COM1", "NC1", "NO2", "COM2", "NC2"], part.name)
        XCTAssertEqual(try y(layout, "NO2") - (try y(layout, "NC1")), 2 * ChipLayout.pitch, part.name)
        XCTAssertEqual(try y(layout, "COM1") - (try y(layout, "NO1")), ChipLayout.pitch, part.name)
    }

    /// A dual-coil latching relay: the set coil above the reset coil. Both
    /// pinouts exist: `S1`, `S2`, `R1`, `R2`, and the three-terminal `S`, `C`, `R`.
    func testDualCoilLatchingLayout() throws {
        let four = try relay { info, part in
            info.latching && info.coils == 2 && part.pins.contains { $0.name == "S1" }
        }
        let fourLayout = layout(four)
        XCTAssertEqual(names(fourLayout, .left), ["S1", "S2", "R1", "R2"], four.name)
        XCTAssertEqual(try y(fourLayout, "R1") - (try y(fourLayout, "S2")), ChipLayout.pitch, four.name)

        let three = try relay { info, part in
            info.latching && info.coils == 2 && part.pins.contains { $0.name == "C" }
        }
        let threeLayout = layout(three)
        XCTAssertEqual(names(threeLayout, .left), ["S", "C", "R"], three.name)
        let shape = try XCTUnwrap(RelayShape(part: three))
        XCTAssertEqual(shape.coils.map(\.label), ["S", "R"])
        XCTAssertTrue(shape.poles.allSatisfy { $0.channel == 0 }, "a latching relay has one channel")
    }

    /// A Form A solid-state relay: the input LED `A`, `K`; the output `D1`
    /// (common) under `D2` (normally open). A Form B one has `D2` as `NC`,
    /// under the common. Neither draws a no-connect pad.
    func testSolidStateLayouts() throws {
        let formA = try relay { info, part in info.type == "solid-state" && info.channels == 1 && part.function == "SPST-NO" }
        let layoutA = layout(formA)
        XCTAssertEqual(names(layoutA, .left), ["A", "K"], formA.name)
        XCTAssertEqual(names(layoutA, .right), ["D2", "D1"], formA.name)
        XCTAssertTrue(layoutA.slots.allSatisfy { $0.pin.role != .noConnect }, formA.name)

        let formB = try relay { info, part in info.type == "solid-state" && part.function == "SPST-NC" }
        XCTAssertEqual(names(layout(formB), .right), ["D1", "D2"], formB.name)
        let shapeB = try XCTUnwrap(RelayShape(part: formB))
        XCTAssertEqual(shapeB.poles.first?.nc?.name, "D2")
        XCTAssertNil(shapeB.poles.first?.no)

        let two = try relay { info, _ in info.type == "solid-state" && info.channels == 2 }
        let layoutTwo = layout(two)
        XCTAssertEqual(names(layoutTwo, .left), ["A1", "K1", "A2", "K2"], two.name)
        XCTAssertEqual(names(layoutTwo, .right), ["D1B", "D1A", "D2B", "D2A"], two.name)
        let shape = try XCTUnwrap(RelayShape(part: two))
        XCTAssertEqual(shape.poles.map(\.channel), [0, 1], "each channel moves its own pole")
    }

    /// A reed relay's `NC` pads (role no-connect) are not drawn; a power
    /// relay's doubled terminals are drawn once.
    func testNoConnectAndDoubledPinsAreNotDrawn() throws {
        let reed = try relay { info, part in info.type == "reed" && part.pins.contains { $0.role == .noConnect } }
        XCTAssertTrue(layout(reed).slots.allSatisfy { $0.pin.role != .noConnect }, reed.name)
        XCTAssertLessThan(layout(reed).slots.count, reed.pins.count, reed.name)
        let doubled = try relay { _, part in
            let names = part.pins.filter { $0.role == .inOut }.map(\.name)
            return Set(names).count < names.count
        }
        XCTAssertLessThan(layout(doubled).slots.count, doubled.pins.filter { $0.role != .noConnect }.count, doubled.name)
    }

    /// A placed relay of every arrangement above renders, at rest and
    /// operated, in the export scene and by the symbol view; flipped too.
    func testRelaysRender() throws {
        let doc = TestSupport.newDocument()
        let picks: [CatalogPart] = [
            try relay { info, part in info.type == "electromechanical" && !info.latching && part.function == "SPDT" },
            try relay { info, part in info.poles == 2 && !info.latching && part.function == "DPDT" },
            try relay { info, part in info.latching && info.coils == 2 && part.pins.contains { $0.name == "S1" } },
            try relay { info, part in info.type == "solid-state" && info.channels == 1 && part.function == "SPST-NO" },
            try relay { info, _ in info.type == "solid-state" && info.channels == 2 },
        ]
        for (index, part) in picks.enumerated() {
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: CGFloat(200 + 220 * index), y: 200))
            var painter = BoundsPainter()
            var scene = SchematicScene(doc)
            scene.chips = [chip]
            scene.paint(into: &painter)
            let rect = try XCTUnwrap(painter.box.rect, part.name)
            XCTAssertGreaterThan(rect.width, 130, part.name)
            XCTAssertGreaterThan(rect.height, 60, part.name)
            for levels in [[Double](), [1.0, 1.0]] {
                var again = BoundsPainter()
                RelaySymbolView.paint(&again, chip: chip, isSelected: true, isHovered: true, connectedPins: [0], levels: levels)
                XCTAssertNotNil(again.box.rect, part.name)
            }
        }
        var flipped = try XCTUnwrap(doc.chips.first)
        flipped.isFlipped = true
        var painter = BoundsPainter()
        RelaySymbolView.paint(&painter, chip: flipped, isSelected: false, isHovered: false)
        XCTAssertNotNil(painter.box.rect)
    }

    /// The pole's arm ends on `NC` at rest and on `NO` operated, whatever the
    /// frame the pole is drawn in.
    func testTheArmSwingsWithTheOperatedState() throws {
        let part = try relay { info, part in
            info.type == "electromechanical" && !info.latching && info.poles == 1 && info.coils == 1
                && part.function == "SPDT"
        }
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: 300, y: 200))
        let layout = chip.layout
        let noY = try XCTUnwrap(layout.slots.first { $0.pin.name.hasPrefix("NO") }).centerY
        let ncY = try XCTUnwrap(layout.slots.first { $0.pin.name.hasPrefix("NC") }).centerY

        func armTip(levels: [Double]) -> CGPoint? {
            var recorder = StrokeRecorder()
            RelaySymbolView.paint(&recorder, chip: chip, isSelected: false, isHovered: false, levels: levels)
            return recorder.strokes.last?.currentPoint
        }
        XCTAssertEqual(try XCTUnwrap(armTip(levels: [])).y, ncY, accuracy: 0.5)
        XCTAssertEqual(try XCTUnwrap(armTip(levels: [0.2])).y, ncY, accuracy: 0.5, "below one half is at rest")
        XCTAssertEqual(try XCTUnwrap(armTip(levels: [1])).y, noY, accuracy: 0.5)
        XCTAssertTrue(RelaySymbolView.isOperated(channel: 0, levels: [0.51]))
        XCTAssertFalse(RelaySymbolView.isOperated(channel: 1, levels: [1]))
    }

    // MARK: - Phase 2: the buzzer symbol and its voices

    func testEveryBuzzerDrawsAsABuzzer() throws {
        let all = parts(PART_BUZZER)
        XCTAssertFalse(all.isEmpty)
        for part in all {
            XCTAssertEqual(symbol(for: part), .buzzer, part.name)
            let layout = layout(part)
            XCTAssertEqual(layout.slots.map(\.pin.name).sorted(), ["+", "-"], part.name)
            XCTAssertEqual(layout.slots.first { $0.pin.name == "+" }?.side, .left, part.name)
            XCTAssertEqual(layout.slots.first { $0.pin.name == "-" }?.side, .right, part.name)
            XCTAssertEqual(layout.size, ChipLayout.diodeSize, part.name)
            XCTAssertTrue(layout.slots.allSatisfy { $0.centerY == ChipLayout.diodeCentreY }, part.name)
        }
        // A buzzer with a third terminal (a feedback electrode) draws two pins.
        let three = try buzzer { _, part in part.pins.filter { !$0.isHidden }.count > 2 }
        XCTAssertEqual(layout(three).slots.count, 2, three.name)
    }

    func testBuzzersRender() throws {
        let doc = TestSupport.newDocument()
        let indicator = try buzzer { info, _ in info.driver == "indicator" }
        let transducer = try buzzer { info, _ in info.driver == "transducer" }
        for (index, part) in [indicator, transducer].enumerated() {
            let chip = try TestSupport.place(doc, part.name, at: CGPoint(x: CGFloat(100 + 200 * index), y: 100))
            var painter = BoundsPainter()
            var scene = SchematicScene(doc)
            scene.chips = [chip]
            scene.paint(into: &painter)
            let rect = try XCTUnwrap(painter.box.rect, part.name)
            XCTAssertGreaterThan(rect.width, 40, part.name)
            XCTAssertEqual(BuzzerSymbolView.label(for: chip), PartFacts.hertz(UInt64(part.buzzer?.hz ?? 0)), part.name)
            // The sound waves are three more arcs while it is on.
            var quiet = StrokeRecorder(), loud = StrokeRecorder()
            BuzzerSymbolView.paint(&quiet, chip: chip, isSelected: false, isHovered: false, level: 0)
            BuzzerSymbolView.paint(&loud, chip: chip, isSelected: false, isHovered: false, level: 0.5)
            XCTAssertEqual(loud.strokes.count, quiet.strokes.count + 3, part.name)
        }
    }

    /// A netlist with buzzers, and snapshots, built by hand.
    private func netlist(_ buzzers: [BuzzerTone.Buzzer]) -> SchematicNetlist {
        SchematicNetlist(partNames: buzzers.map { _ in "B" }, netCount: 1, connections: [], switchesOn: [],
                         chipIDs: buzzers.map(\.chip), netOfWire: [:], buzzers: buzzers)
    }

    private func snapshot(ps: UInt64, changes: UInt64, light: UInt64, parts: [Int] = [0], error: String? = nil)
        -> SimulationSnapshot {
        SimulationSnapshot(wallTime: 0, circuitTime: ps, netValues: [], netChanges: [changes],
                           lightTimes: Dictionary(uniqueKeysWithValues: parts.map { ($0, [light]) }),
                           lcdFrames: [:], shortedNet: nil, error: error)
    }

    private let second: UInt64 = 1_000_000_000_000

    func testIndicatorVoice() {
        let chip = UUID()
        let net = netlist([BuzzerTone.Buzzer(chip: chip, part: 0, isTransducer: false, hz: 2700, plusNet: 0)])
        let before = snapshot(ps: 0, changes: 0, light: 0)
        // On for the whole frame: its rated frequency.
        let on = snapshot(ps: second / 60, changes: 0, light: second / 60)
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: on, previous: before), [chip: 2700])
        // On for part of it: still its rated frequency.
        let part = snapshot(ps: second / 60, changes: 0, light: second / 120)
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: part, previous: before), [chip: 2700])
        // Off: no on-time gained.
        let off = snapshot(ps: second / 60, changes: 0, light: 0)
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: off, previous: before), [:])
        // No earlier snapshot, no circuit time passed, a circuit error: nothing.
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: on, previous: nil), [:])
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: before, previous: before), [:])
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: snapshot(ps: second / 60, changes: 0, light: second / 60,
                                                                          error: "short"), previous: before), [:])
    }

    func testTransducerVoice() {
        let chip = UUID()
        let net = netlist([BuzzerTone.Buzzer(chip: chip, part: 0, isTransducer: true, hz: 2700, plusNet: 0)])
        let before = snapshot(ps: 0, changes: 0, light: 0)
        // 2000 changes in one second of circuit time: 1000 Hz, whatever the rated frequency.
        let driven = snapshot(ps: second, changes: 2000, light: second / 2)
        XCTAssertEqual(try XCTUnwrap(BuzzerTone.voices(netlist: net, snapshot: driven, previous: before)[chip]),
                       1000, accuracy: 0.001)
        // Held on by a steady level: it is on, but no changes: silent.
        let steady = snapshot(ps: second, changes: 0, light: second)
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: steady, previous: before), [:])
        // 30 kHz and 5 Hz are outside the audible range.
        let ultrasound = snapshot(ps: second, changes: 60_000, light: second / 2)
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: ultrasound, previous: before), [:])
        let rumble = snapshot(ps: second, changes: 10, light: second / 2)
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: rumble, previous: before), [:])
        // The edges of the range are in it.
        XCTAssertNotNil(BuzzerTone.voices(netlist: net, snapshot: snapshot(ps: second, changes: 40, light: 1), previous: before)[chip])
        XCTAssertNotNil(BuzzerTone.voices(netlist: net, snapshot: snapshot(ps: second, changes: 40_000, light: 1), previous: before)[chip])
        // Driven but not on in the frame (light gained nothing): silent.
        XCTAssertEqual(BuzzerTone.voices(netlist: net, snapshot: snapshot(ps: second, changes: 2000, light: 0), previous: before), [:])
    }

    /// The pitch is the window's average while the drive is steady, and the
    /// last frame's when the drive has just changed.
    func testTransducerPitchUsesTheWindow() {
        let chip = UUID()
        let net = netlist([BuzzerTone.Buzzer(chip: chip, part: 0, isTransducer: true, hz: 2700, plusNet: 0)])
        let tenth = second / 10
        let frame = second / 60
        // A steady 500 Hz: a frame holds 16 or 17 changes (1000 Hz changes per second), the window 100.
        let window = snapshot(ps: 0, changes: 0, light: 0)
        let previous = snapshot(ps: tenth - frame, changes: 83, light: 1)
        let now = snapshot(ps: tenth, changes: 100, light: 2)
        let steady = BuzzerTone.voices(netlist: net, snapshot: now, previous: previous, window: window)
        XCTAssertEqual(try XCTUnwrap(steady[chip]), 500, accuracy: 0.001)
        // The drive was idle for most of the window and has just started at
        // 2 kHz: 66 changes in the last frame, 66 in the window; the frame
        // says 2 kHz, the window 330 Hz: the frame's wins.
        let started = snapshot(ps: tenth, changes: 66, light: 2)
        let idle = snapshot(ps: tenth - frame, changes: 0, light: 1)
        let onset = BuzzerTone.voices(netlist: net, snapshot: started, previous: idle, window: window)
        XCTAssertEqual(try XCTUnwrap(onset[chip]), 66 / 2 / (1.0 / 60), accuracy: 0.001)
        // A frame with too few changes to trust (a slow simulation) uses the window.
        let slow = snapshot(ps: tenth, changes: 100, light: 2)
        let slowBefore = snapshot(ps: tenth - second / 5000, changes: 99, light: 1)
        let slowVoices = BuzzerTone.voices(netlist: net, snapshot: slow, previous: slowBefore, window: window)
        XCTAssertEqual(try XCTUnwrap(slowVoices[chip]), 500, accuracy: 0.001)
    }

    func testABuzzerWithoutAPlusNetHasNoVoice() {
        let chip = UUID()
        for isTransducer in [false, true] {
            let net = netlist([BuzzerTone.Buzzer(chip: chip, part: 0, isTransducer: isTransducer, hz: 2700, plusNet: nil)])
            let voices = BuzzerTone.voices(netlist: net, snapshot: snapshot(ps: second, changes: 2000, light: second),
                                           previous: snapshot(ps: 0, changes: 0, light: 0))
            XCTAssertEqual(voices, [:], "transducer \(isTransducer)")
        }
    }

    /// The netlist knows each buzzer's driver, rated frequency and `+` net.
    func testTheNetlistListsTheBuzzers() throws {
        let doc = TestSupport.newDocument()
        let part = try buzzer { info, _ in info.driver == "transducer" }
        let other = try buzzer { info, _ in info.driver == "indicator" }
        let a = try TestSupport.place(doc, part.name, at: CGPoint(x: 200, y: 200))
        let b = try TestSupport.place(doc, other.name, at: CGPoint(x: 400, y: 200))
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 100, y: 60))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(vcc, "VCC")), .pin(try TestSupport.pin(a, "+"))))
        let netlist = NetlistBuilder.build(chips: doc.chips, wires: doc.wires, buses: doc.buses, labels: doc.netLabels)
        XCTAssertEqual(netlist.buzzers.count, 2)
        let first = try XCTUnwrap(netlist.buzzers.first { $0.chip == a.id })
        XCTAssertTrue(first.isTransducer)
        XCTAssertEqual(first.hz, Double(part.buzzer?.hz ?? 0))
        XCTAssertNotNil(first.plusNet)
        let indicator = try XCTUnwrap(netlist.buzzers.first { $0.chip == b.id })
        XCTAssertFalse(indicator.isTransducer)
        XCTAssertNil(indicator.plusNet, "an unwired + has no net")
    }

    // MARK: - Phase 3: the sound

    /// The renderer: a square wave of the pitch, -24 dBFS, ramped in 10 ms,
    /// silent when the table is empty; a pitch nearby glides, a far one ramps.
    func testToneRenderer() {
        let rate = 48000.0
        let renderer = ToneRenderer(sampleRate: rate)
        var samples = [Float](repeating: 0, count: 4800)
        func render() -> [Float] {
            samples.withUnsafeMutableBufferPointer { renderer.render(into: $0.baseAddress!, frames: $0.count) }
            return samples
        }
        func risingEdges(_ values: [Float]) -> Int {
            zip(values, values.dropFirst()).filter { $0 <= 0 && $1 > 0 }.count
        }
        // Silent before any voice.
        XCTAssertTrue(render().allSatisfy { $0 == 0 })
        // 1 kHz: 100 cycles in 0.1 s, the level -24 dBFS, and a ramp: the first sample is quiet.
        renderer.set([1000])
        let tone = render()
        XCTAssertLessThan(abs(tone[0]), 0.01)
        XCTAssertEqual(tone.map(abs).max() ?? 0, ToneRenderer.amplitude, accuracy: 0.0005)
        XCTAssertEqual(ToneRenderer.amplitude, Float(pow(10.0, -24.0 / 20.0)), accuracy: 0.0001)
        XCTAssertLessThanOrEqual(abs(risingEdges(tone) - 100), 1)
        // Ramped: the first 10 ms are not at the full level for every cycle.
        XCTAssertLessThan(tone.prefix(240).map(abs).max() ?? 1, ToneRenderer.amplitude * 0.9)
        // A nearby pitch glides: no drop in level.
        renderer.set([1040])
        let glide = render()
        XCTAssertEqual(glide.map(abs).min() ?? 0, ToneRenderer.amplitude, accuracy: 0.0005)
        XCTAssertLessThanOrEqual(abs(risingEdges(glide) - 104), 1)
        // A far pitch ramps down, then up at the new pitch.
        renderer.set([2000])
        let jump = render()
        XCTAssertLessThan(jump.prefix(480).map(abs).min() ?? 1, 0.005, "the level reaches 0 between the pitches")
        XCTAssertGreaterThan(risingEdges(Array(jump.suffix(2400))), 90)
        // Two voices sum.
        renderer.set([2000, 3000])
        _ = render()
        let both = render()
        XCTAssertEqual(both.map(abs).max() ?? 0, 2 * ToneRenderer.amplitude, accuracy: 0.001)
        // Empty: ramps out within 10 ms, then silence.
        renderer.set([])
        let out = render()
        XCTAssertTrue(out.suffix(4000).allSatisfy { $0 == 0 })
    }

    /// The source node `BuzzerSound` plays through, in an engine that renders
    /// offline (no audio device): the render block runs off the main thread
    /// and fills the buffers with the tone.
    func testTheSourceNodeRendersTheToneOffline() throws {
        let rate = 44100.0
        let engine = AVAudioEngine()
        let format = try XCTUnwrap(AVAudioFormat(standardFormatWithSampleRate: rate, channels: 1))
        try engine.enableManualRenderingMode(.offline, format: format, maximumFrameCount: 4096)
        let renderer = ToneRenderer(sampleRate: rate, gain: ToneRenderer.monoMixerCompensation)
        let node = BuzzerSound.makeNode(renderer: renderer, format: format)
        engine.attach(node)
        engine.connect(node, to: engine.mainMixerNode, format: format)
        try engine.start()
        defer { engine.stop() }
        renderer.set([2000])
        let buffer = try XCTUnwrap(AVAudioPCMBuffer(pcmFormat: engine.manualRenderingFormat, frameCapacity: 4096))
        var edges = 0
        var peak: Float = 0
        var last: Float = 0
        for _ in 0..<5 {   // 5 x 4096 frames at 44.1 kHz: about 0.46 s
            let status = try engine.renderOffline(4096, to: buffer)
            XCTAssertEqual(status, .success)
            let samples = UnsafeBufferPointer(start: try XCTUnwrap(buffer.floatChannelData)[0], count: Int(buffer.frameLength))
            for sample in samples {
                if last <= 0, sample > 0 { edges += 1 }
                last = sample
                peak = max(peak, abs(sample))
            }
        }
        XCTAssertLessThanOrEqual(abs(edges - Int(2000 * 5 * 4096 / rate)), 2, "2 kHz")
        XCTAssertEqual(peak, ToneRenderer.amplitude, accuracy: 0.002, "-24 dBFS through the mixer")
        renderer.set([])
    }

    /// At most 8 voices, the lowest chip ids; a chip keeps its slot.
    func testVoiceSlots() {
        var slots = VoiceSlots()
        let ids = (0..<10).map { _ in UUID() }.sorted { $0.uuidString < $1.uuidString }
        var voices: [UUID: Double] = [:]
        for (index, id) in ids.enumerated() { voices[id] = 400 + Double(index) * 100 }
        let all = slots.pitches(for: voices)
        XCTAssertEqual(all.count, ToneRenderer.maxVoices)
        XCTAssertEqual(all.filter { $0 > 0 }.count, 8)
        XCTAssertEqual(Set(slots.chips.compactMap { $0 }), Set(ids.prefix(8)), "the 8 lowest ids sound")
        // The lowest one stops: the others keep their slots, and the 9th takes the free one.
        let slotOfThird = slots.chips.firstIndex(of: ids[2])
        voices[ids[0]] = nil
        _ = slots.pitches(for: voices)
        XCTAssertEqual(slots.chips.firstIndex(of: ids[2]), slotOfThird)
        XCTAssertEqual(Set(slots.chips.compactMap { $0 }), Set(ids[1...8]))
        // A new frequency of a chip changes its slot's pitch only.
        voices[ids[2]] = 777
        let pitches = slots.pitches(for: voices)
        XCTAssertEqual(pitches[try! XCTUnwrap(slotOfThird)], 777)
        slots.reset()
        XCTAssertTrue(slots.chips.allSatisfy { $0 == nil })
    }

    /// No test opens the audio device.
    func testNoAudioUnderTest() {
        XCTAssertTrue(BuzzerSoundSetting.isUnderTest)
        XCTAssertFalse(BuzzerSoundSetting.mayPlay)
        XCTAssertEqual(BuzzerSoundSetting.key, "playBuzzerSounds")
    }

    func testBuzzerHelpLine() {
        let entry = EditorPage.schematic.helpSections.flatMap(\.entries)
            .first { $0.keys == "Buzzers (in play mode)" }
        XCTAssertEqual(entry?.action, "Buzzers play through the Mac's speakers; turn them off in Settings")
    }

    // MARK: - Play mode, in the real simulation

    /// `5V` → gate (input on GND) → relay coil `A1` (`A2` on GND); `5V` on
    /// `COM`; `NO` → a resistor → an LED → GND and → an indicator buzzer's `+`.
    private struct Circuit {
        let doc: SchematicDocument
        let gate: PlacedChip
        let relay: PlacedChip
        let led: PlacedChip
        let buzzer: PlacedChip
    }

    /// `inputHigh`: the gate's input on 5V instead of GND.
    private func buildCircuit(inputHigh: Bool = false) throws -> Circuit {
        let relayPart = try relay { info, part in
            info.type == "electromechanical" && !info.latching && info.poles == 1 && info.coils == 1
                && info.coilMV == 5000 && part.pins.contains { $0.name == "NO" } && part.pins.contains { $0.name == "NC" }
        }
        let buzzerPart = try buzzer { info, _ in info.driver == "indicator" }
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 500))
        let gnd2 = try TestSupport.place(doc, "GND", at: CGPoint(x: 900, y: 500))
        let gate = try TestSupport.place(doc, "SN74HC04N", at: CGPoint(x: 240, y: 200))
        let relay = try TestSupport.place(doc, relayPart.name, at: CGPoint(x: 460, y: 200))
        let resistor = try TestSupport.place(doc, "GENERIC-RESISTOR", at: CGPoint(x: 700, y: 160))
        let led = try TestSupport.place(doc, "GENERIC-LED", at: CGPoint(x: 800, y: 300))
        let buzzer = try TestSupport.place(doc, buzzerPart.name, at: CGPoint(x: 800, y: 400))
        func connect(_ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) throws {
            let message = doc.connect(.pin(try TestSupport.pin(a, x)), .pin(try TestSupport.pin(b, y)))
            XCTAssertNil(message, "\(x) to \(y)")
        }
        try connect(vcc, "VCC", gate, "VCC")
        try connect(gnd, "GND", gate, "GND")
        try connect(inputHigh ? vcc : gnd, inputHigh ? "VCC" : "GND", gate, "1A")
        try connect(gate, "1Y", relay, "A1")
        try connect(gnd, "GND", relay, "A2")
        try connect(vcc, "VCC", relay, "COM")
        try connect(relay, "NO", resistor, "1")
        try connect(resistor, "2", led, "A")
        try connect(led, "K", gnd2, "GND")
        try connect(relay, "NO", buzzer, "+")
        try connect(buzzer, "-", gnd2, "GND")
        return Circuit(doc: doc, gate: gate, relay: relay, led: led, buzzer: buzzer)
    }

    private func spin(for seconds: TimeInterval = 0.3) {
        RunLoop.main.run(until: Date().addingTimeInterval(seconds))
    }

    /// A gate output on the coil operates the relay (after its 1 ms delay),
    /// which lights the LED and sounds the indicator at its rated frequency;
    /// with the gate's input flipped all three stop.
    func testTheRelayLightsAnLEDAndSoundsAnIndicator() throws {
        let circuit = try buildCircuit()
        let rated = Double(try XCTUnwrap(circuit.buzzer.part.buzzer).hz)
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)

        simulation.start(document: circuit.doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        spin()
        XCTAssertGreaterThan(simulation.lights[circuit.relay.id]?.first ?? 0, 0.5, "operated")
        XCTAssertGreaterThan(simulation.lights[circuit.led.id]?.first ?? 0, 0.5, "LED lit")
        XCTAssertEqual(simulation.buzzerVoices, [circuit.buzzer.id: rated])
        simulation.stop()
        XCTAssertTrue(simulation.buzzerVoices.isEmpty, "no voice outside play mode")

        // The gate's input on 5V: its output is low, the coil is not driven.
        let rest = try buildCircuit(inputHigh: true)
        simulation.start(document: rest.doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        spin()
        XCTAssertNotNil(simulation.lights[rest.relay.id])
        XCTAssertLessThan(simulation.lights[rest.relay.id]?.first ?? 1, 0.5, "at rest")
        XCTAssertLessThan(simulation.lights[rest.led.id]?.first ?? 1, 0.01, "LED dark")
        XCTAssertTrue(simulation.buzzerVoices.isEmpty)
        simulation.stop()
    }

    /// A relay wired to interrupt its own coil toggles every millisecond and
    /// gives a transducer on its contact a 500 Hz voice.
    func testASelfInterruptingRelayGivesATransducerA500HzVoice() throws {
        let relayPart = try relay { info, part in
            info.type == "electromechanical" && !info.latching && info.poles == 1 && info.coils == 1
                && info.coilMV == 5000 && part.pins.contains { $0.name == "NC" } && part.pins.contains { $0.name == "COM" }
        }
        let transducer = try buzzer { info, _ in info.driver == "transducer" }
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 500))
        let relay = try TestSupport.place(doc, relayPart.name, at: CGPoint(x: 360, y: 200))
        let buzzer = try TestSupport.place(doc, transducer.name, at: CGPoint(x: 600, y: 300))
        func connect(_ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) throws {
            XCTAssertNil(doc.connect(.pin(try TestSupport.pin(a, x)), .pin(try TestSupport.pin(b, y))), "\(x) to \(y)")
        }
        try connect(vcc, "VCC", relay, "NC")
        try connect(relay, "COM", relay, "A1")
        try connect(relay, "COM", buzzer, "+")
        try connect(gnd, "GND", relay, "A2")
        try connect(gnd, "GND", buzzer, "-")

        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        spin(for: 0.5)
        let hz = try XCTUnwrap(simulation.buzzerVoices[buzzer.id], simulation.errorMessage ?? "no voice")
        XCTAssertEqual(hz, 500, accuracy: 10, "within 2 % of 500 Hz")
        simulation.stop()
        XCTAssertTrue(simulation.buzzerVoices.isEmpty)
    }
}

/// Records the paths stroked, in order.
private struct StrokeRecorder: SchematicPainter {
    var strokes: [Path] = []
    mutating func fill(_ path: Path, with ink: Ink) {}
    mutating func stroke(_ path: Path, with ink: Ink, style: StrokeStyle) { strokes.append(path) }
    mutating func draw(_ text: PaintedText, at point: CGPoint, anchor: UnitPoint) {}
    mutating func translateBy(x: CGFloat, y: CGFloat) {}
    mutating func rotate(by angle: Angle) {}
}
