//
//  schematicalApp.swift
//  schematical
//
//  Created by Justin Zaun on 10/31/26.
//

import AppKit
import SwiftUI

/// Quits the app when its last window closes, rather than leaving it running
/// in the Dock with no window.
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

    /// Quitting asks about each window with unsaved changes first.
    func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
        Task {
            sender.reply(toApplicationShouldTerminate: await SchematicFileController.confirmQuit())
        }
        return .terminateLater
    }
}

/// A schematic window's root: holds the window's model for its life
/// (plan-61-B phase 1; the document class takes this over).
private struct SchematicWindowRoot: View {
    @State private var model = SchematicEditorModel()

    var body: some View {
        ContentView(model: model)
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
        // Opening a file from a window that isn't blank opens a new one of
        // these, as a tab of that window.
        WindowGroup(id: "schematic") {
            SchematicWindowRoot()
        }
        .windowToolbarStyle(.unifiedCompact(showsTitle: false))
        .commands {
            AboutCommands()
            SchematicFileCommands()
            BillOfMaterialsCommands()
            DesignRuleCheckCommands()
            RouteCommands()
            UndoCommands()
            EditCommands()
            SimulationCommands()
            KeyboardHelpCommands()
        }

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

        WindowGroup("Processor", id: "cpu", for: CpuWindowTarget.self) { $target in
            if let target {
                CpuWindowView(target: target)
            }
        }
        .defaultSize(width: 420, height: 360)
        // Like a memory window: tied to a schematic window's simulation.
        .restorationBehavior(.disabled)

        // One register window per converter, RTC or display driver part (plan-24-I).
        WindowGroup("Registers", id: "registers", for: RegisterWindowTarget.self) { $target in
            if let target {
                RegisterWindowView(target: target)
            }
        }
        .defaultSize(width: 440, height: 420)
        .restorationBehavior(.disabled)

        // One terminal per serial part (plan-12-K).
        WindowGroup("Terminal", id: "terminal", for: TerminalWindowTarget.self) { $target in
            if let target {
                TerminalWindowView(target: target)
            }
        }
        .defaultSize(width: 640, height: 420)
        .restorationBehavior(.disabled)

        // One Bill of Materials per schematic window (Window menu, ⇧⌘B).
        WindowGroup("Bill of Materials", id: "bom", for: BillOfMaterialsTarget.self) { $target in
            if let target {
                BillOfMaterialsWindowView(target: target)
            }
        }
        .defaultSize(width: 760, height: 420)
        .restorationBehavior(.disabled)

        // One Design Rule Check per schematic window (Window menu, ⇧⌘D).
        WindowGroup("Design Rule Check", id: "drc", for: DesignRuleCheckTarget.self) { $target in
            if let target {
                DesignRuleCheckWindowView(target: target)
            }
        }
        .defaultSize(width: 720, height: 420)
        .restorationBehavior(.disabled)

        // One Board Preview per schematic window (the PCB page's Preview button).
        WindowGroup("Board Preview", id: "board-preview", for: BoardPreviewTarget.self) { $target in
            if let target {
                BoardPreviewWindowView(target: target)
            }
        }
        .defaultSize(width: 1000, height: 720)
        .restorationBehavior(.disabled)

        // One window per datasheet, opened from a part's Open Datasheet button.
        WindowGroup("Datasheet", id: "datasheet", for: DatasheetWindowTarget.self) { $target in
            if let target {
                DatasheetWindowView(target: target)
            }
        }
        .defaultSize(width: 820, height: 1000)
        .restorationBehavior(.disabled)

        // One window per Markdown datasheet, opened from Show Datasheet.
        WindowGroup("Datasheet", id: "datasheet-md", for: MarkdownDatasheetTarget.self) { $target in
            if let target {
                MarkdownDatasheetWindowView(target: target)
            }
        }
        .defaultSize(width: 1040, height: 1000)
        .restorationBehavior(.disabled)

        // Window › Verify Datasheet, shown only while Option is held: the
        // converted datasheet beside the PDF, with its citations.
        Window(VerifyDatasheetScene.title, id: VerifyDatasheetScene.id) {
            VerifyDatasheetWindowView()
        }
        .defaultSize(width: 1500, height: 950)
        .restorationBehavior(.disabled)

        // The User Library (plan-53-F): one window, File › User Library….
        Window("User Library", id: UserLibraryScene.id) {
            UserLibraryWindowView(catalog: .shared)
        }
        .defaultSize(width: 760, height: 420)
        .restorationBehavior(.disabled)

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

        // Help › Schematic Help and PCB Help: one window per page.
        WindowGroup("Help", id: "help", for: EditorPage.self) { $page in
            KeyboardHelpView(page: page ?? .schematic)
        }
        .defaultSize(width: 620, height: 640)
        .restorationBehavior(.disabled)

        Settings {
            PreferencesView()
        }
    }
}
