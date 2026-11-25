// plan-61 probe: does a window's Undo menu item follow an UndoManager subclass whose
// canUndo is false while "locked"? (No window is shown.)
import AppKit
final class LockableUndoManager: UndoManager {
    var isLocked = false
    override var canUndo: Bool { !isLocked && super.canUndo }
    override var canRedo: Bool { !isLocked && super.canRedo }
}
final class Doc: NSDocument {}
final class Target { var value = 0 }
_ = NSApplication.shared
let doc = Doc()
let manager = LockableUndoManager()
manager.groupsByEvent = false
doc.undoManager = manager
let window = NSWindow(contentRect: NSRect(x: 0, y: 0, width: 100, height: 100), styleMask: [.titled], backing: .buffered, defer: true)
let controller = NSWindowController(window: window)
doc.addWindowController(controller)
let target = Target()
manager.beginUndoGrouping()
manager.registerUndo(withTarget: target) { $0.value = 0 }
manager.setActionName("Move")
manager.endUndoGrouping()
let item = NSMenuItem(title: "Undo", action: Selector(("undo:")), keyEquivalent: "z")
print("window.undoManager is the document's:", window.undoManager === manager)
print("unlocked: validate =", window.validateMenuItem(item), "title:", item.title)
manager.isLocked = true
print("locked:   validate =", window.validateMenuItem(item), "title:", item.title)
manager.isLocked = false
print("unlocked: validate =", window.validateMenuItem(item), "title:", item.title)
