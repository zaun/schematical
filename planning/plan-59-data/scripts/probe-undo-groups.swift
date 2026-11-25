// plan-61 probe: one explicit undo group per step (groupsByEvent off) on an NSDocument's
// undo manager; does the document's edited state follow, with and without a run loop turn?
import AppKit
final class Doc: NSDocument { override class var autosavesInPlace: Bool { true } }
final class Target { var value = 0 }
_ = NSApplication.shared
let doc = Doc()
let manager = doc.undoManager!
manager.groupsByEvent = false
let target = Target()
func set(_ v: Int, name: String) {
    let old = target.value
    manager.beginUndoGrouping()
    manager.registerUndo(withTarget: target) { _ in set(old, name: name) }
    manager.setActionName(name)
    manager.endUndoGrouping()
    target.value = v
}
print("edited at start", doc.isDocumentEdited)
set(1, name: "Move"); print("edit 1: edited", doc.isDocumentEdited, "|", manager.undoMenuItemTitle, "| level", manager.groupingLevel)
set(2, name: "Add Wire"); print("edit 2: edited", doc.isDocumentEdited, "|", manager.undoMenuItemTitle)
manager.undo(); print("undo: value", target.value, "edited", doc.isDocumentEdited, "|", manager.undoMenuItemTitle, "/", manager.redoMenuItemTitle)
manager.undo(); print("undo: value", target.value, "edited", doc.isDocumentEdited, "|", manager.redoMenuItemTitle)
manager.redo(); print("redo: value", target.value, "edited", doc.isDocumentEdited, "|", manager.undoMenuItemTitle)
RunLoop.main.run(until: Date().addingTimeInterval(0.2))
print("after a run loop turn: edited", doc.isDocumentEdited)
doc.updateChangeCount(.changeCleared)
set(5, name: "Move")
print("cleared, edit, no run loop: edited", doc.isDocumentEdited)
RunLoop.main.run(until: Date().addingTimeInterval(0.2))
print("then a run loop turn: edited", doc.isDocumentEdited)
