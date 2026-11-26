import CoreGraphics
import Foundation

/// The document's placed parts and the rules for editing them (plan-68-F):
/// placing and naming, options, footprint choices and sockets, flips and
/// turns, switch states. Units are in `PartList+Units`, footprints on the
/// PCB in `PartList+Footprints`. What an edit needs from the board (its
/// grid) comes in as a parameter; the wires on a part's pins are the
/// document's to re-fit.
struct PartList {
    var chips: [PlacedChip]
    /// Lowest number tried for the next part's reference ("U" plus a number).
    var nextReference: Int

    func index(_ id: UUID) -> Int? {
        chips.firstIndex { $0.id == id }
    }

    // MARK: Placing and naming

    /// The reference for a new part, taken, with its prefix from Settings ▸
    /// Reference Defaults (`ReferencePrefixes`). A part with the Integrated
    /// Circuit prefix gets the first number from `nextReference` on that no
    /// part uses, which moves `nextReference` past it; any other part the
    /// lowest number no part uses. `alsoUsed` are references taken but not
    /// yet in `chips` (a paste in progress); `gender` is a header's.
    mutating func takeReference(for part: CatalogPart, gender: String?, alsoUsed: [String] = []) -> String {
        let used = Set(chips.map(\.reference) + alsoUsed)
        let prefix = ReferencePrefixes.prefix(for: part, gender: gender)
        let counted = prefix == ReferencePrefixes.prefix(for: .integratedCircuit)
        var number = counted ? nextReference : 1
        while used.contains(ReferenceParts.reference(prefix: prefix, number: number)) {
            number += 1
        }
        if counted { nextReference = number + 1 }
        return ReferenceParts.reference(prefix: prefix, number: number)
    }

    /// Places a part with its centre under `point`, snapped to the grid,
    /// with a new reference and a footprint spot beside the board's parts
    /// (on `grid`). Returns its id.
    mutating func place(part: CatalogPart, symbol: PartSymbol, centeredAt point: CGPoint, color: String?,
                        footprintID: String?, gender: String?, ohms: Double?, farads: Double?, grid: Double) -> UUID {
        let layout = ChipLayout(part: part, symbol: symbol)
        let origin = SchematicGrid.snap(CGPoint(x: point.x - layout.size.width / 2, y: point.y - layout.size.height / 2))
        var placed = PlacedChip(reference: takeReference(for: part, gender: gender), part: part, symbol: symbol,
                                origin: origin, color: color, footprintID: footprintID, gender: gender,
                                ohms: ohms, farads: farads)
        placed.board = placed.footprint.map {
            BoardLayout.nextSpot(for: $0, among: chips, pad1: placed.part.firstPinDesignator,
                                 hasPinOneMark: placed.part.hasPinOneMark, grid: grid)
        }
        chips.append(placed)
        return placed.id
    }

    /// Renames a part to `prefix` (uppercased) plus `number`. Returns why it
    /// can't (a bad prefix or number, or a reference another part has).
    mutating func rename(_ id: UUID, prefix: String, number: Int?) -> String? {
        if let message = ReferenceParts.validate(prefix: prefix, number: number) { return message }
        guard let number, let index = index(id) else { return nil }
        let reference = ReferenceParts.reference(prefix: prefix, number: number)
        if chips.contains(where: { $0.id != id && $0.reference == reference }) {
            return "\(reference) is already used by another part."
        }
        chips[index].reference = reference
        return nil
    }

    /// Every part gets its default prefix back and is numbered again from 1
    /// within its prefix, in reading order on the Schematic (top to bottom,
    /// then left to right); the next new part continues the Integrated
    /// Circuit prefix's numbers.
    mutating func resetReferences() {
        let order = chips.indices.sorted { a, b in
            let ca = chips[a], cb = chips[b]
            if ca.origin.y != cb.origin.y { return ca.origin.y < cb.origin.y }
            if ca.origin.x != cb.origin.x { return ca.origin.x < cb.origin.x }
            return ca.reference.localizedStandardCompare(cb.reference) == .orderedAscending
        }
        var counts: [String: Int] = [:]
        for index in order {
            let prefix = ReferencePrefixes.prefix(for: chips[index].part, gender: chips[index].gender)
            let number = (counts[prefix] ?? 0) + 1
            counts[prefix] = number
            let reference = ReferenceParts.reference(prefix: prefix, number: number)
            if chips[index].reference != reference { chips[index].reference = reference }
        }
        nextReference = (counts[ReferencePrefixes.prefix(for: .integratedCircuit)] ?? 0) + 1
    }

