import CoreGraphics
import Foundation

// MARK: - Grid

nonisolated enum SchematicGrid {
    /// Every placed block corner, pin tip and wire bend snaps to this spacing.
    static let spacing: CGFloat = 10

    static func snap(_ value: CGFloat) -> CGFloat {
        (value / spacing).rounded() * spacing
    }

    static func snap(_ point: CGPoint) -> CGPoint {
        CGPoint(x: snap(point.x), y: snap(point.y))
    }
}

// MARK: - Tools

enum SchematicTool: String, CaseIterable, Identifiable {
    case pointer
    case eraser
    case line
    case bus
    case busPort
    case text
    case netLabel
    case vcc
    case ground
    case pullUp
    case pullDown

    var id: String { rawValue }

    /// The toolbar's first group: editing and drawing tools.
    static let drawingTools: [SchematicTool] = [.pointer, .eraser, .line, .bus, .busPort, .text, .netLabel]

    /// The toolbar's second group, after a gap: the supply tools. VCC and
    /// Pull-up come last so they sit beside the supply voltage menu they use.
    static let powerTools: [SchematicTool] = [.ground, .pullDown, .vcc, .pullUp]

    var title: String {
        switch self {
        case .pointer: "Pointer"
        case .eraser: "Eraser"
        case .line: "Line"
        case .bus: "Bus"
        case .busPort: "Bus Port"
        case .text: "Text"
        case .netLabel: "Net Label"
        case .vcc: "VCC"
        case .ground: "GND"
        case .pullUp: "Pull-up"
        case .pullDown: "Pull-down"
        }
    }

    /// The toolbar tooltip: the title, plus the drawing keys for Line and Bus.
    var tooltip: String {
        switch self {
        case .line, .bus: "\(title) — Space: lock a corner · /: flip the bend · Esc: undo a corner"
        case .text: "Text — click to add a text block"
        case .netLabel: "Net Label — click to add a label; labels with the same name are one net"
        default: title
        }
    }

    var symbol: String {
        switch self {
        case .pointer: "cursorarrow"
        case .eraser: "eraser"
        case .line: "line.diagonal"
        case .bus: "line.3.horizontal"
        case .busPort: "rectangle.connected.to.line.below"
        case .text: "textformat"
        case .netLabel: "tag"
        case .vcc: "bolt"
        case .ground: "arrow.down.to.line"
        case .pullUp: "arrow.up.circle"
        case .pullDown: "arrow.down.circle"
        }
    }

    /// The supply placed by this tool, if it places one: `voltage` (a
    /// `SupplyChoice` name) for VCC, GND for ground, `PULLUP-<voltage>` for
    /// Pull-up and PULLDOWN for Pull-down.
    func supplyName(voltage: String) -> String? {
        switch self {
        case .vcc: voltage
        case .ground: "GND"
        case .pullUp: "PULLUP-" + voltage
        case .pullDown: "PULLDOWN"
        default: nil
        }
    }
}

/// The supplies the VCC tool can place, in menu order (library part names),
/// and the names of the rails of any other voltage (the menu's Custom…).
nonisolated enum SupplyChoice {
    static let names = ["5V", "3V3", "2V5", "1V8", "1V2", "12V"]
    static let defaultName = "5V"
    /// The highest rail the backend loads (`supply.c`).
    static let maxVolts = 500.0

    /// The rail name of a voltage, the decimal point written as the `V`:
    /// 170 is "170V", 3.3 "3V3", 1.25 "1V25". Millivolt steps; nil outside
    /// 0 < volts <= `maxVolts`.
    static func name(volts: Double) -> String? {
        guard volts.isFinite else { return nil }
        let mv = Int((volts * 1000).rounded())
        guard mv > 0, mv <= Int(maxVolts * 1000) else { return nil }
        var fraction = String(format: "%03d", mv % 1000)
        while fraction.hasSuffix("0") { fraction.removeLast() }
        return "\(mv / 1000)V\(fraction)"
    }

    /// The voltage a rail name says ("170V", "3V3"), in millivolts; nil for
    /// any other text, and for a name `name(volts:)` would not write.
    static func millivolts(name: String) -> Int? {
        let halves = name.split(separator: "V", maxSplits: 1, omittingEmptySubsequences: false)
        guard halves.count == 2, let whole = Int(halves[0]), halves[0].allSatisfy(\.isNumber),
              halves[1].count <= 3, halves[1].allSatisfy(\.isNumber) else { return nil }
        let fraction = Int(halves[1].padding(toLength: 3, withPad: "0", startingAt: 0)) ?? 0
        let mv = whole * 1000 + fraction
        return SupplyChoice.name(volts: Double(mv) / 1000) == name ? mv : nil
    }

    /// What someone types for a voltage ("170", "3.3", "3,3 V", "170V",
    /// "3V3") as a rail name; nil when it is no voltage in range.
    static func name(typed text: String) -> String? {
        var text = text.trimmingCharacters(in: .whitespaces).uppercased()
        if let mv = millivolts(name: text) { return name(volts: Double(mv) / 1000) }
        if text.hasSuffix("V") { text.removeLast() }
        text = text.trimmingCharacters(in: .whitespaces).replacingOccurrences(of: ",", with: ".")
        guard let volts = Double(text), text.allSatisfy({ $0.isNumber || $0 == "." }) else { return nil }
        return name(volts: volts)
    }
}
