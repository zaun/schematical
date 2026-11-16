import SwiftUI

/// "Which chip does yours behave like?" (plan-46-I): the models of a kind
/// with variants, each with a sentence on what it is and real parts of that
/// model. Choosing the model is most of the work: it decides which settings
/// the part has at all.
struct VariantStepPage: View {
    @Bindable var model: PartEditorModel
    /// The value waiting for the "settings will be removed" answer.
    @State private var pending: String?

    var body: some View {
        if let schema = model.draft.schema, let variantKey = schema.variantKey {
            let examples = BundledPartFile.examplesByVariant(kind: model.draft.kind)
            Section {
                ForEach(schema.variantValues, id: \.self) { value in
                    Button {
                        if model.keysLost(choosingVariant: value).isEmpty {
                            model.chooseVariant(value)
                        } else {
                            pending = value
                        }
                    } label: {
                        HStack(alignment: .top) {
                            VStack(alignment: .leading, spacing: 2) {
                                Text(value).font(.headline)
                                if let blurb = PartKinds.variantBlurb(kind: model.draft.kind, value: value) {
                                    Text(blurb).font(.callout).foregroundStyle(.secondary)
                                        .fixedSize(horizontal: false, vertical: true)
                                }
                                if let parts = examples[value], !parts.isEmpty {
                                    Text("In the library: " + parts.joined(separator: ", "))
                                        .font(.caption).foregroundStyle(.tertiary)
                                }
                            }
                            Spacer()
                            if model.draft.variant == value {
                                Image(systemName: "checkmark").foregroundStyle(.tint)
                                    .accessibilityLabel("Chosen")
                            }
                        }
                        .contentShape(Rectangle())
                    }
                    .buttonStyle(.plain)
                    .accessibilityIdentifier("variant-\(value)")
                }
            } footer: {
                Text("This is the part's “\(variantKey)”. Pick the one whose behavior matches your chip; "
                    + "the next steps ask only for what that one needs.")
            }
            .alert("Change to \(pending ?? "")?", isPresented: Binding(
                get: { pending != nil },
                set: { if !$0 { pending = nil } })) {
                Button("Change") {
                    if let value = pending { model.chooseVariant(value) }
                    pending = nil
                }
                Button("Cancel", role: .cancel) { pending = nil }
            } message: {
                Text(Self.lossMessage(model.keysLost(choosingVariant: pending ?? "")))
            }
        }
    }

    /// "These settings do not exist for it and will be removed: sections."
    static func lossMessage(_ keys: [String]) -> String {
        let names = keys.compactMap { KindKey(qualified: $0)?.key }
        return "These settings do not exist for it and will be removed: " + names.joined(separator: ", ") + "."
    }
}

extension PartEditorModel {
    /// The side-qualified keys that hold values and that the model `value`
    /// does not have.
    func keysLost(choosingVariant value: String) -> [String] {
        draft.keysOutside(variant: value)
    }

    /// Sets the kind's model, removing the values of keys the new model does
    /// not have (the loader refuses them as unknown keys). The view asks
    /// first when there are any.
    func chooseVariant(_ value: String) {
        guard let variantKey = draft.schema?.variantKey else { return }
        var next = draft.removing(keys: draft.keysOutside(variant: value))
        next.kindDevice[variantKey] = .string(value)
        draft = next
    }
}