    // MARK: Options

    /// Changes part `id` when `allowed` says its part may.
    private mutating func change(_ id: UUID, if allowed: (PlacedChip) -> Bool, _ edit: (inout PlacedChip) -> Void) {
        guard let index = index(id), allowed(chips[index]) else { return }
        var chip = chips[index]
        edit(&chip)
        if chip != chips[index] { chips[index] = chip }
    }

    /// An LED's color, one the part has.
    mutating func setColor(_ id: UUID, _ color: String) {
        change(id, if: { $0.part.kind == PART_LED && $0.part.colors.contains(color) }) { $0.color = color }
    }

    mutating func setShowsReference(_ id: UUID, _ shows: Bool) {
        change(id, if: { _ in true }) { $0.showsReference = shows }
    }

    /// A header's gender, one the part lists.
    mutating func setGender(_ id: UUID, _ gender: String) {
        change(id, if: { $0.part.kind == PART_HEADER && $0.part.genders.contains(gender) }) { $0.gender = gender }
    }

    /// The resistance of a part with an ohms field, a valid value.
    mutating func setOhms(_ id: UUID, _ ohms: Double) {
        change(id, if: { $0.part.hasOhmsField && ResistorValue.isValid(ohms) }) { $0.ohms = ohms }
    }

    /// The generic capacitor's capacitance, a valid value.
    mutating func setFarads(_ id: UUID, _ farads: Double) {
        change(id, if: { $0.part.isGenericCapacitor && CapacitorValue.isValid(farads) }) { $0.farads = farads }
    }

    /// A part's load file: an EEPROM's contents or a microcontroller's firmware.
    mutating func setMemoryFile(_ id: UUID, _ file: MemoryFile?) {
        change(id, if: { $0.part.hasLoadFile }) { $0.memoryFile = file }
    }

    /// Whether header `id` can become `newPart`: both headers, another size.
    func canResizeHeader(_ id: UUID, to newPart: CatalogPart) -> Bool {
        guard let index = index(id) else { return false }
        return chips[index].part.kind == PART_HEADER && newPart.kind == PART_HEADER && chips[index].part != newPart
    }

    /// Puts part `id` on `newPart` (a header's new size, or a library part
    /// updated in place), keeping its reference, origin, flip, options and
    /// board placement, with pad 1 at its board point. The wires on pins
    /// `newPart` lacks are the caller's to loosen first.
    mutating func swap(_ id: UUID, to newPart: CatalogPart, symbol newSymbol: PartSymbol?) {
        guard let index = index(id) else { return }
        let old = chips[index]
        let pad1 = old.part.firstPinDesignator
        let anchor = old.board.flatMap { board in
            old.footprint?.pads.first { $0.designator == pad1 }.map { board.board($0.centre) }
        }
        chips[index] = old.resized(to: newPart, symbol: newSymbol)
        // The reference was placed for the old body.
        chips[index].board?.referenceOffset = .zero
        if let anchor, var board = chips[index].board,
           let pad = chips[index].footprint?.pads.first(where: { $0.designator == pad1 }) {
            let landed = board.board(pad.centre)
            board.centre.x += anchor.x - landed.x
            board.centre.y += anchor.y - landed.y
            chips[index].board = board
        }
    }

    /// Picks one of a part's footprint choices, keeping pad 1 at its board point.
    mutating func setFootprint(_ id: UUID, _ footprintID: String) {
        guard let index = index(id), chips[index].footprintID != footprintID,
              chips[index].part.footprintChoices.contains(where: { $0.footprint.id == footprintID }) else { return }
        changingFootprint(of: index) { $0.footprintID = footprintID }
    }

