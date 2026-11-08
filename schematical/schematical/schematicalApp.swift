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

@main
struct schematicalApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var appDelegate

    var body: some Scene {
        WindowGroup {
            ContentView()
        }
        .windowToolbarStyle(.unifiedCompact(showsTitle: false))
        .commands {
            SchematicFileCommands()
            BillOfMaterialsCommands()
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

        // One register window per converter or RTC part (plan-24-I).
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

        Settings {
            PreferencesView()
        }
    }
}
