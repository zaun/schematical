import CoreGraphics
import Foundation

// MARK: - Placed items

/// An EEPROM's load file: where it is, and the app-scoped security bookmark
/// that lets the sandboxed app read it after a restart.
nonisolated struct MemoryFile: Hashable, Sendable {
    var url: URL
    var bookmark: Data?
    var name: String { url.lastPathComponent }
}

/// A DIP or PLCC chip's socket (plan-51-H): `part` is a `PART_SOCKET` catalog
/// part whose `fits` hold the chip's footprint, and `mode` says whether the
/// chip is soldered into it or left out. A chip with no `ChipSocket` is a plain IC.
nonisolated struct ChipSocket: Hashable, Sendable {
    /// The file's values are the raw values.
    enum Mode: String, Sendable {
        /// The chip sits in the socket: the Bill of Materials lists both.
        case withSocket
        /// The socket is on the board, the chip is not inserted.
        case socketOnly
    }

    var part: CatalogPart
    var mode: Mode

    /// Whether `part` is a socket that holds `chip`'s base footprint.
    static func fits(_ socket: CatalogPart, _ chip: CatalogPart) -> Bool {
        guard socket.kind == PART_SOCKET, chip.footprintChoices.isEmpty, let base = chip.footprint else { return false }
        return socket.fits.contains { $0.id == base.id }
    }
}

/// Where one unit of a part drawn as its units is (plan-67): its own
/// position, flip and turn.
nonisolated struct UnitPlacement: Hashable, Sendable {
    /// `PartUnit.key`.
    let key: String
    /// Top-left corner of the unit's body. Always on the grid.
    var origin: CGPoint
    var isFlipped = false
    /// 0, 90, 180 or 270; 0 unless the unit's symbol turns (a gate does, a
    /// block does not).
    var rotation = 0
}

/// One unit of a placed part.
nonisolated struct UnitRef: Hashable, Sendable {
    let chipID: UUID
    let key: String
}

