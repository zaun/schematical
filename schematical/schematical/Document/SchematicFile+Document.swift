import CoreGraphics
import Foundation
import UniformTypeIdentifiers

// MARK: - Converting to and from the document

extension SchematicFile {
    /// A snapshot of the document. The selection is not saved. `fileURL` is
    /// where it will be (or was) saved: load-file paths are relative to its
    /// folder, or absolute when it is nil.
    /// Custom parts' file text comes from `catalog` (the app's catalog when
    /// nil; read only when the schematic places a custom part).
    @MainActor
    init(document: SchematicDocument, fileURL: URL?, catalog: PartCatalogStore? = nil) {
        format = Self.formatName
        var custom: [JSONValue] = []
        var embedded = Set<String>()
        for chip in document.chips {
            guard let uuid = chip.part.uuid, embedded.insert(uuid).inserted else { continue }
            let store = catalog ?? PartCatalogStore.shared
            if let text = store.registeredText(uuid: uuid), let value = try? JSONValue(text: text) {
                custom.append(value)
            }
        }
        customParts = custom.isEmpty ? nil : custom
        version = custom.isEmpty ? Self.plainVersion : Self.customPartsVersion
        nextReference = document.nextReference
        parts = document.chips.map { chip in
            Part(
                id: chip.id,
                reference: chip.reference,
                partNumber: chip.partNumber,
                x: chip.origin.x,
                y: chip.origin.y,
                flipped: chip.isFlipped && !chip.isSplit,
                rotation: chip.rotation == 0 || chip.isSplit ? nil : chip.rotation,
                units: chip.isSplit ? chip.unitPlacements.map {
                    Unit(unit: $0.key, x: $0.origin.x, y: $0.origin.y, flipped: $0.isFlipped ? true : nil,
                         rotation: $0.rotation == 0 ? nil : $0.rotation)
                } : nil,
                // DIP and slide switches save their contacts; a tactile switch never does.
                switches: chip.switchesOn.isEmpty || chip.part.isTactileSwitch ? nil : chip.switchesOn,
                pcb: chip.board.map {
                    Placement(x: $0.centre.x, y: $0.centre.y, rotation: $0.rotation,
                              origin: $0.origin.rawValue, outline: $0.outline.rawValue,
                              pin1: $0.pin1.rawValue, reference: $0.reference.rawValue,
                              style: chip.part.connectorFootprint == nil ? nil : $0.style.rawValue,
                              side: $0.side.rawValue, locked: $0.locked ? true : nil,
                              refX: $0.referenceOffset.x == 0 ? nil : Double($0.referenceOffset.x),
                              refY: $0.referenceOffset.y == 0 ? nil : Double($0.referenceOffset.y))
                },
                memoryFile: chip.memoryFile.map {
                    MemoryFileEntry(path: MemoryFileAccess.path(of: $0.url, schematic: fileURL),
                                    bookmark: $0.bookmark)
                },
                color: chip.color,
                footprint: chip.footprintID,
                gender: chip.gender,
                ohms: chip.ohms,
                partUUID: chip.part.uuid,
                farads: chip.farads,
                showReference: chip.showsReference ? nil : false,
                socket: chip.socket.map { SocketEntry(part: $0.part.name, mode: $0.mode.rawValue) }
            )
        }
        junctions = document.junctions.map {
            Junction(id: $0.id, x: $0.point.x, y: $0.point.y, loose: $0.isLoose ? true : nil)
        }
        wires = document.wires.map { wire in
            Wire(
                id: wire.id,
                from: End(wire.start, chips: document.chips),
                to: End(wire.end, chips: document.chips),
                bends: wire.interior.map { Point(x: $0.x, y: $0.y) }
            )
        }
        buses = document.buses.map { bus in
            Bus(
                id: bus.id,
                name: bus.name,
                type: bus.card.isCustom ? nil : bus.card.type.rawValue,
                card: bus.card.type == .customCard ? Self.entry(bus.card.customCard) : nil,
                cardOptions: bus.card.type.hasBusOptions ? Self.entry(bus.card.cardOptions, of: bus.card.type, bus: true)
                    : nil,
                lines: bus.lines.map {
                    BusLineEntry(id: $0.id, a: Point(x: $0.a.x, y: $0.a.y), b: Point(x: $0.b.x, y: $0.b.y))
                },
                wires: bus.wires.map { BusWireEntry(id: $0.id, name: $0.name) },
                ports: bus.ports.map { port in
                    PortEntry(id: port.id, line: port.line, start: port.start,
                              side: Self.sideName(port.side), wires: bus.pinWires(of: port).map(\.id))
                }
            )
        }
        notes = document.notes.map { note in
            Note(id: note.id, text: note.text, x: note.origin.x, y: note.origin.y,
                 size: note.size, framed: note.framed)
        }
        documentInfo = document.documentInfo == DocumentInfo() ? nil : Self.entry(document.documentInfo)
        labels = document.netLabels.isEmpty ? nil : document.netLabels.map { label in
            Label(id: label.id, name: label.name, x: label.origin.x, y: label.origin.y,
                  type: label.style.rawValue, flipped: label.isFlipped ? true : nil)
        }
        erc = Self.entry(document.erc)
        let settings = document.board
        board = Board(
            width: settings.width,
            height: settings.height,
            layers: settings.layers,
            color: settings.color.rawValue,
            units: settings.units.rawValue,
            grid: settings.grid,
            type: settings.type == .custom ? nil : settings.type.rawValue,
            edgeBus: document.edgeBusID,
            card: settings.type == .customCard ? Self.entry(settings.customCard) : nil,
            cardOptions: settings.type.hasOptions ? Self.entry(settings.cardOptions, of: settings.type, bus: false) : nil,
            layerStyles: settings.copperLayers.map { layer in
                let style = settings.style(of: layer)
                return Board.LayerStyleEntry(layer: layer.rawValue, color: style.color.hex, type: style.type.rawValue,
                                             net: style.net?.fileValue, clearance: style.clearance)
            },
            texts: document.texts.map { text in
                Board.Text(id: text.id, text: text.text, x: text.anchor.x, y: text.anchor.y,
                           size: text.size, layer: text.layer.rawValue, rotation: text.rotation,
                           side: text.side.rawValue, locked: text.locked ? true : nil)
            },
            traces: document.traces.map { trace in
                Board.TraceEntry(id: trace.id, layer: trace.layer.rawValue, width: trace.width,
                                 points: trace.points.map { Point(x: $0.x, y: $0.y) },
                                 locked: trace.locked ? true : nil, auto: trace.auto ? true : nil)
            },
            vias: document.vias.isEmpty ? nil : document.vias.map { via in
                Board.ViaEntry(id: via.id, x: via.centre.x, y: via.centre.y, drill: via.drill, diameter: via.diameter,
                               locked: via.locked ? true : nil, auto: via.auto ? true : nil,
                               openTop: via.tentedTop ? nil : true, openBottom: via.tentedBottom ? nil : true)
            },
            holes: document.holes.isEmpty ? nil : document.holes.map { hole in
                Board.HoleEntry(id: hole.id, x: hole.centre.x, y: hole.centre.y, size: hole.size.rawValue,
                                drill: hole.drill, diameter: hole.diameter, plated: hole.plated ? true : nil,
                                net: hole.net?.fileValue, locked: hole.locked ? true : nil)
            },
            pours: document.pours.isEmpty ? nil : document.pours.map { pour in
                Board.PourEntry(id: pour.id, x: pour.rect.minX, y: pour.rect.minY, width: pour.rect.width,
                                height: pour.rect.height, layer: pour.layer.rawValue, net: pour.net?.fileValue,
                                clearance: pour.clearance, connection: pour.connection.rawValue,
                                thermalGap: pour.thermalGap, spokeWidth: pour.spokeWidth, minWidth: pour.minWidth,
                                locked: pour.locked ? true : nil)
            },
            keepouts: document.keepouts.isEmpty ? nil : document.keepouts.map { keepout in
                Board.KeepoutEntry(id: keepout.id, x: keepout.rect.minX, y: keepout.rect.minY,
                                   width: keepout.rect.width, height: keepout.rect.height,
                                   layers: keepout.layers.map { $0.map(\.rawValue).sorted() },
                                   locked: keepout.locked ? true : nil)
            },
            outline: settings.isFreeForm ? settings.customOutline?.map { Point(x: $0.x, y: $0.y) } : nil,
            cutouts: document.cutouts.isEmpty ? nil : document.cutouts.map { cutout in
                switch cutout.shape {
                case let .rounded(rect, radius):
                    Board.CutoutEntry(id: cutout.id, shape: "rounded", x: rect.minX, y: rect.minY, width: rect.width,
                                      height: rect.height, radius: radius, locked: cutout.locked ? true : nil)
                case let .polygon(points):
                    Board.CutoutEntry(id: cutout.id, shape: "polygon", points: points.map { Point(x: $0.x, y: $0.y) },
                                      locked: cutout.locked ? true : nil)
                }
            },
            netClasses: settings.netClasses.isEmpty ? nil : settings.netClasses.map { netClass in
                Board.NetClassEntry(id: netClass.id, name: netClass.name, width: netClass.width,
                                    clearance: netClass.clearance, viaDrill: netClass.viaDrill,
                                    viaDiameter: netClass.viaDiameter, members: netClass.members.map(\.fileValue),
                                    layerMode: netClass.layerRule?.mode.rawValue,
                                    layers: netClass.layerRule?.layers.map(\.rawValue))
            },
            netLayerRules: settings.netOverrides.isEmpty ? nil : settings.netOverrides.compactMap { entry in
                entry.layerRule.map {
                    Board.NetLayerRuleEntry(net: entry.net.fileValue, mode: $0.mode.rawValue,
                                            layers: $0.layers.map(\.rawValue))
                }
            },
            netGroups: settings.netGroups.isEmpty ? nil : settings.netGroups.map { group in
                Board.NetGroupEntry(id: group.id, name: group.name, mode: group.mode.rawValue,
                                    members: group.members.map(\.fileValue), tolerance: group.tolerance, gap: group.gap,
                                    maxUncoupled: group.maxUncoupled, maxMeander: group.maxMeander)
            },
            designRules: Self.entry(settings.rules, manufacturer: settings.rulesManufacturer),
            stackup: Board.StackupEntry(
                copper: settings.stackup.copper,
                dielectrics: settings.stackup.dielectrics.map {
                    Board.DielectricEntry(type: $0.kind.rawValue, thickness: $0.thickness, er: $0.er)
                },
                finish: settings.stackup.finish.rawValue,
                temperatureRise: settings.stackup.temperatureRise)
        )
    }

