import CoreGraphics
import Foundation

extension AutomationTools {
    // MARK: Moving and changing

    /// Selects `items`, moves them by `delta` as the app's group drag does, and settles the wires.
    static func moveGroup(_ document: SchematicDocument, _ items: [SchematicItem], by delta: CGSize) {
        document.recordEdit(.schematic, "Move") {
            document.clearAllSelections()
            for item in items { document.toggleSelection(item) }
            let base = document.schematicGroupBase()
            document.moveSchematicGroup(base, by: delta)
            document.settleSchematicGroup(base)
            document.clearAllSelections()
        }
    }

    /// Where an item is, for "move to x,y".
    static func position(_ item: SchematicItem, _ context: AutomationContext) -> CGPoint? {
        let document = context.document
        switch item {
        case let .chip(id): return document.chips.first { $0.id == id }.map { context.centre(of: $0) }
        case let .label(id): return document.netLabels.first { $0.id == id }?.origin
        case let .note(id): return document.notes.first { $0.id == id }?.origin
        case let .junction(id): return document.junctions.first { $0.id == id }?.point
        case .titleBlock: return document.documentInfo.titleBlock
        case .wire, .bus: return nil
        }
    }

    static func moveItems(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let specs = try args.strings("items")
        guard !specs.isEmpty else { throw AutomationFailure("items is empty") }
        let items: [SchematicItem] = try specs.map {
            guard case let .schematic(found) = try item($0, context, forDelete: false) else {
                throw AutomationFailure("\($0) isn't on the schematic page (PCB items move with move_footprint, set_trace…)")
            }
            return found
        }
        let delta: CGSize
        if let x = args.optDouble("x"), let y = args.optDouble("y") {
            guard items.count == 1, let at = position(items[0], context) else {
                throw AutomationFailure("x and y move one part, label, note or junction; use dx and dy for the rest")
            }
            delta = CGSize(width: x - at.x, height: y - at.y)
        } else if let dx = args.optDouble("dx"), let dy = args.optDouble("dy") {
            delta = CGSize(width: dx, height: dy)
        } else {
            throw AutomationFailure("give dx and dy, or x and y for one item")
        }
        moveGroup(document, items, by: delta)
        let places = zip(specs, items).compactMap { spec, item in
            position(item, context).map { "\(spec) at (\(num($0.x)),\(num($0.y)))" }
        }
        return "moved \(specs.count) item\(specs.count == 1 ? "" : "s")" + (places.isEmpty ? "" : ": " + places.joined(separator: "; "))
    }

    static func setNetLabel(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("label")
        if !spec.contains(":") { spec = "label:" + spec }
        guard case let .schematic(.label(id)) = try item(spec, context, forDelete: false) else {
            throw AutomationFailure("\(spec) isn't a net label")
        }
        guard args.optString("name") != nil || args.optString("style") != nil || args.optBool("flip") == true else {
            throw AutomationFailure("nothing to change: give name, style or flip")
        }
        var problem: String?
        document.recordEdit(.schematic, "Change Net Label") {
            document.editorMessage = nil
            if let name = args.optString("name"), !document.setNetLabelName(id, name) {
                problem = document.editorMessage ?? "\(name) isn't a usable label name"
            }
            if problem == nil, let style = args.optString("style") {
                guard let kind = NetLabel.Style(rawValue: style) else { problem = "unknown style \(style)"; return }
                document.setNetLabelStyle(id, kind)
                if document.netLabels.first(where: { $0.id == id })?.style != kind {
                    problem = document.editorMessage ?? "the type change was refused"
                }
            }
            if problem == nil, args.optBool("flip") == true { document.flipNetLabel(id) }
        }
        document.endCoalescing()
        if let problem { throw AutomationFailure(problem) }
        guard let label = document.netLabels.first(where: { $0.id == id }) else { return "changed" }
        return "label \(context.short(id)) is \(label.name), \(label.style.rawValue)"
    }

