// plan-59 probe F3: each example through the candidate's read path and write
// path, compared as decoded values.
//
// Built from a spike worktree (its sources hold the candidate's document class):
//   APP=<spike .app> scripts/harness/build.sh <this file> <out> && \
//     <out>/H.app/Contents/MacOS/harness <file.schematical>…
//
// G: SchematicDocumentModel.load(_:fileURL:) then encodedSnapshot().
// H: the same two calls on its copy of the helpers (plan-59-C).
// Prints, per file: candidate == original, and candidate == what the base
// path (SchematicFile.apply + SchematicFile(document:fileURL:)) writes.
import Foundation

MainActor.assumeIsolated { run() }

@MainActor func run() {
    let paths = Array(CommandLine.arguments.dropFirst())
    guard !paths.isEmpty else { print("usage: harness <file.schematical>…"); exit(2) }
    var equalOriginal = 0, equalBase = 0
    for path in paths {
        let url = URL(fileURLWithPath: path)
        do {
            let data = try Data(contentsOf: url)
            let original = try SchematicFile.decode(data)
            let model = SchematicDocumentModel()
            try model.load(data, fileURL: url)
            let written = try SchematicFile.decode(model.encodedSnapshot())

            let baseDocument = SchematicDocument(library: PartCatalogStore.shared.library)
            try original.apply(to: baseDocument, catalog: PartCatalogStore.shared, fileURL: url)
            let base = try SchematicFile.decode(SchematicFile(document: baseDocument, fileURL: url).encoded())

            if written == original { equalOriginal += 1 }
            if written == base { equalBase += 1 }
            print("\(url.lastPathComponent): parts \(original.parts.count) "
                + "candidate==original \(written == original) candidate==base \(written == base) "
                + "base==original \(base == original)")
        } catch {
            print("\(url.lastPathComponent): FAILED \(error)")
        }
    }
    print("F3: candidate==original \(equalOriginal) of \(paths.count); candidate==base \(equalBase) of \(paths.count)")
}
