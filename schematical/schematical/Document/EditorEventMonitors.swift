import AppKit
import SwiftUI

/// The `NSEvent` monitors of one document's editor: the Edit keys
/// (`ContentView`) and each canvas's keys while a wire or trace is drawn.
///
/// A monitor's closure holds the view that installed it, and through that
/// view the document. The views take their monitors off in `onDisappear`,
/// but a view hosted in a document's window gets no `onDisappear` when the
/// document closes, so the monitors (and with them the closed document)
/// lived on, still handling keys (plan-59 A6, plan-61-B phase 5). The
/// document's model owns this object and empties it when the document
/// closes.
@MainActor
final class EditorEventMonitors {
    private var monitors: [UUID: Any] = [:]

    /// Monitors of views that are in no document's editor (a canvas shown on
    /// its own).
    static let detached = EditorEventMonitors()

    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when
    /// a closing document frees this.
    nonisolated deinit {}

    /// Installs a local monitor; the token takes it off again.
    func add(matching mask: NSEvent.EventTypeMask, _ handler: @escaping (NSEvent) -> NSEvent?) -> UUID? {
        guard let monitor = NSEvent.addLocalMonitorForEvents(matching: mask, handler: handler) else { return nil }
        let token = UUID()
        monitors[token] = monitor
        return token
    }

    func remove(_ token: UUID?) {
        guard let token, let monitor = monitors.removeValue(forKey: token) else { return }
        NSEvent.removeMonitor(monitor)
    }

    /// Takes every monitor off: the document is closing.
    func removeAll() {
        for monitor in monitors.values { NSEvent.removeMonitor(monitor) }
        monitors = [:]
    }

    var count: Int { monitors.count }
}

extension EnvironmentValues {
    /// The editor's monitors, set by `ContentView` for the views under it.
    @Entry var editorEventMonitors: EditorEventMonitors = .detached
}
