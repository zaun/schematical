import CoreGraphics
import Foundation

/// What a DXF import makes of the loops (plan-65-H).
nonisolated enum DXFImportMode: String, CaseIterable, Sendable {
    /// The largest outer loop is the board's edge, the loops inside it are cutouts.
    case outlineAndCutouts
    /// Every outer loop is a cutout of the board the document has.
    case cutoutsOnly

    var title: String {
        switch self {
        case .outlineAndCutouts: "Board outline and cutouts"
        case .cutoutsOnly: "Cutouts only"
        }
    }
}

/// Why a loop is left out of an import.
nonisolated enum DXFIgnoredReason: Int, CaseIterable, Comparable, Sendable {
    case crossesAnotherLoop, outsideTheOutline, insideACutout

    static func < (a: DXFIgnoredReason, b: DXFIgnoredReason) -> Bool { a.rawValue < b.rawValue }

    var title: String {
        switch self {
        case .crossesAnotherLoop: "cross another loop"
        case .outsideTheOutline: "outside the outline"
        case .insideACutout: "inside a cutout"
        }
    }
}

nonisolated struct DXFIgnored: Hashable, Sendable {
    var reason: DXFIgnoredReason
    var count: Int
}

/// The loops of a drawing sorted into a board outline, cutouts and ignored ones. Polygons are
/// counter-clockwise in the DXF's y-up frame, mm.
nonisolated struct DXFImportPlan: Hashable, Sendable {
    var outline: [CGPoint]?
    var cutouts: [[CGPoint]]
    var ignored: [DXFIgnored]

    static func make(loops: [DXFLoop], mode: DXFImportMode) -> DXFImportPlan {
        let polygons = loops.map(\.points)
        func segments(_ loop: [CGPoint]) -> [(CGPoint, CGPoint)] { Array(zip(loop, loop.dropFirst() + loop.prefix(1))) }
        let edges = polygons.map(segments)
        // A loop whose edges cross another's takes no part in the nesting.
        var crossing = Set<Int>()
        for i in polygons.indices {
            for j in polygons.indices where j > i {
                let crosses = edges[i].contains { a in edges[j].contains { CopperGeometry.segmentsCross(a.0, a.1, $0.0, $0.1) } }
                if crosses { crossing.insert(i); crossing.insert(j) }
            }
        }
        let live = polygons.indices.filter { !crossing.contains($0) }
        /// The loops in `live` that contain all of loop `i`.
        func containers(of i: Int) -> [Int] {
            live.filter { j in j != i && polygons[i].allSatisfy { BoardEdge.contains($0, in: polygons[j]) } }
        }
        let parents = Dictionary(uniqueKeysWithValues: live.map { ($0, containers(of: $0)) })

        var counts: [DXFIgnoredReason: Int] = [:]
        counts[.crossesAnotherLoop] = crossing.count
        var outline: [CGPoint]?
        var cutouts: [[CGPoint]] = []
        switch mode {
        case .cutoutsOnly:
            for i in live {
                if parents[i]?.isEmpty == true { cutouts.append(polygons[i]) } else { counts[.insideACutout, default: 0] += 1 }
            }
        case .outlineAndCutouts:
            let outer = live.filter { parents[$0]?.isEmpty == true }
            let outlineIndex = outer.max { loops[$0].area < loops[$1].area }
            outline = outlineIndex.map { polygons[$0] }
            for i in live {
                let depth = parents[i]?.count ?? 0
                if i == outlineIndex { continue }
                if depth == 0 {
                    counts[.outsideTheOutline, default: 0] += 1
                } else if depth == 1 {
                    if parents[i]?.first == outlineIndex { cutouts.append(polygons[i]) } else { counts[.outsideTheOutline, default: 0] += 1 }
                } else {
                    counts[.insideACutout, default: 0] += 1
                }
            }
        }
        let ignored = DXFIgnoredReason.allCases.compactMap { reason in
            counts[reason].flatMap { $0 > 0 ? DXFIgnored(reason: reason, count: $0) : nil }
        }
        return DXFImportPlan(outline: outline, cutouts: cutouts, ignored: ignored)
    }
}
