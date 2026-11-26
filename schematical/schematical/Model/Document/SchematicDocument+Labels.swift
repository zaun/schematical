import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Net labels (Schematic)

    /// The lowest "N<n>" no label is named, for a new label.
    func nextNetLabelName() -> String {
        sheet.labels.nextName
    }

    /// Adds a net label with its point at `point` (snapped), named
    /// `nextNetLabelName()`, and selects it. A loose wire end at that point
    /// joins it.
    @discardableResult
    func addNetLabel(at point: CGPoint, style: NetLabel.Style = .input) -> UUID {
        return edit(.schematic, "Add Net Label") { scope -> UUID in
            let label = sheet.updateLabels(scope) { $0.add(at: point, style: style) }
            select(.label(label.id))
            adoptLooseEnds(labels: [label.id])
            return label.id
        }
    }

    /// Renames a label (`NetLabel.cleanName`). Refused, with the name
    /// kept, when nothing is left of `value`, or when joining the nets would
    /// put a voltage out of range on a part (`editorMessage` says why).
    /// Returns whether the label now has the cleaned name. Typing one name
    /// is one undo step, until `endCoalescing()`.
    @discardableResult
    func setNetLabelName(_ id: UUID, _ value: String) -> Bool {
        edit(.schematic, "Rename Net Label", coalescing: "label-name:\(id)") { scope -> Bool in
            guard let name = NetLabel.cleanName(value),
                  let label = netLabels.first(where: { $0.id == id }) else { return false }
            guard label.name != name else { return true }
            var renamed = sheet.labels
            renamed.setName(id, name)
            // Joining two nets can put a voltage out of range on a part; a name
            // no other label has joins nothing (and leaving a net only
            // removes connections), so there is nothing to check.
            if sheet.labels.joins(id, named: name), refusesLabels(renamed.labels) { return false }
            sheet.updateLabels(scope) { $0 = renamed }
            return true
        }
    }

    /// Whether the labels `after` would put a new voltage out of range on a
    /// part; sets `editorMessage` when they would.
    func refusesLabels(_ after: [NetLabel]) -> Bool {
        guard let library,
              let issue = VoltageCheck.firstNew(before: chips, wires, after: chips, wires, buses: buses,
                                                labels: netLabels, afterLabels: after,
                                                known: currentVoltageIssues(library), library: library) else { return false }
        editorMessage = VoltageCheck.message(issue, chips: chips, edit: .connect)
        return true
    }

    /// Sets a label's type. Refused, with the type kept, when the change
    /// would join nets that put a voltage out of range on a part (a No
    /// Connect turning back into a named label); `editorMessage` says why.
    func setNetLabelStyle(_ id: UUID, _ style: NetLabel.Style) {
        edit(.schematic, "Change Net Label Type") { scope in
            guard let label = netLabels.first(where: { $0.id == id }), label.style != style else { return }
            var changed = sheet.labels
            changed.setStyle(id, style)
            // Only becoming or leaving No Connect changes what joins what.
            if label.isNoConnect != style.isNoConnect, refusesLabels(changed.labels) { return }
            sheet.updateLabels(scope) { $0 = changed }
        }
    }

    /// Turns a label to run the other way from its point. Its wires stay on
    /// the point.
    func flipNetLabel(_ id: UUID) {
        edit(.schematic, "Flip") { scope in
            sheet.updateLabels(scope) { $0.flip(id) }
            settle(scope) { $0 == .label(id) }
        }
    }

    /// Deletes a label; its wires stay, with loose ends at its point.
    func deleteNetLabel(_ id: UUID) {
        edit(.schematic, "Delete") { scope in
            loosen(scope) { $0 == .label(id) }
            sheet.updateLabels(scope) { $0.remove([id]) }
            pruneSelection()
        }
    }

    /// The label under a point: the last one drawn there.
    func netLabel(at point: CGPoint) -> NetLabel? {
        sheet.labels.label(at: point)
    }

    /// The label whose point is within `radius` of `point`.
    func netLabelTip(at point: CGPoint, radius: CGFloat) -> UUID? {
        sheet.labels.tip(at: point, radius: radius)
    }

    func netLabelTip(_ id: UUID) -> CGPoint? {
        sheet.labels.tip(id)
    }
}
