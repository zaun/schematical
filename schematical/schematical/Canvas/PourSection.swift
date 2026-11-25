import CoreGraphics
import Foundation
import SwiftUI

/// The eight handles of a selected pour or keep-out (plan-52-F): the four
/// corners and the four edge midpoints. A handle moves the edge or corner it
/// names; the opposite edges stay.
nonisolated enum PourHandle: CaseIterable, Hashable, Sendable {
    case topLeft, top, topRight, right, bottomRight, bottom, bottomLeft, left

    /// Where the handle sits on `rect`.
    func position(in rect: CGRect) -> CGPoint {
        switch self {
        case .topLeft: CGPoint(x: rect.minX, y: rect.minY)
        case .top: CGPoint(x: rect.midX, y: rect.minY)
        case .topRight: CGPoint(x: rect.maxX, y: rect.minY)
        case .right: CGPoint(x: rect.maxX, y: rect.midY)
        case .bottomRight: CGPoint(x: rect.maxX, y: rect.maxY)
        case .bottom: CGPoint(x: rect.midX, y: rect.maxY)
        case .bottomLeft: CGPoint(x: rect.minX, y: rect.maxY)
        case .left: CGPoint(x: rect.minX, y: rect.midY)
        }
    }

    private var movesLeft: Bool { self == .topLeft || self == .left || self == .bottomLeft }
    private var movesRight: Bool { self == .topRight || self == .right || self == .bottomRight }
    private var movesTop: Bool { self == .topLeft || self == .top || self == .topRight }
    private var movesBottom: Bool { self == .bottomLeft || self == .bottom || self == .bottomRight }

    /// `rect` with the edges this handle names moved to `point`, each side
    /// kept at least `BoardPour.minimumSide`, never inverted.
    func resized(_ rect: CGRect, to point: CGPoint) -> CGRect {
        let least = CGFloat(BoardPour.minimumSide)
        var minX = rect.minX, maxX = rect.maxX, minY = rect.minY, maxY = rect.maxY
        if movesLeft { minX = min(point.x, maxX - least) }
        if movesRight { maxX = max(point.x, minX + least) }
        if movesTop { minY = min(point.y, maxY - least) }
        if movesBottom { maxY = max(point.y, minY + least) }
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }
}

extension SchematicDocument {
    /// The sole selected pour or keep-out, if it is not locked.
    private var resizableItem: (item: BoardItem, rect: CGRect)? {
        switch boardSelection.sole {
        case let .pour(id)?:
            guard let pour = pours.first(where: { $0.id == id }), !pour.locked else { return nil }
            return (.pour(id), pour.rect)
        case let .keepout(id)?:
            guard let keepout = keepouts.first(where: { $0.id == id }), !keepout.locked else { return nil }
            return (.keepout(id), keepout.rect)
        case let .cutout(id)?:
            // A rounded cutout has handles; a polygon moves and deletes whole (plan-65-E).
            guard let cutout = cutouts.first(where: { $0.id == id }), !cutout.locked,
                  case let .rounded(rect, _) = cutout.shape else { return nil }
            return (.cutout(id), rect)
        default:
            return nil
        }
    }

    /// The rectangle of the sole selected pour or keep-out, for drawing its handles; nil when it can't be resized.
    var resizableRect: CGRect? { resizableItem?.rect }

    /// The handle of the sole selected, unlocked pour or keep-out within
    /// `tolerance` mm of `point`.
    func pourHandle(at point: CGPoint, tolerance: CGFloat) -> (item: BoardItem, handle: PourHandle)? {
        guard let target = resizableItem else { return nil }
        let nearest = PourHandle.allCases
            .map { (handle: $0, distance: Self.distance(from: point, to: $0.position(in: target.rect))) }
            .filter { $0.distance <= tolerance }
            .min { $0.distance < $1.distance }
        return nearest.map { (target.item, $0.handle) }
    }

    /// One frame of a resize drag: the edges `handle` names go to `point`, snapped to the trace grid.
    func resizePourItem(_ item: BoardItem, handle: PourHandle, to point: CGPoint) {
        let snapped = BoardGrid.snap(point, spacing: board.traceGrid)
        switch item {
        case let .pour(id):
            guard let rect = pours.first(where: { $0.id == id })?.rect else { return }
            setPourRect(id, handle.resized(rect, to: snapped))
        case let .keepout(id):
            guard let rect = keepouts.first(where: { $0.id == id })?.rect else { return }
            setKeepoutRect(id, handle.resized(rect, to: snapped))
        case let .cutout(id):
            guard case let .rounded(rect, _)? = cutouts.first(where: { $0.id == id })?.shape else { return }
            setCutoutRect(id, handle.resized(rect, to: snapped))
        default:
            break
        }
    }

