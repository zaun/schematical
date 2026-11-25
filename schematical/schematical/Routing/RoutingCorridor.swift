import CoreGraphics
import Foundation

/// The places a piece of copper may be laid again when push and shove moves
/// it (plan-66-D § 3): for each node, -1 where the search may not go, else a
/// price that grows with the distance from where the copper was. Built from
/// the piece's own traces and vias, read by `RoutingSearch.search(corridor:)`.
///
/// The array is as long as the grid's nodes (`layer * cells + cell`) and is
/// kept between uses: only the cells set are put back, so making a corridor
/// costs the size of the piece, not of the board.
nonisolated struct RoutingCorridor: Sendable {
    /// What a node holds when nothing was set there: -1 (off limits) for a corridor, 0 for a
    /// field of prices that forbids nothing (`makeAvoiding`).
    let background: Float
    /// By node.
    private(set) var values: [Float] = []
    private var cells: [Int] = []

    init(background: Float = -1) { self.background = background }

    var isEmpty: Bool { cells.isEmpty }

    /// Prices every cell within `radius` mm of `copper`: `penalty` per cell of travel at its
    /// core, less towards the edge, nothing at `radius`. A connection laid under it keeps off the
    /// line that copper had, leaving room for it to be laid again beside.
    mutating func makeAvoiding(_ copper: [(traces: [RoutedTrace], vias: [RoutedVia])], grid: RoutingGrid,
                               radius: Double, penalty: Double) {
        clear()
        let size = grid.cellCount * grid.layers.count
        if values.count != size { values = Array(repeating: background, count: size) }
        for piece in copper {
            func price(_ core: CopperShape.Core, layers: [Int]) {
                let box = CopperShape(core: core, radius: 0, layers: nil, node: 0).bounds
                guard let range = grid.cellRange(box, margin: radius) else { return }
                for y in range.y {
                    for x in range.x {
                        let cell = grid.cell(x, y)
                        let d = RoutingGrid.distance(grid.point(cell), core)
                        guard d < radius else { continue }
                        let value = Float(penalty * (radius - d) / grid.pitch)
                        for layer in layers {
                            let node = layer * grid.cellCount + cell
                            if values[node] == background { cells.append(node) }
                            values[node] = max(values[node], value)
                        }
                    }
                }
            }
            for trace in piece.traces {
                guard let layer = grid.layerIndex(trace.layer) else { continue }
                for (a, b) in zip(trace.points, trace.points.dropFirst()) { price(.segment(a, b), layers: [layer]) }
            }
            for via in piece.vias { price(.segment(via.centre, via.centre), layers: Array(grid.layers.indices)) }
        }
    }

    /// Sets every cell within `reach` mm of `copper` — a trace's cells on its own layer, a
    /// via's on every layer — to `deviation` per cell of distance (distance ÷ pitch).
    mutating func make(for copper: (traces: [RoutedTrace], vias: [RoutedVia]), grid: RoutingGrid,
                       reach: Double, deviation: Double) {
        clear()
        let size = grid.cellCount * grid.layers.count
        if values.count != size { values = Array(repeating: background, count: size) }
        let perCell = Float(deviation / grid.pitch)
        func set(_ core: CopperShape.Core, layers: [Int]) {
            let box = CopperShape(core: core, radius: 0, layers: nil, node: 0).bounds
            guard let range = grid.cellRange(box, margin: reach) else { return }
            for y in range.y {
                for x in range.x {
                    let cell = grid.cell(x, y)
                    let d = RoutingGrid.distance(grid.point(cell), core)
                    guard d <= reach else { continue }
                    let price = Float(d) * perCell
                    for layer in layers {
                        let node = layer * grid.cellCount + cell
                        if values[node] < 0 {
                            values[node] = price
                            cells.append(node)
                        } else if price < values[node] {
                            values[node] = price
                        }
                    }
                }
            }
        }
        for trace in copper.traces {
            guard let layer = grid.layerIndex(trace.layer) else { continue }
            for (a, b) in zip(trace.points, trace.points.dropFirst()) { set(.segment(a, b), layers: [layer]) }
        }
        for via in copper.vias { set(.segment(via.centre, via.centre), layers: Array(grid.layers.indices)) }
    }

    /// Takes out of the corridor every node a piece laid this transaction keeps a trace of the net
    /// being laid out of: a node whose centre is closer to a trace than `gap + its half width + reach`
    /// (a via: `gap + its radius + reach`). A push may not push what it has just laid.
    mutating func forbid(_ copper: [(traces: [RoutedTrace], vias: [RoutedVia], gap: Double)], grid: RoutingGrid, reach: Double) {
        guard values.count == grid.cellCount * grid.layers.count else { return }
        for piece in copper {
            func take(_ core: CopperShape.Core, radius: Double, layers: [Int]) {
                let box = CopperShape(core: core, radius: 0, layers: nil, node: 0).bounds
                guard let range = grid.cellRange(box, margin: radius) else { return }
                for y in range.y {
                    for x in range.x {
                        let cell = grid.cell(x, y)
                        guard RoutingGrid.distance(grid.point(cell), core) < radius else { continue }
                        for layer in layers {
                            let node = layer * grid.cellCount + cell
                            if values[node] != -1 { cells.append(node) }
                            values[node] = -1
                        }
                    }
                }
            }
            for trace in piece.traces {
                guard let layer = grid.layerIndex(trace.layer) else { continue }
                for (a, b) in zip(trace.points, trace.points.dropFirst()) {
                    take(.segment(a, b), radius: piece.gap + trace.width / 2 + reach, layers: [layer])
                }
            }
            for via in piece.vias {
                take(.segment(via.centre, via.centre), radius: piece.gap + via.diameter / 2 + reach, layers: Array(grid.layers.indices))
            }
        }
    }

    /// Puts every cell back to the background.
    mutating func clear() {
        for node in cells { values[node] = background }
        cells.removeAll(keepingCapacity: true)
    }
}