    static func decode(_ data: Data) throws -> SchematicFile {
        // Check the header first, so a newer file gets a clear message rather
        // than a decoding error about fields this version doesn't know.
        struct Header: Decodable {
            var format: String?
            var version: Int?
        }
        guard let header = try? JSONDecoder().decode(Header.self, from: data),
              header.format == formatName,
              let version = header.version else {
            throw SchematicFileError.notASchematic
        }
        guard version <= currentVersion else {
            throw SchematicFileError.newerVersion(version)
        }
        let file: SchematicFile
        do {
            file = try JSONDecoder().decode(SchematicFile.self, from: data)
        } catch {
            throw SchematicFileError.unreadable(error.localizedDescription)
        }
        // One pin type per version: numbers in version 1, designators after.
        for wire in file.wires {
            for case let .pin(_, pin) in [wire.from, wire.to] {
                switch (file.version, pin) {
                case (1, .designator):
                    throw SchematicFileError.unreadable("version 1 files name pins by number")
                case (2..., .position):
                    throw SchematicFileError.unreadable("version 2 files name pins by designator, such as \"A1\"")
                default:
                    break
                }
            }
        }
        return file
    }

    func encoded() throws -> Data {
        let encoder = JSONEncoder()
        // Sorted keys, so saving the same schematic twice writes the same bytes.
        encoder.outputFormatting = [.prettyPrinted, .withoutEscapingSlashes, .sortedKeys]
        return try encoder.encode(self)
    }

