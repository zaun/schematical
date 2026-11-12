import SwiftUI

/// File › New Part…: every kind, the ones with a form enabled; choosing one
/// opens a Part Editor window for a blank part of that kind.
struct PartKindPicker: View {
    @Environment(\.openWindow) private var openWindow
    @Environment(\.dismissWindow) private var dismissWindow

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Choose the kind of part").font(.headline)
            List(PartDraft.kindNames, id: \.self) { kind in
                let enabled = PartDraft.enabledKinds.contains(kind)
                Button {
                    openWindow(id: PartEditorScene.id, value: PartEditorTarget(source: .new(kind: kind)))
                    dismissWindow(id: PartKindPicker.windowID)
                } label: {
                    HStack {
                        Text(PartDraft.displayName(of: kind))
                        Spacer()
                        if !enabled {
                            Text("Coming soon").foregroundStyle(.secondary)
                        }
                    }
                    .contentShape(Rectangle())
                }
                .buttonStyle(.plain)
                .disabled(!enabled)
            }
        }
        .padding()
        .frame(minWidth: 320, minHeight: 420)
    }

    static let windowID = "new-part"
}

enum PartEditorScene {
    static let id = "part-editor"
}
