import CoreGraphics
import Foundation

/// The Gerber job file (`<base>-job.gbrjob`): JSON a fab reads for the board's layer stack, thickness, finish,
/// design rules and the role of each file, written as Ucamco's job-file schema (draft-07, revision 2023.06) lays
/// it out. Deterministic: no creation date, keys sorted. See `docs/export.md` § Gerber.
@MainActor
enum GerberJob {
    /// The job file for `document`, listing `files` (the zip's other files).
    static func data(for document: SchematicDocument, base: String, rules: DesignRules, version: String,
                     files: [GerberExport.File]) -> Data {
        let board = document.board
        let stackup = board.stackup
        let layers = board.copperLayers
        let outline = board.outline
        let xs = outline.map(\.x), ys = outline.map(\.y)
        let width = Double((xs.max() ?? 0) - (xs.min() ?? 0)), height = Double((ys.max() ?? 0) - (ys.min() ?? 0))

        var designRules: [[String: Any]] = []
        for place in layers.count > 2 ? ["Outer", "Inner"] : ["Outer"] {
            designRules.append([
                "Layers": place,
                "PadToPad": number(rules.minClearance), "PadToTrack": number(rules.minClearance),
                "TrackToTrack": number(rules.minClearance), "TrackToRegion": number(rules.minClearance),
                "RegionToRegion": number(rules.minClearance), "MinLineWidth": number(rules.minTraceWidth),
                "MinRing": number(rules.minAnnularRing), "MinClearanceToProfile": number(rules.minCopperToEdge),
            ])
        }

        let attributes: [[String: Any]] = files.filter { $0.function != nil }.map { file in
            var entry: [String: Any] = ["Path": file.name]
            if let function = file.function { entry["FileFunction"] = function }
            if let polarity = file.polarity { entry["FilePolarity"] = polarity }
            return entry
        }

        func group(_ side: String, mask: Bool) -> [[String: Any]] {
            let silk: [String: Any] = ["Type": "Legend", "Name": "\(side) Silk Screen"]
            let paste: [String: Any] = ["Type": "SolderPaste", "Name": "\(side) Solder Paste"]
            let solderMask: [String: Any] = ["Type": "SolderMask", "Thickness": number(BoardStackup.maskThickness),
                                             "Name": "\(side) Solder Mask", "Color": board.color.title]
            return mask ? [solderMask, paste, silk] : [silk, paste, solderMask]
        }
        var material: [[String: Any]] = group("Top", mask: false)
        for (index, layer) in layers.enumerated() {
            material.append(["Type": "Copper", "Thickness": number(stackup.copper[index]), "Name": layer.title])
            if stackup.dielectrics.indices.contains(index) {
                let dielectric = stackup.dielectrics[index]
                material.append(["Type": "Dielectric", "Thickness": number(dielectric.thickness), "Material": "FR4",
                                 "DielectricConstant": number(dielectric.er),
                                 "Name": layers.count == 1 ? "Substrate" : "\(layer.title) to \(layers[index + 1].title)"])
            }
        }
        if layers.count > 1 { material += group("Bottom", mask: true) }

        let uuid = KiCad.uuid("gerber/\(base)")
        let job: [String: Any] = [
            "Header": ["GenerationSoftware": ["Vendor": "jgz", "Application": "Schematical", "Version": version]],
            "GeneralSpecs": [
                "ProjectId": ["Name": base, "GUID": uuid, "Revision": "1"],
                "Size": ["X": number(width), "Y": number(height)],
                "LayerNumber": layers.count,
                "BoardThickness": number(stackup.thickness),
                "Finish": stackup.finish.kiCadName,
            ] as [String: Any],
            "DesignRules": designRules,
            "FilesAttributes": attributes,
            "MaterialStackup": material,
        ]
        let data = (try? JSONSerialization.data(withJSONObject: job, options: [.prettyPrinted, .sortedKeys])) ?? Data()
        return data + Data("\n".utf8)
    }

    /// `value` rounded to 6 decimals, so the JSON has no float noise.
    private static func number(_ value: Double) -> NSDecimalNumber { NSDecimalNumber(string: String(format: "%.6f", value)) }
}
