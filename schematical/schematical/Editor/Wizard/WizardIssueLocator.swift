import Foundation

/// Finds the wizard step a loader message is about (plan-46-K), so Review
/// can offer "Go to Gates" beside `devices[0] (custom): parts[0] (MY-NAND):
/// …`. The message is the backend's own and is never reworded; a message no
/// rule places simply gets no button.
enum WizardIssueLocator {
    /// The step of `steps` the message points at, or nil.
    static func step(for message: String, steps: [WizardStep], draft: PartDraft) -> WizardStep.ID? {
        // "<source>: devices[0] (id): parts[0] (name): <text>": the places
        // are the parts before the text.
        let parts = message.components(separatedBy: ": ")
        let places = parts.dropLast().map { $0.trimmingCharacters(in: .whitespaces) }
        let text = parts.last ?? message

        func step(page: WizardPage) -> WizardStep.ID? { steps.first { $0.page == page }?.id }

        // The file's own lists.
        for place in places {
            if place.hasPrefix("footprints[") { return step(page: .footprint) }
            if place.hasPrefix("datasheets[") { return step(page: .datasheet) }
        }
        // A place inside one of the kind's keys: "gates[0]", "timing.HC".
        for place in places {
            if let found = kindStep(named: identifier(of: place), steps: steps, draft: draft) { return found }
        }
        let quoted = quotedWords(in: message)
        // A kind key quoted in the message: `unknown key "gates"`.
        for word in quoted {
            if let found = kindStep(named: word, steps: steps, draft: draft) { return found }
        }
        // The pin table.
        let pinoutNames = Set(draft.pinouts.map(\.name))
        for place in places {
            let name = identifier(of: place)
            if name == "pinouts" || pinoutNames.contains(name) { return step(page: .pinout) }
            if place.hasPrefix("pinouts.") { return step(page: .pinout) }
        }
        let lower = text.lowercased()
        if lower.contains("unknown pin") || lower.contains("duplicate pin") || lower.contains("pin count")
            || lower.contains("pin \"") || lower.contains("a pinout") {
            return step(page: .pinout)
        }
        // The keys every kind has.
        for word in quoted {
            switch word {
            case "name", "manufacturer", "package", "id", "function", "description", "drawer", "flags":
                return step(page: .identity)
            case "datasheet":
                return step(page: .datasheet)
            case "footprint", "connectorFootprint", "footprintChoices", "fits":
                return step(page: .footprint)
            case "vcc", "inputMax":
                return step(page: .supply) ?? kindStep(named: word, steps: steps, draft: draft)
            case "pinout", "pinouts":
                return step(page: .pinout)
            default:
                continue
            }
        }
        // Said in words rather than as a key.
        if lower.contains("footprint") || lower.contains(" pads") || lower.contains(" pad ") {
            return step(page: .footprint)
        }
        if lower.contains("datasheet") { return step(page: .datasheet) }
        if lower.contains("part name") || lower.contains("empty name") || lower.contains("manufacturer") {
            return step(page: .identity)
        }
        for word in words(in: text) {
            if let found = kindStep(named: word, steps: steps, draft: draft) { return found }
        }
        return nil
    }

    /// "gates[0]" → "gates"; "timing.HC" → "timing"; "devices[0] (1G00)" → "devices".
    static func identifier(of place: String) -> String {
        String(place.prefix { $0.isLetter || $0.isNumber || $0 == "_" })
    }

    /// Every double-quoted word of a message, in order.
    static func quotedWords(in message: String) -> [String] {
        var words: [String] = []
        var current: String?
        for character in message {
            if character == "\"" {
                if let word = current {
                    words.append(word)
                    current = nil
                } else {
                    current = ""
                }
            } else {
                current?.append(character)
            }
        }
        return words
    }

    /// The bare words of a text (letters and digits), in order.
    static func words(in text: String) -> [String] {
        text.split { !$0.isLetter && !$0.isNumber }.map(String.init)
    }

    /// The step holding a kind key by name: a top-level key (device before
    /// part), else the top-level key it is nested in.
    static func kindStep(named name: String, steps: [WizardStep], draft: PartDraft) -> WizardStep.ID? {
        guard !name.isEmpty, let schema = draft.schema else { return nil }
        func holder(_ qualified: String) -> WizardStep.ID? {
            steps.first { $0.keys.contains(qualified) }?.id
        }
        let top = schema.topLevelFields(variant: draft.variant)
        if let direct = top.first(where: { $0.key.key == name }), let found = holder(direct.key.qualified) {
            return found
        }
        if let parent = top.first(where: { $0.field.type.nestedKeys.contains(name) }) {
            return holder(parent.key.qualified)
        }
        return nil
    }
}
