import AppKit
import SwiftUI

/// Two-finger trackpad scrolling (and the mouse wheel) pans the canvas, in
/// every tool mode. `onScroll` gets how far the canvas moved (screen points),
/// so a press in progress can follow it.
extension View {
    func scrollPans(_ viewport: CanvasViewport, onScroll: @escaping (CGSize) -> Void = { _ in }) -> some View {
        background(ScrollPanView(viewport: viewport, onScroll: onScroll))
    }
}

private struct ScrollPanView: NSViewRepresentable {
    let viewport: CanvasViewport
    let onScroll: (CGSize) -> Void

    func makeNSView(context: Context) -> ScrollPanNSView {
        let view = ScrollPanNSView()
        view.viewport = viewport
        view.onScroll = onScroll
        return view
    }

    func updateNSView(_ view: ScrollPanNSView, context: Context) {
        view.viewport = viewport
        view.onScroll = onScroll
    }
}

/// Watches scroll events for its window and takes those over its own bounds.
/// A local monitor, because SwiftUI gives a non-scrolling view no scroll
/// callback, and a view behind the canvas never receives the event itself.
/// Not private, so a test can hand it an event (`handle`).
final class ScrollPanNSView: NSView {
    var viewport: CanvasViewport?
    var onScroll: (CGSize) -> Void = { _ in }
    private var monitor: Any?

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        if let monitor { NSEvent.removeMonitor(monitor) }
        monitor = nil
        guard window != nil else { return }
        monitor = NSEvent.addLocalMonitorForEvents(matching: .scrollWheel) { [weak self] in self?.handle($0) ?? $0 }
    }

    deinit {
        if let monitor { NSEvent.removeMonitor(monitor) }
    }

    /// What the monitor does with a scroll: nil when it panned the canvas,
    /// else the event, left for others.
    func handle(_ event: NSEvent) -> NSEvent? {
        takes(event.window, at: event.locationInWindow, modifiers: event.modifierFlags,
              dx: event.scrollingDeltaX, dy: event.scrollingDeltaY) ? nil : event
    }

    /// Whether a scroll by (`dx`, `dy`) at `location` in `eventWindow` is the
    /// canvas's: in its window, over its bounds, without ⌘ or ⌃. A taken
    /// scroll pans the viewport by that much (a zero one does nothing).
    func takes(_ eventWindow: NSWindow?, at location: CGPoint, modifiers: NSEvent.ModifierFlags,
               dx: CGFloat, dy: CGFloat) -> Bool {
        guard let viewport, let window, eventWindow === window else { return false }
        guard bounds.contains(convert(location, from: nil)) else { return false }
        // ⌘/⌃-scroll is left alone for anything that wants it.
        guard modifiers.intersection([.command, .control]).isEmpty else { return false }
        guard dx != 0 || dy != 0 else { return true }
        viewport.pan(to: CGSize(width: viewport.offset.width + dx, height: viewport.offset.height + dy))
        onScroll(CGSize(width: dx, height: dy))
        return true
    }
}
