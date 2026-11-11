//
//  KeyboardHelp.swift
//  schematical
//
//  Help › Schematic Help and Help › PCB Help: one window per page listing
//  its tools, keys and mouse actions. ⌘? opens the one for the page the
//  front window shows.
//

import SwiftUI

/// One line of a help sheet: the keys or mouse action, and what it does.
nonisolated struct HelpEntry: Hashable, Sendable {
    let keys: String
    let action: String
}

/// A titled group of help lines.
nonisolated struct HelpSection: Hashable, Sendable {
    let title: String
    let entries: [HelpEntry]
}

extension EditorPage {
    /// The help sheet for this page, in reading order.
    nonisolated var helpSections: [HelpSection] {
        switch self {
        case .schematic: KeyboardHelp.schematic
        case .pcb: KeyboardHelp.pcb
        }
    }

    nonisolated var helpTitle: String {
        switch self {
        case .schematic: "Schematic Help"
        case .pcb: "PCB Help"
        }
    }
}

/// The two help sheets. Keep them in step with the canvases' key and mouse
/// handling (`ContentView.editKey`, the canvases' drawing keys, the tools'
/// tooltips).
nonisolated enum KeyboardHelp {
    private static func entry(_ keys: String, _ action: String) -> HelpEntry {
        HelpEntry(keys: keys, action: action)
    }

    /// Shared by both pages.
    static let window = HelpSection(title: "Window", entries: [
        entry("⌘N", "New window"),
        entry("⌘O", "Open a schematic"),
        entry("⌘S / ⇧⌘S", "Save / Save As"),
        entry("⌘Z / ⇧⌘Z", "Undo / Redo on the page shown"),
        entry("⌘= / ⌘-", "Zoom in / out"),
        entry("⌘0", "Actual size (100 %)"),
        entry("⇧⌘B", "Bill of Materials"),
        entry("⌘?", "This help, for the page shown"),
    ])

    static let schematic: [HelpSection] = [
        HelpSection(title: "Selecting and moving", entries: [
            entry("Click", "Select a part, wire, junction, net label, bus, port or text block"),
            entry("⇧-click", "Add to the selection, or take out of it"),
            entry("Drag", "Move a part, junction, net label or text block; move a wire's bend or segment"),
            entry("⇧-drag, or drag a selected item", "Move the whole selection"),
            entry("Drag empty space", "Pan (and clear the selection)"),
            entry("⇧-drag empty space", "Box-select: add everything the box touches"),
            entry("Double-click a wire", "Add a bend there"),
            entry("Double-click a text block", "Edit its words"),
            entry("Double-click a memory, processor, serial or converter part",
                  "Open its window (in play mode too)"),
        ]),
        HelpSection(title: "Editing", entries: [
            entry("F", "Flip the selected part or net label left to right"),
            entry("⌘⌫", "Delete the selection"),
            entry("Click a switch lever", "Flip that DIP switch position (also in play mode)"),
        ]),
        HelpSection(title: "Line and Bus tools (while dragging)", entries: [
            entry("Space", "Lock a corner"),
            entry("/", "Flip the bend"),
            entry("Esc", "Undo the last corner; with none, cancel the line"),
        ]),
        HelpSection(title: "Tools", entries: [
            entry("Pointer", "Select, move and pan"),
            entry("Eraser", "Click a part, wire or bus line to remove it"),
            entry("Line", "Drag from pin to pin, or to a point on a wire, to connect them"),
            entry("Bus", "Drag to draw a bus line; start or end on a bus to join it"),
            entry("Bus Port", "Click a bus line to add a port; tick its wires in the sidebar"),
            entry("Text", "Click to add a text block"),
            entry("Net Label", "Click to add a label; labels with the same name are one net"),
            entry("GND, Pull-down, VCC, Pull-up", "Click to place; the menu picks the supply voltage"),
        ]),
        HelpSection(title: "Simulation", entries: [
            entry("⌘R", "Run or stop the simulation"),
        ]),
        window,
    ]

    static let pcb: [HelpSection] = [
        HelpSection(title: "Selecting and moving", entries: [
            entry("Click", "Select a text, trace, via or footprint"),
            entry("⇧-click", "Add to the selection, or take out of it"),
            entry("Drag", "Move the item, snapped to the grid"),
            entry("⇧-drag, or drag a selected item", "Move the whole selection"),
            entry("Drag empty space", "Pan (and clear the selection)"),
            entry("⇧-drag empty space", "Box-select: add every shown item the box touches"),
        ]),
        HelpSection(title: "Editing", entries: [
            entry("R or ⌥⌘R", "Rotate the selected footprint or text 90°"),
            entry("⌘⌫", "Delete the selected texts, traces and vias (footprints stay)"),
            entry("⌘C", "Copy the selected traces"),
            entry("⌘V", "Paste traces, centred on the pointer"),
        ]),
        HelpSection(title: "Trace tool (while dragging)", entries: [
            entry("Space", "Lock a corner"),
            entry("/", "Put the angled leg first"),
            entry("A", "Change the angle (20°, 40°, 45°, 60°, 80°, 90°)"),
            entry("Esc", "Undo the last corner; with none, cancel the trace"),
        ]),
        HelpSection(title: "Tools", entries: [
            entry("Select", "Select, move and pan"),
            entry("Text", "Click to place a text"),
            entry("Trace", "Drag to draw a trace on the layer the menu picks"),
            entry("Via", "Click to place a via, drag to move it"),
            entry("Delete", "Click a trace, via or text to delete it"),
        ]),
        HelpSection(title: "Checks", entries: [
            entry("⇧⌘D", "Design Rule Check"),
        ]),
        window,
    ]
}

// MARK: - Menu commands

extension FocusedValues {
    /// The page the front schematic window shows, for ⌘?.
    @Entry var editorPage: EditorPage?
}

/// Help › Schematic Help, PCB Help, and ⌘? for the page shown.
struct KeyboardHelpCommands: Commands {
    @FocusedValue(\.editorPage) private var page
    @Environment(\.openWindow) private var openWindow

    var body: some Commands {
        CommandGroup(replacing: .help) {
            Button("Help for This Page") { openWindow(id: "help", value: page ?? .schematic) }
                .keyboardShortcut("?", modifiers: .command)
            Divider()
            ForEach(EditorPage.allCases) { page in
                Button(page.helpTitle) { openWindow(id: "help", value: page) }
            }
        }
    }
}

// MARK: - Window

/// One page's help: each section's lines, keys on the left.
struct KeyboardHelpView: View {
    let page: EditorPage

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 18) {
                ForEach(page.helpSections, id: \.title) { section in
                    VStack(alignment: .leading, spacing: 6) {
                        Text(section.title)
                            .font(.headline)
                        Grid(alignment: .leadingFirstTextBaseline, horizontalSpacing: 16, verticalSpacing: 4) {
                            ForEach(section.entries, id: \.self) { entry in
                                GridRow {
                                    Text(entry.keys)
                                        .font(.body.monospaced())
                                        .frame(minWidth: 180, alignment: .leading)
                                    Text(entry.action)
                                        .foregroundStyle(.secondary)
                                        .fixedSize(horizontal: false, vertical: true)
                                }
                            }
                        }
                    }
                }
            }
            .padding(20)
            .frame(maxWidth: .infinity, alignment: .leading)
        }
        .frame(minWidth: 520, minHeight: 360)
        .navigationTitle(page.helpTitle)
    }
}