    /// Rebuilds the document's contents, looking up each part in the catalog.
    /// Throws without touching the document if anything doesn't resolve.
    /// `fileURL` is where the file was read from; EEPROM load-file paths are
    /// resolved against its folder. A missing load file never fails here.
    /// Returns the custom parts the file embeds, as registered (after any
    /// new uuid, plan-53-A), each once, in the file's order.
    @MainActor
    @discardableResult
    func apply(to document: SchematicDocument, catalog: PartCatalogStore, fileURL: URL?) throws -> [CatalogPart] {
        var seen = Set<UUID>()
        func claim(_ id: UUID) throws {
            guard seen.insert(id).inserted else { throw SchematicFileError.duplicateID(id) }
        }

        let settings = try boardSettings()
        let ercSettings = try Self.ercSettings(erc)

        // Custom parts first (plan-32-B), so placed parts can resolve to them.
        // Registering one is for the session, like File › Load Part….
        // A part whose uuid is loaded with other content is registered under a
        // new uuid (plan-53-A); the placed parts that name the old one use it.
        var remapped: [String: String] = [:]
        var embedded: [CatalogPart] = []
        for (index, entry) in (customParts ?? []).enumerated() {
            do {
                let text = try entry.text()
                let part = try catalog.register(fileText: text, source: "customParts[\(index)]")
                if !embedded.contains(part) { embedded.append(part) }
                if let own = try? PartFileObject(text: text).uuid, let registered = part.uuid, own != registered {
                    remapped[own] = registered
                }
            } catch let error as PartRegistrationError {
                throw SchematicFileError.badCustomPart(error.localizedDescription)
            }
        }

        var chips: [PlacedChip] = []
        var unknown: [String] = []
        for part in parts {
            try claim(part.id)
            let legacy = Self.legacyLEDs[part.partNumber]
            let name = legacy != nil ? Self.genericLED : Self.renamedParts[part.partNumber] ?? part.partNumber
            let found = part.partUUID.map { catalog.part(uuid: remapped[$0] ?? $0) } ?? catalog.part(named: name)
            guard let catalogPart = found else {
                unknown.append(part.partNumber)
                continue
            }
            let partSymbol = symbol(for: catalogPart)
            // The LED options: a stored value must be one the part offers;
            // missing is the part's default (an old LED name gives its own).
            let color = part.color ?? legacy?.color
            if let color, catalogPart.kind == PART_LED, !catalogPart.colors.contains(color) {
                throw SchematicFileError.badPartOption(
                    reference: part.reference, detail: "\"\(color)\" is not a color of \(catalogPart.name)")
            }
            let footprintID = part.footprint ?? legacy?.footprint
            if let footprintID, !catalogPart.footprintChoices.isEmpty,
               !catalogPart.footprintChoices.contains(where: { $0.footprint.id == footprintID }) {
                throw SchematicFileError.badPartOption(
                    reference: part.reference, detail: "\"\(footprintID)\" is not a footprint of \(catalogPart.name)")
            }
            if let gender = part.gender, catalogPart.kind == PART_HEADER, !catalogPart.genders.contains(gender) {
                throw SchematicFileError.badPartOption(
                    reference: part.reference, detail: "\"\(gender)\" is not a gender of \(catalogPart.name)")
            }
            // A pull resistor's or GENERIC-RESISTOR's resistance; missing is
            // 1 kΩ, ignored elsewhere.
            if catalogPart.hasOhmsField, let ohms = part.ohms, !ResistorValue.isValid(ohms) {
                throw SchematicFileError.badOhms(reference: part.reference, ohms: ohms)
            }
            // The generic capacitor's capacitance; missing is 0.1 µF, ignored elsewhere.
            if catalogPart.isGenericCapacitor, let farads = part.farads, !CapacitorValue.isValid(farads) {
                throw SchematicFileError.badFarads(reference: part.reference, farads: farads)
            }
            // The socket (plan-51-H): a bundled socket part that holds this chip's footprint, in a known mode.
            var socket: ChipSocket?
            if let entry = part.socket {
                guard let mode = ChipSocket.Mode(rawValue: entry.mode) else {
                    throw SchematicFileError.badPartOption(
                        reference: part.reference, detail: "\"\(entry.mode)\" is not a socket mode")
                }
                guard let socketPart = catalog.part(named: entry.part), socketPart.kind == PART_SOCKET else {
                    throw SchematicFileError.badPartOption(
                        reference: part.reference, detail: "\"\(entry.part)\" is not an IC socket")
                }
                guard ChipSocket.fits(socketPart, catalogPart) else {
                    throw SchematicFileError.badPartOption(
                        reference: part.reference,
                        detail: "\"\(entry.part)\" does not fit \(catalogPart.footprint?.id ?? catalogPart.name)")
                }
                socket = ChipSocket(part: socketPart, mode: mode)
            }
            // A load file counts only on an EEPROM or a simulated
            // microcontroller (its firmware).
            var memoryFile: MemoryFile?
            if catalogPart.hasLoadFile, let entry = part.memoryFile {
                memoryFile = MemoryFileAccess.file(path: entry.path, bookmark: entry.bookmark, schematic: fileURL)
            }
            // Built before its placement, which is snapped with the chosen footprint.
            var chip = PlacedChip(
                id: part.id,
                reference: part.reference,
                part: catalogPart,
                symbol: partSymbol,
                origin: SchematicGrid.snap(CGPoint(x: part.x, y: part.y)),
                isFlipped: part.flipped && partSymbol.canFlip,
                rotation: PlacedChip.normalized(part.rotation ?? 0),
                switchesOn: part.switches ?? [],
                memoryFile: memoryFile,
                color: color,
                footprintID: footprintID,
                gender: part.gender,
                ohms: catalogPart.hasOhmsField ? part.ohms : nil,
                farads: catalogPart.isGenericCapacitor ? part.farads : nil,
                socket: socket
            )
            chip.showsReference = part.showReference ?? true
            // Drawn as its units (plan-67) when the list names exactly the
            // part's units; any other list is ignored and the part is a block.
            if let units = part.units {
                chip.setPlacements(units.map {
                    UnitPlacement(key: $0.unit, origin: SchematicGrid.snap(CGPoint(x: $0.x, y: $0.y)),
                                  isFlipped: $0.flipped ?? false, rotation: $0.rotation ?? 0)
                })
            }
            // A stored placement counts only for a part with a footprint.
            var board: BoardPlacement?
            if let pcb = part.pcb, catalogPart.footprint != nil {
                guard BoardPlacement.rotations.contains(pcb.rotation) else {
                    throw SchematicFileError.badRotation(reference: part.reference, rotation: pcb.rotation)
                }
                // A placement from an older file (no origin) is snapped by its
                // top-left, as those builds did. One with an origin is kept as
                // saved: changing the origin doesn't move a footprint, so its
                // origin point may be off the grid, and opening must not move it.
                var stored = try placement(pcb, reference: part.reference)
                if stored.side == .bottom, settings.layers == 1 {
                    throw SchematicFileError.badPCB("\(part.reference) is on the bottom of a 1-layer board")
                }
                if catalogPart.connectorFootprint != nil {
                    stored.style = try Self.value(pcb.style, of: FootprintStyle.self, "\(part.reference) style",
                                                  default: .module)
                }
                chip.board = stored
                let footprint = chip.footprint!
                board = pcb.origin == nil
                    ? BoardLayout.snapped(stored, footprint: footprint, pad1: catalogPart.firstPinDesignator,
                                          grid: settings.grid)
                    : stored
            }
            chip.board = board
            chips.append(chip)
        }
        if !unknown.isEmpty {
            throw SchematicFileError.unknownParts(Array(Set(unknown)).sorted())
        }
        // Footprints with no stored place (older files) go after the stored
        // ones, in file order.
        for index in chips.indices where chips[index].board == nil {
            if let footprint = chips[index].footprint {
                chips[index].board = BoardLayout.nextSpot(for: footprint, among: chips,
                                                          pad1: chips[index].part.firstPinDesignator,
                                                          hasPinOneMark: chips[index].part.hasPinOneMark,
                                                          grid: settings.grid)
            }
        }

        var placedJunctions: [SchematicModelJunction] = []
        for junction in junctions {
            try claim(junction.id)
            placedJunctions.append(SchematicModelJunction(id: junction.id, point: CGPoint(x: junction.x, y: junction.y),
                                                         isLoose: junction.loose ?? false))
        }
        let junctionIDs = Set(placedJunctions.map(\.id))
        let chipsByID = Dictionary(uniqueKeysWithValues: chips.map { ($0.id, $0) })

        // Buses come before wires, since wires may end on their port pins.
        var placedBuses: [SchematicModelBus] = []
        for bus in buses ?? [] {
            try claim(bus.id)
            for line in bus.lines { try claim(line.id) }
            for wire in bus.wires { try claim(wire.id) }
            for port in bus.ports ?? [] { try claim(port.id) }
            placedBuses.append(try Self.modelBus(bus))
        }
        // Buses of one name are linked, and carry the same wires and type.
        var busNames: [String: SchematicModelBus] = [:]
        for bus in placedBuses where !bus.name.isEmpty {
            guard let first = busNames[bus.name.lowercased()] else {
                busNames[bus.name.lowercased()] = bus
                continue
            }
            guard first.wires.map(\.name) == bus.wires.map(\.name), first.card == bus.card else {
                throw SchematicFileError.badBus(
                    "the buses named \"\(bus.name)\" are linked but don't have the same type and wires")
            }
        }
        if let (first, second, point) = SchematicModelBus.firstTouch(placedBuses) {
            throw SchematicFileError.badBus(
                "buses \(first.uuidString) and \(second.uuidString) touch at \(Self.pointText(point))")
        }
        let portsByID = Dictionary(
            placedBuses.flatMap { bus in bus.ports.map { ($0.id, (bus: bus, port: $0)) } },
            uniquingKeysWith: { first, _ in first })

        var placedLabels: [NetLabel] = []
        for entry in labels ?? [] {
            try claim(entry.id)
            placedLabels.append(try Self.label(entry))
        }
        let labelIDs = Set(placedLabels.map(\.id))

        func resolve(_ end: End, wire: UUID) throws -> WireEnd {
            switch end {
            case let .pin(partID, pin):
                guard let chip = chipsByID[partID] else {
                    throw SchematicFileError.missingPart(wire: wire, part: partID)
                }
                // Only pins the symbol draws can take a wire. Version 1 pin n
                // is the part's n-th pin in its pinout.
                let slot: ChipLayout.PinSlot?
                let label: String
                switch pin {
                case let .designator(designator):
                    slot = chip.layout.slots.first { $0.pin.designator == designator }
                    label = designator
                case let .position(number):
                    slot = chip.layout.slots.first { $0.pin.index == number - 1 }
                    label = "\(number)"
                }
                guard let slot else {
                    throw SchematicFileError.missingPin(wire: wire, reference: chip.reference, pin: label)
                }
                return .pin(PinRef(chipID: partID, index: slot.pin.index))
            case let .junction(id):
                guard junctionIDs.contains(id) else {
                    throw SchematicFileError.missingJunction(wire: wire, junction: id)
                }
                return .junction(id)
            case let .port(portID, busWire):
                guard let found = portsByID[portID] else {
                    throw SchematicFileError.missingPort(wire: wire, port: portID)
                }
                guard found.port.wireIDs.contains(busWire) else {
                    throw SchematicFileError.missingPortPin(wire: wire, port: portID, busWire: busWire)
                }
                return .busPin(BusPinRef(portID: portID, wireID: busWire))
            case let .label(id):
                guard labelIDs.contains(id) else {
                    throw SchematicFileError.missingLabel(wire: wire, label: id)
                }
                return .label(id)
            }
        }

        var placedWires: [SchematicModelWire] = []
        for wire in wires {
            try claim(wire.id)
            placedWires.append(SchematicModelWire(
                id: wire.id,
                start: try resolve(wire.from, wire: wire.id),
                end: try resolve(wire.to, wire: wire.id),
                interior: wire.bends.map { CGPoint(x: $0.x, y: $0.y) }
            ))
        }

        var texts: [BoardText] = []
        for text in board?.texts ?? [] {
            try claim(text.id)
            let placed = try boardText(text, grid: settings.grid)
            if placed.side == .bottom, settings.layers == 1 {
                throw SchematicFileError.badPCB("text \(text.id.uuidString) is on the bottom of a 1-layer board")
            }
            texts.append(placed)
        }

        var traces: [BoardTrace] = []
        for entry in board?.traces ?? [] {
            try claim(entry.id)
            traces.append(try Self.boardTrace(entry, settings: settings))
        }

        var vias: [BoardVia] = []
        for entry in board?.vias ?? [] {
            try claim(entry.id)
            let name = "via \(entry.id.uuidString)"
            guard settings.layers > 1 else {
                throw SchematicFileError.badPCB("\(name) is on a 1-layer board")
            }
            let via = BoardVia(id: entry.id, centre: CGPoint(x: entry.x, y: entry.y),
                               drill: entry.drill ?? BoardVia.defaultDrill,
                               diameter: entry.diameter ?? BoardVia.defaultDiameter,
                               locked: entry.locked ?? false, auto: entry.auto ?? false,
                               tentedTop: !(entry.openTop ?? false), tentedBottom: !(entry.openBottom ?? false))
            if let problem = BoardVia.problem(drill: via.drill, diameter: via.diameter) {
                throw SchematicFileError.badPCB("\(name) \(problem)")
            }
            vias.append(via)
        }

        var holes: [BoardHole] = []
        for entry in board?.holes ?? [] {
            try claim(entry.id)
            let name = "hole \(entry.id.uuidString)"
            var hole = BoardHole(id: entry.id, centre: CGPoint(x: entry.x, y: entry.y))
            hole.apply(try Self.value(entry.size, of: HoleSize.self, "\(name) size", default: .m3))
            hole.drill = entry.drill ?? hole.drill
            hole.diameter = entry.diameter ?? hole.diameter
            hole.plated = entry.plated ?? false
            hole.locked = entry.locked ?? false
            if let raw = entry.net {
                guard let net = PlaneNet(fileValue: raw) else {
                    throw SchematicFileError.badPCB("\(name) net \"\(raw)\" must be ground, power:<mV> or pin:<part id>/<pin>")
                }
                hole.net = net
            }
            if let problem = BoardHole.problem(drill: hole.drill, diameter: hole.diameter) {
                throw SchematicFileError.badPCB("\(name) \(problem)")
            }
            holes.append(hole)
        }

        var pours: [BoardPour] = []
        for entry in board?.pours ?? [] {
            try claim(entry.id)
            pours.append(try Self.boardPour(entry, settings: settings))
        }

        var keepouts: [BoardKeepout] = []
        for entry in board?.keepouts ?? [] {
            try claim(entry.id)
            keepouts.append(try Self.boardKeepout(entry))
        }

        var cutouts: [BoardCutout] = []
        for entry in board?.cutouts ?? [] {
            try claim(entry.id)
            cutouts.append(try Self.boardCutout(entry))
        }

        var placedNotes: [SchematicNote] = []
        for entry in notes ?? [] {
            try claim(entry.id)
            placedNotes.append(try Self.note(entry))
        }

        // Never hand out an Integrated Circuit reference (U<n>) that's
        // already used, even if the stored counter is behind.
        let counted = ReferencePrefixes.prefix(for: .integratedCircuit)
        let highest = chips.map { ReferenceParts($0.reference) }
            .filter { $0.prefix == counted }
            .compactMap(\.number)
            .max() ?? 0
        document.replaceContents(
            chips: chips,
            wires: placedWires,
            junctions: placedJunctions,
            buses: placedBuses,
            nextReference: max(nextReference, highest + 1),
            board: settings,
            texts: texts,
            traces: traces,
            vias: vias,
            holes: holes,
            pours: pours,
            keepouts: keepouts,
            cutouts: cutouts,
            notes: placedNotes,
            netLabels: placedLabels,
            erc: ercSettings,
            documentInfo: try Self.documentInfo(documentInfo)
        )
        return embedded
    }

