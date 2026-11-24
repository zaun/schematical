// plan-51-I: the socket mode of a placed chip, through the real sidebar and sheet. The Mounting control (IC / IC +
// Socket / Socket Only) of the Schematic sidebar, the compatible-sockets dialog, and what a switch does to the
// footprint, the Bill of Materials and undo. Prints one ok/FAIL line per check; exits 1 on any FAIL.
//
//   rm -rf /tmp/p51-harness && APP=/tmp/p51-dd/Build/Products/Debug/schematical.app \
//     scripts/harness/build.sh scripts/harness/socket-mode-ui.swift /tmp/p51-harness && \
//     /tmp/p51-harness/H.app/Contents/MacOS/harness [image dir]
//
// Images (default dir /tmp): p51-socket-sidebar-gate.png, p51-socket-sidebar-socket.png,
// p51-socket-sidebar-led.png and p51-socket-sheet.png (read them).
import CoreGraphics
import Foundation
import SwiftUI

MainActor.assumeIsolated { runSocketModeUIChecks() }

@MainActor func runSocketModeUIChecks() {
    let images = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp"
    var failures = 0
    func check(_ name: String, _ body: () throws -> String?) {
        do {
            if let problem = try body() {
                print("FAIL \(name): \(problem)")
                failures += 1
            } else {
                print("ok   \(name)")
            }
        } catch {
            print("FAIL \(name): \(error.localizedDescription)")
            failures += 1
        }
    }
    let catalog = Harness.catalog
    guard let gatePart = catalog.part(named: "SN74HC00N"), let ledPart = catalog.part(named: "GENERIC-LED"),
          let generic = catalog.part(named: "GENERIC-SOCKET-DIP-14-300"),
          let maker = catalog.part(named: "A 14-LC-TR") else {
        print("FAIL catalog: SN74HC00N, GENERIC-LED or the sockets are missing")
        exit(1)
    }
    let doc = Harness.newDocument()
    doc.placeChip(part: gatePart, symbol: symbol(for: gatePart), centeredAt: CGPoint(x: 200, y: 200))
    let gateID = doc.chips.last!.id
    doc.placeChip(part: ledPart, symbol: symbol(for: ledPart), centeredAt: CGPoint(x: 500, y: 200))
    let ledID = doc.chips.last!.id
    func chip(_ id: UUID) -> PlacedChip { doc.chips.first { $0.id == id }! }
    func bom() -> [String] {
        BillOfMaterials.lines(for: doc.chips).map { "\($0.partNumber) \($0.referenceText)" }
    }
    func image(_ name: String) -> String {
        let path = "\(images)/\(name)"
        try? FileManager.default.removeItem(atPath: path)
        return path
    }

    check("1 the Mounting control applies to the gate and not to the LED") {
        if !PartOptionsSection.applies(to: chip(gateID), in: catalog) { return "no Options for the gate" }
        if !PartOptionsSection.isSocketable(chip(gateID).part, in: catalog) { return "the gate is not socketable" }
        if PartOptionsSection.isSocketable(chip(ledID).part, in: catalog) { return "the LED is socketable" }
        return nil
    }

    check("2 sidebar of the gate in IC mode → \(images)/p51-socket-sidebar-gate.png") {
        doc.select(.chip(gateID))
        let path = image("p51-socket-sidebar-gate.png")
        Harness.renderSidebar(doc, to: path, size: CGSize(width: 320, height: 900))
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    check("3 the LED's sidebar has its own options and no socket row → \(images)/p51-socket-sidebar-led.png") {
        doc.select(.chip(ledID))
        let path = image("p51-socket-sidebar-led.png")
        Harness.renderSidebar(doc, to: path, size: CGSize(width: 320, height: 900))
        if PartFacts.rows(for: chip(ledID)).contains(where: { $0.label == "Socket" }) { return "a Socket row on the LED" }
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    check("4 IC + Socket (the control's own closure): the generic socket, its footprint on the PCB") {
        doc.select(.chip(gateID))
        let pad = chip(gateID).boardPads.first { $0.designator == "1" }!.rect
        PartOptionsSection.choose(.withSocket, for: chip(gateID), in: doc, catalog: catalog)
        let now = chip(gateID)
        if now.socket?.part.name != generic.name || now.socket?.mode != .withSocket {
            return "socket \(now.socket?.part.name ?? "nil") \(String(describing: now.socket?.mode))"
        }
        if now.footprint?.id != "SKT-DIP14-300-TH-MM-110" { return "footprint \(now.footprint?.id ?? "nil")" }
        let moved = now.boardPads.first { $0.designator == "1" }!.rect
        if hypot(moved.midX - pad.midX, moved.midY - pad.midY) > 1e-9 { return "pad 1 moved" }
        if now.boardPads.count != 14 { return "\(now.boardPads.count) pads" }
        let path = image("p51-socket-sidebar-socket.png")
        Harness.renderSidebar(doc, to: path, size: CGSize(width: 320, height: 900))
        print("     sidebar → \(path)")
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    check("5 the Bill of Materials lists the chip and XU1 in IC + Socket") {
        let lines = bom()
        return lines.contains("SN74HC00N U1") && lines.contains("\(generic.name) XU1") ? nil : "\(lines)"
    }

    check("6 the sheet lists the sockets that fit, the generic first; writes the picture → \(images)/p51-socket-sheet.png") {
        let sockets = catalog.sockets(fitting: chip(gateID).part.footprint!)
        let rows = SocketPickerSheet.rows(sockets, search: "", mounting: .all, maker: nil)
        if rows.first?.name != generic.name { return "first row \(rows.first?.name ?? "nil")" }
        if rows.count < 50 { return "only \(rows.count) rows" }
        if !rows.allSatisfy({ $0.fits.contains { $0.id == "PDIP-14" } }) { return "a row that does not fit PDIP-14" }
        let path = image("p51-socket-sheet.png")
        Harness.renderHosted(SocketPickerSheet(catalog: catalog, document: doc, chipID: gateID) { _ in },
                             size: CGSize(width: 820, height: 460), to: path, settle: 0.6)
        return FileManager.default.fileExists(atPath: path) ? nil : "no image"
    }

    check("7 Use Socket (the sheet's closure) switches the chip to a maker socket, keeping the mode") {
        let before = chip(gateID).board
        doc.setPartSocket(gateID, mode: chip(gateID).socket!.mode, socket: maker)
        let now = chip(gateID)
        if now.socket?.part.name != maker.name || now.socket?.mode != .withSocket { return "socket \(now.socket?.part.name ?? "nil")" }
        if now.footprint?.id != "SKT-DIP14-300-TH-AS-ASS-6449" { return "footprint \(now.footprint?.id ?? "nil")" }
        if now.board == nil || before == nil { return "no board placement" }
        return nil
    }

    check("8 Socket Only drops the chip from the Bill of Materials and keeps XU1") {
        PartOptionsSection.choose(.socketOnly, for: chip(gateID), in: doc, catalog: catalog)
        let lines = bom()
        if chip(gateID).socket?.part.name != maker.name { return "the socket changed to \(chip(gateID).socket?.part.name ?? "nil")" }
        return !lines.contains("SN74HC00N U1") && lines.contains("\(maker.name) XU1") ? nil : "\(lines)"
    }

    check("9 one undo steps back a switch, and IC takes the socket off") {
        doc.endCoalescing()
        if !doc.undo(on: .schematic) { return "nothing to undo" }
        if chip(gateID).socket?.mode != .withSocket { return "after undo: \(String(describing: chip(gateID).socket?.mode))" }
        PartOptionsSection.choose(.ic, for: chip(gateID), in: doc, catalog: catalog)
        doc.endCoalescing()
        if chip(gateID).socket != nil || chip(gateID).footprint?.id != "PDIP-14" { return "not back to PDIP-14" }
        if !doc.undo(on: .schematic) || chip(gateID).socket == nil { return "undo of IC did not restore the socket" }
        return nil
    }

    check("10 the sockets and the chip's mode survive a save and an open") {
        let path = "\(NSTemporaryDirectory())p51-socket-harness.schematical"
        try Harness.save(doc, to: path)
        let reopened = try Harness.open(path)
        let gate = reopened.chips.first { $0.part.name == "SN74HC00N" }!
        if gate.socket?.part.name != maker.name || gate.socket?.mode != .withSocket { return "reopened \(String(describing: gate.socket))" }
        return nil
    }

    print(failures == 0 ? "all ok" : "\(failures) FAILED")
    exit(failures == 0 ? 0 : 1)
}
