import SwiftUI

/// The Part Wizard's step list (plan-46-E): every step's title, a check when
/// the step has nothing left to do and a dot when it has. Clicking a step
/// goes to it; the steps can be visited in any order.
struct WizardStepList: View {
    @Bindable var model: PartEditorModel

    var body: some View {
        let steps = model.steps
        List(selection: Binding<String?>(
            get: { model.currentStep?.id },
            set: { if let id = $0 { model.go(to: id) } })) {
            ForEach(Array(steps.enumerated()), id: \.element.id) { index, step in
                WizardStepRow(number: index + 1, step: step, state: Self.state(of: step, in: model))
                    .tag(Optional(step.id))
            }
        }
        .listStyle(.sidebar)
        .accessibilityLabel("Steps")
    }

    enum StepState: Equatable { case done, issues, plain }

    /// A check for a complete step, a dot for one with issues. Steps with
    /// nothing to check (Datasheet, Review) show neither.
    static func state(of step: WizardStep, in model: PartEditorModel) -> StepState {
        if !model.issues(of: step).isEmpty { return .issues }
        switch step.page {
        case .datasheet, .review: return .plain
        default: return .done
        }
    }
}

private struct WizardStepRow: View {
    let number: Int
    let step: WizardStep
    let state: WizardStepList.StepState

    var body: some View {
        HStack(spacing: 6) {
            Group {
                switch state {
                case .done:
                    Image(systemName: "checkmark.circle.fill").foregroundStyle(.green)
                        .accessibilityLabel("Complete")
                case .issues:
                    Image(systemName: "circle.fill").foregroundStyle(.orange).font(.system(size: 8))
                        .accessibilityLabel("Needs attention")
                case .plain:
                    Image(systemName: "circle").foregroundStyle(.tertiary)
                        .accessibilityHidden(true)
                }
            }
            .frame(width: 16)
            VStack(alignment: .leading, spacing: 1) {
                Text("\(number). \(step.title)").lineLimit(2)
                if step.optional {
                    Text("Optional").font(.caption2).foregroundStyle(.secondary)
                }
            }
        }
        .padding(.vertical, 2)
    }
}
