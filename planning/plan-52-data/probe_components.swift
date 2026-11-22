import CoreGraphics
import Foundation
// Each component from componentsSeparated: one outer loop plus its holes?
func grown(_ p: CGPath, _ c: Double) -> CGPath { p.union(p.copy(strokingWithWidth: 2 * c, lineCap: .round, lineJoin: .round, miterLimit: 1), using: .winding) }
srand48(5)
let board = CGPath(rect: CGRect(x: 0, y: 0, width: 100, height: 80), transform: nil)
let all = CGMutablePath()
for _ in 0..<800 {
    let x = 2 + drand48() * 96, y = 2 + drand48() * 76
    if drand48() < 0.5 { all.addPath(grown(CGPath(roundedRect: CGRect(x: x, y: y, width: 1.6, height: 0.9), cornerWidth: 0.2, cornerHeight: 0.2, transform: nil), 0.2)) }
    else { let m = CGMutablePath(); m.move(to: CGPoint(x: x, y: y)); m.addLine(to: CGPoint(x: x + drand48() * 16 - 8, y: y + drand48() * 16 - 8))
           all.addPath(grown(m.copy(strokingWithWidth: 0.25, lineCap: .round, lineJoin: .round, miterLimit: 1), 0.2)) }
}
let comps = board.subtracting(all, using: .winding).componentsSeparated(using: .winding)
func subpaths(_ p: CGPath) -> [CGPath] {
    var out: [CGPath] = []; var cur = CGMutablePath()
    p.applyWithBlock { e in
        if e.pointee.type == .moveToPoint, !cur.isEmpty { out.append(cur); cur = CGMutablePath() }
        switch e.pointee.type {
        case .moveToPoint: cur.move(to: e.pointee.points[0])
        case .addLineToPoint: cur.addLine(to: e.pointee.points[0])
        case .addQuadCurveToPoint: cur.addQuadCurve(to: e.pointee.points[1], control: e.pointee.points[0])
        case .addCurveToPoint: cur.addCurve(to: e.pointee.points[2], control1: e.pointee.points[0], control2: e.pointee.points[1])
        case .closeSubpath: cur.closeSubpath()
        @unknown default: break
        }
    }
    if !cur.isEmpty { out.append(cur) }
    return out
}
var withHoles = 0, outerBoxContainsAll = 0, total = comps.count, maxSubpaths = 0
for c in comps {
    let subs = subpaths(c)
    maxSubpaths = max(maxSubpaths, subs.count)
    if subs.count > 1 {
        withHoles += 1
        let boxes = subs.map { $0.boundingBoxOfPath }
        let biggest = boxes.max { $0.width * $0.height < $1.width * $1.height }!
        if boxes.allSatisfy({ biggest.insetBy(dx: -1e-6, dy: -1e-6).contains($0) }) { outerBoxContainsAll += 1 }
    }
}
print("components \(total), with more than one loop \(withHoles), of those whose largest loop's box holds every other loop's box \(outerBoxContainsAll), most loops in one component \(maxSubpaths)")
