import Foundation

/// The harness does not compile schematicalApp.swift (it has the app's
/// `@main`), which since plan-61 also defines `AppDelegate`. The app's other
/// sources read one thing from it, `AppDelegate.isTestHost`
/// (`SchematicNSDocument.scheduleAutosaving`). A harness is like the test
/// host there: its documents are made in code and must not be autosaved.
@MainActor
final class AppDelegate {
    static let isTestHost = true
}
