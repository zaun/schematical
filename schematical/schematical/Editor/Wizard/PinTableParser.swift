import Foundation

/// A pasted pin table, split into cells.
struct PinTable: Equatable, Sendable {
    var rows: [[String]]

    var columnCount: Int { rows.map(\.count).max() ?? 0 }

    func cell(_ row: Int, _ column: Int) -> String {
        rows[row].indices.contains(column) ? rows[row][column] : ""
    }

    /// The non-empty cells of a column.
    func cells(inColumn column: Int) -> [String] {
        rows.indices.map { cell($0, column) }.filter { !$0.isEmpty }
    }
}

/// What a column of a pasted pin table holds.
enum PinColumn: String, CaseIterable, Sendable {
    case number, ball, name, type, ignore

    var title: String {
        switch self {
        case .number: "Pin number"
        case .ball: "Ball"
        case .name: "Name"
        case .type: "Type"
        case .ignore: "Ignore"
        }
    }
}

/// Turns the text of a datasheet's pin table, copied from a PDF, into pins
/// (plan-46-H). Nothing is invented: a row the parser cannot place is
/// skipped and counted.
enum PinTableParser {
    /// Rows split on newlines (blank lines dropped); columns on tabs when any
    /// line has one, else on runs of two or more spaces when any line has
    /// one, else on single spaces. Cells are trimmed.
    static func parse(_ text: String) -> PinTable {
        let lines = text.components(separatedBy: .newlines)
            .filter { !$0.trimmingCharacters(in: .whitespaces).isEmpty }
        let rows: [[String]]
        if lines.contains(where: { $0.contains("\t") }) {
            rows = lines.map { $0.components(separatedBy: "\t") }
        } else if lines.contains(where: { $0.trimmingCharacters(in: .whitespaces).contains("  ") }) {
            rows = lines.map { split($0.trimmingCharacters(in: .whitespaces), minimumSpaces: 2) }
        } else {
            rows = lines.map { split($0.trimmingCharacters(in: .whitespaces), minimumSpaces: 1) }
        }
        return PinTable(rows: rows.map { $0.map { $0.trimmingCharacters(in: .whitespaces) } })
    }

    private static func split(_ line: String, minimumSpaces: Int) -> [String] {
        var cells: [String] = []
        var current = ""
        var spaces = 0
        for character in line {
            if character == " " {
                spaces += 1
                continue
            }
            if spaces >= minimumSpaces, !current.isEmpty {
                cells.append(current)
                current = ""
            } else if spaces > 0, !current.isEmpty {
                current += String(repeating: " ", count: spaces)
            }
            spaces = 0
            current.append(character)
        }
        if !current.isEmpty { cells.append(current) }
        return cells
    }

    /// "4" or "4, 7".
    static func isNumberCell(_ cell: String) -> Bool {
        cell.range(of: #"^\d+(\s*,\s*\d+)*$"#, options: .regularExpression) != nil
    }

    /// "A1", "AB12".
    static func isBallCell(_ cell: String) -> Bool {
        cell.range(of: #"^[A-Z]{1,2}\d{1,2}$"#, options: .regularExpression) != nil
    }

    private static func share(_ cells: [String], _ test: (String) -> Bool) -> Double {
        cells.isEmpty ? 0 : Double(cells.filter(test).count) / Double(cells.count)
    }

    /// The guess for each column: a column of pin numbers (the first; later
    /// ones are other packages' and are ignored), else a column of ball
    /// names; a column of type tokens; the first other column with letters
    /// is the name; the rest are ignored. A column counts when at least 80 %
    /// of its non-empty cells fit, so a header row does not spoil it.
    static func guessColumns(_ table: PinTable) -> [PinColumn] {
        var columns = [PinColumn](repeating: .ignore, count: table.columnCount)
        var taken = Set<Int>()
        let cells = (0 ..< table.columnCount).map { table.cells(inColumn: $0) }

        let numbers = cells.indices.filter { share(cells[$0]) { isNumberCell($0) } >= 0.8 }
        if let first = numbers.first {
            columns[first] = .number
            taken.formUnion(numbers)
        } else if let ball = cells.indices.first(where: { share(cells[$0]) { isBallCell($0) } >= 0.8 }) {
            // Only without a number column: pin names such as A0 … A14 look
            // like ball names.
            columns[ball] = .ball
            taken.insert(ball)
        }
        if let type = cells.indices.first(where: {
            !taken.contains($0) && share(cells[$0]) { PinRoleGuesser.isTypeToken($0) } >= 0.8
        }) {
            columns[type] = .type
            taken.insert(type)
        }
        if let name = cells.indices.first(where: { index in
            !taken.contains(index) && cells[index].contains { $0.contains(where: \.isLetter) }
        }) {
            columns[name] = .name
        }
        return columns
    }

    /// The pinout style the columns give: ball when a ball column is chosen.
    static func style(_ columns: [PinColumn]) -> PinoutDraft.Style {
        columns.contains(.ball) ? .ball : .numbered
    }

    /// The pins of the rows that have a designator and a name. A row without
    /// (a header, a section title) is skipped and counted. A cell `4, 7`
    /// gives two pins with the same name. Numbered pins come out in number
    /// order (a pinout lists 1, 2, … N), balls in table order.
    static func pins(_ table: PinTable, columns: [PinColumn]) -> (pins: [PinDraft], skipped: Int) {
        let designatorColumn = columns.firstIndex(of: .number) ?? columns.firstIndex(of: .ball)
        let balls = columns.contains(.ball) && !columns.contains(.number)
        guard let designatorColumn, let nameColumn = columns.firstIndex(of: .name) else {
            return ([], table.rows.count)
        }
        let typeColumn = columns.firstIndex(of: .type)
        var pins: [PinDraft] = []
        var skipped = 0
        for row in table.rows.indices {
            let designator = table.cell(row, designatorColumn)
            let name = table.cell(row, nameColumn)
            guard !name.isEmpty, balls ? isBallCell(designator) : isNumberCell(designator) else {
                skipped += 1
                continue
            }
            let type = typeColumn.map { table.cell(row, $0) }
            let designators = balls ? [designator]
                : designator.split(separator: ",").map { $0.trimmingCharacters(in: .whitespaces) }
            for one in designators {
                // "07" is pin 7.
                let text = balls ? one : String(Int(one) ?? 0)
                pins.append(PinRoleGuesser.pin(designator: text, name: name, type: type))
            }
        }
        if !balls {
            // A stable sort: two rows claiming one number stay in table order.
            pins = pins.enumerated().sorted { a, b in
                let x = Int(a.element.designator) ?? 0, y = Int(b.element.designator) ?? 0
                return x != y ? x < y : a.offset < b.offset
            }.map(\.element)
        }
        return (pins, skipped)
    }
}
