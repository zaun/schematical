import Foundation

// Throwaway spike: grid A* router over a dumped board scene. Not product code.

struct Pad {
    var x0, y0, x1, y1: Double
    var drill: Double
    var plated: Bool
    var side: String
    var net: Int
    var round: Bool
    var label: String
    var through: Bool { drill > 0 }
    var cx: Double { (x0 + x1) / 2 }
    var cy: Double { (y0 + y1) / 2 }
    // distance from point to the pad's copper
    func dist(_ px: Double, _ py: Double) -> Double {
        if round {
            let r = min(x1 - x0, y1 - y0) / 2
            let ax0 = x0 + r, ax1 = x1 - r, ay0 = y0 + r, ay1 = y1 - r
            let qx = min(max(px, ax0), ax1), qy = min(max(py, ay0), ay1)
            return max(0, hypot(px - qx, py - qy) - r)
        }
        let qx = min(max(px, x0), x1), qy = min(max(py, y0), y1)
        return hypot(px - qx, py - qy)
    }
}

let args = CommandLine.arguments
let name = args[1]
let pitch = Double(args[2])!
let traceW = Double(args[3])!      // mm
let order = args.count > 4 ? args[4] : "short"
let data = try! Data(contentsOf: URL(fileURLWithPath: "/tmp/route-probe/\(name).json"))
let j = try! JSONSerialization.jsonObject(with: data) as! [String: Any]
let clr = j["minClearance"] as! Double
let edge = j["minEdge"] as! Double
let viaDia = j["minViaDia"] as! Double
let viaDrill = j["minViaDrill"] as! Double
let outline = (j["outline"] as! [[Double]]).map { ($0[0], $0[1]) }
var pads: [Pad] = (j["pads"] as! [[String: Any]]).map { d in
    Pad(x0: d["x0"] as! Double, y0: d["y0"] as! Double, x1: d["x1"] as! Double, y1: d["y1"] as! Double,
        drill: d["drill"] as! Double, plated: d["plated"] as! Bool, side: d["side"] as! String,
        net: d["net"] as! Int, round: d["round"] as! Bool, label: d["label"] as! String)
}
var rootOf: [Int] = (j["pads"] as! [[String: Any]]).map { $0["root"] as! Int }
let layersN = (j["layers"] as! Int) >= 2 ? 2 : 1

var minX = Double.infinity, minY = Double.infinity, maxX = -Double.infinity, maxY = -Double.infinity
for p in outline { minX = min(minX, p.0); minY = min(minY, p.1); maxX = max(maxX, p.0); maxY = max(maxY, p.1) }
let nx = Int((maxX - minX) / pitch) + 1, ny = Int((maxY - minY) / pitch) + 1
print("\(name): grid \(nx)x\(ny) x\(layersN) layers, pitch \(pitch), pads \(pads.count), clr \(clr), edge \(edge), via \(viaDia)/\(viaDrill)")

func inside(_ x: Double, _ y: Double) -> Bool {
    var c = false
    var k = outline.count - 1
    for i in 0..<outline.count {
        let (xi, yi) = outline[i], (xk, yk) = outline[k]
        if (yi > y) != (yk > y), x < (xk - xi) * (y - yi) / (yk - yi) + xi { c.toggle() }
        k = i
    }
    return c
}
func edgeDist(_ x: Double, _ y: Double) -> Double {
    var best = Double.infinity
    for i in 0..<outline.count {
        let a = outline[i], b = outline[(i + 1) % outline.count]
        let dx = b.0 - a.0, dy = b.1 - a.1
        let l2 = dx * dx + dy * dy
        var t = l2 == 0 ? 0 : ((x - a.0) * dx + (y - a.1) * dy) / l2
        t = min(max(t, 0), 1)
        best = min(best, hypot(x - (a.0 + t * dx), y - (a.1 + t * dy)))
    }
    return best
}

// owner grids: trace-bloat and via-bloat. -1 free, -2 blocked, n net
let cellCount = nx * ny
var ownT = [[Int32]](repeating: [Int32](repeating: -1, count: cellCount), count: layersN)
var ownV = ownT
func cellX(_ i: Int) -> Double { minX + Double(i) * pitch }
func cellY(_ i: Int) -> Double { minY + Double(i) * pitch }

