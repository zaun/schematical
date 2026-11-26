import CoreGraphics
import Foundation

/// Parts drawn as their units (plan-67, plan-68-F), hits, and the parts'
/// side of a Schematic group drag.
extension PartList {
    /// Draws a part with units as its units, or as one block again. Returns
    /// whether it changed.
    mutating func setSplit(_ id: UUID, _ split: Bool) -> Bool {
        guard let index = index(id), !chips[index].part.units.isEmpty, chips[index].isSplit != split else { return false }
        if split { chips[index].split() } else { chips[index].join() }
        return true
    }

    /// The part `ref` names a unit of, when it is drawn as its units.
    func placementIndex(_ ref: UnitRef) -> Int? {
        guard let index = index(ref.chipID),
              chips[index].unitPlacements.contains(where: { $0.key == ref.key }) else { return nil }
        return index
    }

    /// Moves one unit's body corner to `origin` (snapped to the grid).
    mutating func moveUnit(_ ref: UnitRef, to origin: CGPoint) {
        guard let index = placementIndex(ref) else { return }
        chips[index].updatePlacement(ref.key) { $0.origin = SchematicGrid.snap(origin) }
    }

    /// Mirrors one unit left to right.
    mutating func flipUnit(_ ref: UnitRef) {
        guard let index = placementIndex(ref) else { return }
        chips[index].updatePlacement(ref.key) { $0.isFlipped.toggle() }
    }

    /// Whether unit `ref` turns (a gate, not the block of leftover pins).
    func canRotateUnit(_ ref: UnitRef) -> Bool {
        guard let index = placementIndex(ref) else { return false }
        return chips[index].piece(forUnit: ref.key)?.symbol.canRotate == true
    }

    /// Turns one unit a quarter turn.
    mutating func rotateUnit(_ ref: UnitRef, counterclockwise: Bool) {
        guard let index = placementIndex(ref) else { return }
        chips[index].updatePlacement(ref.key) {
            $0.rotation = PlacedChip.normalized($0.rotation + (counterclockwise ? 270 : 90))
        }
    }

    /// One frame of a Schematic group drag: each part in `origins` and unit
    /// in `units` goes to `shift` of where it was at the press.
    mutating func moveGroup(origins: [UUID: CGPoint], units: [UnitRef: CGPoint], by shift: (CGPoint) -> CGPoint) {
        for index in chips.indices {
            guard let origin = origins[chips[index].id] else { continue }
            let moved = shift(origin)
            if chips[index].origin != moved { chips[index].origin = moved }
        }
        for (ref, origin) in units {
            guard let index = index(ref.chipID) else { continue }
            let moved = shift(origin)
            if chips[index].unitPlacements.first(where: { $0.key == ref.key })?.origin != moved {
                chips[index].updatePlacement(ref.key) { $0.origin = moved }
            }
        }
    }

    /// The part drawn under a point: the last one there.
    func chip(at point: CGPoint) -> PlacedChip? {
        chips.last { $0.piece(at: point) != nil }
    }

    /// What is drawn under a point: a part, or the piece of a part drawn as
    /// its units (`PlacedChip.unitKey` says which).
    func piece(at point: CGPoint) -> PlacedChip? {
        chip(at: point)?.piece(at: point)
    }
}
