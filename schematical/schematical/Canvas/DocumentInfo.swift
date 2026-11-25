import CoreGraphics
import Foundation

/// What a schematic says about itself (plan-64): a title, author, revision,
/// description, created and updated dates, and whether (and where) a title
/// block shows it on the Schematic page. Saved with the file; an edit is one
/// undo step.
nonisolated struct DocumentInfo: Equatable, Sendable {
    var title = ""
    var author = ""
    var revision = ""
    var description = ""
    /// Nil until set.
    var created: Date?
    var updated: Date?
    /// On: a save sets `updated` (and `created` while it is empty), and the
    /// UI shows both dates read-only. Off: the dates are the user's.
    var automaticDates = true
    /// The title block's top-left on the Schematic page, on the grid; nil
    /// when it is not on the schematic.
    var titleBlock: CGPoint?

    /// `date` as the title block and the window show it, `2026-11-25 14:07`,
    /// in `zone`; empty for nil. A fixed format, so exports and tests don't
    /// depend on the locale.
    static func dateText(_ date: Date?, in zone: TimeZone = .current) -> String {
        guard let date else { return "" }
        let formatter = DateFormatter()
        formatter.locale = Locale(identifier: "en_US_POSIX")
        formatter.timeZone = zone
        formatter.dateFormat = "yyyy-MM-dd HH:mm"
        return formatter.string(from: date)
    }

    /// What a save does to the dates: with `automaticDates` on, `created`
    /// is set once and `updated` every time; off, nothing.
    mutating func stamp(now: Date) {
        guard automaticDates else { return }
        if created == nil { created = now }
        updated = now
    }
}
