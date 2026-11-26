import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Chips

    /// Runs `change` on the parts as one undo step named `name` on `page`.
    func editParts<T>(_ name: String, page: EditorPage = .schematic, _ change: (inout PartList) -> T) -> T {
        edit(page, name) { scope in updateParts(scope, change) }
    }

    /// Places a block with its centre under `point`, snapped to the grid.
    /// The new part is not selected, so the parts library stays open for
    /// the next drop.
    /// `color`, `footprintID` and `ohms` are the part's options (LED, parts
    /// with footprint choices, pull resistors); nil (or a value the part
    /// doesn't have) is the part's default.
    func placeChip(part: CatalogPart, symbol: PartSymbol, centeredAt point: CGPoint,
                   color: String? = nil, footprintID: String? = nil, gender: String? = nil,
                   ohms: Double? = nil, farads: Double? = nil) {
        let grid = board.grid
        edit(.schematic, "Add Part") { scope in
            let id = updateParts(scope) {
                $0.place(part: part, symbol: symbol, centeredAt: point, color: color, footprintID: footprintID,
                         gender: gender, ohms: ohms, farads: farads, grid: grid)
            }
            adoptLooseEnds(chips: [id])
        }
    }

    /// Sets an LED's color. Does nothing for a part that isn't an LED, or a
    /// color the part doesn't have.
    func setPartColor(_ id: UUID, _ color: String) {
        editParts("Change Color") { $0.setColor(id, color) }
    }

    /// Turns the reference above a part on the Schematic on or off.
    func setPartShowsReference(_ id: UUID, _ shows: Bool) {
        editParts(shows ? "Show Reference" : "Hide Reference") { $0.setShowsReference(id, shows) }
    }

    /// Sets a header's gender. Does nothing for a part that isn't a header,
    /// or a gender the part doesn't list.
    func setPartGender(_ id: UUID, _ gender: String) {
        editParts("Change Gender") { $0.setGender(id, gender) }
    }

    /// Changes a header's size by swapping its part for `newPart` (the
    /// caller looks up `HeaderFamily.name(rows:perRow:)` in its catalog).
    /// Reference, origin, flip, gender, footprint type and board placement
    /// are kept, and on the PCB pad 1 keeps its board point. Wires on pins
    /// the new size doesn't have stay, with loose ends at those pins' old
    /// tips; the rest are re-fitted. Does
    /// nothing unless both are headers and `newPart` is another size.
    func setHeaderSize(_ id: UUID, to newPart: CatalogPart) {
        guard parts.canResizeHeader(id, to: newPart) else { return }
        edit(.schematic, "Change Size") { scope in swapPart(of: id, to: newPart, scope) }
    }

    /// Sets the resistance of a pull resistor or `GENERIC-RESISTOR`
    /// (`hasOhmsField`). Does nothing for any other part, or a value that
    /// isn't `ResistorValue.isValid`.
    func setPartOhms(_ id: UUID, _ ohms: Double) {
        editParts("Change Resistance") { $0.setOhms(id, ohms) }
    }

    /// Sets the generic capacitor's capacitance (plan-33-H). Does nothing for
    /// any other part, or a value that isn't `CapacitorValue.isValid`.
    func setPartFarads(_ id: UUID, _ farads: Double) {
        editParts("Change Capacitance") { $0.setFarads(id, farads) }
    }

    /// Picks one of a part's footprint choices. On the PCB the pad of the
    /// part's first pin keeps its board point (no re-snap, like
    /// `setFootprintStyle`); rotation, origin, marks and style are kept. Does
    /// nothing for an id that isn't one of the part's choices, or the current one.
    func setPartFootprint(_ id: UUID, _ footprintID: String) {
        editParts("Change Footprint") { $0.setFootprint(id, footprintID) }
    }

    /// Puts a chip in or out of a socket (plan-51-H). `mode` nil is a plain IC;
    /// otherwise `socket` is the socket part (a chip already in a socket mode
    /// keeps its own when `socket` is nil). Does nothing for a socket that
    /// does not hold the chip's footprint, a chip a socket cannot take, or
    /// when nothing changes. Going between IC and a socket mode puts the
    /// socket's footprint (or the chip's back) on the PCB with the first pin's
    /// pad where it was; between the two socket modes only the mode changes,
    /// and between two sockets that share a footprint nothing moves.
    func setPartSocket(_ id: UUID, mode: ChipSocket.Mode?, socket: CatalogPart?) {
        editParts("Change Socket") { $0.setSocket(id, mode: mode, socket: socket) }
    }

    /// Renames a part to `prefix` (uppercased) plus `number`. Returns why it
    /// can't (a bad prefix or number, or a reference another part has), or
    /// nil when it was renamed. `page` is the page the rename was made on;
    /// the step is on both pages' lists either way.
    @discardableResult
    func renameChip(_ id: UUID, prefix: String, number: Int?, page: EditorPage) -> String? {
        editParts("Rename", page: page) { $0.rename(id, prefix: prefix, number: number) }
    }

    /// Edit › Reset References: every part gets its prefix from Settings ▸
    /// Reference Defaults back (`ReferencePrefixes`) and is numbered again from 1
    /// within its prefix, in reading order on the Schematic (top to bottom,
    /// then left to right), as one step. The next new part continues the
    /// Integrated Circuit prefix's numbers.
    /// Returns whether any reference changed.
    @discardableResult
    func resetReferences(page: EditorPage) -> Bool {
        var renamed = parts
        renamed.resetReferences()
        guard renamed.chips != chips || renamed.nextReference != nextReference else { return false }
        editParts("Reset References", page: page) { $0 = renamed }
        return true
    }

    /// After a part's pins moved: re-fits its wires and connects loose ends
    /// its pins landed on.
    func settled(_ id: UUID) {
        settleWires(attachedTo: id)
        adoptLooseEnds(chips: [id])
    }

    /// Swaps the side of every pin on a placed part, and re-fits the wires
    /// attached to it.
    func flipChip(_ id: UUID) {
        edit(.schematic, "Flip") { scope in
            if updateParts(scope, { $0.flip(id) }) { settled(id) }
        }
    }

    /// Turns a part that can rotate (`PartSymbol.canRotate`) a quarter turn,
    /// clockwise or not, and re-fits the wires attached to it.
    func rotateChip(_ id: UUID, counterclockwise: Bool = false) {
        edit(.schematic, "Rotate") { scope in
            if updateParts(scope, { $0.rotate(id, counterclockwise: counterclockwise) }) { settled(id) }
        }
    }

    // MARK: Units (plan-67)

    /// The sidebar's View option: draws a part as its units (a gate IC as
    /// separate gates) or as one block again. The wires stay on their pins.
    /// Nothing for a part with no units, or one already drawn that way.
    func setPartSplit(_ id: UUID, _ split: Bool) {
        guard let chip = chips.first(where: { $0.id == id }), !chip.part.units.isEmpty, chip.isSplit != split else { return }
        edit(.schematic, "Change View") { scope in
            _ = updateParts(scope) { $0.setSplit(id, split) }
            settled(id)
        }
    }

    /// Moves one unit's body corner to `origin` (snapped to the grid).
    func moveUnit(_ ref: UnitRef, to origin: CGPoint) {
        guard parts.placementIndex(ref) != nil else { return }
        edit(.schematic, "Move") { scope in
            updateParts(scope) { $0.moveUnit(ref, to: origin) }
            settled(ref.chipID)
        }
    }

    /// Mirrors one unit left to right.
    func flipUnit(_ ref: UnitRef) {
        guard parts.placementIndex(ref) != nil else { return }
        edit(.schematic, "Flip") { scope in
            updateParts(scope) { $0.flipUnit(ref) }
            settled(ref.chipID)
        }
    }

    /// Turns one unit a quarter turn. Nothing for a unit drawn as a block.
    func rotateUnit(_ ref: UnitRef, counterclockwise: Bool = false) {
        guard parts.canRotateUnit(ref) else { return }
        edit(.schematic, "Rotate") { scope in
            updateParts(scope) { $0.rotateUnit(ref, counterclockwise: counterclockwise) }
            settled(ref.chipID)
        }
    }

    /// What Rotate or Flip acts on: a part, or one unit of a part drawn as
    /// its units.
    enum EditTarget: Equatable {
        case chip(UUID)
        case unit(UnitRef)
    }

    /// What Rotate would turn on the Schematic: the sole selected part when
    /// its symbol turns, or, of a part drawn as its units, the one picked
    /// unit when that turns (a gate; not the block of leftover pins).
    var rotateTarget: EditTarget? { editTarget { $0.symbol.canRotate } }

    /// What Flip would mirror: as `rotateTarget`, for symbols that flip.
    var flipTarget: EditTarget? { editTarget { $0.symbol.canFlip } }

    func editTarget(_ can: (PlacedChip) -> Bool) -> EditTarget? {
        guard let id = selectedChipID, let chip = chips.first(where: { $0.id == id }) else { return nil }
        guard chip.isSplit else { return can(chip) ? .chip(id) : nil }
        let keys = picked(chip)
        guard keys.count == 1, let key = keys.first, let piece = chip.piece(forUnit: key), can(piece) else { return nil }
        return .unit(UnitRef(chipID: id, key: key))
    }

    /// The picked units of a part: every unit unless some were picked.
    func picked(_ chip: PlacedChip) -> Set<String> {
        let all = Set(chip.unitPlacements.map(\.key))
        guard isSelected(.chip(chip.id)), let some = pickedUnits[chip.id]?.intersection(all), !some.isEmpty else { return all }
        return some
    }

    /// A click on a unit: it alone is picked; with `extend` (Shift) it is
    /// added to, or taken out of, the picked ones.
    func pick(_ ref: UnitRef, extend: Bool) {
        guard extend, var some = pickedUnits[ref.chipID] else {
            pickedUnits[ref.chipID] = [ref.key]
            return
        }
        if some.contains(ref.key) { some.remove(ref.key) } else { some.insert(ref.key) }
        pickedUnits[ref.chipID] = some.isEmpty ? nil : some
    }

    /// Whether a drawn thing is shown selected: a selected part, or a picked
    /// unit of one (`piece` is a part or a piece of one).
    func isPicked(_ piece: PlacedChip) -> Bool {
        guard isSelected(.chip(piece.id)) else { return false }
        guard let key = piece.unitKey, let some = pickedUnits[piece.id], !some.isEmpty else { return true }
        return some.contains(key)
    }

    /// A unit of a selected part that is not itself picked: shown faintly,
    /// so it reads as part of the same IC.
    func isSibling(_ piece: PlacedChip) -> Bool {
        piece.unitKey != nil && isSelected(.chip(piece.id)) && !isPicked(piece)
    }

    /// A Select press on what is drawn at a point (`piece(at:)`): a part, as
    /// `pressSelection`; or a unit of a part drawn as its units, which picks
    /// that unit.
    ///
    /// - No Shift: the unit becomes the only picked one, unless it is one of
    ///   several picked things: then the drag moves them all and a click
    ///   keeps only it.
    /// - Shift: the unit joins the picked ones; a click on a picked unit
    ///   takes it out again, and the part with it when it was the last.
    func pressPiece(_ piece: PlacedChip, shift: Bool) -> SelectionPress<SchematicItem> {
        guard let key = piece.unitKey, let chip = chips.first(where: { $0.id == piece.id }) else {
            return pressSelection(.chip(piece.id), shift: shift)
        }
        let item = SchematicItem.chip(piece.id)
        let wasSelected = isSelected(item)
        let before = picked(chip)
        pendingPick = nil
        if shift {
            if !wasSelected {
                let press = schematicSelection.press(item, shift: true)
                pickedUnits[piece.id] = [key]
                return press
            }
            if before == [key] {
                // The last picked unit: a click takes the part out of the selection.
                return schematicSelection.press(item, shift: true)
            }
            var some = before
            if some.contains(key) { some.remove(key) } else { some.insert(key) }
            pickedUnits[piece.id] = some
            return SelectionPress(dragsGroup: true, onClick: .none)
        }
        if wasSelected, before.contains(key), before.count > 1 || schematicSelection.count > 1 {
            pendingPick = UnitRef(chipID: piece.id, key: key)
            let press = schematicSelection.press(item, shift: false)
            return SelectionPress(dragsGroup: true, onClick: press.onClick)
        }
        let press = schematicSelection.press(item, shift: false)
        pickedUnits[piece.id] = [key]
        return press
    }

    /// Picks exactly these units of their parts (a box select).
    func pickUnits(_ refs: Set<UnitRef>) {
        for chipID in Set(refs.map(\.chipID)) {
            pickedUnits[chipID] = Set(refs.filter { $0.chipID == chipID }.map(\.key))
        }
    }

    /// Forgets the picks of parts that are no longer selected, so a part
    /// selected again starts with every unit picked.
    func prunePicks() {
        guard !pickedUnits.isEmpty else { return }
        let selected = schematicSelectedParts
        let kept = pickedUnits.filter { selected.contains($0.key) }
        if kept.count != pickedUnits.count { pickedUnits = kept }
    }

    func chip(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.piece(at: point) != nil }
    }

    /// What is drawn under a point: a part, or the piece of a part drawn as
    /// its units (plan-67; `PlacedChip.unitKey` says which).
    func piece(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.piece(at: point) != nil }?.piece(at: point)
    }

    /// The junction within `tolerance` of `point`, if any.
    func junctionHit(at point: CGPoint, tolerance: CGFloat = 6) -> Junction? {
        junctions.last { Self.distance(from: point, to: $0.point) <= tolerance }
    }

    // MARK: Footprints (PCB)

    /// Moves a footprint's centre (board mm), then snaps its origin point to
    /// the board grid.
    func moveFootprint(_ id: UUID, to centre: CGPoint) {
        let grid = board.grid
        editParts("Move", page: .pcb) { $0.moveFootprint(id, to: centre, grid: grid) }
    }

    /// Moves a footprint so its origin point lands on `point` (board mm),
    /// snapped to the grid.
    func moveFootprintOrigin(_ id: UUID, to point: CGPoint) {
        let grid = board.grid
        editParts("Move", page: .pcb) { $0.moveFootprintOrigin(id, to: point, grid: grid) }
    }

    /// Turns a footprint 90° clockwise about its origin point, which stays
    /// where it was.
    func rotateFootprint(_ id: UUID, counterclockwise: Bool = false) {
        let grid = board.grid
        editParts("Rotate", page: .pcb) { $0.rotateFootprint(id, counterclockwise: counterclockwise, grid: grid) }
    }

    /// Moves a footprint to the other side of the board, mirrored left to
    /// right as seen from above. Its origin point stays where it was, then
    /// snaps, as in `rotateFootprint`. Does nothing on a 1-layer board or when
    /// the side doesn't change.
    func setFootprintSide(_ id: UUID, _ side: BoardSide) {
        let layers = board.layers, grid = board.grid
        editParts("Change Side", page: .pcb) { $0.setFootprintSide(id, side, layers: layers, grid: grid) }
    }

    /// Changes which point of a footprint is its origin. The footprint stays
    /// where it is and is not re-snapped.
    func setFootprintOrigin(_ id: UUID, _ origin: FootprintOrigin) {
        editParts("Change Origin", page: .pcb) { $0.setFootprintOrigin(id, origin) }
    }

    /// Switches a part between its footprint and its connector footprint
    /// (the PCB Style option). Pin 1's first pad stays exactly where it was
    /// (not re-snapped: like an origin change, this may leave the origin
    /// point off the grid); rotation, origin and marks are kept. Does nothing for a part without a connector
    /// footprint or when the style does not change.
    func setFootprintStyle(_ id: UUID, _ style: FootprintStyle) {
        editParts("Change Style", page: .pcb) { $0.setFootprintStyle(id, style) }
    }

    /// Sets where a footprint's marks are drawn. A nil layer is left as it is.
    func setFootprintMarks(_ id: UUID, outline: MarkLayer? = nil, pin1: MarkLayer? = nil,
                           reference: MarkLayer? = nil) {
        setFootprintMarks([id], outline: outline, pin1: pin1, reference: reference)
    }

    /// Sets where the marks of every footprint in `ids` are drawn, as one
    /// undo step. A nil layer is left as it is; a part with no footprint is
    /// skipped.
    func setFootprintMarks(_ ids: some Collection<UUID>, outline: MarkLayer? = nil, pin1: MarkLayer? = nil,
                           reference: MarkLayer? = nil) {
        editParts("Change Marks", page: .pcb) { $0.setFootprintMarks(ids, outline: outline, pin1: pin1, reference: reference) }
    }

    /// Moves a footprint's reference so its text is centred at `point` (board
    /// mm), snapped to the trace grid. The reference is stored relative to the
    /// part (`BoardPlacement.referenceOffset`), so it follows the part. A
    /// locked footprint's reference can still be moved.
    func moveReference(_ id: UUID, toCentre point: CGPoint) {
        let grid = board.traceGrid
        editParts("Move Reference", page: .pcb) { $0.moveReference(id, toCentre: point, grid: grid) }
    }

    /// Puts a footprint's reference back on the body centre.
    func resetReference(_ id: UUID) {
        editParts("Reset Reference", page: .pcb) { $0.resetReference(id) }
    }

    // MARK: Switches

    /// Sets or clears a part's load file: an EEPROM's contents, or a
    /// simulated microcontroller's firmware (plan-62-H). Does nothing for
    /// other parts. The memory and processor windows call it only while the
    /// simulation is stopped.
    func setMemoryFile(_ id: UUID, _ file: MemoryFile?) {
        editParts("Change Load File") { $0.setMemoryFile(id, file) }
    }

    /// Whether switch `index` set to `on` would put a voltage out of range
    /// on a part; sets `editorMessage` when it would.
    func refusesSwitches(at index: Int, _ on: [Bool]) -> Bool {
        guard let library else { return false }
        var candidate = chips
        candidate[index].switchesOn = on
        guard let issue = VoltageCheck.firstNew(before: chips, wires, after: candidate, wires,
                                                buses: buses, labels: netLabels,
                                                known: currentVoltageIssues(library), library: library) else { return false }
        editorMessage = VoltageCheck.message(issue, chips: chips, edit: .turnOn)
        return true
    }

    /// Presses or releases a tactile switch (plan-27-J). Nil for any other
    /// chip, or when a press would close a contact the voltage check refuses
    /// (`editorMessage` says why); otherwise the new pressed state. Releasing
    /// never checks. Not undoable and does not mark the document changed:
    /// the pressed state is never saved.
    @discardableResult
    func setButton(_ id: UUID, pressed: Bool) -> Bool? {
        guard let index = parts.index(id), chips[index].part.isTactileSwitch,
              !chips[index].switchesOn.isEmpty else { return nil }
        var on = chips[index].switchesOn
        on[0] = pressed
        if pressed, !chips[index].part.isNormallyClosed, !chips[index].isPressed, refusesSwitches(at: index, on) {
            return nil
        }
        if pressed { editorMessage = nil }
        updateInputs { $0.setSwitches(at: index, on) }
        return pressed
    }

    /// Every switch's positions (DIP and tactile), by part, for
    /// `restoreSwitches` to put back.
    var switchPositions: [UUID: [Bool]] {
        Dictionary(uniqueKeysWithValues: chips.lazy.filter { !$0.switchesOn.isEmpty }.map { ($0.id, $0.switchesOn) })
    }

    /// Puts switches back to `saved` (from `switchPositions`), for example
    /// when play mode ends: flips made while the simulation ran are input to
    /// that run, not edits. Parts not in `saved`, or whose position count
    /// changed, are left alone.
    func restoreSwitches(_ saved: [UUID: [Bool]]) {
        updateInputs { $0.restoreSwitches(saved) }
    }

    /// Releases every pressed tactile switch (play mode ended).
    func releaseAllButtons() {
        updateInputs { $0.releaseAllButtons() }
    }

    /// Flips a switch position (1-based) between ON and OFF. Returns the new
    /// state, or nil when the part is not a switch or has no such position,
    /// or when turning it ON would put a voltage out of range on a part (the
    /// switch stays OFF and `editorMessage` says why).
    @discardableResult
    func toggleSwitch(_ id: UUID, position: Int) -> Bool? {
        return edit(.schematic, "Flip Switch") { scope -> Bool? in
            guard let toggled = parts.toggled(id, position: position) else { return nil }
            if toggled.on[position - 1], refusesSwitches(at: toggled.index, toggled.on) { return nil }
            editorMessage = nil
            updateParts(scope) { $0.setSwitches(at: toggled.index, toggled.on) }
            return toggled.on[position - 1]
        }
    }

    /// Moves a slide switch's knob to position `state` (1-based): its contacts
    /// become exactly those the position closes. Returns the position, or nil
    /// when the chip is not a slide switch, has no such position, or the move
    /// would close a contact the voltage check refuses (the knob stays and
    /// `editorMessage` says why). Outside play mode a recorded edit ("Move
    /// Switch"); in play mode an input, put back by `restoreSwitches`.
    @discardableResult
    func setSlideState(_ id: UUID, to state: Int) -> Int? {
        return edit(.schematic, "Move Switch") { scope -> Int? in
            guard let index = parts.index(id), let slide = chips[index].part.slideStates,
                  (1...slide.positions).contains(state) else { return nil }
            let closed = PlacedChip.contacts(mask: slide.masks[state - 1], count: chips[index].part.slideContacts)
            if closed.indices.contains(where: { closed[$0] && !chips[index].switchesOn[$0] }),
               refusesSwitches(at: index, closed) { return nil }
            editorMessage = nil
            updateParts(scope) { $0.setSlide(at: index, state: state, closed: closed) }
            return state
        }
    }
}
