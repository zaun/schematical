//
//  AutomationSimulation.swift
//  schematical
//
//  Automation tools for the logic simulator: run the schematic, flip
//  switches, read pins, memory and LEDs. The design can't be edited while it
//  runs, as in the app.
//

import Foundation

extension AutomationTools {
    static let simulationTools: [AutomationTool] = [
        AutomationTool(
            name: "simulation_start",
            summary: "Start the logic simulation of the schematic (as the Play button does). Refused, with the "
                + "reason, when a pin would be driven out of its voltage range. While it runs the design is "
                + "read-only; use set_switch, wait and read_pins, then simulation_stop.",
            properties: ["speed": Schema.number("Circuit time per wall time: 1 = real time (default: the app's setting)")],
            run: { context, args in
                let context = context!
                if let speed = args.optDouble("speed") {
                    guard speed > 0 else { throw AutomationFailure("speed must be above 0") }
                    context.simulation.setSpeed(speed)
                }
                if !context.simulation.isRunning { context.simulation.start(document: context.document) }
                try await Task.sleep(nanoseconds: 250_000_000)
                return Out.text(status(context))
            }),
        AutomationTool(
            name: "simulation_stop",
            summary: "Stop the simulation. Switches go back to where they were when it started.",
            run: { context, _ in
                context!.simulation.stop()
                return Out.text("stopped")
            }),
        AutomationTool(
            name: "simulation_status",
            summary: "Whether the simulation is running, its circuit time, speed and any error.",
            readOnly: true,
            run: { context, _ in Out.text(status(context!)) }),
        AutomationTool(
            name: "wait",
            summary: "Let the simulation run for some milliseconds (at most 10000) before the next read.",
            properties: ["milliseconds": Schema.integer("1–10000")], required: ["milliseconds"],
            needsWindow: false, readOnly: true,
            run: { _, args in
                let time = args.optInt("milliseconds") ?? 0
                guard (1...10_000).contains(time) else { throw AutomationFailure("milliseconds must be 1–10000") }
                try await Task.sleep(nanoseconds: UInt64(time) * 1_000_000)
                return Out.text("waited \(time) ms")
            }),
        AutomationTool(
            name: "set_switch",
            summary: "Set a switch: a DIP switch position (1-based) ON or OFF, press (on=true) / release a "
                + "tactile button, put a slide switch's knob in a position (1 … 3, required; on is not used), "
                + "or blow (on=false) / restore (on=true) a fuse while the simulation runs. "
                + "Works while the simulation runs; outside it the DIP position or the knob is an edit. "
                + "(While running, flips are inputs to that run and are undone when it stops.)",
            properties: [
                "reference": Schema.string("e.g. SW1"),
                "position": Schema.integer("1-based position (default 1); a slide switch's knob position (required)"),
                "on": Schema.boolean("ON / pressed (default true); not for a slide switch"),
            ],
            required: ["reference"],
            run: { context, args in Out.text(try setSwitch(context!, args)) }),
        AutomationTool(
            name: "read_pins",
            summary: "Logic levels (0, 1, X unknown, Z floating) of a part's pins in the running simulation, and "
                + "an LED's brightness. pins limits it, e.g. [\"1\", \"Q0\"]; default all pins.",
            properties: [
                "reference": Schema.string("e.g. U3"), "pins": Schema.strings("Pin designators or names"),
            ],
            required: ["reference"], readOnly: true,
            run: { context, args in Out.text(try readPins(context!, args)) }),
        AutomationTool(
            name: "read_memory",
            summary: "Words of a RAM/ROM/EEPROM in the running simulation, in hex (? marks unknown bits).",
            properties: [
                "reference": Schema.string("e.g. U5"), "start": Schema.integer("First address (default 0)"),
                "count": Schema.integer("Words (default 16, at most 256)"),
            ],
            required: ["reference"], readOnly: true,
            run: { context, args in Out.text(try readMemory(context!, args)) }),
    ]

    private static func status(_ context: AutomationContext) -> String {
        let simulation = context.simulation
        guard simulation.isRunning else {
            return "not running" + (simulation.errorMessage.map { ": \($0)" } ?? "")
        }
        var text = "running; circuit time \(num(Double(simulation.circuitTime) / 1e9)) ms"
        if let reached = simulation.reachedSpeed { text += "; speed \(num(reached))x" }
        if let error = simulation.errorMessage { text += "; stopped with error: \(error)" }
        return text
    }

