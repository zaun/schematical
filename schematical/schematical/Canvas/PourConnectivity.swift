import CoreGraphics
import Foundation
import SwiftUI

/// Pours as copper (plan-52-D): the fills the connectivity joins by, kept
/// between redraws, and the adapter that hands the board's copper to
/// `PourFiller`.
extension SchematicDocument {
    /// Every pad, via and trace of `copper` as the pour fill reads it. A
    /// node's net is its group's (`BoardConnectivity.netOfRoot`).
    func pourCopper(for copper: BoardConnectivity) -> [PourCopper] {
        let netOfRoot = copper.netOfRoot()
        var items: [PourCopper] = []
        for (index, entry) in copper.pads.enumerated() {
            let pad = entry.pad
            if pad.plated {
                let layers: Set<CopperLayer>? = pad.drill > 0 ? nil : [entry.side.copperLayer]
                items.append(.pad(index, rect: pad.rect, net: netOfRoot[copper.root[index]], layers: layers,
                                  shape: PCBCanvasView.outline(of: pad).cgPath))
            } else if let hole = pad.holeRect {
                items.append(.hole(index, rect: hole,
                                   shape: CGPath(roundedRect: hole, cornerWidth: pad.drill / 2, cornerHeight: pad.drill / 2,
                                                 transform: nil)))
            }
        }
        for (t, trace) in copper.traces.enumerated() {
            items.append(.trace(trace.id, points: trace.points, width: trace.widthMM, layer: trace.layer,
                                net: netOfRoot[copper.root[copper.traceNode(t)]]))
        }
        for (v, via) in copper.vias.enumerated() {
            items.append(.via(via.id, centre: via.centre, diameter: via.diameter,
                              net: netOfRoot[copper.root[copper.viaNode(v)]]))
        }
        return items
    }

    /// The fills of the pours, for the build of `copper` (the connectivity
    /// before any pour joins). A plain build computes them, unless a mouse
    /// gesture is open and no pour or keep-out changed in it: then the last
    /// ones stand, so a drag does not pay for a refill each frame
    /// (`endGesture` makes the next build refill). Builds that leave copper
    /// out or add a trace being drawn use the kept fills when they are for
    /// the same pours and keep-outs.
    func pourFills(for copper: BoardConnectivity, plain: Bool) -> [UUID: PourFill] {
        guard !pours.isEmpty else { return [:] }
        let same = keptPourFills.map { $0.pours == pours && $0.keepouts == keepouts } ?? false
        if plain {
            if isGestureOpen, same, let kept = keptPourFills { return kept.fills }
            let fills = computePourFills(for: copper)
            keptPourFills = (copperRevision, pours, keepouts, fills)
            return fills
        }
        if same, let kept = keptPourFills { return kept.fills }
        return computePourFills(for: copper)
    }

    private func computePourFills(for copper: BoardConnectivity) -> [UUID: PourFill] {
        var nets: [UUID: Int] = [:]
        for pour in pours {
            if let name = pour.net, let net = copper.net(of: name) { nets[pour.id] = net }
        }
        return PourFiller.fill(pours: pours, nets: nets, keepouts: keepouts, copper: pourCopper(for: copper),
                               outline: board.outline, edge: board.rules.minCopperToEdge, rules: board.rules)
    }
}
