import SwiftUI

// The Simulation menu: Run / Stop (⌥⌘P) and the Speed submenu. The
// simulation is the Schematic page's, so the menu is disabled on the PCB.
// Run is also off while a part that isn't simulated is placed (plan-36),
// with a disabled line under it naming the part.

/// What the focused schematic window's simulation can do.
struct SimulationTarget {
    /// Run can start (or Stop stop): the window shows the Schematic page and
    /// every placed part is simulated.
    var isAvailable: Bool
    /// Why Run is off on the Schematic page ("U1 (ATMEGA328P-PU) is not
    /// simulated"); nil when it isn't, and on the PCB page.
    var blockedReason: String?
    var isRunning: Bool
    /// Runs it when stopped, stops it when running.
    var toggle: () -> Void
}


/// The Speed submenu's choices: circuit time per real time, from real time
/// down to 1/1,000,000.
enum SimulationSpeed {
    static let choices: [(label: String, value: Double)] = [
        ("1×", 1), ("1/10", 1e-1), ("1/100", 1e-2), ("1/1,000", 1e-3),
        ("1/10,000", 1e-4), ("1/100,000", 1e-5), ("1/1,000,000", 1e-6),
    ]

    /// The choice nearest `speed`, so a stored value that drifted still
    /// shows a checkmark; 1× when there is none.
    static func choice(nearest speed: Double) -> Double {
        choices.min { abs(log10($0.value) - log10(speed)) < abs(log10($1.value) - log10(speed)) }?.value ?? 1
    }
}

struct SimulationCommands: Commands {
    @FocusedValue(\.schematicEditor) private var editor

    var body: some Commands {
        CommandGroup(replacing: .printItem) {}
        CommandMenu("Simulation") {
            SimulationMenuItems(editor: editor)
        }
    }
}

/// A view, so Run / Stop and the reason under it are rendered again when the
/// page, the parts or the run they read changes (`EditorMenus`).
private struct SimulationMenuItems: View {
    let editor: SchematicEditorModel?
    /// An app preference; each schematic window follows it.
    @AppStorage("simulationSpeed") private var speed = 1.0

    var body: some View {
        let target = editor?.menus.simulation?()
        Group {
            Button(target?.isRunning == true ? "Stop" : "Run") { target?.toggle() }
                // ⌘P is File › Print… (`SchematicFileCommands`).
                .keyboardShortcut("p", modifiers: [.command, .option])
                .disabled(target?.isAvailable != true && target?.isRunning != true)

            if let reason = target?.blockedReason {
                Text(reason)
            }

            Picker("Speed", selection: Binding(
                get: { SimulationSpeed.choice(nearest: speed) },
                set: { speed = $0 }
            )) {
                ForEach(SimulationSpeed.choices, id: \.value) { choice in
                    Text(choice.label).tag(choice.value)
                }
            }
            .pickerStyle(.menu)
            // The Schematic page's, as before plan-36: a part that isn't
            // simulated does not turn it off.
            .disabled(target.map { !$0.isAvailable && $0.blockedReason == nil } ?? true)
        }
    }
}