nonisolated struct PlacedChip: Identifiable, Hashable {
    let id: UUID
    /// "U1", "R3": letters then a number. Changed through
    /// `SchematicDocument.renameChip`, which keeps references unique.
    var reference: String
    /// The library part, copied from the backend at launch.
    let part: CatalogPart
    private(set) var symbol: PartSymbol
    /// Set on a piece only (`piece`): which unit of the part it stands for
    /// ("A", "P"). Nil for a whole part, and for every chip a document holds.
    private(set) var unitKey: String?
    /// Top-left corner of the body while the part is one block. Always on
    /// the grid.
    private var blockOrigin: CGPoint
    /// Where each unit is, in `part.units` order, when the part is drawn as
    /// its units (plan-67); empty = one block. Changed through `split`,
    /// `join`, `setPlacements` and `updatePlacement`.
    private(set) var unitPlacements: [UnitPlacement] = []
    /// Flipped parts have their pins on the opposite sides. For an LED the
    /// whole symbol is also mirrored; see `DiodeSymbolView`.
    var isFlipped = false
    /// How far a part drawn as its own symbol is turned clockwise on the
    /// Schematic: 0, 90, 180 or 270 (`PartSymbol.canRotate`; 0 for every
    /// other part). The flip is applied first, then the turn.
    var rotation = 0
    /// For a DIP switch, whether each position is ON, position 1 first. Empty
    /// for every other part.
    var switchesOn: [Bool] = []
    /// Which way a click moves a slide switch's knob along its positions: 1
    /// toward the last, -1 toward the first. Not saved.
    var slideDirection = 1
    /// Where the footprint sits on the PCB. Set exactly when the part has a
    /// footprint (every part but VCC and GND).
    var board: BoardPlacement?
    /// For an EEPROM, the file loaded into it when the simulation starts.
    /// Nil for every other part, and for an EEPROM that starts erased.
    var memoryFile: MemoryFile?
    /// An LED's color, one of `part.colors`. Nil for every other part.
    var color: String?
    /// For a part with footprint choices, the id of the chosen footprint (one
    /// of `part.footprintChoices`). Nil for every other part.
    var footprintID: String?
    /// A header's gender, one of `part.genders`. Nil for every other part.
    var gender: String?
    /// The resistance in ohms (`ResistorValue.isValid`) of a pull resistor or
    /// `GENERIC-RESISTOR` (`CatalogPart.hasOhmsField`, plan-34-G). Nil for
    /// every other part, maker resistors included (their value is their part's).
    var ohms: Double?
    /// The generic capacitor's capacitance in farads (`CapacitorValue.isValid`,
    /// plan-33-H). Nil for every other part, real capacitors included (their
    /// value is their part's).
    var farads: Double?
    /// The socket the chip is in or on (plan-51-H); nil = a plain IC. Only a
    /// chip a socket fits can have one: with footprint choices, never.
    var socket: ChipSocket?
    /// Whether the Schematic draws the reference above the part; the
    /// sidebar's Display Reference. On unless turned off.
    var showsReference = true

    private var baseLayout: ChipLayout

    init(
        id: UUID = UUID(),
        reference: String,
        part: CatalogPart,
        symbol: PartSymbol,
        origin: CGPoint,
        isFlipped: Bool = false,
        rotation: Int = 0,
        switchesOn: [Bool] = [],
        board: BoardPlacement? = nil,
        memoryFile: MemoryFile? = nil,
        color: String? = nil,
        footprintID: String? = nil,
        gender: String? = nil,
        ohms: Double? = nil,
        farads: Double? = nil,
        socket: ChipSocket? = nil
    ) {
        self.id = id
        self.reference = reference
        self.part = part
        self.symbol = symbol
        self.baseLayout = ChipLayout(part: part, symbol: symbol)
        self.blockOrigin = origin
        self.isFlipped = isFlipped
        self.rotation = symbol.canRotate ? PlacedChip.normalized(rotation) : 0
        self.board = part.footprint == nil ? nil : board
        self.memoryFile = part.hasLoadFile ? memoryFile : nil
        // A nil or unknown option is the part's default; parts without the
        // option have none.
        if part.kind == PART_LED, !part.colors.isEmpty {
            self.color = color.flatMap { part.colors.contains($0) ? $0 : nil } ?? part.colors[0]
        } else {
            self.color = nil
        }
        if part.kind == PART_HEADER, !part.genders.isEmpty {
            self.gender = gender.flatMap { part.genders.contains($0) ? $0 : nil } ?? part.genders[0]
        } else {
            self.gender = nil
        }
        if !part.footprintChoices.isEmpty, let base = part.footprint {
            self.footprintID = footprintID.flatMap { id in
                part.footprintChoices.contains { $0.footprint.id == id } ? id : nil
            } ?? base.id
        } else {
            self.footprintID = nil
        }
        if part.hasOhmsField {
            self.ohms = ohms.flatMap { ResistorValue.isValid($0) ? $0 : nil } ?? ResistorValue.defaultOhms
        } else {
            self.ohms = nil
        }
        if part.isGenericCapacitor {
            self.farads = farads.flatMap { CapacitorValue.isValid($0) ? $0 : nil } ?? CapacitorValue.defaultFarads
        } else {
            self.farads = nil
        }
        // A socket is kept only when it is one that holds this chip's footprint.
        self.socket = socket.flatMap { ChipSocket.fits($0.part, part) ? $0 : nil }
        // A switch has one state per position, OFF unless given; others none.
        // A tactile switch has one, "pressed", and always starts released
        // (the file never stores it; plan-27-J). A slide switch has one per
        // contact, and always is in one of its knob positions: the one the
        // given contacts are, else its rest position (plan-58-G).
        var positions = 0
        if case let .dipSwitch(count, _, _) = part.details {
            positions = count
        }
        if let slide = part.slideStates {
            let contacts = part.slideContacts
            let given = (0..<contacts).map { $0 < switchesOn.count && switchesOn[$0] }
            let mask = given.enumerated().reduce(UInt32(0)) { $1.element ? $0 | (1 << UInt32($1.offset)) : $0 }
            let state = switchesOn.count == contacts ? slide.masks.firstIndex(of: mask) : nil
            self.switchesOn = Self.contacts(mask: slide.masks[state ?? slide.rest - 1], count: contacts)
        } else if part.isTactileSwitch {
            self.switchesOn = [false]
        } else {
            self.switchesOn = (0..<positions).map { $0 < switchesOn.count && switchesOn[$0] }
        }
    }

    /// The orderable part number, as saved in a file.
    var partNumber: String { part.name }

    /// What tells drawn things apart: a part's id, and a piece's unit.
    struct DrawID: Hashable {
        let id: UUID
        let unitKey: String?
    }

    var drawID: DrawID { DrawID(id: id, unitKey: unitKey) }

    /// The reference as the Schematic draws it: a piece adds its unit ("U1-A").
    var drawnReference: String {
        unitKey.map { "\(reference)-\($0)" } ?? reference
    }

    /// The unit a piece stands for; nil for a whole part.
    var unit: PartUnit? {
        guard let unitKey else { return nil }
        return part.units.first { $0.key == unitKey }
    }

    /// One unit of the part as its own chip value (plan-67): the same part
    /// and id, a layout of only that unit's pins, the gate symbol for a gate
    /// and a plain block otherwise. For drawing and geometry only: no
    /// document stores a piece. A turn is kept only where the symbol turns.
    func piece(_ unit: PartUnit, origin: CGPoint, isFlipped: Bool, rotation: Int) -> PlacedChip {
        var piece = self
        let pins = part.pins.filter { unit.pins.contains($0.index) && !$0.isHidden }
        switch unit.shape {
        case let .gate(shape):
            piece.symbol = .gate
            piece.baseLayout = ChipLayout(gate: shape, pins: pins)
        case .block:
            piece.symbol = .block
            piece.baseLayout = ChipLayout(pins: pins)
        }
        piece.unitKey = unit.key
        piece.unitPlacements = []
        piece.blockOrigin = origin
        piece.isFlipped = isFlipped
        piece.rotation = piece.symbol.canRotate ? Self.normalized(rotation) : 0
        return piece
    }

    /// A slide switch's contacts closed by the knob position whose mask is `mask`,
    /// contact 1 first.
    static func contacts(mask: UInt32, count: Int) -> [Bool] {
        (0..<count).map { mask & (1 << UInt32($0)) != 0 }
    }

    /// A slide switch's knob position (1-based): the one whose closed contacts
    /// are exactly `switchesOn`. Nil for any other part.
    var slidePosition: Int? {
        guard let slide = part.slideStates else { return nil }
        let mask = switchesOn.enumerated().reduce(UInt32(0)) { $1.element ? $0 | (1 << UInt32($1.offset)) : $0 }
        return slide.masks.firstIndex(of: mask).map { $0 + 1 }
    }

    /// The knob position a click moves a slide switch to: the next one in the
    /// order 1 → 2 → … → N → N-1 → … → 1, by `slideDirection`. Nil for any
    /// other part.
    var nextSlidePosition: Int? {
        guard let slide = part.slideStates, let position = slidePosition else { return nil }
        let next = position + slideDirection
        return (1...slide.positions).contains(next) ? next : position - slideDirection
    }

    /// The same chip on another part (a header's new size): id, reference,
    /// symbol, origin, flip, board placement, gender, memory file and
    /// Display Reference are kept. The footprint is the new part's choice with the same package as
    /// the current one (`Straight` stays `Straight`), else its default.
    /// `symbol` replaces the chip's symbol (a part updated in place may draw
    /// differently, plan-53-D); a flip is kept only where the symbol can flip.
    func resized(to newPart: CatalogPart, symbol newSymbol: PartSymbol? = nil) -> PlacedChip {
        let package = footprintChoice?.package
        let footprint = newPart.footprintChoices.first { $0.package == package }?.footprint.id
        let symbol = newSymbol ?? self.symbol
        var chip = PlacedChip(id: id, reference: reference, part: newPart, symbol: symbol, origin: origin,
                              isFlipped: isFlipped && symbol.canFlip, rotation: rotation, switchesOn: switchesOn, board: board,
                              memoryFile: memoryFile, color: color, footprintID: footprint, gender: gender,
                              ohms: ohms, farads: farads, socket: socket)
        chip.showsReference = showsReference
        // Still drawn as its units when the new part has the same ones.
        if isSplit { chip.setPlacements(unitPlacements) }
        return chip
    }

    /// The chosen footprint choice, for a part with footprint choices.
    var footprintChoice: CatalogPart.FootprintChoice? {
        guard let footprintID else { return nil }
        return part.footprintChoices.first { $0.footprint.id == footprintID }
    }

    /// A tactile switch is held down (`switchesOn[0]`); false for others.
    var isPressed: Bool {
        part.isTactileSwitch && switchesOn.first == true
    }

    /// A tactile switch's contact is closed: pressed on a Normal Open part,
    /// released on a Normal Close one.
    var tactileContactClosed: Bool {
        isPressed != part.isNormallyClosed
    }

    /// A tactile switch's button, in canvas coordinates.
    var buttonRect: CGRect? {
        guard symbol == .tactileSwitch else { return nil }
        return ChipLayout.tactileButtonRect(bodySize: layout.size).offsetBy(dx: origin.x, dy: origin.y)
    }

    /// Whether `point` is on a tactile switch's button (with the DIP lever's
    /// 4 x 3 pt slop).
    func buttonHit(at point: CGPoint) -> Bool {
        buttonRect?.insetBy(dx: -4, dy: -3).contains(point) ?? false
    }

    /// The switch position (1-based) whose lever is under `point`, if any.
    func switchPosition(at point: CGPoint) -> Int? {
        guard symbol == .dipSwitch else { return nil }
        let local = CGPoint(x: point.x - origin.x, y: point.y - origin.y)
        return switchesOn.indices.map { $0 + 1 }.first { position in
            ChipLayout.switchLeverRect(position: position, bodyWidth: layout.size.width)
                .insetBy(dx: -4, dy: -3)
                .contains(local)
        }
    }

    var layout: ChipLayout {
        if isSplit {
            let pieces = pieces
            return ChipLayout(size: Self.union(pieces.map(\.bodyRect)).size, slots: pieces.flatMap { $0.layout.slots })
        }
        let flipped = isFlipped ? baseLayout.flipped() : baseLayout
        return rotation == 0 ? flipped : flipped.rotated(by: rotation)
    }

    // MARK: Units (plan-67)

    /// Drawn as its units rather than one block.
    var isSplit: Bool { !unitPlacements.isEmpty }

    /// Top-left corner of the body, always on the grid. For a part drawn as
    /// its units, the corner of the box around them: setting it moves every
    /// unit together.
    var origin: CGPoint {
        get { isSplit ? Self.union(pieces.map(\.bodyRect)).origin : blockOrigin }
        set {
            guard isSplit else {
                blockOrigin = newValue
                return
            }
            let old = origin
            let dx = newValue.x - old.x, dy = newValue.y - old.y
            for index in unitPlacements.indices {
                unitPlacements[index].origin.x += dx
                unitPlacements[index].origin.y += dy
            }
        }
    }

    private static func union(_ rects: [CGRect]) -> CGRect {
        guard let first = rects.first else { return .zero }
        return rects.dropFirst().reduce(first) { $0.union($1) }
    }

    /// What the Schematic draws: the part itself, or one piece per unit.
    var pieces: [PlacedChip] {
        guard isSplit else { return [self] }
        return unitPlacements.compactMap { placement in
            guard let unit = part.units.first(where: { $0.key == placement.key }) else { return nil }
            return piece(unit, origin: placement.origin, isFlipped: placement.isFlipped, rotation: placement.rotation)
        }
    }

    /// The piece a unit is drawn as; nil when the part is not split or has
    /// no such unit.
    func piece(forUnit key: String) -> PlacedChip? {
        pieces.first { $0.unitKey == key }
    }

    /// The key of the unit that draws a pin (`ChipPin.index`).
    func unitKey(ofPin index: Int) -> String? {
        part.units.first { $0.pins.contains(index) }?.key
    }

    /// The last-drawn piece under a point (body and pin stubs).
    func piece(at point: CGPoint) -> PlacedChip? {
        pieces.last { $0.hitRect.contains(point) }
    }

    /// The pieces a box touches.
    func pieces(in rect: CGRect) -> [PlacedChip] {
        pieces.filter { $0.hitRect.intersects(rect) }
    }

    /// Draws the part as its units, in a column from where the block was:
    /// each unit a grid step below the one before. Nothing for a part with
    /// no units or one already split.
    mutating func split() {
        guard !isSplit, !part.units.isEmpty else { return }
        var placements: [UnitPlacement] = []
        var corner = SchematicGrid.snap(blockOrigin)
        for unit in part.units {
            placements.append(UnitPlacement(key: unit.key, origin: corner))
            let size = piece(unit, origin: corner, isFlipped: false, rotation: 0).layout.size
            corner.y = SchematicGrid.snap(corner.y + size.height + 2 * SchematicGrid.spacing)
        }
        unitPlacements = placements
    }

    /// Back to one block, at the corner of the box around the units.
    mutating func join() {
        guard isSplit else { return }
        let corner = SchematicGrid.snap(origin)
        unitPlacements = []
        blockOrigin = corner
    }

    /// Takes a whole arrangement (a file's, a copy's): accepted only when it
    /// names every unit of the part exactly once. Kept in unit order; a turn
    /// is kept only where the unit's symbol turns. Returns whether it was
    /// accepted; a refused list changes nothing.
    @discardableResult
    mutating func setPlacements(_ placements: [UnitPlacement]) -> Bool {
        let keys = part.units.map(\.key)
        guard !keys.isEmpty, placements.count == keys.count, Set(placements.map(\.key)) == Set(keys) else { return false }
        unitPlacements = part.units.compactMap { unit in
            guard var placement = placements.first(where: { $0.key == unit.key }) else { return nil }
            let turns = if case .gate = unit.shape { true } else { false }
            placement.rotation = turns ? Self.normalized(placement.rotation) : 0
            return placement
        }
        return true
    }

    /// Changes one unit's placement; nothing when the part is not split or
    /// has no such unit. A turn is kept only where the unit's symbol turns.
    mutating func updatePlacement(_ key: String, _ body: (inout UnitPlacement) -> Void) {
        guard let index = unitPlacements.firstIndex(where: { $0.key == key }) else { return }
        var placement = unitPlacements[index]
        body(&placement)
        let turns = if case .gate? = part.units.first(where: { $0.key == key })?.shape { true } else { false }
        placement.rotation = turns ? Self.normalized(placement.rotation) : 0
        unitPlacements[index] = placement
    }

    /// 0, 90, 180 or 270 for any whole number of degrees.
    static func normalized(_ degrees: Int) -> Int {
        ((degrees % 360) + 360) % 360 / 90 * 90
    }

    /// Whether the pins are on the top and bottom: a supply, or a part
    /// turned a quarter turn. Such a part's view has room for the stubs above
    /// and below its body.
    var hasVerticalPins: Bool {
        symbol.hasVerticalPins || rotation == 90 || rotation == 270
            || (symbol == .crystal && baseLayout.slots.contains { $0.side == .bottom })
    }

    /// The size of the part before it is flipped or turned.
    var unturnedSize: CGSize { baseLayout.size }

    /// Where a pin's wire leaves the body: the end of its stub.
    func tip(of slot: ChipLayout.PinSlot) -> CGPoint {
        if isSplit {
            // The unit that draws the pin says where it is.
            for piece in pieces {
                if let own = piece.layout.slots.first(where: { $0.pin.index == slot.pin.index }) {
                    return piece.tip(of: own)
                }
            }
        }
        let stub = ChipLayout.stubLength
        let size = layout.size
        switch slot.side {
        case .left:
            return CGPoint(x: origin.x - stub, y: origin.y + slot.centerY)
        case .right:
            return CGPoint(x: origin.x + size.width + stub, y: origin.y + slot.centerY)
        case .top:
            return CGPoint(x: origin.x + slot.centerX, y: origin.y - stub)
        case .bottom:
            return CGPoint(x: origin.x + slot.centerX, y: origin.y + size.height + stub)
        }
    }

    /// Top-left corner of the view. Supplies also need room above and below
    /// the body, so their view starts one stub higher.
    var viewOrigin: CGPoint {
        CGPoint(
            x: origin.x - sideMargin,
            y: origin.y - (hasVerticalPins ? ChipLayout.stubLength : 0)
        )
    }

    var bodyRect: CGRect {
        CGRect(origin: origin, size: layout.size)
    }

    /// The body plus its pin stubs, used for hit testing.
    /// The body plus its pin stubs, used for hit testing and hover. Vertical
    /// supplies also have stubs above and below, so those are included too.
    var hitRect: CGRect {
        if isSplit { return Self.union(pieces.map(\.hitRect)) }
        let stub = ChipLayout.stubLength
        let verticalStub = hasVerticalPins ? stub : 0
        return bodyRect.insetBy(dx: -stub, dy: -verticalStub)
    }
}
