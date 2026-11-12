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
        ScrollView(.horizontal) {
            Grid(alignment: .leading, horizontalSpacing: 6, verticalSpacing: 4) {
                GridRow {
                    ForEach(["Pin", "x", "y", "Width", "Height", "Shape", "Drill", "Slot", "Plated", "Radius",
                             "Chamfer", "Corners", ""], id: \.self) { Text($0).font(.caption) }
                }
                ForEach($footprint.pads) { $pad in
                    PadRow(pad: $pad, pinNumbers: pinNumbers) {
                        footprint.pads.removeAll { $0.id == pad.id }
                    }
                }
            }
            .padding(.vertical, 4)
        }
        HStack {
            Button { addPad() } label: { Label("Add Pad", systemImage: "plus.circle") }
            Spacer()
            Button("Remove Custom Footprint", role: .destructive, action: onRemove)
        }
        Text("Pads are in mm, centred on the body (y down). Each pin of the pinout needs a pad with its number; pin 0 is an unnumbered centre pad.")
            .font(.caption).foregroundStyle(.secondary)
    }

    private func addPad() {
        let used = Set(footprint.pads.map(\.pin))
        let pin = pinNumbers.first { !used.contains($0) } ?? 0
        let last = footprint.pads.last
        footprint.pads.append(PadDraft(pin: pin, x: (last?.x ?? 0) + 2.54, y: last?.y ?? 0,
                                       width: last?.width ?? 1.6, height: last?.height ?? 1.6,
                                       shape: last?.shape ?? "round", drill: last?.drill ?? 0.8))
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
                ForEach(pins, id: \.self) { Text("\($0)").tag($0) }
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
            number($pad.radius)
            number($pad.chamfer)
            HStack(spacing: 2) {
                ForEach(PadDraft.cornerNames, id: \.self) { corner in
                    Toggle(corner, isOn: cornerBinding(corner)).toggleStyle(.button).controlSize(.small)
                }
            }
            Button(action: onRemove) { Image(systemName: "minus.circle") }.buttonStyle(.borderless)
        }
    }

    private var pins: [Int] {
        pad.pin != 0 && !pinNumbers.contains(pad.pin) ? pinNumbers + [pad.pin] : pinNumbers
    }

    private func number(_ binding: Binding<Double>) -> some View {
        TextField("", value: Binding<Double?>(get: { binding.wrappedValue }, set: { binding.wrappedValue = $0 ?? 0 }),
                  format: .number.grouping(.never))
            .labelsHidden()
            .frame(width: 58)
    }

    private func cornerBinding(_ corner: String) -> Binding<Bool> {
        Binding(
            get: { pad.corners.contains(corner) },
            set: { on in
                if on {
                    pad.corners = PadDraft.cornerNames.filter { $0 == corner || pad.corners.contains($0) }
                } else {
                    pad.corners.removeAll { $0 == corner }
                }
            }
        )
    }
}