    static func setNote(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("note")
        if !spec.contains(":") { spec = "note:" + spec }
        guard case let .schematic(.note(id)) = try item(spec, context, forDelete: false) else {
            throw AutomationFailure("\(spec) isn't a note")
        }
        let text = args.optString("text"), size = args.optInt("size"), framed = args.optBool("framed")
        guard text != nil || size != nil || framed != nil else { throw AutomationFailure("nothing to change: give text, size or framed") }
        if text?.isEmpty == true { throw AutomationFailure("a note's text can't be empty (delete it instead)") }
        document.recordEdit(.schematic, "Change Text") {
            if let text { document.setNoteText(id, text) }
            if let size { document.setNoteSize(id, size) }
            if let framed { document.setNoteFramed(id, framed) }
        }
        document.endCoalescing()
        guard let note = document.notes.first(where: { $0.id == id }) else { return "changed" }
        return "note \(context.short(id)): size \(note.size), \(note.framed ? "framed" : "no frame")"
    }

    static func setDocumentInfo(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let title = args.optString("title"), author = args.optString("author")
        let revision = args.optString("revision"), description = args.optString("description")
        let created = args.optString("created"), updated = args.optString("updated")
        let automatic = args.optBool("automatic_dates"), block = args.optBool("title_block")
        guard [title, author, revision, description, created, updated].contains(where: { $0 != nil })
                || automatic != nil || block != nil else {
            throw AutomationFailure("nothing to change: give title, author, revision, description, created, updated, "
                                    + "automatic_dates or title_block")
        }
        if created != nil || updated != nil, automatic ?? document.documentInfo.automaticDates {
            throw AutomationFailure("dates are set automatically on save; pass automatic_dates=false to set them")
        }
        func date(_ text: String?, _ key: String) throws -> Date?? {
            guard let text else { return .none }
            if text.isEmpty { return .some(nil) }
            guard let parsed = DocumentInfo.isoDate(text) else {
                throw AutomationFailure("\(key) \"\(text)\" isn't an ISO 8601 date, e.g. 2026-11-25T14:07:00Z")
            }
            return .some(parsed)
        }
        let newCreated = try date(created, "created"), newUpdated = try date(updated, "updated")
        document.recordEdit(.schematic, "Edit Document Info") {
            document.editDocumentInfo("Edit Document Info") { info in
                if let title { info.title = title }
                if let author { info.author = author }
                if let revision { info.revision = revision }
                if let description { info.description = description }
                if let automatic { info.automaticDates = automatic }
                if let newCreated { info.created = newCreated }
                if let newUpdated { info.updated = newUpdated }
            }
            if let block { document.setTitleBlockShown(block) }
        }
        document.endCoalescing()
        let info = document.documentInfo
        return "document info: title \"\(info.title)\", author \"\(info.author)\", revision \"\(info.revision)\", "
            + "dates \(info.automaticDates ? "automatic" : "manual"), title block "
            + (info.titleBlock.map { "at (\(num($0.x)),\(num($0.y)))" } ?? "off")
    }

    static func setWireRoute(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        var spec = try args.string("wire")
        if !spec.contains(":") { spec = "wire:" + spec }
        guard case let .schematic(.wire(id)) = try item(spec, context, forDelete: false),
              let wire = document.wires.first(where: { $0.id == id }),
              let a = document.endTip(wire.start), let b = document.endTip(wire.end) else {
            throw AutomationFailure("\(spec) isn't a wire with both ends placed")
        }
        func anchor(_ end: WireEnd) -> LineAnchor? {
            switch end {
            case let .pin(ref): .pin(ref)
            case let .busPin(ref): .busPin(ref)
            case let .label(id): .label(id)
            case .junction: nil
            }
        }
        let waypoints = try args.points("via").map { SchematicGrid.snap($0) }
        let path = OrthogonalRoute.normalize(orthogonal(
            [a] + waypoints + [b], start: anchor(wire.start).flatMap(document.anchorAxis) ?? .horizontal,
            end: anchor(wire.end).flatMap(document.anchorAxis)))
        document.setWirePath(id, path)
        let now = document.path(forWire: id)
        return "wire \(context.short(id)) now runs " + now.map { "(\(num($0.x)),\(num($0.y)))" }.joined(separator: " ")
    }

