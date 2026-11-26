import CoreGraphics
import Foundation

/// Where the wire ends a sheet value cannot place itself sit: a part's pin
/// tips, bus port pins and net label points (plan-68-D). The document builds
/// one from its parts and its sheet; a test builds one from dictionaries.
struct PinTips {
    var pin: (PinRef) -> CGPoint?
    var busPin: (BusPinRef) -> CGPoint?
    var label: (UUID) -> CGPoint?

    init(pin: @escaping (PinRef) -> CGPoint?, busPin: @escaping (BusPinRef) -> CGPoint?,
         label: @escaping (UUID) -> CGPoint?) {
        (self.pin, self.busPin, self.label) = (pin, busPin, label)
    }

    /// Tips from fixed tables: what a test hands a value.
    init(pins: [PinRef: CGPoint] = [:], busPins: [BusPinRef: CGPoint] = [:], labels: [UUID: CGPoint] = [:]) {
        self.init(pin: { pins[$0] }, busPin: { busPins[$0] }, label: { labels[$0] })
    }
}

/// Every wire's last route, with the wire and the two tips it was worked
/// out from (see `Wiring.path(for:_:)`): a wire is routed again only when
/// it or the place of one of its ends changes. The sheet keeps one; every
/// `Wiring` it hands out shares it.
final class WireRoutes {
    /// Not isolated, as `SchematicDocument`'s: an isolated deinit (this module
    /// defaults to the main actor) frees memory it does not own when the last
    /// `Wiring` holding the cache goes.
    nonisolated deinit {}

    private var kept: [UUID: (wire: Wire, start: CGPoint, end: CGPoint, path: [CGPoint])] = [:]

    /// The route of `wire` between `start` and `end`, `OrthogonalRoute.settled`
    /// on the fly (so a wire looks the same while dragging as after the
    /// drop). `wireCount` bounds the cache: routes of wires that are gone
    /// would pile up.
    func path(for wire: Wire, start: CGPoint, end: CGPoint, wireCount: Int) -> [CGPoint] {
        if let kept = kept[wire.id], kept.start == start, kept.end == end, kept.wire == wire { return kept.path }
        let path = [start] + OrthogonalRoute.settled(start: start, interior: wire.interior, end: end) + [end]
        if kept.count > 2 * wireCount + 16 { kept.removeAll(keepingCapacity: true) }
        kept[wire.id] = (wire, start, end, path)
        return path
    }
}
