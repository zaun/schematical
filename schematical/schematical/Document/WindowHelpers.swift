import AppKit
import SwiftUI

// Two helpers for windows SwiftUI makes: a close guard (the Part Editor's,
// which is a scene and not a document) and a reader of a view's window.

// MARK: - Window close

/// Sits in front of SwiftUI's own window delegate so it can answer
/// `windowShouldClose`. Every other delegate message is passed straight through
/// to SwiftUI's delegate.
final class WindowCloseGuard: NSObject, NSWindowDelegate {
    private let shouldClose: (NSWindow) -> Bool
    private weak var original: NSWindowDelegate?
    private weak var window: NSWindow?

    init(shouldClose: @escaping (NSWindow) -> Bool) {
        self.shouldClose = shouldClose
    }

    func attach(to newWindow: NSWindow?) {
        guard newWindow !== window else { return }
        if let window, window.delegate === self {
            window.delegate = original
        }
        window = newWindow
        original = newWindow?.delegate
        newWindow?.delegate = self
    }

    func windowShouldClose(_ sender: NSWindow) -> Bool {
        if original?.windowShouldClose?(sender) == false {
            return false
        }
        return shouldClose(sender)
    }

    override func responds(to selector: Selector!) -> Bool {
        super.responds(to: selector) || (original?.responds(to: selector) ?? false)
    }

    override func forwardingTarget(for selector: Selector!) -> Any? {
        original?.responds(to: selector) == true ? original : super.forwardingTarget(for: selector)
    }
}

/// Reports the window a view is in, once it's added to one.
struct WindowReader: NSViewRepresentable {
    let onWindow: (NSWindow) -> Void

    func makeNSView(context: Context) -> NSView {
        WindowReaderView(onWindow: onWindow)
    }

    func updateNSView(_ nsView: NSView, context: Context) {}
}

/// `WindowReader`'s view: reports its window when it moves into one.
final class WindowReaderView: NSView {
    let onWindow: (NSWindow) -> Void

    init(onWindow: @escaping (NSWindow) -> Void) {
        self.onWindow = onWindow
        super.init(frame: .zero)
    }

    /// Never archived: a view made from a coder has no callback, so there is none.
    required init?(coder: NSCoder) { nil }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        if let window { onWindow(window) }
    }
}
