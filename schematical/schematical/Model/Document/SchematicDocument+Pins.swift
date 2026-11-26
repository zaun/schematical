import CoreGraphics
import Foundation
import Observation

extension SchematicDocument {
    // MARK: Pins

    /// Where a wire leaves or enters a pin: the end of its pin stub.
    func pinTip(_ ref: PinRef) -> CGPoint? {
        guard let position = chipIndex[ref.chipID] else { return nil }
        let chip = chips[position]
        guard let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return nil }
        return chip.tip(of: slot)
    }

    /// Per chip, the pins (`ChipPin.index`) a wire ends on. The schematic
    /// leaves out those pins' numbers.
    var connectedPins: [UUID: Set<Int>] {
        sheet.wiring.connectedPins
    }

    /// Tip position of every pin on every placed chip and bus port.
    var pinTips: [CGPoint] {
        chips.flatMap { chip in
            chip.layout.slots.compactMap { slot in
                pinTip(PinRef(chipID: chip.id, index: slot.pin.index))
            }
        } + busPinRefs.compactMap { busPinTip($0) } + netLabels.map(\.tip)
    }

    /// Every pin on every bus port.
    var busPinRefs: [BusPinRef] {
        sheet.busNetwork.busPinRefs
    }

    /// Where a wire meets a bus port pin; nil when the port is gone or the
    /// wire isn't ticked on it.
    func busPinTip(_ ref: BusPinRef) -> CGPoint? {
        sheet.busNetwork.busPinTip(ref)
    }

    /// Where the sheet's wire ends on part pins, bus port pins and net
    /// labels are now, for the sheet's values (plan-68-D).
    var tips: PinTips {
        PinTips(pin: { [self] in pinTip($0) }, busPin: { [self] in busPinTip($0) }, label: { [self] in netLabelTip($0) })
    }

    /// The bus port pin whose tip is within `radius` of `point`.
    func busPin(at point: CGPoint, radius: CGFloat = 8) -> BusPinRef? {
        busPinRefs.reversed().first { ref in
            busPinTip(ref).map { Self.distance(from: point, to: $0) <= radius } ?? false
        }
    }

    /// The pin whose tip is within `radius` of `point`.
    func pin(at point: CGPoint, radius: CGFloat = 8) -> PinRef? {
        for chip in chips.reversed() {
            for slot in chip.layout.slots {
                let ref = PinRef(chipID: chip.id, index: slot.pin.index)
                if let tip = pinTip(ref), Self.distance(from: point, to: tip) <= radius {
                    return ref
                }
            }
        }
        return nil
    }

    /// Where a wire leaves or enters one of its ends.
    func endTip(_ end: WireEnd) -> CGPoint? {
        sheet.wiring.endTip(end, tips)
    }

    /// The point a line would start or end at.
    func anchorTip(_ anchor: LineAnchor) -> CGPoint? {
        sheet.wiring.anchorTip(anchor, tips)
    }

    /// The axis a wire must leave or enter `anchor` along: sideways for a
    /// pin on a part's left or right, up/down for one on its top or bottom,
    /// and across the line for a bus port pin. Nil for a point on a wire.
    func anchorAxis(_ anchor: LineAnchor) -> RouteAxis? {
        switch anchor {
        case let .pin(ref):
            guard let chip = chips.first(where: { $0.id == ref.chipID }),
                  let slot = chip.layout.slots.first(where: { $0.pin.index == ref.index }) else { return nil }
            switch slot.side {
            case .left, .right: return .horizontal
            case .top, .bottom: return .vertical
            }
        case let .busPin(ref):
            guard let (bus, port) = port(ref.portID), let line = bus.line(port.line) else { return nil }
            return line.isHorizontal ? .vertical : .horizontal
        case .label:
            return .horizontal
        case .point:
            return nil
        }
    }

    /// What a line can start or end on near `point`: a part pin if one is
    /// within `pinRadius`, then a bus port pin, otherwise the grid point
    /// nearest `point` when an existing wire passes through it and it is
    /// within `gridRadius`. Never a point on a bus line.
    func lineAnchor(at point: CGPoint, pinRadius: CGFloat, gridRadius: CGFloat) -> LineAnchor? {
        if let ref = pin(at: point, radius: pinRadius) {
            return .pin(ref)
        }
        if let ref = busPin(at: point, radius: pinRadius) {
            return .busPin(ref)
        }
        if let id = netLabelTip(at: point, radius: pinRadius) {
            return .label(id)
        }
        let grid = SchematicGrid.snap(point)
        guard Self.distance(from: point, to: grid) <= gridRadius,
              segmentHit(at: grid, tolerance: 0.5) != nil else {
            return nil
        }
        return .point(grid)
    }

    // MARK: Geometry

    nonisolated static func distance(from a: CGPoint, to b: CGPoint) -> CGFloat {
        hypot(a.x - b.x, a.y - b.y)
    }

    nonisolated static func distance(from point: CGPoint, toSegment a: CGPoint, _ b: CGPoint) -> CGFloat {
        let dx = b.x - a.x
        let dy = b.y - a.y
        let lengthSquared = dx * dx + dy * dy

        guard lengthSquared > 0 else {
            return distance(from: point, to: a)
        }

        let t = min(max(((point.x - a.x) * dx + (point.y - a.y) * dy) / lengthSquared, 0), 1)
        return distance(from: point, to: CGPoint(x: a.x + t * dx, y: a.y + t * dy))
    }
}
