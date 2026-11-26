import CoreGraphics
import Foundation
import SwiftUI

/// What the Cutout panel's fields do (plan-65-F); the panel calls these, and
/// the tests do too. Each is one named undo step and returns nothing: the
/// field reads the cutout back.
@MainActor
enum CutoutPanelActions {
    static func shape(_ document: SchematicDocument, _ id: UUID) -> BoardCutout.Shape? {
        document.cutouts.first { $0.id == id }?.shape
    }

    /// X: moves the cutout so its bounds' left edge is at `x`.
    static func setX(_ document: SchematicDocument, _ id: UUID, _ x: Double) {
        guard let box = shape(document, id)?.bounds else { return }
        document.moveCutout(id, to: CGPoint(x: x, y: box.minY))
    }

    static func setY(_ document: SchematicDocument, _ id: UUID, _ y: Double) {
        guard let box = shape(document, id)?.bounds else { return }
        document.moveCutout(id, to: CGPoint(x: box.minX, y: y))
    }

    static func setWidth(_ document: SchematicDocument, _ id: UUID, _ width: Double) {
        guard let box = shape(document, id)?.bounds else { return }
        document.setCutoutRect(id, CGRect(x: box.minX, y: box.minY, width: width, height: box.height))
    }

    static func setHeight(_ document: SchematicDocument, _ id: UUID, _ height: Double) {
        guard let box = shape(document, id)?.bounds else { return }
        document.setCutoutRect(id, CGRect(x: box.minX, y: box.minY, width: box.width, height: height))
    }

    static func setRadius(_ document: SchematicDocument, _ id: UUID, _ radius: Double) {
        document.setCutoutRadius(id, radius)
    }

    /// The corner radius of a rounded cutout, mm; nil for a polygon.
    static func radius(_ document: SchematicDocument, _ id: UUID) -> Double? {
        if case let .rounded(_, radius)? = shape(document, id) { return radius }
        return nil
    }
}

/// The inspector for one selected cutout (plan-65-F): position, size and
/// corner radius, and Lock Position. A polygon (from a DXF) has X and Y; its
/// size is shown but not edited.
struct CutoutPanel: View {
    let document: SchematicDocument
    let cutout: BoardCutout

    private var current: BoardCutout { document.cutouts.first { $0.id == cutout.id } ?? cutout }

    var body: some View {
        let units = document.board.units
        let shown = current
        let box = shown.shape.bounds
        Section("Cutout") {
            LabeledContent("Shape") {
                if case .polygon = shown.shape {
                    Text("Polygon, \(shown.shape.loop.count) points")
                } else {
                    Text(shown.kind.title)
                }
            }
            LockToggle(document: document, item: .cutout(cutout.id))
            Group {
                LengthField(title: "X", millimetres: box.minX, units: units, precise: true) {
                    CutoutPanelActions.setX(document, cutout.id, $0)
                    return current.shape.bounds.minX
                }
                LengthField(title: "Y", millimetres: box.minY, units: units, precise: true) {
                    CutoutPanelActions.setY(document, cutout.id, $0)
                    return current.shape.bounds.minY
                }
                if case .rounded = shown.shape {
                    LengthField(title: "Width", millimetres: box.width, units: units, precise: true) {
                        CutoutPanelActions.setWidth(document, cutout.id, $0)
                        return current.shape.bounds.width
                    }
                    LengthField(title: "Height", millimetres: box.height, units: units, precise: true) {
                        CutoutPanelActions.setHeight(document, cutout.id, $0)
                        return current.shape.bounds.height
                    }
                    LengthField(title: "Radius", millimetres: CutoutPanelActions.radius(document, cutout.id) ?? 0,
                                units: units, precise: true) {
                        CutoutPanelActions.setRadius(document, cutout.id, $0)
                        return CutoutPanelActions.radius(document, cutout.id) ?? 0
                    }
                } else {
                    LabeledContent("Width") { Text(LengthField.format(box.width, units: units, precise: true) + " " + units.title) }
                    LabeledContent("Height") { Text(LengthField.format(box.height, units: units, precise: true) + " " + units.title) }
                }
            }
            .disabled(shown.locked)
        }
    }
}
