import CoreGraphics
import Foundation
let board = CGPath(rect: CGRect(x: 0, y: 0, width: 100, height: 80), transform: nil)
let clearance = 0.2
func grown(_ p: CGPath) -> CGPath {
    p.union(p.copy(strokingWithWidth: 2 * clearance, lineCap: .round, lineJoin: .round, miterLimit: 1), using: .winding)
}
srand48(7)
var obstacles: [CGPath] = []
var raw: [CGPath] = []
for _ in 0..<500 {
    let x = 2 + drand48() * 96, y = 2 + drand48() * 76
    var p: CGPath
    if drand48() < 0.5 {
        p = CGPath(roundedRect: CGRect(x: x, y: y, width: 1.6, height: 0.9), cornerWidth: 0.2, cornerHeight: 0.2, transform: nil)
    } else {
        let m = CGMutablePath(); m.move(to: CGPoint(x: x, y: y)); m.addLine(to: CGPoint(x: x + drand48() * 16 - 8, y: y + drand48() * 16 - 8))
        p = m.copy(strokingWithWidth: 0.25, lineCap: .round, lineJoin: .round, miterLimit: 1)
    }
    raw.append(p); obstacles.append(grown(p))
}
let all = CGMutablePath(); for o in obstacles { all.addPath(o) }
let fill = board.subtracting(all, using: .winding)
var mismatch = 0, total = 200000, inFill = 0
for _ in 0..<total {
    let pt = CGPoint(x: drand48() * 100, y: drand48() * 80)
    let expected = !all.contains(pt, using: .winding)
    let got = fill.contains(pt, using: .winding)
    if got { inFill += 1 }
    if expected != got { mismatch += 1 }
}
print("mismatch \(mismatch) of \(total); inFill \(inFill)")
// clearance gap: two pads 1mm apart, with 0.2 grown each; fill between must exist as 0.6 gap
let a = CGPath(rect: CGRect(x: 10, y: 10, width: 2, height: 2), transform: nil)
let b = CGPath(rect: CGRect(x: 13, y: 10, width: 2, height: 2), transform: nil)
let two = CGMutablePath(); two.addPath(grown(a)); two.addPath(grown(b))
let f = board.subtracting(two, using: .winding)
print("gap centre in fill:", f.contains(CGPoint(x: 12.5, y: 11), using: .winding), " within 0.15 of pad:", f.contains(CGPoint(x: 12.15, y: 11), using: .winding))
// island removal & output contour kinds
var curves = 0
fill.applyWithBlock { e in if e.pointee.type == .addCurveToPoint || e.pointee.type == .addQuadCurveToPoint { curves += 1 } }
print("curve elements in result:", curves)
