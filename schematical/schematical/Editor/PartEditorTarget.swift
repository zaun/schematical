import Foundation

/// What a Part Editor window opens (plan-32-C § 4.1). The id keeps each
/// window distinct, so two New Part windows of one kind are two windows.
nonisolated struct PartEditorTarget: Hashable, Codable, Sendable {
    enum Source: Hashable, Codable, Sendable {
        /// A blank part of a kind.
        case new(kind: String)
        /// A copy of a library part (bundled or loaded), with a new uuid.
        case copy(name: String)
        /// A `.schpart` file.
        case file(URL)
    }

    let id: UUID
    let source: Source

    init(id: UUID = UUID(), source: Source) {
        self.id = id
        self.source = source
    }
}
