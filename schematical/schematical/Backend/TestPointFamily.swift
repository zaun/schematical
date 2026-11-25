import Foundation

/// The generic test point (plan-57): `GENERIC-TESTPOINT` has ten footprints,
/// `TESTPOINT-PAD-<d>` (a surface-mount pad) and `TESTPOINT-THT-<d>` (a plated
/// hole), `<d>` a size in mm with one decimal. A placed test point's Options
/// show the size and the mounting as two controls; this maps them to the
/// footprint id the part stores, and back.
nonisolated enum TestPointFamily {
    /// The sizes of the footprints, in mm.
    static let sizes: [Double] = [1.0, 1.5, 2.0, 2.5, 3.0]
    /// The size of the part's own footprint.
    static let defaultSize = 1.5

    /// The footprint id of a size and mounting: `footprintID(size: 1.5,
    /// throughHole: false)` is "TESTPOINT-PAD-1.5".
    static func footprintID(size: Double, throughHole: Bool) -> String {
        "TESTPOINT-\(throughHole ? "THT" : "PAD")-" + String(format: "%.1f", size)
    }

    /// The size and mounting of a footprint id; nil for any other id.
    static func parse(_ footprintID: String) -> (size: Double, throughHole: Bool)? {
        for throughHole in [false, true] {
            let prefix = "TESTPOINT-\(throughHole ? "THT" : "PAD")-"
            guard footprintID.hasPrefix(prefix), let size = Double(footprintID.dropFirst(prefix.count)),
                  sizes.contains(size) else { continue }
            return (size, throughHole)
        }
        return nil
    }

    /// A size's menu title: "1.5 mm".
    static func label(_ size: Double) -> String {
        String(format: "%.1f mm", size)
    }
}
