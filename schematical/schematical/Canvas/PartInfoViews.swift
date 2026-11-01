import SwiftUI

// Views shared by the Schematic and PCB sidebars for the selected part.

/// The part's reference, part number and description.
struct PartHeader: View {
    let chip: PlacedChip

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(chip.reference)
                .font(.title2.bold())
            Text(chip.part.name)
                .font(.headline)
            if !chip.part.description.isEmpty {
                Text(chip.part.description)
                    .font(.callout)
                    .foregroundStyle(.secondary)
            }
        }
        .textSelection(.enabled)
    }
}

/// The part's reference as prefix and number fields. Return or leaving the
/// pair commits through `SchematicDocument.renameChip`; a refused name shows
/// the reason in red and puts the fields back.
struct ReferenceSection: View {
    let document: SchematicDocument
    let chip: PlacedChip

    @State private var prefix = ""
    @State private var number = ""
    @State private var referenceError: String?
    @FocusState private var referenceFocus: ReferenceField?

    private enum ReferenceField { case prefix, number }

    var body: some View {
        Section("Reference") {
            HStack {
                TextField("Prefix", text: $prefix)
                    .focused($referenceFocus, equals: .prefix)
                TextField("Number", text: $number)
                    .focused($referenceFocus, equals: .number)
            }
            .labelsHidden()
            .onSubmit(commitReference)
            .onChange(of: referenceFocus) { old, new in
                // Leaving the pair of fields commits; moving between them doesn't.
                if old != nil, new == nil { commitReference() }
            }
            if let referenceError {
                Text(referenceError)
                    .font(.caption)
                    .foregroundStyle(.red)
            }
        }
        .onAppear(perform: resetReference)
    }

    private func commitReference() {
        let trimmed = number.trimmingCharacters(in: .whitespaces)
        referenceError = document.renameChip(chip.id, prefix: prefix.trimmingCharacters(in: .whitespaces),
                                             number: Int(trimmed))
        resetReference()
    }

    /// Puts the fields back to the part's reference as stored.
    private func resetReference() {
        let reference = document.chips.first { $0.id == chip.id }?.reference ?? chip.reference
        let parts = ReferenceParts(reference)
        prefix = parts.prefix
        number = parts.number.map(String.init) ?? ""
    }
}
