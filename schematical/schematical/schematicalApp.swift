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
        }
    }
}
