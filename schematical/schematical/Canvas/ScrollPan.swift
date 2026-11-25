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
private final class ScrollPanNSView: NSView {
    var viewport: CanvasViewport?
    var onScroll: (CGSize) -> Void = { _ in }
    private var monitor: Any?

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        if let monitor { NSEvent.removeMonitor(monitor) }
        monitor = nil
        guard window != nil else { return }
        monitor = NSEvent.addLocalMonitorForEvents(matching: .scrollWheel) { [weak self] event in
            self?.handle(event) ?? event
        }
    }

    deinit {
        if let monitor { NSEvent.removeMonitor(monitor) }
    }

    private func handle(_ event: NSEvent) -> NSEvent? {
        guard let viewport, let window, event.window === window else { return event }
        let point = convert(event.locationInWindow, from: nil)
        guard bounds.contains(point) else { return event }
        // ⌘/⌃-scroll is left alone for anything that wants it.
        guard event.modifierFlags.intersection([.command, .control]).isEmpty else { return event }
        let dx = event.scrollingDeltaX, dy = event.scrollingDeltaY
        guard dx != 0 || dy != 0 else { return nil }
        viewport.pan(to: CGSize(width: viewport.offset.width + dx, height: viewport.offset.height + dy))
        onScroll(CGSize(width: dx, height: dy))
        return nil
    }
}
