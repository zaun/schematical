import CoreGraphics
import Foundation

// A reader for ASCII DXF drawings (plan-65-H): what File ▸ Import ▸ DXF…
// needs of one. Only the header's $INSUNITS and the ENTITIES section are
// read; `DXFLoops` turns the entities into closed loops.

/// Why a DXF could not be read.
nonisolated enum DXFError: Error, Equatable, LocalizedError {
    case binary, noEntities, truncated

    var errorDescription: String? {
        switch self {
        case .binary: "Binary DXF isn't supported; save the drawing as ASCII DXF."
        case .noEntities: "The file isn't a DXF drawing: it has no ENTITIES section."
        case .truncated: "The file isn't a DXF drawing: it ends in the middle of a group."
        }
    }
}

/// The drawing units a DXF says it is in (`$INSUNITS`).
nonisolated enum DXFUnits: Int, CaseIterable, Sendable {
    case inch = 1, foot = 2, millimetre = 4, centimetre = 5, metre = 6

    init?(code: Int?) {
        guard let code, let units = DXFUnits(rawValue: code) else { return nil }
        self = units
    }

    /// One drawing unit in mm.
    var millimetres: Double {
        switch self {
        case .inch: 25.4
        case .foot: 304.8
        case .millimetre: 1
        case .centimetre: 10
        case .metre: 1000
        }
    }

    var title: String {
        switch self {
        case .inch: "in"
        case .foot: "ft"
        case .millimetre: "mm"
        case .centimetre: "cm"
        case .metre: "m"
        }
    }
}

/// A polyline vertex and the bulge of the segment that leaves it
/// (`tan(included angle / 4)`, positive counter-clockwise).
nonisolated struct DXFVertex: Hashable, Sendable {
    var point: CGPoint
    var bulge: Double = 0
}

/// A SPLINE's definition.
nonisolated struct DXFSpline: Hashable, Sendable {
    var degree = 3
    var knots: [Double] = []
    var controls: [CGPoint] = []
    var weights: [Double] = []
    var fit: [CGPoint] = []
    var closed = false
}

/// One drawing entity the reader understands, on its layer, in drawing units.
nonisolated enum DXFEntity: Hashable, Sendable {
    case line(layer: String, from: CGPoint, to: CGPoint)
    /// Degrees, counter-clockwise from `start` to `end`.
    case arc(layer: String, centre: CGPoint, radius: Double, start: Double, end: Double)
    case circle(layer: String, centre: CGPoint, radius: Double)
    case polyline(layer: String, vertices: [DXFVertex], closed: Bool)
    /// `major` is the major axis' end point relative to the centre; `start` and `end` are parameters in radians.
    case ellipse(layer: String, centre: CGPoint, major: CGPoint, ratio: Double, start: Double, end: Double)
    case spline(layer: String, spline: DXFSpline)

    var layer: String {
        switch self {
        case let .line(layer, _, _), let .arc(layer, _, _, _, _), let .circle(layer, _, _), let .polyline(layer, _, _),
             let .ellipse(layer, _, _, _, _, _), let .spline(layer, _):
            layer
        }
    }

    /// The DXF name, for counting.
    var typeName: String {
        switch self {
        case .line: "LINE"
        case .arc: "ARC"
        case .circle: "CIRCLE"
        case .polyline: "POLYLINE"
        case .ellipse: "ELLIPSE"
        case .spline: "SPLINE"
        }
    }
}

/// What was read of a DXF.
nonisolated struct DXFDrawing: Hashable, Sendable {
    /// The header's `$INSUNITS`, if it has one.
    var unitsCode: Int?
    var entities: [DXFEntity] = []
    /// The entity types that were left out (TEXT, INSERT, HATCH, …), by DXF name, with how many.
    var skipped: [String: Int] = [:]

    var units: DXFUnits? { DXFUnits(code: unitsCode) }

    /// The layers that have a supported entity, sorted.
    var layers: [String] { Array(Set(entities.map(\.layer))).sorted() }
}