    // MARK: Electrical rules check (plan-63-E)

    /// The file form of a document's ERC settings; nil when they are the defaults.
    static func entry(_ settings: ErcSettings) -> ErcEntry? {
        guard !settings.isDefault else { return nil }
        return ErcEntry(
            severities: settings.severities.isEmpty
                ? nil : Dictionary(uniqueKeysWithValues: settings.severities.map { ($0.key.rawValue, $0.value.rawValue) }),
            pinMap: settings.pinMap.map { $0.map { $0.map(Int.init) } },
            exclusions: settings.exclusions.isEmpty ? nil : settings.exclusions.sorted())
    }

    /// The settings a file's `erc` says; the defaults when it has none. Throws
    /// `badErc` for a rule or severity the app does not know, or a pin map that is
    /// not 12 rows of 12 numbers 0 to 2, symmetric.
    static func ercSettings(_ entry: ErcEntry?) throws -> ErcSettings {
        var settings = ErcSettings()
        guard let entry else { return settings }
        for (name, word) in entry.severities ?? [:] {
            guard let rule = ErcRule(rawValue: name), rule.hasSeverity else {
                throw SchematicFileError.badErc("erc.severities: \"\(name)\" is not a rule with a severity")
            }
            guard let severity = ErcSeverity(rawValue: word) else {
                throw SchematicFileError.badErc("erc.severities.\(name): \"\(word)\" must be error, warning or ignore")
            }
            settings.set(severity, for: rule)
        }
        if let rows = entry.pinMap {
            let map = rows.map { $0.map { UInt8(clamping: $0) } }
            guard rows.allSatisfy({ $0.allSatisfy { (0...2).contains($0) } }), ErcSettings.isValid(pinMap: map) else {
                throw SchematicFileError.badErc("erc.pinMap must be 12 rows of 12 numbers 0, 1 or 2, the same either side of the diagonal")
            }
            settings.setPinMap(map)
        }
        settings.exclusions = Set(entry.exclusions ?? [])
        return settings
    }

