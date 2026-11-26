import SwiftUI

/// The custom footprint's form (plan-32-D § 4.1): id, description, body,
/// and the pad table, one row per pad, with the `explicit` style's keys.
struct CustomFootprintEditor: View {
    @Binding var footprint: CustomFootprintDraft
    /// The pinout's pin numbers, for each pad's pin picker.
    let pinNumbers: [Int]
    let onRemove: () -> Void

    var body: some View {
        LabeledContent("Id", value: footprint.id)
        TextField("Description", text: $footprint.description)
        LabeledContent("Body (mm)") {
            HStack {
                NumberField(label: "width", value: double($footprint.bodyWidth))
                Text("×")
                NumberField(label: "height", value: double($footprint.bodyHeight))
                Toggle("Round", isOn: $footprint.bodyRound)
            }
        }
        Toggle("Edge mount (the housing hangs over the board edge)", isOn: $footprint.edgeMount)
        ScrollView(.horizontal) {
            Grid(alignment: .leading, horizontalSpacing: 6, verticalSpacing: 4) {
                GridRow {
                    ForEach(["Pin", "x", "y", "Width", "Height", "Shape", "Drill", "Slot", "Plated", "Opposite",
                             "Radius", "Chamfer", "Corners", ""], id: \.self) { Text($0).font(.caption) }
                }
                ForEach($footprint.pads) { $pad in
                    PadRow(pad: $pad, pinNumbers: pinNumbers) { footprint.removePad(pad.id) }
                }
            }
            .padding(.vertical, 4)
        }
        HStack {
            Button { footprint.addPad(pinNumbers: pinNumbers) } label: { Label("Add Pad", systemImage: "plus.circle") }
            Spacer()
            Button("Remove Custom Footprint", role: .destructive, action: onRemove)
        }
        Text("Pads are in mm, centred on the body (y down). Each pin of the pinout needs a pad with its number; pin 0 is an unnumbered centre pad. An opposite pad is a surface-mount pad on the other face of the board.")
            .font(.caption).foregroundStyle(.secondary)
    }

    private func double(_ binding: Binding<Double>) -> Binding<Double?> {
        Binding<Double?>(get: { binding.wrappedValue }, set: { binding.wrappedValue = $0 ?? 0 })
    }
}

/// One pad of the table.
private struct PadRow: View {
    @Binding var pad: PadDraft
    let pinNumbers: [Int]
    let onRemove: () -> Void

    var body: some View {
        GridRow {
            Picker("", selection: $pad.pin) {
                Text("0 (centre)").tag(0)
                ForEach(pad.pinChoices(pinNumbers), id: \.self) { Text("\($0)").tag($0) }
            }
            .labelsHidden().frame(width: 90)
            number($pad.x)
            number($pad.y)
            number($pad.width)
            number($pad.height)
            Picker("", selection: $pad.shape) {
                Text("rect").tag("rect")
                Text("round").tag("round")
            }
            .labelsHidden().frame(width: 76)
            number($pad.drill)
            number($pad.slotLength)
            Toggle("", isOn: $pad.plated).labelsHidden()
            // Surface mount only: a pad on the other face of the board.
            Toggle("", isOn: $pad.opposite).labelsHidden()
                .disabled(pad.drill != 0 || pad.slotLength != 0 || !pad.plated)
            number($pad.radius)
            number($pad.chamfer)
            HStack(spacing: 2) {
                ForEach(PadDraft.cornerNames, id: \.self) { corner in
                    Toggle(corner, isOn: cornerBinding(corner)).toggleStyle(.button).controlSize(.small)
                }
            }
            Button(action: onRemove) { Image(systemName: "minus.circle") }.buttonStyle(.borderless)
                .accessibilityLabel("Remove Pad")
        }
    }

    private func number(_ binding: Binding<Double>) -> some View {
        TextField("", value: Binding<Double?>(get: { binding.wrappedValue }, set: { binding.wrappedValue = $0 ?? 0 }),
                  format: .number.grouping(.never))
            .labelsHidden()
            .frame(width: 58)
    }

    private func cornerBinding(_ corner: String) -> Binding<Bool> {
        Binding(get: { pad.corners.contains(corner) }, set: { pad.setCorner(corner, $0) })
    }
}