    /// Runs `change`, which swaps the footprint of the part at `index`, and
    /// keeps the pad of its first pin at its board point: the placement
    /// moves by the difference between the old and new pad 1, the reference
    /// text goes back to its default offset.
    private mutating func changingFootprint(of index: Int, _ change: (inout PlacedChip) -> Void) {
        let before = chips[index].footprint
        change(&chips[index])
        guard var changed = chips[index].board, let before, let after = chips[index].footprint else { return }
        changed.referenceOffset = .zero
        let pad1 = chips[index].part.firstPinDesignator
        if let old = before.pads.first(where: { $0.designator == pad1 }),
           let new = after.pads.first(where: { $0.designator == pad1 }) {
            let anchor = changed.board(old.centre)
            let landed = changed.board(new.centre)
            changed.centre.x += anchor.x - landed.x
            changed.centre.y += anchor.y - landed.y
        }
        chips[index].board = changed
    }

    /// Puts a part in or out of a socket (see `SchematicDocument.setPartSocket`).
    mutating func setSocket(_ id: UUID, mode: ChipSocket.Mode?, socket: CatalogPart?) {
        guard let index = index(id) else { return }
        var new: ChipSocket?
        if let mode {
            guard let part = socket ?? chips[index].socket?.part, ChipSocket.fits(part, chips[index].part) else { return }
            new = ChipSocket(part: part, mode: mode)
        }
        guard chips[index].socket != new else { return }
        if chips[index].socket?.part.footprint?.id == new?.part.footprint?.id {
            chips[index].socket = new
        } else {
            changingFootprint(of: index) { $0.socket = new }
        }
    }

    // MARK: Turning

    /// Swaps the side of every pin of a part that flips. Returns whether it did.
    mutating func flip(_ id: UUID) -> Bool {
        guard let index = index(id), chips[index].symbol.canFlip, !chips[index].isSplit else { return false }
        chips[index].isFlipped.toggle()
        return true
    }

    /// Turns a part that rotates a quarter turn. Returns whether it did.
    mutating func rotate(_ id: UUID, counterclockwise: Bool) -> Bool {
        guard let index = index(id), chips[index].symbol.canRotate, !chips[index].isSplit else { return false }
        chips[index].rotation = PlacedChip.normalized(chips[index].rotation + (counterclockwise ? 270 : 90))
        return true
    }

    // MARK: Switches

    /// The positions a switch would have with position `position` (1-based)
    /// flipped, or nil when it has no such position.
    func toggled(_ id: UUID, position: Int) -> (index: Int, on: [Bool])? {
        guard let index = index(id), chips[index].switchesOn.indices.contains(position - 1) else { return nil }
        var on = chips[index].switchesOn
        on[position - 1].toggle()
        return (index, on)
    }

    /// Sets a switch's positions (already checked).
    mutating func setSwitches(at index: Int, _ on: [Bool]) {
        if chips[index].switchesOn != on { chips[index].switchesOn = on }
    }

    /// Moves a slide switch's knob to `state` (its contacts `closed`),
    /// recording which way it moved.
    mutating func setSlide(at index: Int, state: Int, closed: [Bool]) {
        if let before = chips[index].slidePosition, before != state {
            chips[index].slideDirection = state > before ? 1 : -1
        }
        chips[index].switchesOn = closed
    }

    /// Puts switches back to `saved`: parts not in it, or whose position
    /// count changed, are left alone.
    mutating func restoreSwitches(_ saved: [UUID: [Bool]]) {
        for index in chips.indices {
            guard let positions = saved[chips[index].id], positions.count == chips[index].switchesOn.count,
                  positions != chips[index].switchesOn else { continue }
            chips[index].switchesOn = positions
        }
    }

    /// Releases every pressed tactile switch.
    mutating func releaseAllButtons() {
        for index in chips.indices where chips[index].isPressed {
            chips[index].switchesOn[0] = false
        }
    }
}
