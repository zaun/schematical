import CoreGraphics
import Foundation

/// A sheet's net labels and the rules for editing them (plan-68-D). Labels
/// with the same name are one net; the voltage check on a rename is the
/// document's (it needs the parts).
struct NetLabels {
    var labels: [NetLabel]

    /// The lowest "N<n>" no label is named, for a new label.
    var nextName: String {
        let taken = Set(labels.map(\.netKey))
        var n = 1
        while taken.contains("N\(n)") { n += 1 }
        return "N\(n)"
    }

    /// Adds a label with its point at `point` (snapped), named `nextName`.
    mutating func add(at point: CGPoint, style: NetLabel.Style) -> NetLabel {
        let label = NetLabel(id: UUID(), name: nextName, style: style, origin: SchematicGrid.snap(point))
        labels.append(label)
        return label
    }

    /// Whether renaming label `id` to `name` joins it with another label's net.
    func joins(_ id: UUID, named name: String) -> Bool {
        var renamed = self
        guard renamed.setName(id, name), let label = renamed.labels.first(where: { $0.id == id }) else { return false }
        return labels.contains { $0.id != id && $0.isNet && $0.netKey == label.netKey }
    }

    /// Sets a label's name (already cleaned). Returns whether the label is there.
    @discardableResult
    mutating func setName(_ id: UUID, _ name: String) -> Bool {
        guard let index = labels.firstIndex(where: { $0.id == id }) else { return false }
        if labels[index].name != name { labels[index].name = name }
        return true
    }

    /// Sets a label's type.
    mutating func setStyle(_ id: UUID, _ style: NetLabel.Style) {
        guard let index = labels.firstIndex(where: { $0.id == id }), labels[index].style != style else { return }
        labels[index].style = style
    }

    /// Turns a label to run the other way from its point.
    mutating func flip(_ id: UUID) {
        guard let index = labels.firstIndex(where: { $0.id == id }) else { return }
        labels[index].isFlipped.toggle()
    }

    /// One frame of a group drag: each label in `origins` goes to `shift` of
    /// where it was at the press.
    mutating func move(_ origins: [UUID: CGPoint], by shift: (CGPoint) -> CGPoint) {
        for index in labels.indices {
            guard let origin = origins[labels[index].id] else { continue }
            let moved = shift(origin)
            if labels[index].origin != moved { labels[index].origin = moved }
        }
    }

    mutating func remove(_ ids: Set<UUID>) {
        labels.removeAll { ids.contains($0.id) }
    }

    /// The label under a point: the last one drawn there.
    func label(at point: CGPoint) -> NetLabel? {
        labels.last { $0.rect.contains(point) }
    }

    /// The label whose point is within `radius` of `point`.
    func tip(at point: CGPoint, radius: CGFloat) -> UUID? {
        labels.last { SchematicDocument.distance(from: point, to: $0.tip) <= radius }?.id
    }

    func tip(_ id: UUID) -> CGPoint? {
        labels.first { $0.id == id }?.tip
    }
}