func stamp(_ g: inout [Int32], _ c: Int, _ net: Int) {
    let cur = g[c]
    if cur == -1 { g[c] = Int32(net) } else if cur != Int32(net) { g[c] = -2 }
}

let rT = clr + traceW / 2   // pad copper to trace centre
let rV = clr + viaDia / 2
// board boundary
for l in 0..<layersN {
    for iy in 0..<ny { for ix in 0..<nx {
        let x = cellX(ix), y = cellY(iy)
        let c = iy * nx + ix
        if !inside(x, y) || edgeDist(x, y) < edge + traceW / 2 { ownT[l][c] = -2 }
        if !inside(x, y) || edgeDist(x, y) < edge + viaDia / 2 { ownV[l][c] = -2 }
    } }
}
func layerIndex(_ side: String) -> Int { side == "bottom" && layersN > 1 ? 1 : 0 }
for p in pads where p.plated || p.drill > 0 {
    let net = p.net
    let ls = p.through ? Array(0..<layersN) : [layerIndex(p.side)]
    let ix0 = max(0, Int((p.x0 - rV - minX) / pitch)), ix1 = min(nx - 1, Int((p.x1 + rV - minX) / pitch) + 1)
    let iy0 = max(0, Int((p.y0 - rV - minY) / pitch)), iy1 = min(ny - 1, Int((p.y1 + rV - minY) / pitch) + 1)
    if ix0 > ix1 || iy0 > iy1 { continue }
    for l in ls {
        for iy in iy0...iy1 { for ix in ix0...ix1 {
            let d = p.dist(cellX(ix), cellY(iy))
            let c = iy * nx + ix
            // unplated or net-less pads still block (net -3 = nobody's)
            let owner = p.plated ? (net >= 0 ? net : 100000 + Int(p.x0 * 100)) : 100000 + Int(p.x0 * 100)
            if d < rT { stamp(&ownT[l], c, owner) }
            if d < rV { stamp(&ownV[l], c, owner) }
        } }
    }
    // a through drill blocks via/traces on all layers for non-plated
}
// non-plated holes block everything
for p in pads where !p.plated {
    let r = p.drill / 2 + clr
    for l in 0..<layersN { for iy in 0..<ny { for ix in 0..<nx {
        let dx = cellX(ix) - p.cx, dy = cellY(iy) - p.cy
        if hypot(dx, dy) < r + traceW / 2 { ownT[l][iy * nx + ix] = -2 }
        if hypot(dx, dy) < r + viaDia / 2 { ownV[l][iy * nx + ix] = -2 }
    } } }
}

let baseT = ownT, baseV = ownV
var fixedT = ownT.map { $0.map { _ in false } }
for l in 0..<layersN { for c in 0..<cellCount where baseT[l][c] != -1 { fixedT[l][c] = true } }
// nets
var netPads: [Int: [Int]] = [:]
for (i, p) in pads.enumerated() where p.net >= 0 && p.plated { netPads[p.net, default: []].append(i) }
// groups (pads of one pin = same root)
var connections = 0
struct Job { var net: Int; var groups: [[Int]] }
var jobs: [Job] = []
for (net, ps) in netPads {
    var byRoot: [Int: [Int]] = [:]
    for p in ps { byRoot[rootOf[p], default: []].append(p) }
    if byRoot.count > 1 { jobs.append(Job(net: net, groups: Array(byRoot.values))); connections += byRoot.count - 1 }
}
print("nets to route \(jobs.count), connections \(connections)")

func span(_ job: Job) -> Double {
    var a = Double.infinity, b = Double.infinity, c = -Double.infinity, d = -Double.infinity
    for g in job.groups { for p in g { a = min(a, pads[p].cx); b = min(b, pads[p].cy); c = max(c, pads[p].cx); d = max(d, pads[p].cy) } }
    return (c - a) + (d - b)
}
switch order {
case "long": jobs.sort { span($0) > span($1) }
case "fewest": jobs.sort { $0.groups.count < $1.groups.count }
case "big": jobs.sort { $0.groups.count > $1.groups.count }
default: jobs.sort { span($0) < span($1) }
}

