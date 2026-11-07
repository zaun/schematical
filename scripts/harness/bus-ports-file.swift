// plan-13-B Phase 4: ports and port wire ends through save and open.
//
//   scripts/harness/build.sh scripts/harness/bus-ports-file.swift && \
//     /tmp/schematical-harness/H.app/Contents/MacOS/harness [saved file] [image dir]
import CoreGraphics
import Foundation

MainActor.assumeIsolated { runPortFileChecks() }

@MainActor func runPortFileChecks() {
    let args = CommandLine.arguments
    let path = args.count > 1 ? args[1] : "/tmp/p13b.schematical"
    let images = args.count > 2 ? args[2] : "/tmp/p13img"
    var failures = 0
    func expect(_ ok: Bool, _ what: String) {
        print(ok ? "  ok: \(what)" : "  FAIL: \(what)")
        if !ok { failures += 1 }
    }

    do {
        let proof = try BusProof.build()
        try Harness.save(proof.document, to: path)
        print("saved \(path)")

        print("== reopen and run")
        let doc = try Harness.open(path)
        expect(doc.buses == proof.document.buses, "buses with ports identical after reopen")
        expect(doc.wires == proof.document.wires, "wires (with port ends) identical after reopen")
        let reopened = try BusProof.find(in: doc)
        let off = reopened.ledLevel(on: false)
        let on = reopened.ledLevel(on: true)
        print("  LED off \(off), on \(on)")
        expect(off == 0 && on > 0, "the reopened drawing still lights the LED")
        Harness.renderCanvas(doc, to: "\(images)/b4-reopened-on.png", size: CGSize(width: 720, height: 360),
                             origin: CGPoint(x: -20, y: 40))
        Harness.simulation.stop()
    } catch {
        print("FAIL: \(error)"); failures += 1
    }

    print("== bad files")
    guard let good = try? JSONSerialization.jsonObject(with: Data(contentsOf: URL(fileURLWithPath: path)))
            as? [String: Any] else { print("FAIL: reread"); return }
    func tryBad(_ name: String, _ change: (inout [String: Any]) -> Void) {
        var json = good
        change(&json)
        let bad = "/tmp/p13-bad-\(name).schematical"
        try? JSONSerialization.data(withJSONObject: json).write(to: URL(fileURLWithPath: bad))
        do {
            _ = try Harness.open(bad)
            print("  FAIL: \(name) opened"); failures += 1
        } catch {
            print("  \(name): \(error.localizedDescription)")
        }
    }
    func editPort(_ json: inout [String: Any], _ body: (inout [String: Any]) -> Void) {
        var buses = json["buses"] as! [[String: Any]]
        var ports = buses[0]["ports"] as! [[String: Any]]
        body(&ports[0])
        buses[0]["ports"] = ports
        json["buses"] = buses
    }
    func editEnd(_ json: inout [String: Any], _ body: (inout [String: Any]) -> Void) {
        var wires = json["wires"] as! [[String: Any]]
        for i in wires.indices {
            for key in ["from", "to"] {
                if var end = wires[i][key] as? [String: Any], end["port"] != nil {
                    body(&end)
                    wires[i][key] = end
                    json["wires"] = wires
                    return
                }
            }
        }
    }
    tryBad("port-line") { j in editPort(&j) { $0["line"] = UUID().uuidString } }
    tryBad("port-side") { j in editPort(&j) { $0["side"] = "left" } }
    tryBad("port-wire") { j in editPort(&j) { $0["wires"] = [UUID().uuidString] } }
    // Port 1 is on the line (200,100)–(460,100), which has a T at x 380:
    // start 380 puts its extent 370..390 over the T.
    tryBad("port-fit") { j in editPort(&j) { $0["start"] = 380 } }
    tryBad("port-off") { j in editPort(&j) { $0["start"] = 470 } }
    tryBad("end-port") { j in editEnd(&j) { $0["port"] = UUID().uuidString } }
    tryBad("end-pin") { j in
        let buses = j["buses"] as! [[String: Any]]
        let wires = buses[0]["wires"] as! [[String: Any]]
        editEnd(&j) { $0["wire"] = wires[5]["id"] }
    }

    print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
}
