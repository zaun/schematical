import Foundation

/// What the Schematic sidebar's Options section reads and changes for one
/// placed part (plan-68-AC: the logic `PartOptionsSection`'s bindings held).
/// Every read takes the chip from the document again, so the pickers follow
/// every change; every change goes through the document.
@MainActor
struct PartOptionsModel {
    let catalog: PartCatalogStore
    let document: SchematicDocument
    /// The chip as the section was built with it, for when it has left the document.
    let chip: PlacedChip

    /// The chip as the document has it now.
    var current: PlacedChip { document.chips.first { $0.id == chip.id } ?? chip }

    // MARK: View (plan-67)

    /// The View control's second choice ("Units", or the part's own word), nil for a part without units.
    var unitViewName: String? { current.part.unitViewName ?? (current.part.units.isEmpty ? nil : "Units") }

    var isSplit: Bool { current.isSplit }
    func setSplit(_ split: Bool) { document.setPartSplit(chip.id, split) }

    // MARK: Header (plan-23)

    var headerSize: (rows: Int, perRow: Int)? { current.part.headerSize }

    /// Resizes the header to `rows` rows, when that size is a part.
    func setRows(_ rows: Int) {
        guard let size = headerSize else { return }
        resize(rows: rows, perRow: size.perRow)
    }

    /// Resizes the header to `perRow` pins per row, when that size is a part.
    func setPerRow(_ perRow: Int) {
        guard let size = headerSize else { return }
        resize(rows: size.rows, perRow: perRow)
    }

    private func resize(rows: Int, perRow: Int) {
        guard let part = catalog.part(named: HeaderFamily.name(rows: rows, perRow: perRow)) else { return }
        document.setHeaderSize(chip.id, to: part)
    }

    /// The Pins per row menu: each size and its label (`PartOptionsSection.pinsPerRowLabel`).
    var perRowItems: [(n: Int, label: String)] {
        guard let size = headerSize else { return [] }
        let wired = PartOptionsSection.wiredPins(of: chip.id, in: document)
        return (1...HeaderFamily.maxPerRow).map {
            ($0, PartOptionsSection.pinsPerRowLabel($0, rows: size.rows, wiredPins: wired))
        }
    }

    /// The Gender choices; empty when the part has only one.
    var genders: [String] { current.part.genders.count > 1 ? current.part.genders : [] }
    var gender: String { current.gender ?? current.part.genders.first ?? "" }
    func setGender(_ gender: String) { document.setPartGender(chip.id, gender) }

    // MARK: Color, value

    var colors: [String] { PartOptionsSection.hasColorChoice(current.part) ? current.part.colors : [] }
    var color: String { current.color ?? current.part.colors.first ?? "" }
    func setColor(_ color: String) { document.setPartColor(chip.id, color) }

    /// The resistance the Ohms field shows; nil for a part without one.
    var ohms: Double? { current.part.hasOhmsField ? current.ohms ?? ResistorValue.defaultOhms : nil }
    func setOhms(_ ohms: Double) { document.setPartOhms(chip.id, ohms) }

    /// The capacitance the Capacitance field shows; nil for a part without one.
    var farads: Double? { current.part.isGenericCapacitor ? current.farads ?? CapacitorValue.defaultFarads : nil }
    func setFarads(_ farads: Double) { document.setPartFarads(chip.id, farads) }

    // MARK: Test point (plan-57)

    /// A test point's pad size and whether it is through hole; nil for any other part.
    var testPoint: (size: Double, throughHole: Bool)? {
        guard current.part.isTestPoint else { return nil }
        return TestPointFamily.parse(current.footprintID ?? current.part.footprint?.id ?? "")
            ?? (size: TestPointFamily.defaultSize, throughHole: false)
    }

    func setTestPointSize(_ size: Double) {
        guard let mounting = testPoint else { return }
        document.setPartFootprint(chip.id, TestPointFamily.footprintID(size: size, throughHole: mounting.throughHole))
    }

    func setThroughHole(_ throughHole: Bool) {
        guard let mounting = testPoint else { return }
        document.setPartFootprint(chip.id, TestPointFamily.footprintID(size: mounting.size, throughHole: throughHole))
    }

    // MARK: Footprint

    /// The Footprint menu's choices; empty for a part without, and for a test point (it has Size instead).
    var footprintChoices: [CatalogPart.FootprintChoice] { current.part.isTestPoint ? [] : current.part.footprintChoices }
    var footprintID: String { current.footprintID ?? current.part.footprint?.id ?? "" }
    func setFootprint(_ id: String) { document.setPartFootprint(chip.id, id) }

    // MARK: Socket (plan-51-I)

    var isSocketable: Bool { PartOptionsSection.isSocketable(current.part, in: catalog) }
    var mounting: PartOptionsSection.MountingChoice { .init(current.socket) }
    func setMounting(_ choice: PartOptionsSection.MountingChoice) {
        PartOptionsSection.choose(choice, for: current, in: document, catalog: catalog)
    }

    /// The socket picker's choice: the chip takes `part`, keeping its socket mode; nothing when it has no socket now.
    func useSocket(_ part: CatalogPart) {
        guard let mode = current.socket?.mode, document.chips.contains(where: { $0.id == chip.id }) else { return }
        document.setPartSocket(chip.id, mode: mode, socket: part)
    }
}