let dirs: [(Int, Int, Double)] = [(1,0,1),(-1,0,1),(0,1,1),(0,-1,1),(1,1,1.41421356),(1,-1,1.41421356),(-1,1,1.41421356),(-1,-1,1.41421356)]
let viaCost = 12.0, bendCost = 0.3

struct Heap {
    var a: [(Float, Int32)] = []
    mutating func push(_ f: Float, _ n: Int32) {
        a.append((f, n)); var i = a.count - 1
        while i > 0 { let p = (i - 1) / 2; if a[p].0 <= a[i].0 { break }; a.swapAt(p, i); i = p }
    }
    mutating func pop() -> (Float, Int32)? {
        guard !a.isEmpty else { return nil }
        let top = a[0]; let last = a.removeLast()
        if !a.isEmpty { a[0] = last; var i = 0
            while true { let l = 2*i+1, r = l+1; var m = i
                if l < a.count && a[l].0 < a[m].0 { m = l }
                if r < a.count && a[r].0 < a[m].0 { m = r }
                if m == i { break }; a.swapAt(i, m); i = m } }
        return top
    }
}

var gScore = [Float](repeating: .infinity, count: cellCount * layersN)
var came = [Int32](repeating: -1, count: cellCount * layersN)
var touched: [Int] = []

// A* from a set of source nodes to any node in target set
var relaxed = false
let foreignCost: Float = 25
func search(net: Int, sources: [Int], targets: Set<Int>, tx: Double, ty: Double, maxPops: Int) -> [Int]? {
    for t in touched { gScore[t] = .infinity; came[t] = -1 }
    touched.removeAll(keepingCapacity: true)
    var heap = Heap()
    func h(_ n: Int) -> Float {
        let c = n % cellCount
        let dx = abs(cellX(c % nx) - tx) / pitch, dy = abs(cellY(c / nx) - ty) / pitch
        return Float(max(dx, dy) + 0.41421356 * min(dx, dy))
    }
    for s in sources { gScore[s] = 0; touched.append(s); heap.push(h(s), Int32(s)) }
    var pops = 0
    while let (f, nn) = heap.pop() {
        let n = Int(nn)
        let g = gScore[n]
        if f > g + h(n) + 0.001 { continue }
        pops += 1
        if pops > maxPops { return nil }
        if targets.contains(n) {
            var path = [n]; var cur = n
            while came[cur] >= 0 { cur = Int(came[cur]); path.append(cur) }
            return path.reversed()
        }
        let l = n / cellCount, c = n % cellCount, ix = c % nx, iy = c / nx
        let prev = came[n] >= 0 ? Int(came[n]) : -1
        for (k, d) in dirs.enumerated() {
            let jx = ix + d.0, jy = iy + d.1
            if jx < 0 || jy < 0 || jx >= nx || jy >= ny { continue }
            let cc = jy * nx + jx
            let o = ownT[l][cc]
            var extra: Float = 0
            if o != -1 && o != Int32(net) {
                if relaxed && o != -2 && o < 100000 && !fixedT[l][cc] { extra = foreignCost } else { continue }
            }
            if k >= 4 { // diagonal: both orthogonals must be passable to avoid squeezing corners
                let o1 = ownT[l][iy * nx + jx], o2 = ownT[l][jy * nx + ix]
                if (o1 != -1 && o1 != Int32(net)) || (o2 != -1 && o2 != Int32(net)) { if !relaxed { continue } }
            }
            var step = Float(d.2) + extra
            if prev >= 0 && prev / cellCount == l {
                let pc = prev % cellCount
                let pdx = ix - pc % nx, pdy = iy - pc / nx
                if pdx != d.0 || pdy != d.1 { step += Float(bendCost) }
            }
            let nn2 = l * cellCount + cc
            let ng = g + step
            if ng < gScore[nn2] { if gScore[nn2] == .infinity { touched.append(nn2) }; gScore[nn2] = ng; came[nn2] = Int32(n); heap.push(ng + h(nn2), Int32(nn2)) }
        }
        if layersN > 1 {
            let ol = 1 - l
            let ov = ownV[l][c], ov2 = ownV[ol][c]
            let okv = (ov == -1 || ov == Int32(net)) && (ov2 == -1 || ov2 == Int32(net))
            let okr = relaxed && ov != -2 && ov2 != -2 && ov < 100000 && ov2 < 100000
            if okv || okr {
                let nn2 = ol * cellCount + c
                let ng = g + Float(viaCost) + (okv ? 0 : foreignCost)
                if ng < gScore[nn2] { if gScore[nn2] == .infinity { touched.append(nn2) }; gScore[nn2] = ng; came[nn2] = Int32(n); heap.push(ng + h(nn2), Int32(nn2)) }
            }
        }
    }
    return nil
}

