import AppKit
import SwiftUI

// Edit › Cut, Copy, Paste, Delete, Select All, Flip, Rotate and Reset
// References. In a
// schematic window they act on the page it shows; while a text field is
// being edited, the first five are that field's own, as in any Mac app.

/// What the focused schematic window does for each Edit menu item. Each
/// returns whether it did anything.
struct EditActions {
    var cut: () -> Bool
    var copy: () -> Bool
    var paste: () -> Bool
    var delete: () -> Bool
    var selectAll: () -> Bool
    /// The Schematic's Flip; `canFlip` when something there can flip.
    var flip: () -> Bool
    var canFlip: Bool
    /// The PCB's Rotate, counterclockwise when passed true; `canRotate`
    /// on the PCB with a rotatable selection.
    var rotate: (_ counterclockwise: Bool) -> Bool
    var canRotate: Bool
    /// Asks first; see `SchematicDocument.resetReferences`.
    var resetReferences: () -> Void
    /// Whether Reset References can run: there are parts and the
    /// simulation isn't running.
    var canResetReferences: Bool
}

extension FocusedValues {
    /// Set by each schematic window while it is key.
    @Entry var schematicEdit: EditActions?
}

/// One of the text-editing Edit menu items.
enum EditMenuItem: CaseIterable {
    case cut, copy, paste, delete, selectAll

    /// The responder-chain action a text field (or a window with no
    /// schematic) gets instead.
    var selector: Selector {
        switch self {
        case .cut: #selector(NSText.cut(_:))
        case .copy: #selector(NSText.copy(_:))
        case .paste: #selector(NSText.paste(_:))
        case .delete: #selector(NSText.delete(_:))
        case .selectAll: #selector(NSText.selectAll(_:))
        }
    }
}

/// Replaces the Edit menu's pasteboard items. As with Undo, they are never
/// disabled: the first responder is AppKit state SwiftUI doesn't observe,
/// and a disabled item would also block a text field's own Cut or Paste.
/// The schematic window's key monitor (`ContentView.installEditKeyMonitor`)
/// takes these keys first while no text field is being edited, so the
/// items here run from a menu click, or for a text field.
struct EditCommands: Commands {
    @FocusedValue(\.schematicEdit) private var target

    var body: some Commands {
        CommandGroup(replacing: .pasteboard) {
            Button("Cut") { perform(.cut) }
                .keyboardShortcut("x", modifiers: .command)
            Button("Copy") { perform(.copy) }
                .keyboardShortcut("c", modifiers: .command)
            Button("Paste") { perform(.paste) }
                .keyboardShortcut("v", modifiers: .command)
            Button("Delete") { perform(.delete) }
                .keyboardShortcut(.delete, modifiers: .command)
            Button("Select All") { perform(.selectAll) }
                .keyboardShortcut("a", modifiers: .command)
            Divider()
            // Flip is the Schematic's, Rotate the PCB's. On the Schematic
            // the window's Run button takes ⌘R before this menu does.
            Button("Flip") { _ = target?.flip() }
                .keyboardShortcut("f", modifiers: .command)
                .disabled(target?.canFlip != true)
            Button("Rotate Clockwise") { _ = target?.rotate(false) }
                .keyboardShortcut("r", modifiers: .command)
                .disabled(target?.canRotate != true)
            Button("Rotate Counterclockwise") { _ = target?.rotate(true) }
                .keyboardShortcut("r", modifiers: [.command, .shift])
                .disabled(target?.canRotate != true)
            Divider()
            Button("Reset References…") { target?.resetReferences() }
                .disabled(target?.canResetReferences != true)
        }
        // No Find, Spelling, Substitutions, Transformations or Speech.
        CommandGroup(replacing: .textEditing) {}
    }

    private func perform(_ item: EditMenuItem) {
        let textIsEditing = (NSApp.keyWindow?.firstResponder as? NSTextView)?.isEditable == true
        guard !textIsEditing, let target else {
            NSApp.sendAction(item.selector, to: nil, from: nil)
            return
        }
        let done = switch item {
        case .cut: target.cut()
        case .copy: target.copy()
        case .paste: target.paste()
        case .delete: target.delete()
        case .selectAll: target.selectAll()
        }
        if !done { NSSound.beep() }
    }
}

/// Keeps the Edit menu ending at Reset References: AppKit adds Writing
/// Tools, AutoFill, Start Dictation and Emoji & Symbols to any menu named
/// Edit, and SwiftUI rebuilds the menu bar now and then, so the extras are
/// removed whenever the menu is about to show.
@MainActor
enum EditMenuTrimmer {
    private static var observers: [Any] = []

    static func install() {
        // These two are AppKit's own switches.
        UserDefaults.standard.set(true, forKey: "NSDisabledDictationMenuItem")
        UserDefaults.standard.set(true, forKey: "NSDisabledCharacterPaletteMenuItem")
        guard observers.isEmpty else { return }
        // An item added to the Edit menu (AppKit's extras, or SwiftUI's
        // rebuild) is trimmed once the adding is done; opening any menu
        // trims too, in case one slipped by.
        observers.append(NotificationCenter.default.addObserver(
            forName: NSMenu.didAddItemNotification, object: nil, queue: .main
        ) { note in
            let menu = note.object as? NSMenu
            MainActor.assumeIsolated {
                guard let menu, menu === editMenu else { return }
                DispatchQueue.main.async { trim() }
            }
        })
        observers.append(NotificationCenter.default.addObserver(
            forName: NSMenu.didBeginTrackingNotification, object: nil, queue: .main
        ) { _ in
            MainActor.assumeIsolated { trim() }
        })
    }

    private static var editMenu: NSMenu? {
        NSApp.mainMenu?.items.first(where: { $0.title == "Edit" })?.submenu
    }

    /// Removes every item below Reset References in the Edit menu.
    static func trim() {
        guard let menu = editMenu,
              let last = menu.items.lastIndex(where: { $0.title.hasPrefix("Reset References") }) else { return }
        while menu.items.count > last + 1 {
            menu.removeItem(at: last + 1)
        }
    }
}
