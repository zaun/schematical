import SwiftUI

/// One step of the Part Wizard (plan-46-E): its title, what it still needs,
/// and its page. The pages host the form's own sections, so a value entered
/// here is the value All fields shows.
struct WizardStepView: View {
    @Bindable var model: PartEditorModel
    let step: WizardStep

    var body: some View {
        Form {
            Section {
                VStack(alignment: .leading, spacing: 6) {
                    HStack(alignment: .firstTextBaseline) {
                        Text(step.title).font(.title2).bold()
                        if step.optional {
                            Text("Optional")
                                .font(.caption)
                                .padding(.horizontal, 6).padding(.vertical, 2)
                                .background(.quaternary, in: Capsule())
                        }
                        Spacer()
                    }
                    if let intro = step.intro {
                        Text(intro).foregroundStyle(.secondary)
                            .fixedSize(horizontal: false, vertical: true)
                    }
                    if step.optional {
                        Text("Optional — skip this if you don't need it in the simulation.")
                            .font(.callout).foregroundStyle(.secondary)
                    }
                    ForEach(model.issues(of: step), id: \.self) { issue in
                        Label(issue, systemImage: "exclamationmark.circle")
                            .font(.callout).foregroundStyle(.orange)
                    }
                }
            }
            if let openError = model.openError {
                Section { Text(openError).foregroundStyle(.red) }
            }
            page
        }
        .formStyle(.grouped)
    }

    @ViewBuilder private var page: some View {
        switch step.page {
        case .kind:
            KindStepPage(model: model)
        case .identity:
            IdentityStepPage(model: model)
        case .datasheet:
            Section("Datasheet") {
                DatasheetField(datasheet: $model.draft.datasheet, ids: model.validator.datasheetIDs)
            }
        case .pinout:
            PinoutsSection(pinouts: $model.draft.pinouts, partPinout: $model.draft.pinout)
        case .variant, .supply, .kindFields:
            KindFieldsSection(model: model, keys: step.keys, title: "")
        case .footprint:
            FootprintSection(model: model)
        case .review:
            VerdictSection(model: model)
            Section("Preview") {
                if let part = model.lastValidPart {
                    HStack(alignment: .top, spacing: 12) {
                        VStack(alignment: .leading) {
                            Text("Block").font(.caption).foregroundStyle(.secondary)
                            BlockPreviewView(chip: PartPreview.chip(for: part), revision: model.previewRevision)
                                .frame(minHeight: 220)
                        }
                        VStack(alignment: .leading) {
                            Text("Footprint").font(.caption).foregroundStyle(.secondary)
                            FootprintPreviewView(part: part, revision: model.previewRevision)
                                .frame(minHeight: 220)
                        }
                    }
                } else {
                    Text("The previews show the part once it loads.").foregroundStyle(.secondary)
                }
            }
        }
    }
}