    static func movePart(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let (whole, key) = try context.unit(try args.string("reference"))
        if let key, let piece = whole.piece(forUnit: key) {
            // One unit of a part drawn as its units (plan-67).
            let size = piece.layout.size
            let target: CGPoint
            if let x = args.optDouble("x"), let y = args.optDouble("y") {
                target = CGPoint(x: x - size.width / 2, y: y - size.height / 2)
            } else if let dx = args.optDouble("dx"), let dy = args.optDouble("dy") {
                target = CGPoint(x: piece.origin.x + dx, y: piece.origin.y + dy)
            } else {
                throw AutomationFailure("give x and y (the new centre), or dx and dy")
            }
            document.moveUnit(UnitRef(chipID: whole.id, key: key), to: target)
            // The part and its unit are still there: moving keeps both.
            let moved = document.chips.first(where: { $0.id == whole.id })?.piece(forUnit: key) ?? piece
            let centre = context.centre(of: moved)
            return "\(moved.drawnReference) now centred at (\(num(centre.x)),\(num(centre.y)))"
        }
        let chip = whole
        let size = chip.layout.size
        let target: CGPoint
        if let x = args.optDouble("x"), let y = args.optDouble("y") {
            target = SchematicGrid.snap(CGPoint(x: x - size.width / 2, y: y - size.height / 2))
        } else if let dx = args.optDouble("dx"), let dy = args.optDouble("dy") {
            target = SchematicGrid.snap(CGPoint(x: chip.origin.x + dx, y: chip.origin.y + dy))
        } else {
            throw AutomationFailure("give x and y (the new centre), or dx and dy")
        }
        let delta = CGSize(width: target.x - chip.origin.x, height: target.y - chip.origin.y)
        moveGroup(document, [.chip(chip.id)], by: delta)
        let moved = try context.chip(chip.reference)
        let centre = context.centre(of: moved)
        return "\(moved.reference) now centred at (\(num(centre.x)),\(num(centre.y)))"
    }

    static func setPart(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let chip = try context.chip(try args.string("reference"))
        var done: [String] = []
        try document.recordEdit(.schematic, "Change Part") {
            if let color = args.optString("color") {
                guard chip.part.colors.contains(color) else {
                    throw AutomationFailure("\(chip.part.name) has no color \(color); colors: \(chip.part.colors.joined(separator: ", "))")
                }
                document.setPartColor(chip.id, color)
                done.append("color")
            }
            if let ohms = args.optDouble("ohms") {
                guard ResistorValue.isValid(ohms) else { throw AutomationFailure("\(num(ohms)) ohms isn't a valid resistance") }
                document.setPartOhms(chip.id, ohms)
                done.append("ohms")
            }
            if let farads = args.optDouble("farads") {
                document.setPartFarads(chip.id, farads)
                done.append("farads")
            }
            if let footprint = args.optString("footprint") {
                document.setPartFootprint(chip.id, try footprintID(chip.part, footprint))
                done.append("footprint")
            }
            if let gender = args.optString("gender") {
                document.setPartGender(chip.id, gender)
                done.append("gender")
            }
            if let shows = args.optBool("show_reference") {
                document.setPartShowsReference(chip.id, shows)
                done.append("show_reference")
            }
            let socketName = args.optString("socket")
            if socketName != nil || args.optString("socket_mode") != nil {
                try setSocket(document, chip, mode: args.optString("socket_mode"), socketName: socketName)
                done.append("socket")
            }
            if let view = args.optString("view") {
                let split = try wantsUnits(chip.part, named: chip.reference, view)
                document.setPartSplit(chip.id, split)
                done.append("view \(split ? (chip.part.unitViewName ?? "units").lowercased() : "block")")
            }
            if let name = args.optString("new_reference") {
                guard let match = name.wholeMatch(of: /([A-Za-z]+)(\d+)/), let number = Int(match.2) else {
                    throw AutomationFailure("a reference is letters then a number, like U5")
                }
                if let refusal = document.renameChip(chip.id, prefix: String(match.1), number: number, page: .schematic) {
                    throw AutomationFailure(refusal)
                }
                done.append("reference")
            }
        }
        guard !done.isEmpty else {
            throw AutomationFailure("nothing to change: pass new_reference, color, ohms, farads, footprint, gender, show_reference, socket_mode, socket or view")
        }
        return "\(chip.reference): changed \(done.joined(separator: ", "))"
    }

