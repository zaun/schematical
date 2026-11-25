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
    /// By node; -1 outside the corridor.
    private(set) var values: [Float] = []
    private var cells: [Int] = []

    var isEmpty: Bool { cells.isEmpty }

    /// Sets every cell within `reach` mm of `copper` — a trace's cells on its own layer, a
    /// via's on every layer — to `deviation` per cell of distance (distance ÷ pitch).
    mutating func make(for copper: (traces: [RoutedTrace], vias: [RoutedVia]), grid: RoutingGrid,
                       reach: Double, deviation: Double) {
        clear()
        let size = grid.cellCount * grid.layers.count
        if values.count != size { values = Array(repeating: -1, count: size) }
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

    /// Puts every cell back to -1.
    mutating func clear() {
        for node in cells { values[node] = -1 }
        cells.removeAll(keepingCapacity: true)
    }
}
