// plan-59 fixture for probes F1, F2 and F6: one EEPROM whose load file
// `rom.bin` sits beside the design, its path stored relative.
//
//   scripts/harness/build.sh <this file> <out> && \
//     <out>/H.app/Contents/MacOS/harness <dir>
//
// Writes <dir>/design.schematical and <dir>/rom.bin (256 bytes, 0x00…0xFF).
import Foundation

MainActor.assumeIsolated { run() }

@MainActor func run() {
    let args = CommandLine.arguments
    guard args.count >= 2 else { print("usage: harness <dir>"); exit(2) }
    let dir = URL(fileURLWithPath: args[1])
    try! FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
    let rom = dir.appendingPathComponent("rom.bin")
    let design = dir.appendingPathComponent("design.schematical")
    try! Data((0..<256).map { UInt8($0) }).write(to: rom)
    let doc = Harness.newDocument()
    doc.addStandardNetClasses()
    let part = Harness.catalog.allParts.first { $0.kind == PART_EEPROM }!
    doc.placeChip(part: part, symbol: symbol(for: part), centeredAt: CGPoint(x: 300, y: 300))
    doc.setMemoryFile(doc.chips[0].id, MemoryFile(url: rom, bookmark: nil))
    let file = SchematicFile(document: doc, fileURL: design)
    try! file.encoded().write(to: design)
    print("wrote \(design.path): part \(part.name), memoryFile.path \(file.parts[0].memoryFile?.path ?? "nil"), "
        + "bookmark \(file.parts[0].memoryFile?.bookmark == nil ? "none" : "present")")
}
