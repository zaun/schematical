import SwiftUI

// The Simulation menu: Run / Stop (⇧⌘R) and the Speed submenu. The
// simulation is the Schematic page's, so the menu is disabled on the PCB.

/// What the focused schematic window's simulation can do.
struct SimulationTarget {
    /// The window shows the Schematic page.
    var isAvailable: Bool
    var isRunning: Bool
    /// Runs it when stopped, stops it when running.
    var toggle: () -> Void
}

extension FocusedValues {
    /// Set by each schematic window while it is key.
    @Entry var schematicSimulation: SimulationTarget?
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
    @FocusedValue(\.schematicSimulation) private var target
    /// An app preference; each schematic window follows it.
    @AppStorage("simulationSpeed") private var speed = 1.0

    var body: some Commands {
        CommandMenu("Simulation") {
            Button(target?.isRunning == true ? "Stop" : "Run") { target?.toggle() }
                // Also Edit › Rotate Counterclockwise's (PCB only); the
                // window's key monitor picks by page.
                .keyboardShortcut("r", modifiers: [.command, .shift])
                .disabled(target?.isAvailable != true)

            Picker("Speed", selection: Binding(
                get: { SimulationSpeed.choice(nearest: speed) },
                set: { speed = $0 }
            )) {
                ForEach(SimulationSpeed.choices, id: \.value) { choice in
                    Text(choice.label).tag(choice.value)
                }
            }
            .pickerStyle(.menu)
            .disabled(target?.isAvailable != true)
        }
    }
}
