import SwiftUI
import AppKit

/// A multi-line text field inside a document window must not register its own
/// undo steps. The document's undo manager (`SchematicUndoManager`) opens and
/// closes one group per recorded step, so a step an `NSTextView` registers
/// while typing finds no open group and AppKit raises. The text the field
/// edits is a model value, and the model records it as one coalesced step
/// (`recordEdit`), so ⌘Z already follows the typing.
///
/// `TextEditor` has no way to turn this off, so this finds the text view the
/// editor draws and sets `allowsUndo` to false.
extension View {
    /// Apply to a `TextEditor` whose text is a model value that records its
    /// own undo steps.
    func modelRecordsUndo() -> some View {
        background(TextViewUndoOff())
    }
}

private struct TextViewUndoOff: NSViewRepresentable {
    func makeNSView(context: Context) -> NSView { Finder() }

    func updateNSView(_ view: NSView, context: Context) {
        (view as? Finder)?.turnOff()
    }

    /// Sits behind the editor, so its frame is the editor's frame.
    private final class Finder: NSView {
        override func viewDidMoveToWindow() {
            super.viewDidMoveToWindow()
            turnOff()
            // The editor's text view may be made after this view is placed.
            DispatchQueue.main.async { [weak self] in self?.turnOff() }
        }

        func turnOff() {
            guard let root = window?.contentView else { return }
            let mine = convert(bounds, to: nil)
            for textView in Self.textViews(in: root) where textView.allowsUndo {
                let theirs = textView.convert(textView.bounds, to: nil)
                if mine.intersects(theirs) { textView.allowsUndo = false }
            }
        }

        private static func textViews(in view: NSView) -> [NSTextView] {
            var found: [NSTextView] = []
            if let textView = view as? NSTextView { found.append(textView) }
            for child in view.subviews { found += textViews(in: child) }
            return found
        }
    }
}