func padNodes(_ p: Pad) -> [Int] {
    var out: [Int] = []
    let ls = p.through ? Array(0..<layersN) : [layerIndex(p.side)]
    let ix0 = max(0, Int(ceil((p.x0 - minX) / pitch))), ix1 = min(nx - 1, Int(floor((p.x1 - minX) / pitch)))
    let iy0 = max(0, Int(ceil((p.y0 - minY) / pitch))), iy1 = min(ny - 1, Int(floor((p.y1 - minY) / pitch)))
    var cells: [Int] = []
    if ix0 <= ix1 && iy0 <= iy1 {
        for iy in iy0...iy1 { for ix in ix0...ix1 where p.dist(cellX(ix), cellY(iy)) == 0 { cells.append(iy * nx + ix) } }
    }
    if cells.isEmpty { // nearest cell to centre
        cells = [min(max(Int(((p.cy - minY) / pitch).rounded()), 0), ny - 1) * nx + min(max(Int(((p.cx - minX) / pitch).rounded()), 0), nx - 1)]
    }
    for l in ls { for c in cells { out.append(l * cellCount + c) } }
    return out
}

func stampPath(_ path: [Int], net: Int) {
    var prevL = -1
    for n in path {
        let l = n / cellCount, c = n % cellCount
        let ix = c % nx, iy = c / nx
        let x = cellX(ix), y = cellY(iy)
        let rr = Int(ceil((clr + traceW) / pitch))
        for dy in -rr...rr { for dx in -rr...rr {
            let jx = ix + dx, jy = iy + dy
            if jx < 0 || jy < 0 || jx >= nx || jy >= ny { continue }
            let d = hypot(Double(dx), Double(dy)) * pitch
            let cc = jy * nx + jx
            if d < clr + traceW { stamp(&ownT[l], cc, net) }
            if d < clr + traceW / 2 + viaDia / 2 { stamp(&ownV[l], cc, net) }
        } }
        if prevL >= 0 && prevL != l { // via: both layers, bigger
            let r2 = Int(ceil((clr + viaDia) / pitch))
            for ll in 0..<layersN { for dy in -r2...r2 { for dx in -r2...r2 {
                let jx = ix + dx, jy = iy + dy
                if jx < 0 || jy < 0 || jx >= nx || jy >= ny { continue }
                let d = hypot(Double(dx), Double(dy)) * pitch
                let cc = jy * nx + jx
                if d < clr + viaDia / 2 + traceW / 2 { stamp(&ownT[ll], cc, net) }
                if d < clr + viaDia { stamp(&ownV[ll], cc, net) }
            } } }
        }
        _ = x; _ = y
        prevL = l
    }
}


struct Conn { var net: Int; var path: [Int] }
var routes: [Int: [[Int]]] = [:]   // net -> paths
var treeOf: [Int: [Int]] = [:]
var jobOf: [Int: Job] = [:]
for j in jobs { jobOf[j.net] = j }