    /// `set_part`'s `socket_mode` (ic, ic+socket, socket-only) and `socket` (plan-51-H). `ic` puts
    /// the chip back in plain IC mode; a socket mode without a `socket` keeps the chip's own socket,
    /// or picks the generic one for its footprint; a `socket` without a mode keeps the chip's mode
    /// (IC + Socket for a chip not in a socket yet).
    static func setSocket(_ document: SchematicDocument, _ chip: PlacedChip, mode: String?,
                                  socketName: String?) throws {
        let modes: [String: ChipSocket.Mode?] = ["ic": nil, "ic+socket": .withSocket, "socket-only": .socketOnly]
        let newMode: ChipSocket.Mode?
        if let mode {
            guard let known = modes[mode] else {
                throw AutomationFailure("socket_mode is ic, ic+socket or socket-only, not \(mode)")
            }
            newMode = known
        } else {
            newMode = chip.socket?.mode ?? .withSocket
        }
        if newMode == nil {
            document.setPartSocket(chip.id, mode: nil, socket: nil)
            return
        }
        guard let base = chip.part.footprint, chip.part.footprintChoices.isEmpty else {
            throw AutomationFailure("\(chip.part.name) cannot go in a socket")
        }
        let catalog = PartCatalogStore.shared
        let socket: CatalogPart
        if let socketName {
            guard let named = catalog.part(named: socketName), named.kind == PART_SOCKET else {
                throw AutomationFailure("\(socketName) is not an IC socket; use search_parts")
            }
            guard ChipSocket.fits(named, chip.part) else {
                throw AutomationFailure("\(socketName) does not hold \(base.id), the footprint of \(chip.part.name)")
            }
            socket = named
        } else if let current = chip.socket?.part {
            socket = current
        } else if let generic = catalog.genericSocket(fitting: base) {
            socket = generic
        } else {
            throw AutomationFailure("no socket holds \(base.id)")
        }
        document.setPartSocket(chip.id, mode: newMode, socket: socket)
    }
}

extension AutomationTools {
    // MARK: Labels, notes, buses

    static func addNetLabel(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let name = try args.string("name")
        let style = NetLabel.Style(rawValue: try args.choice("style", among: NetLabel.Style.allCases.map(\.rawValue),
                                                              default: "input")) ?? .input
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        var result: UUID?
        document.recordEdit(.schematic, "Add Net Label") {
            let id = document.addNetLabel(at: point, style: style)
            if document.setNetLabelName(id, name) { result = id } else { document.deleteNetLabel(id) }
        }
        guard let id = result, let label = document.netLabels.first(where: { $0.id == id }) else {
            throw AutomationFailure(document.editorMessage ?? "the name “\(name)” isn't allowed for a net label")
        }
        document.clearAllSelections()
        return "added net label \(label.name) (\(context.short(id))) at (\(num(label.origin.x)),\(num(label.origin.y)))"
    }

    static func addNote(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        let text = try args.string("text")
        var id: UUID?
        document.recordEdit(.schematic, "Add Text") {
            let note = document.addNote(at: point)
            document.setNoteText(note, text)
            if let size = args.optInt("size") { document.setNoteSize(note, size) }
            if let framed = args.optBool("framed") { document.setNoteFramed(note, framed) }
            id = note
        }
        document.editingNoteID = nil
        document.clearAllSelections()
        return "added note \(id.map(context.short) ?? "")"
    }

    static func addBus(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let start = SchematicGrid.snap(CGPoint(x: try args.double("x1"), y: try args.double("y1")))
        let end = SchematicGrid.snap(CGPoint(x: try args.double("x2"), y: try args.double("y2")))
        var notes: [String] = []
        var created: UUID?
        document.recordEdit(.schematic, "Add Bus") {
            created = document.addBus(from: start, to: end)
            guard let id = created else { return }
            if let name = args.optString("name"), let refusal = document.setBusName(id, name) { notes.append(refusal) }
            if let count = args.optInt("wires"), let refusal = document.setBusWireCount(id, count) { notes.append(refusal) }
        }
        guard let id = created, let bus = document.bus(id) else {
            throw AutomationFailure("couldn't draw a bus there: it must be one horizontal or vertical line, "
                                    + "not touching another bus")
        }
        document.clearAllSelections()
        return "added bus \(bus.name) with \(bus.wires.count) wires (\(bus.wires.prefix(4).map(\.name).joined(separator: ","))…)"
            + (notes.isEmpty ? "" : "; note: " + notes.joined(separator: "; "))
    }

