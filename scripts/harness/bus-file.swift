// plan-13-A Phase 4: buses through save and open (file format 3).
//
//   scripts/harness/build.sh scripts/harness/bus-file.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness <saved file> <old v2 file> [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runFileChecks() }

@MainActor func runFileChecks() {
    let args = CommandLine.arguments
    let savedPath = args.count > 1 ? args[1] : "/tmp/p13a.schematical"
    let oldPath = args.count > 2 ? args[2] : "test.schematical"
    let images = args.count > 3 ? args[3] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }

    // An L bus plus a T line on it, 12 wires, B1 renamed D1.
    let doc = Harness.newDocument()
    guard let id = doc.addBus(from: CGPoint(x: 100, y: 100), to: CGPoint(x: 300, y: 200)) else {
        print("FAIL: no bus"); return
    }
    doc.addBus(from: CGPoint(x: 200, y: 100), to: CGPoint(x: 200, y: 40))
    _ = doc.setBusWireCount(id, 12)
    _ = doc.renameBusWire(bus: id, wire: doc.bus(id)!.wires[1].id, to: "D1")
    do {
        try Harness.save(doc, to: savedPath)
        print("saved \(savedPath)")
    } catch {
        print("FAIL: save: \(error)"); return
    }

    print("== reopen")
    do {
        let reopened = try Harness.open(savedPath)
        expect(reopened.buses == doc.buses, "buses identical after reopen (ids, lines, wires)")
        expect(reopened.buses.first?.jointShapes == doc.buses.first?.jointShapes, "same joints")
        reopened.selectedBusID = reopened.buses.first?.id
        Harness.renderCanvas(reopened, to: "\(images)/a4-reopened-canvas.png",
                             size: CGSize(width: 400, height: 260), origin: CGPoint(x: 60, y: 10))
        Harness.renderSidebar(reopened, to: "\(images)/a4-reopened-panel.png", size: CGSize(width: 320, height: 800))
    } catch {
        print("  FAIL: reopen: \(error)"); failures += 1
    }

    print("== version 2 file")
    do {
        let old = try Harness.open(oldPath)
        print("  opened \(oldPath): \(old.chips.count) parts, \(old.wires.count) wires, \(old.buses.count) buses")
        expect(old.chips.count > 0 && old.buses.isEmpty, "opens, no buses")
        Harness.renderCanvas(old, to: "\(images)/a4-v2-canvas.png", size: CGSize(width: 900, height: 600),
                             origin: CGPoint(x: 20, y: 20))
    } catch {
        print("  FAIL: \(error)"); failures += 1
    }

    print("== bad files")
    guard let good = try? JSONSerialization.jsonObject(with: Data(contentsOf: URL(fileURLWithPath: savedPath)))
            as? [String: Any] else { print("FAIL: reread"); return }
    func tryBad(_ name: String, _ change: (inout [String: Any]) -> Void) {
        var json = good
        change(&json)
        let path = "/tmp/p13-bad-\(name).schematical"
        try? JSONSerialization.data(withJSONObject: json).write(to: URL(fileURLWithPath: path))
        do {
            _ = try Harness.open(path)
            print("  FAIL: \(name) opened")
            failures += 1
        } catch {
            print("  \(name): \(error.localizedDescription)")
        }
    }
    func editBus(_ json: inout [String: Any], _ body: (inout [String: Any]) -> Void) {
        var buses = json["buses"] as! [[String: Any]]
        body(&buses[0])
        json["buses"] = buses
    }
    func editLine(_ bus: inout [String: Any], _ index: Int, a: (Double, Double)? = nil, b: (Double, Double)? = nil) {
        var lines = bus["lines"] as! [[String: Any]]
        if let a { lines[index]["a"] = ["x": a.0, "y": a.1] }
        if let b { lines[index]["b"] = ["x": b.0, "y": b.1] }
        bus["lines"] = lines
    }
    let lines = (good["buses"] as! [[String: Any]])[0]["lines"] as! [[String: Any]]
    func point(_ any: Any?) -> (Double, Double) {
        let p = any as! [String: Any]
        return ((p["x"] as! NSNumber).doubleValue, (p["y"] as! NSNumber).doubleValue)
    }
    // Find the T line (vertical, ends at y 40) to reshape it.
    let tIndex = lines.firstIndex { point($0["a"]).1 == 40 || point($0["b"]).1 == 40 }!
    tryBad("nolines") { j in editBus(&j) { $0["lines"] = [] } }
    tryBad("diagonal") { j in editBus(&j) { editLine(&$0, tIndex, a: (200, 100), b: (230, 40)) } }
    tryBad("overlap") { j in editBus(&j) { editLine(&$0, tIndex, a: (150, 100), b: (250, 100)) } }
    tryBad("joint") { j in editBus(&j) { editLine(&$0, tIndex, a: (300, 100), b: (300, 40)) } }
    tryBad("piece") { j in editBus(&j) { editLine(&$0, tIndex, a: (200, 90), b: (200, 40)) } }
    tryBad("wires") { j in editBus(&j) { $0["wires"] = Array(($0["wires"] as! [Any]).prefix(1)) } }
    tryBad("name") { j in editBus(&j) {
        var w = $0["wires"] as! [[String: Any]]; w[0]["name"] = "ADDR0"; $0["wires"] = w } }
    tryBad("twice") { j in editBus(&j) {
        var w = $0["wires"] as! [[String: Any]]; w[0]["name"] = "d1"; $0["wires"] = w } }
    tryBad("touch") { j in
        var buses = j["buses"] as! [[String: Any]]
        buses.append(["id": UUID().uuidString,
                      "lines": [["id": UUID().uuidString, "a": ["x": 150, "y": 100], "b": ["x": 150, "y": 160]]],
                      "wires": [["id": UUID().uuidString, "name": "A"], ["id": UUID().uuidString, "name": "B"]]])
        j["buses"] = buses
    }
    tryBad("dupid") { j in editBus(&j) {
        var w = $0["wires"] as! [[String: Any]]; w[0]["id"] = $0["id"]; $0["wires"] = w } }

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