    /// A stored net label, with defaults for missing keys. Its point is
    /// snapped to the grid.
    static func label(_ entry: Label) throws -> NetLabel {
        let name = "net label \(entry.id.uuidString)"
        guard NetLabel.cleanName(entry.name) == entry.name else {
            throw SchematicFileError.badLabel(
                "\(name) name \"\(entry.name)\" must be 1 to \(NetLabel.maxNameLength) characters with no spaces")
        }
        var style = NetLabel.Style.input
        if let type = entry.type {
            guard let parsed = NetLabel.Style(rawValue: type) else {
                throw SchematicFileError.badLabel("\(name) type \"\(type)\" must be input, output, inout, passive, noConnect or powerFlag")
            }
            style = parsed
        }
        return NetLabel(id: entry.id, name: entry.name, style: style,
                        origin: SchematicGrid.snap(CGPoint(x: entry.x, y: entry.y)),
                        isFlipped: entry.flipped ?? false)
    }

    /// A stored text block, with defaults for missing keys. Its top-left is
    /// snapped to the grid.
    static func note(_ entry: Note) throws -> SchematicNote {
        let name = "text block \(entry.id.uuidString)"
        guard !entry.text.isEmpty else { throw SchematicFileError.badNote("\(name) is empty") }
        var result = SchematicNote(id: entry.id, text: entry.text,
                                   origin: SchematicGrid.snap(CGPoint(x: entry.x, y: entry.y)))
        if let size = entry.size {
            let range = SchematicNote.sizeRange
            guard range.contains(size) else {
                throw SchematicFileError.badNote(
                    "\(name) size \(size) must be \(range.lowerBound) to \(range.upperBound) points")
            }
            result.size = size
        }
        result.framed = entry.framed ?? result.framed
        return result
    }

