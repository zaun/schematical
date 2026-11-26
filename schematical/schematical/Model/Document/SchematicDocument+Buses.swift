import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Buses

    func bus(_ id: UUID) -> Bus? {
        sheet.busNetwork.bus(id)
    }

    /// Whether no line end of one bus touches a line of another.
    static func busesAreApart(_ buses: [Bus]) -> Bool {
        BusNetwork.areApart(buses)
    }

    /// What a Bus tool drag from `start` to `end` would make (both snapped).
    /// Changes nothing; the canvas previews it.
    func busDraft(from start: CGPoint, to end: CGPoint) -> Result<BusDraft, BusJoinRefusal> {
        BusJoin.join(buses, from: SchematicGrid.snap(start), to: SchematicGrid.snap(end))
    }

    /// What the Bus tool would make of a drawn route: the first of
    /// `candidates` (grid paths, as `RouteDraft.busCandidates` gives) the
    /// bus rules accept.
    func busDraft(candidates: [[CGPoint]]) -> Result<BusDraft, BusJoinRefusal> {
        BusJoin.join(buses, candidates: candidates)
    }

    /// The Bus tool: draws a line or an L from `start` to `end`, as a new bus
    /// or joined to the bus it starts or ends on (a T, an L, or a longer
    /// line). Selects that bus and returns its id. A refused drag changes
    /// nothing, sets `editorMessage`, and returns nil.
    @discardableResult
    func addBus(from start: CGPoint, to end: CGPoint) -> UUID? {
        return edit(.schematic, "Add Bus") { scope -> UUID? in
            applyBusDraft(busDraft(from: start, to: end), scope)
        }
    }

    /// The Bus tool along a drawn route: `addBus(from:to:)` for the first of
    /// `candidates` the bus rules accept.
    @discardableResult
    func addBus(candidates: [[CGPoint]]) -> UUID? {
        return edit(.schematic, "Add Bus") { scope -> UUID? in
            applyBusDraft(busDraft(candidates: candidates), scope)
        }
    }

    func applyBusDraft(_ result: Result<BusDraft, BusJoinRefusal>, _ scope: EditScope) -> UUID? {
        switch result {
        case let .failure(refusal):
            if let message = refusal.message { editorMessage = message }
            return nil
        case let .success(draft):
            if let message = sheet.updateBuses(scope, { $0.place(draft) }) {
                editorMessage = message
                return nil
            }
            if let gone = draft.absorbedIndex {
                absorb(buses[gone], wireMap: draft.wireMap, scope)
                sheet.updateBuses(scope) { $0.buses.remove(at: gone) }
            }
            editorMessage = nil
            selectedBusID = draft.bus.id
            return draft.bus.id
        }
    }

    /// After the Bus tool joined `gone` into another bus: wires on `gone`'s port
    /// pins move to the pin for the wire it became, and are re-fitted (the
    /// pins may have moved along the port).
    func absorb(_ gone: Bus, wireMap: [UUID: UUID], _ scope: EditScope) {
        let portIDs = Set(gone.ports.map(\.id))
        sheet.updateWiring(scope) { wiring in
            wiring.remapEnds { end in
                guard case let .busPin(ref) = end, portIDs.contains(ref.portID),
                      let wireID = wireMap[ref.wireID] else { return end }
                return .busPin(BusPinRef(portID: ref.portID, wireID: wireID))
            }
        }
        for portID in portIDs {
            settle(scope) { BusNetwork.endsOn($0, port: portID) }
        }
    }

    /// Sets how many wires a bus carries, clamped to 2–999. New wires get
    /// fresh ids and default names; removing drops wires from the end. It
    /// can't drop a wire a port uses: that is refused with a message.
    func setBusWireCount(_ id: UUID, _ count: Int) -> String? {
        return edit(.schematic, "Change Bus Width") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setWireCount(id, count, edge: edge) }
        }
    }

    /// Renames one wire of a bus. Another wire of the bus may have the same
    /// name (the wire list warns); each wire is still its own net. Returns
    /// why when the name is refused, and changes nothing then.
    func renameBusWire(bus id: UUID, wire wireID: UUID, to name: String) -> String? {
        return edit(.schematic, "Rename Bus Wire") { scope -> String? in
            sheet.updateBuses(scope) { $0.renameWire(bus: id, wire: wireID, to: name) }
        }
    }

    /// Sets a bus's wires to `names`, in order (the Bus panel's Import).
    /// Wires keep their places and ids (so ports and the wires on their pins
    /// stay) and are renamed; wires are added or dropped at the end. One
    /// undo step. Refused, with why, and nothing changes, when the bus type
    /// sets the wires, a name is invalid, the count is out of range, a port
    /// uses a wire that would go, or the card edge uses the bus and the
    /// count would change.
    func setBusWireNames(_ id: UUID, _ names: [String]) -> String? {
        return edit(.schematic, "Import Bus Wires") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setWireNames(id, names, edge: edge) }
        }
    }

    /// Moves one wire of a bus to place `to` (0-based, in the order after
    /// the move), keeping its id and name, so its net and the wires on its
    /// port pins go with it. Linked buses move the wire at the same place
    /// the same way, so wire i of each stays one net. Ports show their pins
    /// in bus order, so their wires are re-fitted. Refused, with why, while
    /// the bus type sets the wires.
    func moveBusWire(bus id: UUID, wire wireID: UUID, to place: Int) -> String? {
        return edit(.schematic, "Reorder Bus Wires") { scope -> String? in
            let moved = sheet.updateBuses(scope) { $0.moveWire(bus: id, wire: wireID, to: place) }
            for port in moved.ports {
                settle(scope) { BusNetwork.endsOn($0, port: port) }
            }
            return moved.message
        }
    }

    /// The bus the board's card edge uses, while the board is a card and
    /// the bus has as many wires as the card has pins.
    var edgeBusID: UUID? {
        guard board.type.isCard, let id = board.edgeBus, let bus = bus(id),
              bus.wires.count == board.pinCount else { return nil }
        return id
    }

    /// The buses a card edge can use: those with as many wires as the card
    /// has pins. Only the count matters, not the names.
    var edgeBusChoices: [Bus] {
        guard board.type.isCard else { return [] }
        return buses.filter { $0.wires.count == board.pinCount }
    }

    /// Chooses the bus the card edge uses, or none. A bus with the wrong
    /// wire count is ignored.
    func setEdgeBus(_ id: UUID?) {
        if let id, !edgeBusChoices.contains(where: { $0.id == id }) { return }
        editBoard("Change Card Bus") { $0.edgeBus = id }
    }

    /// The bus that renaming bus `id` to `name` would link it with: another
    /// bus of that name (ignoring case) not linked with it already. Nil when
    /// the rename links nothing new.
    func busLinkTarget(_ id: UUID, named name: String) -> Bus? {
        sheet.busNetwork.linkTarget(id, named: name)
    }

    /// Renames a bus. Refused, with why, when the name is empty. A bus
    /// linked with others (same name) leaves them; a new case for the same
    /// name renames them all.
    ///
    /// Another bus's name links the two (`busLinkTarget`), and only with
    /// `link`: this bus takes that one's type and wires (its wires keep
    /// their places and ids; they are renamed, added or dropped at the end)
    /// and its exact name. Without `link` that is refused. Refused too when
    /// a port uses a wire the other bus doesn't have, or this bus is the
    /// card edge and the wire count would change.
    func setBusName(_ id: UUID, _ name: String, link: Bool = false) -> String? {
        return edit(.schematic, "Rename Bus") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setName(id, name, link: link, edge: edge) }
        }
    }

    /// Sets what a bus carries. A card type makes the wires its pins, in
    /// order: existing wires keep their places (and ids, so ports and their
    /// wire ends stay) and are renamed, and wires are added or dropped at
    /// the end. Custom keeps the wires as they are, to edit freely.
    /// Refused, with why, when a port uses a wire the card has no pin for,
    /// or the card has fewer than 2 pins.
    func setBusCard(_ id: UUID, _ card: BusCard) -> String? {
        return edit(.schematic, "Change Bus Type") { scope -> String? in
            let edge = edgeBusID
            return sheet.updateBuses(scope) { $0.setCard(id, card, edge: edge) }
        }
    }

    // MARK: Bus ports

    /// The bus and index of a port.
    func portLocation(_ portID: UUID) -> (bus: Int, port: Int)? {
        sheet.busNetwork.portLocation(portID)
    }

    /// A port and the bus it is on.
    func port(_ portID: UUID) -> (bus: Bus, port: BusPort)? {
        sheet.busNetwork.port(portID)
    }

    /// The Bus Port tool: adds an empty port where `point` is on a bus line,
    /// and selects it. Refused (with `editorMessage` set, and returned) on or
    /// near a line end or a T, or when one slot doesn't fit there.
    @discardableResult
    func addPort(at point: CGPoint, tolerance: CGFloat = 6) -> String? {
        return edit(.schematic, "Add Port") { scope -> String? in
            switch sheet.updateBuses(scope, { $0.addPort(at: point, tolerance: tolerance) }) {
            case nil:
                return nil
            case let .failure(refusal)?:
                editorMessage = refusal.message
                return editorMessage
            case let .success(placed)?:
                editorMessage = nil
                selectedPortID = placed.id
                return nil
            }
        }
    }

    /// Sets which bus wires are pins on a port (ticks, Select All, Deselect
    /// All). More pins than fit is refused with a message and changes
    /// nothing; a port that grows may move left (up) to fit. The wires on
    /// unticked pins stay, with loose ends.
    func setPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        return edit(.schematic, "Change Port Pins") { scope -> String? in
            switch sheet.busNetwork.portWiresFit(portID, ids) {
            case nil:
                return nil
            case let .failure(refusal)?:
                return refusal.message
            case let .success(fit)?:
                if !fit.removing.isEmpty {
                    loosen(scope) { BusNetwork.endsOn($0, port: portID, wireIDs: fit.removing) }
                    pruneJunctions(scope)
                }
                sheet.updateBuses(scope) { $0.setPortWires(portID, ids, start: fit.start) }
                // Pins after an added one shift by a pitch; re-fit their wires.
                settle(scope) { BusNetwork.endsOn($0, port: portID) }
                return nil
            }
        }
    }

    /// Sets the side a port's pins come out of, if it suits the line. Ports
    /// on opposite sides may share a place, so a port on the new side that is
    /// in the way refuses the switch with a message.
    @discardableResult
    func setPortSide(_ portID: UUID, _ side: ChipLayout.Side) -> String? {
        return edit(.schematic, "Change Port Side") { scope -> String? in
            switch sheet.updateBuses(scope, { $0.setPortSide(portID, side) }) {
            case .unchanged:
                return nil
            case let .refused(message):
                return message
            case .changed:
                settle(scope) { BusNetwork.endsOn($0, port: portID) }
                return nil
            }
        }
    }

    /// Moves a port along its line to `start`. Stores it only if the fit
    /// rule keeps it unchanged (on its line, clear of the other ports and the
    /// Ts). Returns whether it did.
    @discardableResult
    func slidePort(_ portID: UUID, to start: CGFloat) -> Bool {
        return edit(.schematic, "Move Port") { scope -> Bool in
            sheet.updateBuses(scope) { $0.slidePort(portID, to: start) }
        }
    }

    /// One frame of a port slide: `base` (the start at the press) plus
    /// `along` (the drag along the line), snapped to the grid and clamped to
    /// where the port may go. It never leaves its line or passes a line
    /// end, another port or a T.
    func slidePort(_ portID: UUID, from base: CGFloat, by along: CGFloat) {
        recordEdit(.schematic, "Move Port") {
            guard let (bus, port) = port(portID), let range = bus.slideRange(of: port) else { return }
            let wanted = SchematicGrid.snap(base + along)
            slidePort(portID, to: min(max(wanted, range.lowerBound), range.upperBound))
        }
    }

    /// The port whose body is under `point` (topmost first).
    func portHit(at point: CGPoint, tolerance: CGFloat = 0) -> UUID? {
        sheet.busNetwork.portHit(at: point, tolerance: tolerance)
    }

    /// The one way a drag changes a bus. Stores `lines` only if the bus stays
    /// valid, keeps the same joints (points may move), keeps every line's
    /// direction, stays apart from other buses, and every port still fits
    /// where it is (its line long enough, no T slid into it). Ports ride
    /// along with their line. Returns whether it stored the lines.
    @discardableResult
    func setBusLines(_ id: UUID, _ lines: [BusLine]) -> Bool {
        return edit(.schematic, "Move Bus") { scope -> Bool in
            sheet.updateBuses(scope) { $0.setLines(id, lines) }
        }
    }

    /// One frame of a bus drag. `step(offset)` gives the lines for a drag of
    /// `offset` from the press (against the press-time bus). The drag walks
    /// from `done` (the last offset stored) toward `target` one grid step at
    /// a time, storing each step through `setBusLines`, and stops at the
    /// first refused one. So a drag never jumps over something it may not
    /// pass, such as a T end over a port. Returns the offset reached.
    @discardableResult
    func dragBus(_ id: UUID, from done: CGSize, to target: CGSize, step: (CGSize) -> [BusLine]?) -> CGSize {
        return recordEdit(.schematic, "Move Bus") { () -> CGSize in
            let dx = target.width - done.width
            let dy = target.height - done.height
            let count = Int((max(abs(dx), abs(dy)) / SchematicGrid.spacing).rounded(.up))
            var reached = done
            guard count > 0 else { return done }
            for k in 1...count {
                let t = CGFloat(k) / CGFloat(count)
                let offset = CGSize(width: SchematicGrid.snap(done.width + dx * t),
                                    height: SchematicGrid.snap(done.height + dy * t))
                guard offset != reached else { continue }
                guard let lines = step(offset), setBusLines(id, lines) else { break }
                reached = offset
            }
            return reached
        }
    }

    /// The bus line under `point`, if any (topmost bus first).
    func busHit(at point: CGPoint, tolerance: CGFloat = 6) -> (bus: UUID, line: UUID)? {
        sheet.busNetwork.busHit(at: point, tolerance: tolerance)
    }

    /// The bus line end under `point`, if any. A corner is two ends at one
    /// point; either is returned, and the drag moves both lines.
    func busEndHit(at point: CGPoint, tolerance: CGFloat = 6) -> (bus: UUID, line: UUID, end: BusEnd)? {
        sheet.busNetwork.busEndHit(at: point, tolerance: tolerance)
    }

    // MARK: Removing ports and bus lines

    /// A removal waiting for the user to confirm: it would take more than the
    /// thing clicked (wires on port pins, or a line's ports).
    struct PendingRemoval: Identifiable {
        let id: UUID
        let message: String
        let action: Action

        init(message: String, action: Action) {
            (id, self.message, self.action) = (UUID(), message, action)
        }

        enum Action {
            case port(UUID)
            case busLine(bus: UUID, line: UUID)
            case untick(port: UUID, keeping: Set<UUID>)
            /// A selection with a part drawn as its units (plan-67): the
            /// items, and what `deleteSchematicSelection` was asked.
            case parts(items: Set<SchematicItem>, name: String, includingTitleBlock: Bool)
        }
    }

    func confirmPendingRemoval() {
        if case let .parts(items, name, includingTitleBlock)? = pendingRemoval?.action {
            pendingRemoval = nil
            if items != schematicSelection.items { schematicSelection.set(items) }
            deleteSchematicSelection(named: name, includingTitleBlock: includingTitleBlock, confirmed: true)
            return
        }
        edit(.schematic, "Delete") { scope in
            guard let pending = pendingRemoval else { return }
            pendingRemoval = nil
            switch pending.action {
            case let .port(id): removePort(id, scope)
            case let .busLine(bus, line): removeBusLine(bus: bus, line: line, scope)
            case let .untick(port, keeping): _ = setPortWires(port, keeping)
            case .parts: break
            }
        }
    }

    /// What the confirm asks before parts drawn as several symbols go: a
    /// click on one gate would otherwise take the whole IC unannounced.
    /// `split` are those parts, of `count` selected parts.
    static func removalQuestion(split: [PlacedChip], of count: Int) -> String {
        if count == 1, let chip = split.first {
            return "Delete \(chip.reference) and all \(chip.unitPlacements.count) of its symbols?"
        }
        let names = split.map(\.reference).sorted { $0.localizedStandardCompare($1) == .orderedAscending }
        let shown = names.prefix(3)
        var list = shown.dropLast().joined(separator: ", ")
        if let last = shown.last { list = list.isEmpty ? last : "\(list) and \(last)" }
        if names.count > shown.count { list = shown.joined(separator: ", ") + " and \(names.count - shown.count) more" }
        let verb = names.count == 1 ? "is" : "are"
        return "Delete the \(count) selected parts? \(list) \(verb) drawn as several symbols; all of them go."
    }

    func cancelPendingRemoval() {
        pendingRemoval = nil
    }

    /// How many wires end on the pins of a port that unticking `ids` (or,
    /// with nil, deleting the port) would remove.
    func wiresOnPins(port portID: UUID, removing ids: Set<UUID>? = nil) -> Int {
        wires.filter {
            BusNetwork.endsOn($0.start, port: portID, wireIDs: ids) || BusNetwork.endsOn($0.end, port: portID, wireIDs: ids)
        }.count
    }

    /// The port panel's ticks: like `setPortWires`, but unticking a pin that
    /// has wires asks first (through `pendingRemoval`) and returns nil.
    func requestPortWires(_ portID: UUID, _ ids: Set<UUID>) -> String? {
        return recordEdit(.schematic, "Change Port Pins") { () -> String? in
            guard let (_, port) = port(portID) else { return nil }
            let removing = port.wireIDs.subtracting(ids)
            let count = wiresOnPins(port: portID, removing: removing)
            if count > 0 {
                pendingRemoval = PendingRemoval(
                    message: "Remove \(Self.counted(removing.count, "pin"))? The \(Self.counted(count, "wire")) on them stay, with loose ends.",
                    action: .untick(port: portID, keeping: ids))
                return nil
            }
            return setPortWires(portID, ids)
        }
    }

    /// Removes a port. The wires on its pins stay, with loose ends.
    func removePort(_ portID: UUID, _ scope: EditScope) {
        guard portLocation(portID) != nil else { return }
        loosen(scope) { BusNetwork.endsOn($0, port: portID) }
        sheet.updateBuses(scope) { $0.removePort(portID) }
        pruneJunctions(scope)
        if selectedPortID == portID { selectedPortID = nil }
    }

    /// Removes one bus line, with its ports; the wires on their pins stay,
    /// with loose ends (`BusNetwork.removeLine`): a bus with no lines left
    /// goes, and the pieces a split leaves become buses of their own, the
    /// wire ends on their ports moving to the new wire ids. The card edge's
    /// bus keeps its last line.
    func removeBusLine(bus id: UUID, line lineID: UUID, _ scope: EditScope) {
        guard let bus = bus(id) else { return }
        if bus.id == edgeBusID, bus.lines.count == 1 {
            editorMessage = BusNetwork.edgeBusUndeletable(bus)
            return
        }
        // Loosen first, while the removed ports' pin tips still resolve.
        for port in bus.ports where port.line == lineID {
            loosen(scope) { BusNetwork.endsOn($0, port: port.id) }
            if selectedPortID == port.id { selectedPortID = nil }
        }
        defer { pruneJunctions(scope) }
        guard let remaps = sheet.updateBuses(scope, { $0.removeLine(bus: id, line: lineID) }) else {
            if selectedBusID == id { selectedBusID = nil }
            return
        }
        sheet.updateWiring(scope) { wiring in
            for remap in remaps {
                wiring.remapEnds { end in
                    guard case let .busPin(ref) = end, remap.ports.contains(ref.portID),
                          let wireID = remap.wires[ref.wireID] else { return end }
                    return .busPin(BusPinRef(portID: ref.portID, wireID: wireID))
                }
            }
        }
    }

    /// "1 wire", "3 wires".
    static func counted(_ n: Int, _ noun: String) -> String {
        "\(n) \(noun)\(n == 1 ? "" : "s")"
    }
}
