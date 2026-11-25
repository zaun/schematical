//
//  schematicalApp.swift
//  schematical
//
//  Created by Justin Zaun on 10/31/26.
//

import AppKit
import SwiftUI

/// Quits the app when its last window closes, rather than leaving it running
/// in the Dock with no window. Quitting with unsaved documents is AppKit's
/// own review (plan-61-B): documents autosave in place, so it asks only
/// about one that has never been saved.
@MainActor
final class AppDelegate: NSObject, NSApplicationDelegate {
    /// The Edit menu ends at Reset References (`EditMenuTrimmer`).
    func applicationWillFinishLaunching(_ notification: Notification) {
        EditMenuTrimmer.install()
        OptionOnlyMenuItems.install()
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }
}

@main
struct schematicalApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var appDelegate

    init() {
        // Read the parts once, at launch, rather than per window.
        _ = PartCatalogStore.shared
        // Spike: the agent-control server, off unless Settings > AI Agents turns it on.
        AutomationServer.shared.startIfEnabled()
    }

    var body: some Scene {
        // A schematic's window is its document's (`SchematicNSDocument`,
        // plan-61-B), not a scene. The scenes below are the app's other
        // windows; none opens at launch, takes an opened file, or adds a
        // File › New item. The first one carries the app's menus.
        // One window per memory part, opened by double-clicking it.
        WindowGroup("Memory", id: "memory", for: MemoryWindowTarget.self) { $target in
            if let target {
                MemoryWindowView(target: target)
            }
        }
        .defaultSize(width: 760, height: 560)
        // A memory window belongs to a schematic window and its running
        // simulation, so it is not reopened at launch.
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)
        .commands {
            AboutCommands()
            DocumentMenuCommands()
            SchematicFileCommands()
            BillOfMaterialsCommands()
            DesignRuleCheckCommands()
            RouteCommands()
            EditCommands()
            SimulationCommands()
            KeyboardHelpCommands()
        }

        WindowGroup("Processor", id: "cpu", for: CpuWindowTarget.self) { $target in
            if let target {
                CpuWindowView(target: target)
            }
        }
        .defaultSize(width: 420, height: 360)
        // Like a memory window: tied to a schematic window's simulation.
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One register window per converter, RTC or display driver part (plan-24-I).
        WindowGroup("Registers", id: "registers", for: RegisterWindowTarget.self) { $target in
            if let target {
                RegisterWindowView(target: target)
            }
        }
        .defaultSize(width: 440, height: 420)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One terminal per serial part (plan-12-K).
        WindowGroup("Terminal", id: "terminal", for: TerminalWindowTarget.self) { $target in
            if let target {
                TerminalWindowView(target: target)
            }
        }
        .defaultSize(width: 640, height: 420)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One Bill of Materials per schematic window (Window menu, ⇧⌘B).
        WindowGroup("Bill of Materials", id: "bom", for: BillOfMaterialsTarget.self) { $target in
            if let target {
                BillOfMaterialsWindowView(target: target)
            }
        }
        .defaultSize(width: 760, height: 420)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One Design Rule Check per schematic window (Window menu, ⇧⌘D).
        WindowGroup("Design Rule Check", id: "drc", for: DesignRuleCheckTarget.self) { $target in
            if let target {
                DesignRuleCheckWindowView(target: target)
            }
        }
        .defaultSize(width: 720, height: 420)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One Board Preview per schematic window (the PCB page's Preview button).
        WindowGroup("Board Preview", id: "board-preview", for: BoardPreviewTarget.self) { $target in
            if let target {
                BoardPreviewWindowView(target: target)
            }
        }
        .defaultSize(width: 1000, height: 720)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One window per datasheet, opened from a part's Open Datasheet button.
        WindowGroup("Datasheet", id: "datasheet", for: DatasheetWindowTarget.self) { $target in
            if let target {
                DatasheetWindowView(target: target)
            }
        }
        .defaultSize(width: 820, height: 1000)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One window per Markdown datasheet, opened from Show Datasheet.
        WindowGroup("Datasheet", id: "datasheet-md", for: MarkdownDatasheetTarget.self) { $target in
            if let target {
                MarkdownDatasheetWindowView(target: target)
            }
        }
        .defaultSize(width: 1040, height: 1000)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // Window › Verify Datasheet, shown only while Option is held: the
        // converted datasheet beside the PDF, with its citations.
        Window(VerifyDatasheetScene.title, id: VerifyDatasheetScene.id) {
            VerifyDatasheetWindowView()
        }
        .defaultSize(width: 1500, height: 950)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // The User Library (plan-53-F): one window, File › User Library….
        Window("User Library", id: UserLibraryScene.id) {
            UserLibraryWindowView(catalog: .shared)
        }
        .defaultSize(width: 760, height: 420)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // One Part Editor per part being made (plan-32-C). File › New Part…
        // opens one on the Part Wizard's first step (plan-46-F).
        WindowGroup("Part Editor", id: PartEditorScene.id, for: PartEditorTarget.self) { $target in
            if let target {
                PartEditorWindowView(target: target)
            }
        }
        // Wide enough for the Part Wizard's step list, step and preview.
        .defaultSize(width: 1120, height: 760)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        // Help › Schematic Help and PCB Help: one window per page.
        WindowGroup("Help", id: "help", for: EditorPage.self) { $page in
            KeyboardHelpView(page: page ?? .schematic)
        }
        .defaultSize(width: 620, height: 640)
        .restorationBehavior(.disabled)
        .handlesExternalEvents(matching: [])
        .commandsRemoved()
        .defaultLaunchBehavior(.suppressed)

        Settings {
            PreferencesView()
        }
    }
}
