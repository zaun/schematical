import Foundation

/// A bus's wire names as a file: CSV with a `wire,name` header and one
/// line per wire, in bus order. The Bus panel's Export and Import use it.
/// Import takes the name from the last field of each line, so a plain list
/// (one name per line, no header) loads too; the wire column is ignored and
/// the order of the lines is the order of the wires.
enum BusWireList {
    static let header = "wire,name"

    /// The file text for wires named `names`.
    static func text(_ names: [String]) -> String {
        let lines = names.enumerated().map { "\($0.offset),\(field($0.element))" }
        return ([header] + lines).joined(separator: "\n") + "\n"
    }

    /// The wire names in `text`, or why it can't be read.
    static func names(in text: String) -> Result<[String], ParseError> {
        var names: [String] = []
        let rows = text.components(separatedBy: .newlines)
        for (number, row) in rows.enumerated() {
            let line = row.trimmingCharacters(in: .whitespaces)
            if line.isEmpty { continue }
            let fields = split(line)
            if names.isEmpty, fields.count > 1,
               fields.map({ $0.lowercased() }) == header.components(separatedBy: ",") { continue }
            let name = (fields.last ?? "").trimmingCharacters(in: .whitespaces)
            if name.isEmpty { return .failure(ParseError("Line \(number + 1) has no wire name.")) }
            if name.count > Bus.nameLengthRange.upperBound {
                return .failure(ParseError("Line \(number + 1): \"\(name)\" is longer than \(Bus.nameLengthRange.upperBound) characters."))
            }
            if name.contains(where: \.isWhitespace) {
                return .failure(ParseError("Line \(number + 1): a wire name can't contain spaces."))
            }
            names.append(name)
        }
        guard Bus.wireCountRange.contains(names.count) else {
            return .failure(ParseError("The file has \(names.count) wire \(names.count == 1 ? "name" : "names"); a bus carries \(Bus.wireCountRange.lowerBound)–\(Bus.wireCountRange.upperBound)."))
        }
        return .success(names)
    }

    struct ParseError: Error, Equatable {
        let message: String
        init(_ message: String) { self.message = message }
    }

    /// A CSV field: quoted when it holds a comma, quote or line break.
    private static func field(_ value: String) -> String {
        guard value.contains(where: { $0 == "," || $0 == "\"" || $0.isNewline }) else { return value }
        return "\"" + value.replacingOccurrences(of: "\"", with: "\"\"") + "\""
    }

    /// The fields of one CSV line (quotes allowed, `""` is a quote).
    private static func split(_ line: String) -> [String] {
        var fields: [String] = []
        var current = ""
        var quoted = false
        var index = line.startIndex
        while index < line.endIndex {
            let c = line[index]
            let next = line.index(after: index)
            if quoted {
                if c == "\"" {
                    if next < line.endIndex, line[next] == "\"" { current.append("\""); index = line.index(after: next); continue }
                    quoted = false
                } else {
                    current.append(c)
                }
            } else if c == "\"" {
                quoted = true
            } else if c == "," {
                fields.append(current)
                current = ""
            } else {
                current.append(c)
            }
            index = next
        }
        fields.append(current)
        return fields
    }
}
