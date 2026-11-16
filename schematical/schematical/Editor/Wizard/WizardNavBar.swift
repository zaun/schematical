import SwiftUI

/// Back, Skip and Next under a wizard step (plan-46-E). Next is never
/// disabled by a step's issues: a user who does not know a value yet moves on
/// and comes back (Review lists what is left). The last step has no Next.
struct WizardNavBar: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        HStack {
            Button("Back") { model.goBack() }
                .disabled(!model.canGoBack)
            Spacer()
            Text("Step \(model.currentStepIndex + 1) of \(model.steps.count)")
                .font(.caption).foregroundStyle(.secondary)
            Spacer()
            if model.canGoNext {
                if model.currentStep?.optional == true {
                    Button("Skip") { model.goNext() }
                        .help("Leave this step as it is and go on")
                }
                Button("Next") { model.goNext() }
                    .keyboardShortcut(.defaultAction)
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 10)
        .background(.bar)
    }
}