nonisolated enum DXFReader {
    /// Reads `data` as text (UTF-8, else Latin-1) and parses it.
    static func parse(data: Data) throws -> DXFDrawing {
        if data.starts(with: Array("AutoCAD Binary DXF".utf8)) { throw DXFError.binary }
        let text = String(data: data, encoding: .utf8) ?? String(data: data, encoding: .isoLatin1) ?? ""
        return try parse(text)
    }

    /// Parses an ASCII DXF.
    static func parse(_ text: String) throws -> DXFDrawing {
        if text.hasPrefix("AutoCAD Binary DXF") { throw DXFError.binary }
        var lines = text.components(separatedBy: "\n").map { $0.trimmingCharacters(in: CharacterSet(charactersIn: "\r")) }
        while lines.last?.trimmingCharacters(in: .whitespaces).isEmpty == true { lines.removeLast() }
        guard lines.count % 2 == 0 else { throw DXFError.truncated }
        var groups: [(code: Int, value: String)] = []
        groups.reserveCapacity(lines.count / 2)
        for i in stride(from: 0, to: lines.count, by: 2) {
            guard let code = Int(lines[i].trimmingCharacters(in: .whitespaces)) else { throw DXFError.truncated }
            groups.append((code, lines[i + 1].trimmingCharacters(in: .whitespaces)))
        }

        var drawing = DXFDrawing()
        var sawEntities = false
        var index = 0
        while index < groups.count {
            guard groups[index].code == 0, groups[index].value == "SECTION", index + 1 < groups.count,
                  groups[index + 1].code == 2 else { index += 1; continue }
            let name = groups[index + 1].value
            var end = index + 2
            while end < groups.count, !(groups[end].code == 0 && groups[end].value == "ENDSEC") { end += 1 }
            let body = Array(groups[(index + 2)..<min(end, groups.count)])
            switch name {
            case "HEADER": drawing.unitsCode = headerUnits(body)
            case "ENTITIES":
                sawEntities = true
                readEntities(body, into: &drawing)
            default: break
            }
            index = end + 1
        }
        guard sawEntities else { throw DXFError.noEntities }
        return drawing
    }

    private static func headerUnits(_ body: [(code: Int, value: String)]) -> Int? {
        for (i, group) in body.enumerated() where group.code == 9 && group.value == "$INSUNITS" {
            for next in body[(i + 1)...] {
                if next.code == 9 { break }
                if next.code == 70 { return Int(next.value) }
            }
        }
        return nil
    }

    /// The entities in an ENTITIES body: each starts at a group 0.
    private static func readEntities(_ body: [(code: Int, value: String)], into drawing: inout DXFDrawing) {
        var items: [(type: String, groups: [(code: Int, value: String)])] = []
        for group in body {
            if group.code == 0 { items.append((group.value, [])) } else if !items.isEmpty { items[items.count - 1].groups.append(group) }
        }
        var i = 0
        while i < items.count {
            let item = items[i]
            i += 1
            func skip() { drawing.skipped[item.type, default: 0] += 1 }
            let g = item.groups
            let layer = g.first { $0.code == 8 }?.value ?? "0"
            switch item.type {
            case "LINE":
                guard let a = point(g, 10, 20), let b = point(g, 11, 21) else { skip(); continue }
                drawing.entities.append(.line(layer: layer, from: a, to: b))
            case "CIRCLE":
                guard let c = point(g, 10, 20), let r = number(g, 40) else { skip(); continue }
                drawing.entities.append(.circle(layer: layer, centre: c, radius: r))
            case "ARC":
                guard let c = point(g, 10, 20), let r = number(g, 40), let a = number(g, 50), let b = number(g, 51) else { skip(); continue }
                drawing.entities.append(.arc(layer: layer, centre: c, radius: r, start: a, end: b))
            case "LWPOLYLINE":
                var vertices: [DXFVertex] = []
                var pendingX: Double?
                for group in g {
                    switch group.code {
                    case 10: pendingX = Double(group.value)
                    case 20:
                        if let x = pendingX, let y = Double(group.value) { vertices.append(DXFVertex(point: CGPoint(x: x, y: y))) }
                        pendingX = nil
                    case 42: if !vertices.isEmpty { vertices[vertices.count - 1].bulge = Double(group.value) ?? 0 }
                    default: break
                    }
                }
                let flags = Int(g.first { $0.code == 70 }?.value ?? "") ?? 0
                guard vertices.count >= 2 else { skip(); continue }
                drawing.entities.append(.polyline(layer: layer, vertices: vertices, closed: flags & 1 != 0))
            case "POLYLINE":
                let flags = Int(g.first { $0.code == 70 }?.value ?? "") ?? 0
                var vertices: [DXFVertex] = []
                while i < items.count, items[i].type == "VERTEX" || items[i].type == "SEQEND" {
                    let vertex = items[i]
                    i += 1
                    if vertex.type == "SEQEND" { break }
                    // A mesh or 3D polyline's vertices are not a plan-view outline: flag bits 8, 16 and 64 mark them.
                    if let p = point(vertex.groups, 10, 20) {
                        vertices.append(DXFVertex(point: p, bulge: number(vertex.groups, 42) ?? 0))
                    }
                }
                guard flags & (8 | 16 | 32 | 64) == 0, vertices.count >= 2 else { skip(); continue }
                drawing.entities.append(.polyline(layer: layer, vertices: vertices, closed: flags & 1 != 0))
            case "ELLIPSE":
                guard let c = point(g, 10, 20), let major = point(g, 11, 21), let ratio = number(g, 40) else { skip(); continue }
                drawing.entities.append(.ellipse(layer: layer, centre: c, major: major, ratio: ratio,
                                                 start: number(g, 41) ?? 0, end: number(g, 42) ?? 2 * Double.pi))
            case "SPLINE":
                var spline = DXFSpline()
                spline.degree = Int(g.first { $0.code == 71 }?.value ?? "") ?? 3
                let flags = Int(g.first { $0.code == 70 }?.value ?? "") ?? 0
                spline.closed = flags & 1 != 0
                spline.knots = g.filter { $0.code == 40 }.compactMap { Double($0.value) }
                spline.weights = g.filter { $0.code == 41 }.compactMap { Double($0.value) }
                spline.controls = points(g, 10, 20)
                spline.fit = points(g, 11, 21)
                guard spline.controls.count >= 2 || spline.fit.count >= 2 else { skip(); continue }
                drawing.entities.append(.spline(layer: layer, spline: spline))
            case "VERTEX", "SEQEND":
                break
            default:
                skip()
            }
        }
    }

    private static func number(_ groups: [(code: Int, value: String)], _ code: Int) -> Double? {
        groups.first { $0.code == code }.flatMap { Double($0.value) }
    }

    private static func point(_ groups: [(code: Int, value: String)], _ x: Int, _ y: Int) -> CGPoint? {
        guard let px = number(groups, x), let py = number(groups, y) else { return nil }
        return CGPoint(x: px, y: py)
    }

    /// Every x/y pair in order (a spline's control or fit points).
    private static func points(_ groups: [(code: Int, value: String)], _ x: Int, _ y: Int) -> [CGPoint] {
        var result: [CGPoint] = []
        var pending: Double?
        for group in groups {
            if group.code == x { pending = Double(group.value) } else if group.code == y, let px = pending, let py = Double(group.value) {
                result.append(CGPoint(x: px, y: py))
                pending = nil
            }
        }
        return result
    }
}
