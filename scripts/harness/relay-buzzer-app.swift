// plan-47-N: relays and buzzers in the app, end to end on the app's real
// catalog, document, simulation and views. Checks that `symbol(for:)` is
// `.relay` for every relay part and `.buzzer` for every buzzer part; the
// layouts of an SPDT, a DPDT, a dual-coil latching and a two-channel
// solid-state relay; the play-mode circuit
//   5V/GND -> SN74HC04N (input on GND) -> 1Y -> relay coil A1 (A2 on GND),
//   5V on COM, NO -> GENERIC-RESISTOR -> GENERIC-LED -> GND and -> an
//   indicator buzzer's + (its - on GND)
// in the real SimulationController: the relay operated, the LED lit and the
// buzzer's voice at its rated frequency; with the gate's input on 5V the
// relay at rest, the LED dark and no voice; and the self-interrupting relay
// (5V on NC, COM on its own A1 and on a transducer's +, A2 and the -
// on GND), whose voice is within 2 % of 500 Hz. The parts are picked by
// property from the catalog (the first by name), and their numbers printed.
// No sound is played: the harness silences `BuzzerSound`. Renders a relay at
// rest and operated, a solid-state relay and a buzzer (sounding and not).
// Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p47-harness && APP=/tmp/p47-dd-n/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/relay-buzzer-app.swift /tmp/p47-harness && \
//     /tmp/p47-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p47-relay-rest.png, p47-relay-operated.png,
// p47-relay-layouts.png (SPDT, DPDT, dual-coil latching, solid-state,
// two-channel solid-state), p47-buzzer.png, p47-buzzer-on.png.
import CoreGraphics
import Foundation
import SwiftUI

BuzzerSoundSetting.isSilenced = true
MainActor.assumeIsolated { runRelayBuzzerChecks() }

