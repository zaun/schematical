import CoreGraphics
import Foundation
func now() -> Double { Date().timeIntervalSinceReferenceDate }
func grown(_ p: CGPath, _ c: Double) -> CGPath { p.union(p.copy(strokingWithWidth: 2 * c, lineCap: .round, lineJoin: .round, miterLimit: 1), using: .winding) }
func combine(_ ps: [CGPath]) -> CGPath { let m = CGMutablePath(); for p in ps { m.addPath(p) }; return m }
srand48(11)
func rnd(_ a: Double, _ b: Double) -> Double { a + drand48() * (b - a) }
func run(other: Int, same: Int) {
    let rect = CGRect(x: 1, y: 1, width: 98, height: 78)
    let region = CGPath(rect: rect, transform: nil)
    var others: [CGPath] = []
    for _ in 0..<other {
        let x = rnd(2, 98), y = rnd(2, 78)
        if drand48() < 0.5 { others.append(grown(CGPath(roundedRect: CGRect(x: x, y: y, width: 1.6, height: 0.9), cornerWidth: 0.2, cornerHeight: 0.2, transform: nil), 0.2)) }
        else { let m = CGMutablePath(); m.move(to: CGPoint(x: x, y: y)); m.addLine(to: CGPoint(x: x + rnd(-8, 8), y: y + rnd(-8, 8)))
               others.append(grown(m.copy(strokingWithWidth: 0.25, lineCap: .round, lineJoin: .round, miterLimit: 1), 0.2)) }
    }
    var padSamples: [[CGPoint]] = [], rings: [CGPath] = [], spokes: [CGPath] = []
    for _ in 0..<same {
        let x = rnd(3, 96), y = rnd(3, 76)
        let pad = CGPath(roundedRect: CGRect(x: x, y: y, width: 1.6, height: 0.9), cornerWidth: 0.2, cornerHeight: 0.2, transform: nil)
        rings.append(grown(pad, 0.5))
        let cx = x + 0.8, cy = y + 0.45
        padSamples.append([CGPoint(x: cx, y: cy), CGPoint(x: x, y: y), CGPoint(x: x + 1.6, y: y + 0.9)])
        spokes.append(CGPath(rect: CGRect(x: cx - 2.0, y: cy - 0.25, width: 4.0, height: 0.5), transform: nil))
        spokes.append(CGPath(rect: CGRect(x: cx - 0.25, y: cy - 1.5, width: 0.5, height: 3.0), transform: nil))
    }
    let t0 = now()
    let ringCut = combine(rings).subtracting(combine(spokes), using: .winding)
    let t1 = now()
    let cuts = combine(others + [ringCut])
    let fill = region.subtracting(cuts, using: .winding)
    let t2 = now()
    let all = fill.componentsSeparated(using: .winding)
    let t3 = now()
    var kept = 0
    for piece in all {
        let b = piece.boundingBoxOfPath
        if min(b.width, b.height) < 0.2 { continue }
        if padSamples.contains(where: { s in s.contains { b.contains($0) && piece.contains($0, using: .winding) } }) { kept += 1 }
    }
    let t4 = now()
    func ms(_ a: Double, _ b: Double) -> String { String(format: "%.0f", (b - a) * 1000) }
    print("other \(other) same \(same): ringcut \(ms(t0,t1)) fill \(ms(t1,t2)) separate \(ms(t2,t3)) touch \(ms(t3,t4)) TOTAL \(ms(t0,t4)) ms; pieces \(all.count) kept \(kept)")
}
run(other: 300, same: 100)
run(other: 700, same: 300)
run(other: 1500, same: 500)