    /// The pour, when it is the only thing selected on the PCB page.
    var selectedPourID: UUID? {
        if case let .pour(id)? = boardSelection.sole { return id }
        return nil
    }

    /// The keep-out, when it is the only thing selected on the PCB page.
    var selectedKeepoutID: UUID? {
        if case let .keepout(id)? = boardSelection.sole { return id }
        return nil
    }

    /// Sets a pour to the bounds of the board's outline.
    func fitPourToBoard(_ id: UUID) {
        guard let rect = boardOutlineBounds else { return }
        setPourRect(id, rect, name: "Fit to Board")
    }

    func fitKeepoutToBoard(_ id: UUID) {
        guard let rect = boardOutlineBounds else { return }
        setKeepoutRect(id, rect, name: "Fit to Board")
    }

    /// The bounds of the board's outline (mm).
    var boardOutlineBounds: CGRect? {
        let outline = board.outline
        guard let minX = outline.map(\.x).min(), let maxX = outline.map(\.x).max(),
              let minY = outline.map(\.y).min(), let maxY = outline.map(\.y).max() else { return nil }
        return CGRect(x: minX, y: minY, width: maxX - minX, height: maxY - minY)
    }
}

/// What the pour panels show and do that a test can check.
nonisolated enum PourSection {
    /// The layers a pour can be on: the board's trace layers.
    @MainActor
    static func layerChoices(_ board: BoardSettings) -> [CopperLayer] { board.traceLayers }

    /// "Fill: 1 piece, 2 pins joined", or "No fill: choose a net" with no net,
    /// or "No fill: its net is not in the schematic".
    @MainActor
    static func fillSummary(_ pour: BoardPour, copper: BoardConnectivity) -> String {
        guard let name = pour.net else { return "No fill: choose a net" }
        guard copper.net(of: name) != nil else { return "No fill: its net is not in the schematic" }
        let fill = copper.pourFills[pour.id]
        let pieces = fill?.pieces.count ?? 0
        let pins = Set((fill?.joined ?? []).flatMap { $0 }).count
        return "Fill: \(pieces) piece\(pieces == 1 ? "" : "s"), \(pins) pin\(pins == 1 ? "" : "s") joined"
    }

    /// A keep-out's layers after switching `layer` to `on`. `current` nil is
    /// every layer of the board; the result is nil again when all are on.
    @MainActor
    static func setting(_ layer: CopperLayer, on: Bool, in current: Set<CopperLayer>?, board: BoardSettings) -> Set<CopperLayer>? {
        var layers = current ?? Set(board.copperLayers)
        if on { layers.insert(layer) } else { layers.remove(layer) }
        return layers.isSuperset(of: Set(board.copperLayers)) ? nil : layers
    }
}

// MARK: - Panels

/// The selected pour: where it is, its layer and net, how it keeps clear of
/// other nets and joins its own. ⌘⌫ or the Delete tool deletes it.
struct PourPanel: View {
    let document: SchematicDocument
    let pour: BoardPour

    var body: some View {
        let units = document.board.units
        Section("Pour") {
            LockToggle(document: document, item: .pour(pour.id))
            Group {
                LengthField(title: "X", millimetres: pour.rect.minX, units: units, precise: true) {
                    document.setPourRect(pour.id, CGRect(x: $0, y: pour.rect.minY, width: pour.rect.width, height: pour.rect.height))
                    return current.rect.minX
                }
                LengthField(title: "Y", millimetres: pour.rect.minY, units: units, precise: true) {
                    document.setPourRect(pour.id, CGRect(x: pour.rect.minX, y: $0, width: pour.rect.width, height: pour.rect.height))
                    return current.rect.minY
                }
                LengthField(title: "Width", millimetres: pour.rect.width, units: units, precise: true) {
                    document.setPourRect(pour.id, CGRect(x: pour.rect.minX, y: pour.rect.minY, width: $0, height: pour.rect.height))
                    return current.rect.width
                }
                LengthField(title: "Height", millimetres: pour.rect.height, units: units, precise: true) {
                    document.setPourRect(pour.id, CGRect(x: pour.rect.minX, y: pour.rect.minY, width: pour.rect.width, height: $0))
                    return current.rect.height
                }
            }
            .disabled(pour.locked)
            Button("Fit to Board") { document.fitPourToBoard(pour.id) }
                .disabled(pour.locked)
                .help("Make the pour as big as the board; its fill still stays back from the edge")
            Picker("Layer", selection: Binding(get: { pour.layer }, set: { document.setPourLayer(pour.id, $0) })) {
                ForEach(PourSection.layerChoices(document.board), id: \.self) { layer in
                    Text(layer.title).tag(layer)
                }
            }
            netPicker
            Text(PourSection.fillSummary(pour, copper: document.connectivity()))
                .foregroundStyle(.secondary)
        }
        Section("Clearance and Connection") {
            LengthField(title: "Clearance", millimetres: pour.clearance ?? document.board.rules.minClearance, units: units,
                        precise: true) {
                document.setPourClearance(pour.id, $0)
                return current.clearance ?? document.board.rules.minClearance
            }
            .help("The gap to copper on other nets; the DRC's minimum unless set")
            Picker("Connection", selection: Binding(get: { pour.connection }, set: { document.setPourConnection(pour.id, $0) })) {
                ForEach(PourConnection.allCases, id: \.self) { Text($0.title).tag($0) }
            }
            .help("How the pour joins the pads of its net: a ring with four spokes, or solid up to the pad")
            if pour.connection == .thermal {
                LengthField(title: "Thermal Gap", millimetres: pour.thermalGap, units: units, precise: true) {
                    document.setPourThermalGap(pour.id, $0)
                    return current.thermalGap
                }
                .help("The ring cut round each pad of the net")
                LengthField(title: "Spoke Width", millimetres: pour.spokeWidth, units: units, precise: true) {
                    document.setPourSpokeWidth(pour.id, $0)
                    return current.spokeWidth
                }
                .help("The width of the four spokes left across the ring")
            }
            LengthField(title: "Minimum Width", millimetres: pour.minWidth, units: units, precise: true) {
                document.setPourMinWidth(pour.id, $0)
                return current.minWidth
            }
            .help("A piece of fill narrower than this is left out")
        }
    }

