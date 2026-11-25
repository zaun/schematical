import AppKit
import SwiftUI

// The windows a schematic document opens beside its own (plan-61-B): Memory,
// Processor, Registers, Terminal, Bill of Materials, Design Rule Check and
// Board Preview. Each is an AppKit window hosting its SwiftUI view. They are
// not scenes: the editor is hosted in its document's window and has no scene
// to open one from, and a child belongs to its document, not to the app.

/// The open child windows, by their document's id and their target. A
/// document's children close with it (`SchematicNSDocument.close()`).
@MainActor
final class ChildWindowCenter {
    static let shared = ChildWindowCenter()

    private var windows: [UUID: [AnyHashable: NSWindow]] = [:]
    private var delegates: [ObjectIdentifier: Delegate] = [:]

    /// Shows the window for `target`, making it the first time.
    func open<Target: Hashable, Content: View>(_ target: Target, document id: UUID, title: String, size: NSSize,
                                                @ViewBuilder content: () -> Content) {
        if let window = windows[id]?[AnyHashable(target)] {
            window.makeKeyAndOrderFront(nil)
            return
        }
        let host = NSHostingController(rootView: AnyView(content()))
        // The view's `.navigationTitle` and `.toolbar` reach the window.
        host.sceneBridgingOptions = [.title, .toolbars]
        let window = NSWindow(contentViewController: host)
        window.title = title
        window.setContentSize(size)
        window.isReleasedWhenClosed = false
        // A child belongs to an open document and its running simulation,
        // so it is not reopened at launch, and it is never a tab.
        window.isRestorable = false
        window.tabbingMode = .disallowed
        window.center()
        let delegate = Delegate { [weak self] in self?.forget(target, document: id) }
        window.delegate = delegate
        delegates[ObjectIdentifier(window)] = delegate
        windows[id, default: [:]][AnyHashable(target)] = window
        window.makeKeyAndOrderFront(nil)
    }

    /// Closes the window for `target`, if it is open.
    func close<Target: Hashable>(_ target: Target, document id: UUID) {
        windows[id]?[AnyHashable(target)]?.close()
    }

    /// Closes every child window of a document.
    func closeAll(_ id: UUID) {
        for window in windows(of: id) { window.close() }
    }

    /// A document's open child windows.
    func windows(of id: UUID) -> [NSWindow] {
        Array((windows[id] ?? [:]).values)
    }

    /// A window closed (by the user, or by `close`): it is forgotten and its
    /// view tree, which holds the document, is let go.
    private func forget<Target: Hashable>(_ target: Target, document id: UUID) {
        guard let window = windows[id]?.removeValue(forKey: AnyHashable(target)) else { return }
        if windows[id]?.isEmpty == true { windows[id] = nil }
        delegates[ObjectIdentifier(window)] = nil
        window.delegate = nil
        (window.contentViewController as? NSHostingController<AnyView>)?.rootView = AnyView(EmptyView())
        window.contentViewController = nil
    }

    private final class Delegate: NSObject, NSWindowDelegate {
        private let onClose: () -> Void

        init(onClose: @escaping () -> Void) {
            self.onClose = onClose
        }

        func windowWillClose(_ notification: Notification) {
            onClose()
        }
    }
}

/// Stands where `@Environment(\.openWindow)` stood in `ContentView`, with its
/// call syntax, for the seven child windows.
@MainActor
struct ChildWindowOpener {
    let documentID: UUID

    func callAsFunction<Value: Hashable>(id: String, value: Value) {
        let center = ChildWindowCenter.shared
        switch value {
        case let target as MemoryWindowTarget:
            center.open(target, document: documentID, title: "Memory", size: NSSize(width: 760, height: 560)) {
                MemoryWindowView(target: target)
            }
        case let target as CpuWindowTarget:
            center.open(target, document: documentID, title: "Processor", size: NSSize(width: 420, height: 360)) {
                CpuWindowView(target: target)
            }
        case let target as RegisterWindowTarget:
            center.open(target, document: documentID, title: "Registers", size: NSSize(width: 440, height: 420)) {
                RegisterWindowView(target: target)
            }
        case let target as TerminalWindowTarget:
            center.open(target, document: documentID, title: "Terminal", size: NSSize(width: 640, height: 420)) {
                TerminalWindowView(target: target)
            }
        case let target as BillOfMaterialsTarget:
            center.open(target, document: documentID, title: "Bill of Materials", size: NSSize(width: 760, height: 420)) {
                BillOfMaterialsWindowView(target: target)
            }
        case let target as DesignRuleCheckTarget:
            center.open(target, document: documentID, title: "Design Rule Check", size: NSSize(width: 720, height: 420)) {
                DesignRuleCheckWindowView(target: target)
            }
        case let target as BoardPreviewTarget:
            center.open(target, document: documentID, title: "Board Preview", size: NSSize(width: 1000, height: 720)) {
                BoardPreviewWindowView(target: target)
            }
        default:
            assertionFailure("no child window for \(id)")
        }
    }
}

/// Stands where `@Environment(\.dismissWindow)` stood in `ContentView`.
@MainActor
struct ChildWindowCloser {
    let documentID: UUID

    func callAsFunction<Value: Hashable>(id: String, value: Value) {
        ChildWindowCenter.shared.close(value, document: documentID)
    }
}
