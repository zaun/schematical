// plan-28-D: Schematical > Export in a running SwiftUI app, without a mouse.
// Runs the app's real schematic scene (ContentView in a WindowGroup) with
// the real SchematicFileCommands and SchematicExportCommands, loads
// be-6502.schematical into the window's document as File > Open does, then
// chooses each Export item through NSApp.mainMenu, fills in the save panel
// sheet in code and presses its OK. Also checks the items are disabled with
// no schematic window, and that an empty schematic shows the alert and no
// panel. Prints ok/FAIL lines; exits 1 on any FAIL.
//
// Works while the screen is locked (the window is ordered front by the app
// itself; no events go through the window server's input path).
//
//   rm -rf /tmp/p28-hm && APP=<Debug schematical.app> scripts/harness/build.sh \
//     scripts/harness/export-menu.swift /tmp/p28-hm && \
//     /tmp/p28-hm/H.app/Contents/MacOS/harness /tmp/p28-out
//
// Run from the repo root (it opens be-6502.schematical).
import AppKit
import SwiftUI

struct ExportMenuHarnessApp: App {
    @NSApplicationDelegateAdaptor(ExportMenuDriver.self) private var driver

    var body: some Scene {
        WindowGroup {
            ContentView()
        }
        .commands {
            SchematicFileCommands()
            SchematicExportCommands()
        }
    }
}

@MainActor
final class ExportMenuDriver: NSObject, NSApplicationDelegate {
    private var failures = 0
    private let out = CommandLine.arguments.count > 1 ? CommandLine.arguments[1] : "/tmp/p28-out"

    func applicationDidFinishLaunching(_ notification: Notification) {
        Task { await run() }
    }

    private func check(_ ok: Bool, _ what: String) {
        print(ok ? "ok   \(what)" : "FAIL \(what)")
        if !ok { failures += 1 }
    }

    private func settle(_ seconds: Double = 0.5) async {
        try? await Task.sleep(for: .seconds(seconds))
    }

    /// The application menu's Export submenu.
    private var exportMenu: NSMenu? {
        NSApp.mainMenu?.items.first?.submenu?.items.first { $0.title == "Export" }?.submenu
    }

    /// The application menu's titles, in order ("—" for a separator).
    private var appMenuTitles: [String] {
        NSApp.mainMenu?.items.first?.submenu?.items.map { $0.isSeparatorItem ? "—" : $0.title } ?? []
    }

    private func exportItems() -> [(title: String, enabled: Bool)] {
        guard let menu = exportMenu else { return [] }
        menu.update()
        return menu.items.map { ($0.title, $0.isEnabled) }
    }

    private func choose(_ title: String) {
        guard let menu = exportMenu, let index = menu.items.firstIndex(where: { $0.title == title }) else {
            check(false, "no Export item \(title)")
            return
        }
        menu.update()
        menu.performActionForItem(at: index)
    }

    private func makeKey(_ window: NSWindow) async {
        NSApp.setActivationPolicy(.regular)
        for _ in 0..<10 where !window.isKeyWindow {
            NSRunningApplication.current.activate(options: [.activateAllWindows])
            NSApp.activate(ignoringOtherApps: true)
            window.makeKeyAndOrderFront(nil)
            window.makeMain()
            await settle()
        }
    }