    /// The net: None, GND, a power rail or a signal net. A net the schematic
    /// no longer has stays listed, as "Missing net", until another is chosen.
    private var netPicker: some View {
        let choices = document.planeNetChoices()
        let missing = pour.net.map { net in !choices.contains { $0.net == net } } ?? false
        return Picker("Net", selection: Binding(get: { pour.net }, set: { document.setPourNet(pour.id, $0) })) {
            Text("None").tag(PlaneNet?.none)
            if missing, let net = pour.net {
                Text("Missing net").tag(Optional(net))
            }
            ForEach(choices, id: \.net) { choice in
                Text(choice.title).tag(Optional(choice.net))
            }
        }
        .help("The net the pour carries")
    }

    /// The pour as the document now has it, after an edit clamped it.
    private var current: BoardPour { document.pours.first { $0.id == pour.id } ?? pour }
}

/// The selected keep-out: where it is and the layers it applies to.
struct KeepoutPanel: View {
    let document: SchematicDocument
    let keepout: BoardKeepout

    var body: some View {
        let units = document.board.units
        Section("Keep-out") {
            LockToggle(document: document, item: .keepout(keepout.id))
            Group {
                LengthField(title: "X", millimetres: keepout.rect.minX, units: units, precise: true) {
                    document.setKeepoutRect(keepout.id, CGRect(x: $0, y: keepout.rect.minY, width: keepout.rect.width,
                                                               height: keepout.rect.height))
                    return current.rect.minX
                }
                LengthField(title: "Y", millimetres: keepout.rect.minY, units: units, precise: true) {
                    document.setKeepoutRect(keepout.id, CGRect(x: keepout.rect.minX, y: $0, width: keepout.rect.width,
                                                               height: keepout.rect.height))
                    return current.rect.minY
                }
                LengthField(title: "Width", millimetres: keepout.rect.width, units: units, precise: true) {
                    document.setKeepoutRect(keepout.id, CGRect(x: keepout.rect.minX, y: keepout.rect.minY, width: $0,
                                                               height: keepout.rect.height))
                    return current.rect.width
                }
                LengthField(title: "Height", millimetres: keepout.rect.height, units: units, precise: true) {
                    document.setKeepoutRect(keepout.id, CGRect(x: keepout.rect.minX, y: keepout.rect.minY,
                                                               width: keepout.rect.width, height: $0))
                    return current.rect.height
                }
            }
            .disabled(keepout.locked)
            Button("Fit to Board") { document.fitKeepoutToBoard(keepout.id) }
                .disabled(keepout.locked)
        }
        Section("Layers") {
            Toggle("All layers", isOn: Binding(
                get: { keepout.layers == nil },
                set: { document.setKeepoutLayers(keepout.id, $0 ? nil : Set(document.board.copperLayers)) }))
            .help("Pours on every copper layer stay out")
            ForEach(document.board.copperLayers, id: \.self) { layer in
                Toggle(layer.title, isOn: Binding(
                    get: { keepout.applies(to: layer) },
                    set: {
                        document.setKeepoutLayers(keepout.id, PourSection.setting(layer, on: $0, in: keepout.layers,
                                                                                   board: document.board))
                    }))
            }
        }
    }

    private var current: BoardKeepout { document.keepouts.first { $0.id == keepout.id } ?? keepout }
}