    static func bus(_ document: SchematicDocument, _ name: String) throws -> Bus {
        guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame }) else {
            throw AutomationFailure("no bus \(name); buses: " + document.buses.map(\.name).joined(separator: ", "))
        }
        return bus
    }

    static func wire(_ bus: Bus, _ name: String) throws -> BusWire {
        guard let wire = bus.wires.first(where: { $0.name == name }) else {
            throw AutomationFailure("bus \(bus.name) has no wire \(name); wires: " + bus.wires.map(\.name).joined(separator: ","))
        }
        return wire
    }

    static func setBus(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let id = try bus(document, try args.string("bus")).id
        let names = args.raw["wire_names"] != nil ? try args.strings("wire_names") : nil
        let renames = (args.raw["rename"] as? [String: String]) ?? [:]
        let moving = args.optString("move_wire")
        guard names != nil || args.optInt("wires") != nil || !renames.isEmpty || moving != nil
                || args.optString("name") != nil else {
            throw AutomationFailure("nothing to change: give name, wires, wire_names, rename or move_wire")
        }
        var problem: String?
        func check(_ refusal: String?) { if problem == nil { problem = refusal } }
        document.recordEdit(.schematic, "Change Bus") {
            if let name = args.optString("name") { check(document.setBusName(id, name)) }
            if problem == nil, let count = names?.count ?? args.optInt("wires") {
                check(document.setBusWireCount(id, count))
            }
            if problem == nil, let names, let current = document.bus(id) {
                for (place, name) in names.enumerated() where current.wires[place].name != name {
                    check(document.renameBusWire(bus: id, wire: current.wires[place].id, to: name))
                    if problem != nil { break }
                }
            }
            if problem == nil {
                for (old, new) in renames.sorted(by: { $0.key < $1.key }) {
                    guard let current = document.bus(id), let target = current.wires.first(where: { $0.name == old }) else {
                        problem = "no wire \(old) on this bus"
                        break
                    }
                    check(document.renameBusWire(bus: id, wire: target.id, to: new))
                    if problem != nil { break }
                }
            }
            if problem == nil, let moving, let current = document.bus(id) {
                let target = try? wire(current, moving)
                if let target { check(document.moveBusWire(bus: id, wire: target.id, to: args.optInt("move_to") ?? 0)) }
                else { problem = "no wire \(moving) on this bus" }
            }
        }
        document.clearAllSelections()
        if let problem {
            let now = document.bus(id).map { "; the bus now has wires " + $0.wires.map(\.name).joined(separator: ",") } ?? ""
            throw AutomationFailure(problem + " (changes before this one stay applied" + now + ")")
        }
        guard let now = document.bus(id) else { return "changed" }
        return "bus \(now.name) has \(now.wires.count) wires: " + now.wires.map(\.name).joined(separator: ",")
    }

    static func setBusPort(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let bus = try bus(document, try args.string("bus"))
        let port = try busPort(bus, args.optInt("port"))
        var wanted = port.wireIDs
        if args.raw["wires"] != nil { wanted = Set(try args.strings("wires").map { try wire(bus, $0).id }) }
        if args.raw["add_wires"] != nil { wanted.formUnion(try args.strings("add_wires").map { try wire(bus, $0).id }) }
        if args.raw["remove_wires"] != nil { wanted.subtract(try args.strings("remove_wires").map { try wire(bus, $0).id }) }
        let side = try args.optString("side").map { name -> ChipLayout.Side in
            guard let side = [ChipLayout.Side.top, .bottom, .left, .right].first(where: { "\($0)" == name.lowercased() }) else {
                throw AutomationFailure("side is top or bottom (on a horizontal bus line), left or right (on a vertical one)")
            }
            guard let line = bus.line(port.line), BusPort.sides(for: line).contains(side) else {
                throw AutomationFailure("this port is on a \(bus.line(port.line)?.isHorizontal == false ? "vertical" : "horizontal") "
                                        + "bus line: its side is "
                                        + (bus.line(port.line)?.isHorizontal == false ? "left or right" : "top or bottom"))
            }
            return side
        }
        let along = args.optDouble("at").map { SchematicGrid.snap(CGFloat($0)) }
        guard wanted != port.wireIDs || side != nil || along != nil else {
            throw AutomationFailure("nothing to change: give wires, add_wires, remove_wires, side or at")
        }
        var problem: String?
        document.recordEdit(.schematic, "Change Port") {
            if wanted != port.wireIDs { problem = document.setPortWires(port.id, wanted) }
            if problem == nil, let side, document.port(port.id)?.port.side != side {
                problem = document.setPortSide(port.id, side)
            }
            if problem == nil, let along, !document.slidePort(port.id, to: along) {
                problem = "the port can't go to \(num(Double(along))): it must stay on its line, clear of its ends, "
                    + "the joints and the other ports"
            }
        }
        document.clearAllSelections()
        let now = document.port(port.id)
        let text = now.map { "port \(describe($0.port, in: $0.bus))" } ?? "the port"
        if let problem { throw AutomationFailure(problem + " (changes before this one stay applied; \(text))") }
        return text
    }

    /// "pins D0,D1 on the top, starting at 120" for get_design and the port tools.
    static func describe(_ port: BusPort, in bus: Bus) -> String {
        let pins = bus.wires.filter { port.wireIDs.contains($0.id) }.map(\.name).joined(separator: ",")
        let axis = bus.line(port.line)?.isHorizontal == false ? "y" : "x"
        return "pins \(pins); side \(port.side); first pin at \(axis)=\(num(Double(port.start)))"
    }

    static func busPort(_ bus: Bus, _ number: Int?) throws -> BusPort {
        guard let number, bus.ports.indices.contains(number - 1) else {
            throw AutomationFailure("bus \(bus.name) has \(bus.ports.count) ports (numbered from 1); add one with add_bus_port")
        }
        return bus.ports[number - 1]
    }

    static func addBusPort(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let name = try args.string("bus")
        guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame }) else {
            throw AutomationFailure("no bus \(name); buses: " + document.buses.map(\.name).joined(separator: ", "))
        }
        let wanted: Set<UUID>
        if let names = try? args.strings("wires") {
            let hits = bus.wires.filter { names.contains($0.name) }
            guard hits.count == Set(names).count else {
                throw AutomationFailure("bus \(bus.name) wires are: " + bus.wires.map(\.name).joined(separator: ","))
            }
            wanted = Set(hits.map(\.id))
        } else {
            wanted = Set(bus.wires.map(\.id))
        }
        let point = CGPoint(x: try args.double("x"), y: try args.double("y"))
        let known = Set(bus.ports.map(\.id))
        var problem: String?
        document.recordEdit(.schematic, "Add Port") {
            if let refusal = document.addPort(at: point) { problem = refusal; return }
            guard let port = document.bus(bus.id)?.ports.first(where: { !known.contains($0.id) }) else {
                problem = "no port was added"
                return
            }
            if let refusal = document.setPortWires(port.id, wanted) { problem = refusal }
        }
        document.clearAllSelections()
        if let problem { throw AutomationFailure(problem) }
        return "added a port on \(bus.name) with \(wanted.count) pins"
    }
}