func rebuild() {
    ownT = baseT; ownV = baseV
    for (net, ps) in routes { for p in ps { stampPath(p, net: net) } }
}
// route one net fully; returns nil on success or the failing (sources,targets,tx,ty)
func routeNet(_ job: Job) -> (src: [Int], tgt: Set<Int>, tx: Double, ty: Double)? {
    var tree: [Int] = job.groups[0].flatMap { padNodes(pads[$0]) }
    var remaining = Array(job.groups.dropFirst())
    routes[job.net] = []
    while !remaining.isEmpty {
        var bi = 0, bd = Double.infinity
        for (i, g) in remaining.enumerated() { for p in g { for t in tree {
            let c = t % cellCount
            let d = hypot(cellX(c % nx) - pads[p].cx, cellY(c / nx) - pads[p].cy)
            if d < bd { bd = d; bi = i } } } }
        let g = remaining.remove(at: bi)
        let targets = Set(g.flatMap { padNodes(pads[$0]) })
        let p0 = pads[g[0]]
        relaxed = false
        if let path = search(net: job.net, sources: tree, targets: targets, tx: p0.cx, ty: p0.cy, maxPops: 3_000_000) {
            stampPath(path, net: job.net)
            routes[job.net]!.append(path)
            tree += path; tree += Array(targets)
        } else { return (tree, targets, p0.cx, p0.cy) }
    }
    return nil
}
func unroute(_ net: Int) { routes[net] = nil }

let t0 = Date()
var queue = jobs.map { $0.net }
var ripCount: [Int: Int] = [:]
var ops = 0
let maxOps = 4000
var failedNets = Set<Int>()
var reportPoint = 0
while let net = queue.first, ops < maxOps {
    queue.removeFirst(); ops += 1
    let job = jobOf[net]!
    if let f = routeNet(job) {
        // relaxed search to find blockers
        relaxed = true
        let rp = search(net: net, sources: f.src, targets: f.tgt, tx: f.tx, ty: f.ty, maxPops: 3_000_000)
        relaxed = false
        guard let path = rp else { failedNets.insert(net); unroute(net); rebuild(); continue }
        // victims: nets with path cells near relaxed path
        var cellNet: [Int: Int] = [:]
        for (n, ps) in routes where n != net { for p in ps { for node in p { cellNet[node] = n } } }
        var victims = Set<Int>()
        let rr = Int(ceil((clr + traceW + viaDia) / pitch)) + 1
        for node in path {
            let l = node / cellCount, c = node % cellCount, ix = c % nx, iy = c / nx
            for dy in -rr...rr { for dx in -rr...rr {
                let jx = ix + dx, jy = iy + dy
                if jx < 0 || jy < 0 || jx >= nx || jy >= ny { continue }
                if let n = cellNet[l * cellCount + jy * nx + jx] ?? cellNet[(1 - l) * cellCount + jy * nx + jx] { 
                    if hypot(Double(dx), Double(dy)) * pitch < clr + viaDia { victims.insert(n) } }
            } }
        }
        unroute(net)
        for v in victims { unroute(v); ripCount[v, default: 0] += 1 }
        // limit thrashing
        let victimsOK = victims.filter { ripCount[$0, default: 0] < 12 }
        if victimsOK.count != victims.count { failedNets.insert(net); rebuild(); queue.append(contentsOf: victims.filter { !victimsOK.contains($0) }); continue }
        rebuild()
        // route this net now, then requeue victims
        if routeNet(job) != nil { unroute(net); rebuild(); queue.append(net); ripCount[net, default: 0] += 1; if ripCount[net]! > 12 { failedNets.insert(net); queue.removeAll { $0 == net } } }
        for v in victims.sorted() where !queue.contains(v) { queue.append(v) }
    }
}
let dt = Date().timeIntervalSince(t0)
var routedConn = 0, vias = 0, wire = 0.0
for (net, ps) in routes { routedConn += ps.count; for p in ps { wire += Double(p.count) * pitch; for i in 1..<max(p.count,1) where p[i] / cellCount != p[i-1] / cellCount { vias += 1 } } }
print(String(format: "RIPUP order=%@ routed %d / %d (%.1f%%), failed nets %d, vias %d, wire %.0f mm, ops %d, time %.2f s", order, routedConn, connections, 100 * Double(routedConn) / Double(max(connections, 1)), failedNets.count, vias, wire, ops, dt))
