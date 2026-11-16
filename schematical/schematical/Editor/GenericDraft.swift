import Foundation

/// The generic kind's keys (`backend/src/models/generic.c`, plan-46-B): a
/// part that is only a symbol and a footprint. Hand-written, as `GateDraft`
/// is: the kind ships no parts for `scripts/part-editor-schema.py` to infer
/// controls from.
enum GenericDraft {
    static let schema = KindSchema(
        kind: "generic",
        device: [
            // Always false: the loader refuses anything else, and a new draft
            // starts with a required bool's empty value, which is false.
            Field(key: "simulated", label: "Simulated", type: .bool, required: true,
                  help: "Always off: this kind has no simulation"),
        ] + SchemaValues.supply,
        part: SchemaValues.supply
    )
}