extension AutomationTools {
    // MARK: Deleting and selecting

    enum Item {
        case schematic(SchematicItem)
        case board(BoardItem)
    }

    static func item(_ spec: String, _ context: AutomationContext, forDelete: Bool) throws -> Item {
        let document = context.document
        if spec.lowercased() == "titleblock" {
            guard document.documentInfo.titleBlock != nil else {
                throw AutomationFailure("the title block is not on the schematic: set_document_info title_block=true")
            }
            return .schematic(.titleBlock)
        }
        guard let colon = spec.firstIndex(of: ":") else {
            let (chip, key) = try context.unit(spec)
            if let key, forDelete {
                throw AutomationFailure("\(chip.reference)-\(key) is one symbol of \(chip.reference); delete \(chip.reference) to remove the part")
            }
            return .schematic(.chip(chip.id))
        }
        let kind = String(spec[..<colon]).lowercased()
        let rest = String(spec[spec.index(after: colon)...])
        switch kind {
        case "wire": return .schematic(.wire(try context.find(document.wires, rest, kind: "wire").id))
        case "note": return .schematic(.note(try context.find(document.notes, rest, kind: "note").id))
        case "junction": return .schematic(.junction(try context.find(document.junctions, rest, kind: "junction").id))
        case "label":
            if let label = document.netLabels.first(where: { $0.name == rest }) { return .schematic(.label(label.id)) }
            return .schematic(.label(try context.find(document.netLabels, rest, kind: "net label").id))
        case "bus":
            guard let bus = document.buses.first(where: { $0.name.caseInsensitiveCompare(rest) == .orderedSame }) else {
                throw AutomationFailure("no bus \(rest)")
            }
            return .schematic(.bus(bus.id))
        case "trace": return .board(.trace(try context.find(document.traces, rest, kind: "trace").id))
        case "via": return .board(.via(try context.find(document.vias, rest, kind: "via").id))
        case "hole": return .board(.hole(try context.find(document.holes, rest, kind: "hole").id))
        case "pour": return .board(.pour(try context.find(document.pours, rest, kind: "pour").id))
        case "keepout": return .board(.keepout(try context.find(document.keepouts, rest, kind: "keep-out").id))
        case "cutout": return .board(.cutout(try context.find(document.cutouts, rest, kind: "cutout").id))
        case "text": return .board(.text(try context.find(document.texts, rest, kind: "board text").id))
        case "fp", "footprint":
            if forDelete { throw AutomationFailure("a footprint goes with its part: delete \(rest)") }
            return .board(.footprint(try context.chip(rest).id))
        default:
            throw AutomationFailure("unknown item “\(spec)”: use a reference (U1), wire:ID, label:NAME, note:ID, "
                                    + "titleblock, bus:NAME, junction:ID, trace:ID, via:ID, hole:ID, pour:ID, keepout:ID, cutout:ID, text:ID or fp:U1")
        }
    }