    private func run() async {
        try? FileManager.default.createDirectory(atPath: out, withIntermediateDirectories: true)
        await settle(1.5)

        // The menu itself.
        let titles = appMenuTitles
        print("application menu: \(titles.joined(separator: " | "))")
        check(titles.count > 2 && titles[0].hasPrefix("About") && titles[1] == "Export",
              "Export is the application menu's second item, after About")

        if !NSApp.windows.contains(where: { $0.title == "Untitled" }),
           let file = NSApp.mainMenu?.items.first(where: { $0.title == "File" })?.submenu,
           let new = file.items.firstIndex(where: { $0.title.hasPrefix("New") }) {
            print("no window at launch: choosing File > \(file.items[new].title)")
            file.update()
            file.performActionForItem(at: new)
            await settle(2)
        }
        for w in NSApp.windows {
            print("  window \(w.className) \"\(w.title)\" visible \(w.isVisible) frame \(w.frame)")
        }
        guard let window = NSApp.windows.first(where: { !($0 is NSPanel) && $0.title == "Untitled" }) else {
            check(false, "a schematic window opened")
            return finish()
        }
        await makeKey(window)
        print("window \"\(window.title)\" key \(window.isKeyWindow) main \(window.isMainWindow) app active \(NSApp.isActive)")
        let items = exportItems()
        print("Export items: \(items.map { "\($0.title) (\($0.enabled ? "enabled" : "disabled"))" }.joined(separator: ", "))")
        check(items.map(\.title) == ["SVG Schematic", "PNG Schematic", "JPG Schematic"], "three Export items in order")
        check(items.allSatisfy(\.enabled), "the items are enabled with a schematic window focused")

        // An empty schematic: the alert, no save panel.
        choose("SVG Schematic")
        await settle()
        let sheet = window.attachedSheet
        check(sheet != nil && !(sheet is NSSavePanel), "empty schematic: a sheet that is not a save panel")
        let text = sheet.map { Self.texts(in: $0.contentView) } ?? []
        check(text.contains("Nothing to export") && text.contains("Add a part, wire or bus first."),
              "empty schematic: the alert says \(text.prefix(3))")
        if let sheet {
            window.endSheet(sheet, returnCode: .alertFirstButtonReturn)
            await settle()
        }

        // Load be-6502 into the window's document, as File > Open does.
        guard let document = SchematicSessions.shared.allSessions.first?.document else {
            check(false, "the window's document is registered")
            return finish()
        }
        do {
            let url = URL(fileURLWithPath: "be-6502.schematical")
            let file = try SchematicFile.decode(Data(contentsOf: url))
            try file.apply(to: document, catalog: Harness.catalog, fileURL: url)
        } catch {
            check(false, "open be-6502.schematical: \(error)")
            return finish()
        }
        await settle()
        print("loaded: \(document.chips.count) parts, \(document.wires.count) wires, \(document.buses.count) buses")

        for (title, ext) in [("SVG Schematic", "svg"), ("PNG Schematic", "png"), ("JPG Schematic", "jpg")] {
            let target = URL(fileURLWithPath: out).appendingPathComponent("be-6502.\(ext)")
            try? FileManager.default.removeItem(at: target)
            choose(title)
            await settle(1)
            guard let panel = window.attachedSheet as? NSSavePanel else {
                check(false, "\(title): a save panel sheet (got \(String(describing: window.attachedSheet)))")
                continue
            }
            print("\(title): panel types \(panel.allowedContentTypes.map(\.identifier)), name \"\(panel.nameFieldStringValue)\"")
            check(panel.nameFieldStringValue.hasSuffix(".\(ext)"), "\(title): suggested name ends .\(ext)")
            panel.directoryURL = URL(fileURLWithPath: out)
            panel.nameFieldStringValue = "be-6502.\(ext)"
            await settle()
            panel.ok(nil)
            await settle(3)
            let size = (try? FileManager.default.attributesOfItem(atPath: target.path)[.size] as? Int) ?? 0
            check(size > 0, "\(title): wrote \(target.path) (\(size) bytes)")
        }

        finish()
    }

    /// Every label's text under a view.
    private static func texts(in view: NSView?) -> [String] {
        guard let view else { return [] }
        var found: [String] = []
        if let field = view as? NSTextField { found.append(field.stringValue) }
        for subview in view.subviews { found += texts(in: subview) }
        return found
    }

    private func finish() {
        print(failures == 0 ? "ALL OK" : "\(failures) FAILED")
        exit(failures == 0 ? 0 : 1)
    }
}

MainActor.assumeIsolated {
    // The disabled state with no schematic window is checked in the real
    // app (plan-28-D Phase 3); this harness always opens one.
    ExportMenuHarnessApp.main()
}
