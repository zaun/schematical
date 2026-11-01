//
//  Simulation.swift
//  schematical
//
//  Connects the schematic to the backend simulator. The netlist is built from
//  the wires, one simulation runs while the circuit is in play mode, and its
//  state is read back after every step for the canvas to draw.
//

import Foundation
import SwiftUI

/// The schematic as the backend's netlist text, plus the lookups needed to
/// map simulation results back onto the canvas.
struct SchematicNetlist {
    /// Text for `netlist_sim_open`.
    let text: String
    /// Chip for each part index, in the order the parts were written.
    let chipIDs: [UUID]
    /// Net index for each wire.
    let netOfWire: [UUID: Int]
}

enum NetlistBuilder {
    /// Two wire ends are the same electrical node when a wire joins them, or
    /// when they meet at a junction. Union-find over the wire ends gives the nets.
    static func build(chips: [PlacedChip], wires: [Wire]) -> SchematicNetlist {
        var parent: [WireEnd: WireEnd] = [:]

        func find(_ end: WireEnd) -> WireEnd {
            var current = end
            while let next = parent[current], next != current {
                current = next
            }
            return current
        }

        func union(_ a: WireEnd, _ b: WireEnd) {
            let rootA = find(a)
            let rootB = find(b)
            if rootA != rootB {
                parent[rootA] = rootB
            }
        }

        for wire in wires {
            union(wire.start, wire.end)
        }

        var netIndexByRoot: [WireEnd: Int] = [:]

        func netIndex(of end: WireEnd) -> Int {
            let root = find(end)
            if let index = netIndexByRoot[root] {
                return index
            }
            let index = netIndexByRoot.count
            netIndexByRoot[root] = index
            return index
        }

        var netOfWire: [UUID: Int] = [:]
        for wire in wires {
            netOfWire[wire.id] = netIndex(of: wire.start)
        }

        var partIndexByChip: [UUID: Int] = [:]
        for (index, chip) in chips.enumerated() {
            partIndexByChip[chip.id] = index
        }

        // Each connected pin, once, with the net it sits on.
        var pinNets: [PinRef: Int] = [:]
        for wire in wires {
            for end in [wire.start, wire.end] {
                if case let .pin(ref) = end {
                    pinNets[ref] = netIndex(of: end)
                }
            }
        }

        var lines: [String] = ["nets \(netIndexByRoot.count)"]
        for (index, chip) in chips.enumerated() {
            lines.append("part \(index) \(chip.partNumber)")
        }
        for (ref, net) in pinNets.sorted(by: { ($0.key.number, $0.key.chipID.uuidString) < ($1.key.number, $1.key.chipID.uuidString) }) {
            guard let part = partIndexByChip[ref.chipID] else { continue }
            lines.append("pin \(part) \(ref.number) \(net)")
        }

        return SchematicNetlist(
            text: lines.joined(separator: "\n") + "\n",
            chipIDs: chips.map(\.id),
            netOfWire: netOfWire
        )
    }
}

/// Owns one running simulation. Play mode starts it, Stop frees it.
@MainActor
@Observable
final class SimulationController {
    /// Simulated time per timer tick, in picoseconds. 10 us per 16 ms tick.
    static let picosecondsPerTick: UInt64 = 10_000_000
    static let tickInterval: Duration = .milliseconds(16)

    /// True while in play mode, including after the simulation has stopped with
    /// an error. Stop leaves play mode.
    private(set) var isRunning = false
    /// Why the simulation stopped, if it did.
    private(set) var errorMessage: String?

    private var netlist: SchematicNetlist?
    private var handle: OpaquePointer?
    private var task: Task<Void, Never>?
    private var netValues: [Character] = []
    private var shortedNets: Set<Int> = []
    private(set) var litChips: Set<UUID> = []

    func toggle(document: SchematicDocument) {
        if isRunning {
            stop()
        } else {
            start(document: document)
        }
    }

    func start(document: SchematicDocument) {
        stop()
        errorMessage = nil

        let built = NetlistBuilder.build(chips: document.chips, wires: document.wires)
        var errorPointer: UnsafeMutablePointer<CChar>?
        let opened = netlist_sim_open(built.text, &errorPointer)

        guard let opened else {
            if let errorPointer {
                errorMessage = String(cString: errorPointer)
                free(errorPointer)
            } else {
                errorMessage = "The simulation could not start."
            }
            return
        }

        netlist = built
        handle = opened
        isRunning = true
        refresh()

        task = Task { [weak self] in
            while !Task.isCancelled {
                guard let self else { return }
                if !self.step() { return }
                try? await Task.sleep(for: Self.tickInterval)
            }
        }
    }

    func stop() {
        task?.cancel()
        task = nil
        if let handle {
            netlist_sim_close(handle)
        }
        handle = nil
        netlist = nil
        isRunning = false
        netValues = []
        shortedNets = []
        litChips = []
    }

    /// Advances the simulation one tick. Returns false when it has stopped.
    private func step() -> Bool {
        guard let handle else { return false }
        let ok = netlist_sim_advance(handle, Self.picosecondsPerTick) != 0
        refresh()
        return ok && errorMessage == nil
    }

    /// Reads the simulator's state into the observable properties.
    private func refresh() {
        guard let handle, let netlist else { return }
        guard let raw = netlist_sim_snapshot(handle) else { return }
        defer { free(raw) }

        var values: [Character] = []
        var shorted: Set<Int> = []
        var lit: Set<UUID> = []

        for line in String(cString: raw).split(separator: "\n") {
            let fields = line.split(separator: " ")
            guard let keyword = fields.first else { continue }

            switch keyword {
            case "net":
                if fields.count == 3, let index = Int(fields[1]), let value = fields[2].first {
                    if index >= values.count {
                        values.append(contentsOf: Array(repeating: "Z", count: index - values.count + 1))
                    }
                    values[index] = value
                }
            case "lit":
                if fields.count == 3, let part = Int(fields[1]), part < netlist.chipIDs.count, fields[2] == "1" {
                    lit.insert(netlist.chipIDs[part])
                }
            case "shorted":
                if fields.count == 2, let index = Int(fields[1]) {
                    shorted.insert(index)
                }
            case "error":
                errorMessage = String(line.dropFirst("error ".count))
            default:
                break
            }
        }

        netValues = values
        shortedNets = shorted
        litChips = lit
    }

    /// Color for a wire in play mode. Outside play mode, the normal color.
    func wireColor(_ wireID: UUID) -> Color {
        guard isRunning, let net = netlist?.netOfWire[wireID] else { return .primary }
        if shortedNets.contains(net) { return .red }
        switch netValue(net) {
        case "1": return .green
        case "X": return .orange
        case "Z": return .secondary
        default: return .primary
        }
    }

    /// True when a wire is floating (no driver), drawn dashed in play mode.
    func wireIsFloating(_ wireID: UUID) -> Bool {
        guard isRunning, let net = netlist?.netOfWire[wireID] else { return false }
        return netValue(net) == "Z"
    }

    private func netValue(_ net: Int) -> Character {
        net < netValues.count ? netValues[net] : "Z"
    }
}
