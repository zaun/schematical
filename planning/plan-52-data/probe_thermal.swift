import CoreGraphics
let board = CGPath(rect: CGRect(x: 0, y: 0, width: 100, height: 80), transform: nil)
let pad = CGPath(rect: CGRect(x: 50, y: 40, width: 2, height: 2), transform: nil)
let ring = pad.union(pad.copy(strokingWithWidth: 1.0, lineCap: .round, lineJoin: .round, miterLimit: 1), using: .winding)
let spokes = CGMutablePath()
spokes.addRect(CGRect(x: 48, y: 40.75, width: 6, height: 0.5)); spokes.addRect(CGRect(x: 50.75, y: 38, width: 0.5, height: 6))
let clear = ring.subtracting(spokes, using: .winding)
let fill = board.subtracting(clear, using: .winding).union(pad, using: .winding)
for (n, p) in [("spoke W", CGPoint(x: 49.7, y: 41)), ("gap NW", CGPoint(x: 49.7, y: 40.3)), ("pad", CGPoint(x: 50.2, y: 40.2)), ("outside ring", CGPoint(x: 49.3, y: 41)), ("gap above pad", CGPoint(x: 51.8, y: 39.7))] {
    print(n, fill.contains(p, using: .winding))
}
print("components", fill.componentsSeparated(using: .winding).count)
