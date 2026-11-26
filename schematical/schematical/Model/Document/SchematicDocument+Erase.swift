import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Lock Position (PCB)

    /// Whether a PCB item's position is locked. False for an item that
    /// isn't there.
    func isLocked(_ item: BoardItem) -> Bool {
        switch item {
        case let .footprint(id): chips.first { $0.id == id }?.board?.locked ?? false
        case let .text(id): texts.first { $0.id == id }?.locked ?? false
        case let .trace(id): traces.first { $0.id == id }?.locked ?? false
        case let .via(id): vias.first { $0.id == id }?.locked ?? false
        case let .hole(id): holes.first { $0.id == id }?.locked ?? false
        case let .pour(id): pours.first { $0.id == id }?.locked ?? false
        case let .keepout(id): keepouts.first { $0.id == id }?.locked ?? false
        case let .cutout(id): cutouts.first { $0.id == id }?.locked ?? false
        }
    }

    /// Locks or unlocks a PCB item's position (the sidebar's Lock Position).
    func setLocked(_ item: BoardItem, _ locked: Bool) {
        guard isLocked(item) != locked else { return }
        edit(.pcb, locked ? "Lock Position" : "Unlock Position") { scope in
            switch item {
            case let .footprint(id): updateParts(scope) { $0.setLocked(footprint: id, locked) }
            case let .text(id): layout.updateTexts(scope) { $0.setLocked(id, locked) }
            case let .trace(id): layout.updateCopper(scope) { $0.setLocked(trace: id, locked) }
            case let .via(id): layout.updateCopper(scope) { $0.setLocked(via: id, locked) }
            case .hole, .pour, .keepout, .cutout: layout.updateFeatures(scope) { $0.setLocked(item, locked) }
            }
        }
    }

    /// Locks or unlocks every item in `items` as one undo step.
    func setLocked(_ items: some Collection<BoardItem>, _ locked: Bool) {
        guard items.contains(where: { isLocked($0) != locked }) else { return }
        recordEdit(.pcb, locked ? "Lock Position" : "Unlock Position") {
            for item in items { setLocked(item, locked) }
        }
    }

    /// Puts every footprint and text in `items` on `side` of the board, as
    /// one undo step. Other items have no side and are skipped.
    func setSide(of items: some Collection<BoardItem>, _ side: BoardSide) {
        recordEdit(.pcb, "Change Side") {
            for item in items {
                switch item {
                case let .footprint(id): setFootprintSide(id, side)
                case let .text(id): setTextSide(id, side)
                default: break
                }
            }
        }
    }

    // MARK: Erasing

    /// Removes whatever is under `point`: a block first (its wires stay, with
    /// loose ends where its pins were), then a net label (the same), then
    /// a bus port, then a wire, then a bus line, then a text block. A port with
    /// wires on its pins, or a bus line with ports, asks first (`pendingRemoval`); while
    /// that is up, nothing is erased.
    func erase(at point: CGPoint, tolerance: CGFloat = 6) {
        edit(.schematic, "Delete") { scope in
            guard pendingRemoval == nil else { return }
            if let hit = chip(at: point) {
                if hit.isSplit {
                    // Every symbol of the part would go: ask first (plan-67).
                    schematicSelection.replace(with: .chip(hit.id))
                    pendingRemoval = PendingRemoval(
                        message: Self.removalQuestion(split: [hit], of: 1),
                        action: .parts(items: [.chip(hit.id)], name: "Delete", includingTitleBlock: true))
                    return
                }
                loosen(scope) { attaches($0, to: hit.id) }
                updateParts(scope) { $0.chips.removeAll { $0.id == hit.id } }
            } else if let hit = netLabel(at: point) {
                deleteNetLabel(hit.id)
            } else if let portID = portHit(at: point) {
                askOrRemovePort(portID, scope)
                return
            } else if let hit = segmentHit(at: point, tolerance: tolerance) {
                sheet.updateWiring(scope) { $0.wires.removeAll { $0.id == hit.wire } }
            } else if let hit = busHit(at: point, tolerance: tolerance), let bus = bus(hit.bus) {
                // The card edge's bus can't go: not its last line.
                if bus.id == edgeBusID, bus.lines.count == 1 {
                    editorMessage = BusNetwork.edgeBusUndeletable(bus)
                    return
                }
                let ports = bus.ports.filter { $0.line == hit.line }
                if ports.isEmpty {
                    removeBusLine(bus: hit.bus, line: hit.line, scope)
                } else {
                    let count = ports.map { wiresOnPins(port: $0.id) }.reduce(0, +)
                    pendingRemoval = PendingRemoval(
                        message: "Delete this bus line and its \(Self.counted(ports.count, "port"))? The \(Self.counted(count, "wire")) on their pins stay, with loose ends.",
                        action: .busLine(bus: hit.bus, line: hit.line))
                }
                return
            } else if let hit = note(at: point) {
                deleteNote(hit.id)
                return
            } else if titleBlock(at: point) {
                // Only switches it off: the information stays.
                updateInfo(scope) { $0.titleBlock = nil }
                pruneSelection()
                return
            } else {
                return
            }
            pruneJunctions(scope)
        }
    }

    /// Deletes a port, asking first (`pendingRemoval`) when wires are on its pins.
    func askOrRemovePort(_ portID: UUID, _ scope: EditScope) {
        let count = wiresOnPins(port: portID)
        if count > 0 {
            pendingRemoval = PendingRemoval(
                message: "Delete this port? The \(Self.counted(count, "wire")) on its pins stay, with loose ends.",
                action: .port(portID))
        } else {
            removePort(portID, scope)
        }
    }

    /// ⌘⌫ on the Schematic: deletes the selected parts (their wires stay,
    /// with loose ends, as the Eraser leaves them), wires and text blocks, as
    /// one step; junctions go once no wire ends on them. A selected bus port
    /// is deleted as the Eraser deletes one, asking first when wires are on
    /// its pins. Selected buses go whole, with their ports; the wires on
    /// their pins stay, with loose ends. The card edge's bus stays
    /// (`editorMessage` says why). Returns whether anything was deleted or
    /// asked.
    /// ⌘X passes `name` "Cut" (with no port selected: a port isn't copied).
    /// A selected title block is only switched off (the information stays),
    /// unless `includingTitleBlock` is false: ⌘X doesn't take it, so it stays
    /// shown.
    /// A selection with a part drawn as its units asks first
    /// (`pendingRemoval`), unless `confirmed`: the agent's delete, and the
    /// confirm itself.
    @discardableResult
    func deleteSchematicSelection(named name: String = "Delete", includingTitleBlock: Bool = true,
                                  confirmed: Bool = false) -> Bool {
        guard pendingRemoval == nil else { return false }
        if let portID = selectedPortID {
            edit(.schematic, "Delete") { scope in askOrRemovePort(portID, scope) }
            return true
        }
        var chipIDs: Set<UUID> = [], wireIDs: Set<UUID> = [], noteIDs: Set<UUID> = []
        var labelIDs: Set<UUID> = [], busIDs: Set<UUID> = []
        var hidesTitleBlock = false
        for item in schematicSelection.items {
            switch item {
            case let .chip(id): chipIDs.insert(id)
            case let .wire(id): wireIDs.insert(id)
            case let .note(id): noteIDs.insert(id)
            case let .label(id): labelIDs.insert(id)
            case let .bus(id): busIDs.insert(id)
            case .titleBlock: hidesTitleBlock = includingTitleBlock
            case .junction: break
            }
        }
        var message: String?
        if let edge = edgeBusID, busIDs.contains(edge), let bus = bus(edge) {
            busIDs.remove(edge)
            message = BusNetwork.edgeBusUndeletable(bus)
        }
        guard !chipIDs.isEmpty || !wireIDs.isEmpty || !noteIDs.isEmpty || !labelIDs.isEmpty || !busIDs.isEmpty
            || hidesTitleBlock else {
            if let message { editorMessage = message }
            return false
        }
        let split = chips.filter { chipIDs.contains($0.id) && $0.isSplit }
        if !confirmed, !split.isEmpty {
            pendingRemoval = PendingRemoval(
                message: Self.removalQuestion(split: split, of: chipIDs.count),
                action: .parts(items: schematicSelection.items, name: name, includingTitleBlock: includingTitleBlock))
            return true
        }
        let portIDs = Set(buses.filter { busIDs.contains($0.id) }.flatMap { $0.ports.map(\.id) })
        edit(.schematic, name) { scope in
            sheet.updateWiring(scope) { $0.wires.removeAll { wireIDs.contains($0.id) } }
            loosen(scope) { end in
                switch end {
                case let .label(id): labelIDs.contains(id)
                case let .busPin(ref): portIDs.contains(ref.portID)
                default: chipIDs.contains { attaches(end, to: $0) }
                }
            }
            sheet.updateBuses(scope) { $0.buses.removeAll { busIDs.contains($0.id) } }
            if message != nil { editorMessage = message }
            updateParts(scope) { $0.chips.removeAll { chipIDs.contains($0.id) } }
            sheet.updateLabels(scope) { $0.remove(labelIDs) }
            sheet.updateNotes(scope) { $0.remove(noteIDs) }
            if let editing = editingNoteID, noteIDs.contains(editing) { editingNoteID = nil }
            if hidesTitleBlock { updateInfo(scope) { $0.titleBlock = nil } }
            pruneJunctions(scope)
        }
        return true
    }
}