    static func delete(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let specs = try args.strings("items")
        // Ports are numbered per bus, so find them all before deleting any.
        let ports = try specs.filter { $0.lowercased().hasPrefix("port:") }.map { try portID($0, document) }
        let items = try specs.filter { !$0.lowercased().hasPrefix("port:") }.map { try item($0, context, forDelete: true) }
        guard !items.isEmpty || !ports.isEmpty else { throw AutomationFailure("items is empty") }
        var removed = 0
        var loose = 0
        document.recordEdit(.schematic, "Delete") {
            document.clearAllSelections()
            for id in ports {
                loose += document.wiresOnPins(port: id)
                document.selectedPortID = id
                if document.deleteSchematicSelection() {
                    // The app asks first when wires are on its pins; the agent has said to delete.
                    if document.pendingRemoval != nil { document.confirmPendingRemoval() }
                    removed += 1
                }
                document.clearAllSelections()
            }
            for item in items {
                switch item {
                case let .schematic(item):
                    document.toggleSelection(item)
                case let .board(.trace(id)): document.deleteTrace(id); removed += 1
                case let .board(.via(id)): document.deleteVia(id); removed += 1
                case let .board(.hole(id)): document.deleteHole(id); removed += 1
                case let .board(.pour(id)): document.deletePour(id); removed += 1
                case let .board(.keepout(id)): document.deleteKeepout(id); removed += 1
                case let .board(.cutout(id)): document.deleteCutout(id); removed += 1
                case let .board(.text(id)): document.deleteText(id); removed += 1
                case .board: break
                }
            }
            let schematicCount = items.filter { if case .schematic = $0 { true } else { false } }.count
            // The app asks before a part drawn as several symbols goes; the agent has said to delete.
            if schematicCount > 0, document.deleteSchematicSelection(confirmed: true) { removed += schematicCount }
            document.clearAllSelections()
        }
        guard removed > 0 else { throw AutomationFailure(document.editorMessage ?? "nothing was deleted") }
        return "deleted \(removed) item\(removed == 1 ? "" : "s")"
            + (loose > 0 ? "; \(loose) wire\(loose == 1 ? "" : "s") on the port pins now have loose ends" : "")
    }

    /// The port "port:BUS:N" names (N counts from 1, as get_design lists them).
    static func portID(_ spec: String, _ document: SchematicDocument) throws -> UUID {
        let parts = spec.split(separator: ":", omittingEmptySubsequences: false).map(String.init)
        guard parts.count == 3, let number = Int(parts[2]) else { throw AutomationFailure("a port looks like port:BUS:N, e.g. port:DB:1") }
        return try busPort(try bus(document, parts[1]), number).id
    }

    static func select(_ context: AutomationContext, _ args: AutomationArgs) throws -> String {
        let document = context.document
        let items = try args.strings("items").map { try item($0, context, forDelete: false) }
        if args.optBool("add") != true { document.clearAllSelections() }
        for item in items {
            switch item {
            case let .schematic(item): if !document.isSelected(item) { document.toggleSelection(item) }
            case let .board(item): if !document.isSelected(item) { document.toggleSelection(item) }
            }
        }
        // "U1-A": that unit of the part is the picked one (several: all of them).
        var picks = Set<UnitRef>()
        for spec in try args.strings("items") where !spec.contains(":") {
            if let found = try? context.unit(spec), let key = found.key { picks.insert(UnitRef(chipID: found.chip.id, key: key)) }
        }
        if !picks.isEmpty { document.pickUnits(picks) }
        return "selected \(items.count) item\(items.count == 1 ? "" : "s")"
    }
}
