// plan-59 baselines B2 and B3, on the unmodified app sources (base 2d10a029).
//
//   scripts/harness/build.sh <this file> <out> && \
//     <out>/H.app/Contents/MacOS/harness <repo>/examples/6502.schematical <scratch dir>
//
// B3: median of 20 timings of SchematicFile(document:fileURL:) on the file.
// B2: a design with one EEPROM whose load file is <scratch>/A/rom.bin, encoded
//     with fileURL <scratch>/B/design.schematical; prints the stored path.
import Foundation
import QuartzCore

MainActor.assumeIsolated { run() }

@MainActor func run() {
    let args = CommandLine.arguments
    guard args.count >= 3 else { print("usage: harness <6502.schematical> <scratch dir>"); exit(2) }
    let url = URL(fileURLWithPath: args[1])
    let document = try! Harness.open(args[1])
    var times: [Double] = []
    for _ in 0..<20 {
        let start = CACurrentMediaTime()
        _ = SchematicFile(document: document, fileURL: url)
        times.append((CACurrentMediaTime() - start) * 1000)
    }
    times.sort()
    print(String(format: "B3 snapshot ms: median %.3f min %.3f max %.3f (n=20, parts %d)",
                 (times[9] + times[10]) / 2, times[0], times[19], document.chips.count))

    let scratch = URL(fileURLWithPath: args[2])
    let rom = scratch.appendingPathComponent("A/rom.bin")
    let design = scratch.appendingPathComponent("B/design.schematical")
    try! FileManager.default.createDirectory(at: rom.deletingLastPathComponent(), withIntermediateDirectories: true)
    try! FileManager.default.createDirectory(at: design.deletingLastPathComponent(), withIntermediateDirectories: true)
    try! Data([0xDE, 0xAD, 0xBE, 0xEF]).write(to: rom)
    let doc = Harness.newDocument()
    let part = Harness.catalog.allParts.first { $0.kind == PART_EEPROM }!
    doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 200, y: 200))
    doc.setMemoryFile(doc.chips[0].id, MemoryFile(url: rom, bookmark: nil))
    let file = SchematicFile(document: doc, fileURL: design)
    print("B2 part \(part.name) memoryFile.path: \(file.parts[0].memoryFile?.path ?? "nil")")
    let same = SchematicFile(document: doc, fileURL: scratch.appendingPathComponent("A/design.schematical"))
    print("B2 same-folder memoryFile.path: \(same.parts[0].memoryFile?.path ?? "nil")")
    let unsaved = SchematicFile(document: doc, fileURL: nil)
    print("B2 no-URL memoryFile.path: \(unsaved.parts[0].memoryFile?.path ?? "nil")")
}
