import CoreGraphics
import Foundation

/// A net's routed length (plan-38-I § 3): the sum of its traces' segment
/// lengths; a via adds nothing (board thickness isn't modelled). The engine
/// and the document both measure through here, so they agree.
nonisolated enum RoutingLength {
    static func of(segments: [(CGPoint, CGPoint)]) -> Double {
        segments.reduce(0) { $0 + hypot($1.1.x - $1.0.x, $1.1.y - $1.0.y) }
    }

    static func of(_ traces: [RoutedTrace]) -> Double {
        of(segments: traces.flatMap { Array(zip($0.points, $0.points.dropFirst())) })
    }

    /// Net `net`'s length in `copper`: nil unless it has exactly two pins
    /// and all its pads are joined by copper.
    static func of(net: Int, in copper: BoardConnectivity) -> Double? {
        guard copper.pinCount(ofNet: net) == 2 else { return nil }
        let roots = Set(copper.netOfPad.filter { $0.value == net }.keys.map { copper.root[$0] })
        guard roots.count == 1, let root = roots.first else { return nil }
        let segments = copper.traces.indices.filter { copper.root[copper.traceNode($0)] == root }
            .flatMap { Array(zip(copper.traces[$0].points, copper.traces[$0].points.dropFirst())) }
        return of(segments: segments)
    }
}

extension SchematicDocument {
    /// Net `net`'s routed length in mm (`RoutingLength.of(net:in:)`).
    func netLength(ofNet net: Int, in copper: BoardConnectivity) -> Double? {
        RoutingLength.of(net: net, in: copper)
    }
}
