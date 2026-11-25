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

    /// A plain launch (the app opened with no file) that restored no
    /// document shows an Untitled one (plan-61-C): AppKit leaves a launch
    /// with nothing to show windowless under SwiftUI's app life cycle. The
    /// open-application Apple event marks a plain launch; a launch with a
    /// file brings open-documents and no such event. Restored and opened
    /// documents exist by the next main-queue turn, hence the wait.
    func applicationDidFinishLaunching(_ notification: Notification) {
        let event = NSAppleEventManager.shared().currentAppleEvent?.eventID
        DispatchQueue.main.async {
            let count = NSDocumentController.shared.documents.count
            if Self.opensUntitled(launchEvent: event, documentCount: count, isTestHost: Self.isTestHost) {
                NSDocumentController.shared.newDocument(nil)
            }
        }
    }

    /// The rule of `applicationDidFinishLaunching`. Not in the app that
    /// hosts the unit tests: they make their own documents.
    nonisolated static func opensUntitled(launchEvent: AEEventID?, documentCount: Int, isTestHost: Bool) -> Bool {
        launchEvent == AEEventID(kAEOpenApplication) && documentCount == 0 && !isTestHost
    }

    /// True in the app XCTest launched to host the unit tests.
    nonisolated static var isTestHost: Bool {
        ProcessInfo.processInfo.environment["XCTestConfigurationFilePath"] != nil
    }

    /// False in the unit tests, which open and close windows in the app
    /// that hosts them.
    static var quitsAfterLastWindowCloses = true

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        Self.quitsAfterLastWindowCloses
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
        // plan-61-B), not a scene, and so are the windows a document opens
        // beside it (`ChildWindowCenter`). The scenes below are the app's
        // other windows; none opens at launch, takes an opened file, or adds
        // a File › New item. The first one carries the app's menus.

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
        .commands {
            AboutCommands()
            DocumentMenuCommands()
            SchematicFileCommands()
            BillOfMaterialsCommands()
            DocumentInfoCommands()
            DesignRuleCheckCommands()
            RouteCommands()
            EditCommands()
            SimulationCommands()
            KeyboardHelpCommands()
        }

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
            PartEditorSceneRoot(target: $target)
        }
        // Wide enough for the Part Wizard's step list, step and preview.
        .defaultSize(width: 1120, height: 760)
        .restorationBehavior(.disabled)
        // The one scene that takes a file sent to the app: a `.schpart`
        // opens in a Part Editor (plan-61-C). Schematics are documents and
        // go to `SchematicNSDocument`.
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