    private static func setSwitch(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let chip = try context.chip(try args.string("reference"))
        if chip.part.kind == PART_FUSE {
            // A fuse: on = intact, off = blown; only while the simulation runs
            // (a blown fuse is play-mode state, plan-47-M).
            let intact = args.optBool("on") ?? true
            guard context.simulation.setFuse(chip.id, blown: !intact) else {
                throw AutomationFailure("the simulation isn't running; call simulation_start to blow a fuse")
            }
            return "\(chip.reference) is \(intact ? "intact" : "blown")"
        }
        guard !chip.switchesOn.isEmpty else { throw AutomationFailure("\(chip.reference) isn't a switch") }
        if let slide = chip.part.slideStates {
            // A slide switch is always in one position (plan-58-G).
            guard let position = args.optInt("position"), (1...slide.positions).contains(position) else {
                throw AutomationFailure("\(chip.reference) is a slide switch: give position 1 … \(slide.positions)")
            }
            if args.optBool("on") == false { throw AutomationFailure("a slide switch is always in one position") }
            guard let state = document.setSlideState(chip.id, to: position) else {
                throw AutomationFailure(document.editorMessage ?? "the switch change was refused")
            }
            context.simulation.setSlide(chip.id, state: state)
            return "\(chip.reference) position \(state)"
        }
        let on = args.optBool("on") ?? true
        let position = args.optInt("position") ?? 1
        guard (1...chip.switchesOn.count).contains(position) else {
            throw AutomationFailure("\(chip.reference) has positions 1–\(chip.switchesOn.count)")
        }
        if chip.part.isTactileSwitch {
            guard let pressed = document.setButton(chip.id, pressed: on) else {
                throw AutomationFailure(document.editorMessage ?? "the press was refused")
            }
            context.simulation.setSwitch(chip.id, position: 1, on: pressed != chip.part.isNormallyClosed)
            return "\(chip.reference) \(pressed ? "pressed" : "released")"
        }
        if chip.switchesOn[position - 1] != on {
            guard let now = document.toggleSwitch(chip.id, position: position) else {
                throw AutomationFailure(document.editorMessage ?? "the switch change was refused")
            }
            context.simulation.setSwitch(chip.id, position: position, on: now)
        }
        return "\(chip.reference) position \(position) is \(on ? "ON" : "OFF")"
    }

    private static func readPins(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        guard context.simulation.isRunning else {
            throw AutomationFailure("the simulation isn't running; call simulation_start")
        }
        let chip = try context.chip(try args.string("reference"))
        var pins = chip.part.pins.filter { !$0.isHidden }
        if args.raw["pins"] != nil {
            pins = try args.strings("pins").map { name in
                guard let pin = pins.first(where: { $0.designator == name })
                    ?? pins.first(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame }) else {
                    throw AutomationFailure("\(chip.reference) has no pin \(name)")
                }
                return pin
            }
        }
        var lines = pins.map { pin in
            "\(pin.designator):\(pin.name)=\(context.simulation.level(ofPin: pin.designator, on: chip.id) ?? "-")"
        }
        if let lights = context.simulation.lights[chip.id] {
            lines.append("brightness: " + lights.map { num($0) }.joined(separator: ", "))
        }
        return "\(chip.reference) @ \(num(Double(context.simulation.circuitTime) / 1e9)) ms\n" + lines.joined(separator: "\n")
    }

    private static func readMemory(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        guard context.simulation.isRunning else {
            throw AutomationFailure("the simulation isn't running; call simulation_start")
        }
        let chip = try context.chip(try args.string("reference"))
        guard let size = context.simulation.memorySize(of: chip.id) else {
            throw AutomationFailure("\(chip.reference) isn't a memory")
        }
        let start = max(args.optInt("start") ?? 0, 0)
        let count = min(max(args.optInt("count") ?? 16, 1), 256)
        let words = context.simulation.readMemory(of: chip.id, start: start, count: count)
        guard !words.isEmpty else { throw AutomationFailure("address \(start) is past the end of the memory") }
        let digits = max(1, (size.bits + 3) / 4)
        let lines = words.enumerated().map { offset, word -> String in
            let hex = String(word.value, radix: 16, uppercase: true)
            let padded = String(repeating: "0", count: max(0, digits - hex.count)) + hex
            let marked = word.known == (size.bits >= 32 ? UInt32.max : (UInt32(1) << UInt32(size.bits)) - 1)
                ? padded : padded + "?"
            return "\(String(start + offset, radix: 16, uppercase: true)): \(marked)"
        }
        return "\(chip.reference) \(size.words) words x \(size.bits) bits\n" + lines.joined(separator: "\n")
    }
}