    /// The file form of the document info: empty text, unset dates, automatic
    /// dates and a hidden title block are left out.
    static func entry(_ info: DocumentInfo) -> DocumentInfoEntry {
        DocumentInfoEntry(
            title: info.title.isEmpty ? nil : info.title,
            author: info.author.isEmpty ? nil : info.author,
            revision: info.revision.isEmpty ? nil : info.revision,
            description: info.description.isEmpty ? nil : info.description,
            created: info.created.map(DocumentInfo.isoText),
            updated: info.updated.map(DocumentInfo.isoText),
            automaticDates: info.automaticDates ? nil : false,
            titleBlock: info.titleBlock.map { Point(x: $0.x, y: $0.y) })
    }

    /// The stored document info, with defaults for missing keys. The title
    /// block's top-left is snapped to the grid.
    static func documentInfo(_ entry: DocumentInfoEntry?) throws -> DocumentInfo {
        var info = DocumentInfo()
        guard let entry else { return info }
        info.title = entry.title ?? ""
        info.author = entry.author ?? ""
        info.revision = entry.revision ?? ""
        info.description = entry.description ?? ""
        func date(_ text: String?, _ key: String) throws -> Date? {
            guard let text else { return nil }
            guard let parsed = DocumentInfo.isoDate(text) else {
                throw SchematicFileError.badDocumentInfo("\(key) \"\(text)\" is not a date")
            }
            return parsed
        }
        info.created = try date(entry.created, "created")
        info.updated = try date(entry.updated, "updated")
        info.automaticDates = entry.automaticDates ?? true
        info.titleBlock = entry.titleBlock.map { SchematicGrid.snap(CGPoint(x: $0.x, y: $0.y)) }
        return info
    }

}