@MainActor func runRelayBuzzerChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp"
    var failures = 0
    func check(_ name: String, _ body: () throws -> String?) {
        do {
            if let problem = try body() {
                print("FAIL \(name): \(problem)")
                failures += 1
            } else {
                print("ok   \(name)")
            }
        } catch {
            print("FAIL \(name): \(error)")
            failures += 1
        }
    }
    let all = Harness.catalog.allParts
    func relay(_ test: (RelayInfo, CatalogPart) -> Bool) -> CatalogPart? {
        all.filter { $0.kind == PART_RELAY }.sorted { $0.name < $1.name }.first { part in part.relay.map { test($0, part) } ?? false }
    }
    func buzzer(_ test: (BuzzerInfo, CatalogPart) -> Bool) -> CatalogPart? {
        all.filter { $0.kind == PART_BUZZER }.sorted { $0.name < $1.name }.first { part in part.buzzer.map { test($0, part) } ?? false }
    }
    func layout(_ part: CatalogPart) -> ChipLayout { ChipLayout(part: part, symbol: symbol(for: part)) }
    func names(_ part: CatalogPart, _ side: ChipLayout.Side) -> [String] {
        layout(part).slots.filter { $0.side == side }.sorted { $0.centerY < $1.centerY }.map(\.pin.name)
    }

    // (1) symbol(for:) on every relay and buzzer part.
    check("1 every relay draws as a relay, every buzzer as a buzzer") {
        var relays = 0, buzzers = 0
        for part in all where part.kind == PART_RELAY {
            if symbol(for: part) != .relay { return "\(part.name) is \(symbol(for: part))" }
            relays += 1
        }
        for part in all where part.kind == PART_BUZZER {
            if symbol(for: part) != .buzzer { return "\(part.name) is \(symbol(for: part))" }
            let l = layout(part)
            if l.slots.map(\.pin.name).sorted() != ["+", "-"] { return "\(part.name) draws \(l.slots.map(\.pin.name))" }
            buzzers += 1
        }
        print("     \(relays) relays, \(buzzers) buzzers")
        return relays > 0 && buzzers > 0 ? nil : "no relays or buzzers in the catalog"
    }

    // (2) The layouts of the four arrangements.
    let spdt = relay { i, p in i.type == "electromechanical" && !i.latching && i.poles == 1 && i.coils == 1 && p.function == "SPDT" }
    let dpdt = relay { i, p in i.type == "electromechanical" && !i.latching && i.poles == 2 && i.coils == 1 && p.function == "DPDT" }
    let dual = relay { i, p in i.latching && i.coils == 2 && p.pins.contains { $0.name == "S1" } }
    let dual3 = relay { i, p in i.latching && i.coils == 2 && p.pins.contains { $0.name == "C" } }
    let ssr = relay { i, p in i.type == "solid-state" && i.channels == 1 && p.function == "SPST-NO" }
    let ssr2 = relay { i, _ in i.type == "solid-state" && i.channels == 2 }
    print("     parts: SPDT \(spdt?.name ?? "-"), DPDT \(dpdt?.name ?? "-"), dual-coil \(dual?.name ?? "-") / \(dual3?.name ?? "-"), SSR \(ssr?.name ?? "-"), 2-channel SSR \(ssr2?.name ?? "-")")
    for (label, part, left, right) in [
        ("SPDT", spdt, ["A1", "A2"], ["NO", "COM", "NC"]),
        ("DPDT", dpdt, ["A1", "A2"], ["NO1", "COM1", "NC1", "NO2", "COM2", "NC2"]),
        ("dual-coil latching", dual, ["S1", "S2", "R1", "R2"], nil),
        ("three-terminal latching", dual3, ["S", "C", "R"], nil),
        ("solid-state", ssr, ["A", "K"], ["D2", "D1"]),
        ("two-channel solid-state", ssr2, ["A1", "K1", "A2", "K2"], ["D1B", "D1A", "D2B", "D2A"]),
    ] as [(String, CatalogPart?, [String], [String]?)] {
        guard let part else {
            print("FAIL 2 \(label): no such part in the catalog")
            failures += 1
            continue
        }
        let l = layout(part)
        print("     \(label) \(part.name): left \(names(part, .left)), right \(names(part, .right)), \(Int(l.size.width))x\(Int(l.size.height))")
        check("2 layout \(label) \(part.name): coil on the left, contacts on the right, no NC pad drawn") {
            if names(part, .left) != left { return "left \(names(part, .left))" }
            // An SPDT's contact pins may be numbered (NO1, COM1, NC1) or not.
            func comparable(_ list: [String]) -> [String] { label == "SPDT" ? list.map { $0.filter { !$0.isNumber } } : list }
            if let right, comparable(names(part, .right)) != comparable(right) {
                return "right \(names(part, .right))"
            }
            if l.slots.contains(where: { $0.pin.role == .noConnect }) { return "a no-connect pad is drawn" }
            return nil
        }
    }

    // (3) The play-mode circuit.
    let relayPart = relay { i, p in
        i.type == "electromechanical" && !i.latching && i.poles == 1 && i.coils == 1 && i.coilMV == 5000
            && p.pins.contains { $0.name == "NO" } && p.pins.contains { $0.name == "NC" } && p.pins.contains { $0.name == "COM" }
    }
    let indicator = buzzer { i, _ in i.driver == "indicator" }
    let transducer = buzzer { i, _ in i.driver == "transducer" }
    guard let relayPart, let indicator, let transducer else {
        print("FAIL 3 no 5 V SPDT relay, indicator or transducer in the catalog")
        exit(1)
    }
    print("     circuit parts: relay \(relayPart.name) (\(relayPart.function)), indicator \(indicator.name) (\(indicator.buzzer?.hz ?? 0) Hz), transducer \(transducer.name)")

    struct Circuit {
        let doc: SchematicDocument
        let relay: PlacedChip
        let led: PlacedChip
        let buzzer: PlacedChip
    }
    func buildCircuit(inputHigh: Bool) -> Circuit {
        let d = Harness.newDocument()
        func put(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
            let part = Harness.catalog.part(named: name)!
            d.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
            return d.chips.last!
        }
        func pin(_ chip: PlacedChip, _ name: String) -> LineAnchor {
            let slot = chip.layout.slots.first { $0.pin.name == name }!
            return .pin(PinRef(chipID: chip.id, index: slot.pin.index))
        }
        func connect(_ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) {
            if let message = d.connect(pin(a, x), pin(b, y)) { print("FAIL wiring \(x) to \(y): \(message)"); failures += 1 }
        }
        let vcc = put("5V", 60, 60)
        let gnd = put("GND", 60, 500)
        let gnd2 = put("GND", 940, 500)
        let gate = put("SN74HC04N", 240, 200)
        let relay = put(relayPart.name, 480, 200)
        let resistor = put("GENERIC-RESISTOR", 720, 160)
        let led = put("GENERIC-LED", 820, 300)
        let buzzer = put(indicator.name, 820, 400)
        connect(vcc, "VCC", gate, "VCC")
        connect(gnd, "GND", gate, "GND")
        connect(inputHigh ? vcc : gnd, inputHigh ? "VCC" : "GND", gate, "1A")
        connect(gate, "1Y", relay, "A1")
        connect(gnd, "GND", relay, "A2")
        connect(vcc, "VCC", relay, "COM")
        connect(relay, "NO", resistor, "1")
        connect(resistor, "2", led, "A")
        connect(led, "K", gnd2, "GND")
        connect(relay, "NO", buzzer, "+")
        connect(buzzer, "-", gnd2, "GND")
        return Circuit(doc: d, relay: relay, led: led, buzzer: buzzer)
    }
    let sim = Harness.simulation
    func spin(_ seconds: TimeInterval = 0.3) { RunLoop.main.run(until: Date().addingTimeInterval(seconds)) }

    let on = buildCircuit(inputHigh: false)
    sim.start(document: on.doc)
    var operated = 0.0, litLED = 0.0
    var voices: [UUID: Double] = [:]
    if sim.isRunning {
        spin()
        operated = sim.lights[on.relay.id]?.first ?? 0
        litLED = sim.lights[on.led.id]?.first ?? 0
        voices = sim.buzzerVoices
        Harness.renderCanvas(on.doc, to: "\(images)/p47-relay-operated.png", size: CGSize(width: 1000, height: 560),
                             origin: CGPoint(x: 20, y: 40))
        sim.stop()
    }
    let rated = Double(indicator.buzzer?.hz ?? 0)
    print("     gate low in: relay light \(operated), LED \(litLED), voices \(voices.map { "\($0.value) Hz" }), rated \(rated) Hz")
    check("3 coil driven: relay operated, LED lit, indicator sounds at its rated frequency") {
        if operated < 0.5 { return "relay light \(operated)" }
        if litLED < 0.5 { return "LED \(litLED)" }
        return voices == [on.buzzer.id: rated] ? nil : "voices \(voices), want \(rated)"
    }
    check("3 stopped: no voice, nothing drawn operated") {
        sim.buzzerVoices.isEmpty && sim.lights.isEmpty ? nil : "voices \(sim.buzzerVoices), lights \(sim.lights)"
    }

    let off = buildCircuit(inputHigh: true)
    sim.start(document: off.doc)
    var restRelay = 1.0, restLED = 1.0
    var restVoices: [UUID: Double] = [:]
    if sim.isRunning {
        spin()
        restRelay = sim.lights[off.relay.id]?.first ?? 1
        restLED = sim.lights[off.led.id]?.first ?? 1
        restVoices = sim.buzzerVoices
        Harness.renderCanvas(off.doc, to: "\(images)/p47-relay-rest.png", size: CGSize(width: 1000, height: 560),
                             origin: CGPoint(x: 20, y: 40))
        sim.stop()
    }
    print("     gate high in: relay light \(restRelay), LED \(restLED), voices \(restVoices)")
    check("3 coil not driven: relay at rest, LED dark, no voice") {
        if restRelay > 0.5 { return "relay light \(restRelay)" }
        if restLED > 0.01 { return "LED \(restLED)" }
        return restVoices.isEmpty ? nil : "voices \(restVoices)"
    }

    // (4) The self-interrupting relay and the transducer.
    let d2 = Harness.newDocument()
    func put2(_ name: String, _ x: CGFloat, _ y: CGFloat) -> PlacedChip {
        let part = Harness.catalog.part(named: name)!
        d2.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: y))
        return d2.chips.last!
    }
    func pin2(_ chip: PlacedChip, _ name: String) -> LineAnchor {
        .pin(PinRef(chipID: chip.id, index: chip.layout.slots.first { $0.pin.name == name }!.pin.index))
    }
    func connect2(_ a: PlacedChip, _ x: String, _ b: PlacedChip, _ y: String) {
        if let message = d2.connect(pin2(a, x), pin2(b, y)) { print("FAIL wiring \(x) to \(y): \(message)"); failures += 1 }
    }
    let vcc2 = put2("5V", 60, 60)
    let gnd3 = put2("GND", 60, 500)
    let buzzing = put2(relayPart.name, 380, 200)
    let voice = put2(transducer.name, 620, 300)
    connect2(vcc2, "VCC", buzzing, "NC")
    connect2(buzzing, "COM", buzzing, "A1")
    connect2(buzzing, "COM", voice, "+")
    connect2(gnd3, "GND", buzzing, "A2")
    connect2(gnd3, "GND", voice, "-")
    sim.start(document: d2)
    var hz = 0.0
    if sim.isRunning {
        spin(0.5)
        hz = sim.buzzerVoices[voice.id] ?? 0
        let stats = sim.errorMessage ?? "no error"
        print("     self-interrupting relay \(relayPart.name) with transducer \(transducer.name): voice \(hz) Hz (\(stats))")
        sim.stop()
    }
    check("4 the self-interrupting relay gives the transducer a voice within 2 % of 500 Hz") {
        abs(hz - 500) <= 10 ? nil : "voice \(hz) Hz"
    }
    check("4 stopped: no voice") { sim.buzzerVoices.isEmpty ? nil : "\(sim.buzzerVoices)" }

    // (5) Pictures: the layouts, a solid-state relay and a buzzer.
    let picture = Harness.newDocument()
    var x: CGFloat = 130
    for part in [spdt, dpdt, dual, dual3, ssr, ssr2].compactMap({ $0 }) {
        picture.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: x, y: 150))
        x += 230
    }
    let layoutsPath = "\(images)/p47-relay-layouts.png"
    Harness.renderCanvas(picture, to: layoutsPath, size: CGSize(width: 1500, height: 320))
    let buzzers = Harness.newDocument()
    for (index, part) in [indicator, transducer].enumerated() {
        buzzers.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 120 + 160 * index, y: 100))
    }
    let buzzerPath = "\(images)/p47-buzzer.png"
    Harness.renderCanvas(buzzers, to: buzzerPath, size: CGSize(width: 400, height: 200))
    // The sounding buzzer: the symbol with its sound waves.
    let sounding = Harness.newDocument()
    sounding.placeChip(part: indicator, symbol: symbol(for: indicator), centeredAt: CGPoint(x: 120, y: 100))
    let soundingPath = "\(images)/p47-buzzer-on.png"
    let chip = sounding.chips[0]
    Harness.render(BuzzerSymbolView(chip: chip, isSelected: false, isHovered: false, level: 1)
        .padding(20).background(Color.white), size: CGSize(width: 140, height: 90), to: soundingPath)
    check("5 images written") {
        let missing = ["\(images)/p47-relay-rest.png", "\(images)/p47-relay-operated.png", layoutsPath, buzzerPath, soundingPath]
            .filter { !FileManager.default.fileExists(atPath: $0) }
        return missing.isEmpty ? nil : "missing \(missing)"
    }

    print(failures == 0 ? "all checks ok" : "\(failures) checks failed")
    exit(failures == 0 ? 0 : 1)
}
