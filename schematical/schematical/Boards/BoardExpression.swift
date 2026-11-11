import Foundation

/// A value in `boards.json`: a number (mm), a flag or a word. Option
/// choices, variables and expression results are all values.
nonisolated enum BoardValue: Hashable, Sendable, Codable, CustomStringConvertible {
    case number(Double)
    case bool(Bool)
    case string(String)

    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()
        if let flag = try? container.decode(Bool.self) {
            self = .bool(flag)
        } else if let number = try? container.decode(Double.self) {
            self = .number(number)
        } else {
            self = .string(try container.decode(String.self))
        }
    }

    func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()
        switch self {
        case .number(let value):
            if value == value.rounded(), abs(value) < 1e15 { try container.encode(Int(value)) } else { try container.encode(value) }
        case .bool(let value): try container.encode(value)
        case .string(let value): try container.encode(value)
        }
    }

    /// As a message or a pin name writes it: 4, 0.5, true, ddr4.
    var description: String {
        switch self {
        case .number(let value): value == value.rounded() && abs(value) < 1e15 ? String(Int(value)) : String(value)
        case .bool(let value): String(value)
        case .string(let value): value
        }
    }

    /// As a file message quotes it: words in quotes.
    var quoted: String {
        if case .string(let value) = self { return "\"\(value)\"" }
        return description
    }
}

/// An expression in `boards.json`, parsed once and evaluated per board
/// variant (and per pin). A number in the file is an expression too.
///
/// Grammar: numbers (`2.54`, with an optional unit: `0.1in`, `100mil`,
/// `2mm`), words in single quotes, `true` / `false`, names (variables,
/// options, loop counters, `width`, `height`), `( )`, unary `-` and `!`,
/// `* / %`, `+ -`, `< <= > >=`, `== !=`, `in [a, b, …]`, `&&`, `||` and
/// `c ? a : b`, and the functions `min`, `max`, `abs`, `floor`, `ceil`,
/// `round`, `sqrt`, `sin` and `cos` (degrees) and `interp(x, x0, y0, x1,
/// y1, …)` (straight between the points; the end values beyond them).
nonisolated indirect enum BoardExpression: Hashable, Sendable {
    case value(BoardValue)
    case name(String)
    case unary(String, BoardExpression)
    case binary(String, BoardExpression, BoardExpression)
    case member(BoardExpression, [BoardExpression])
    case choice(BoardExpression, BoardExpression, BoardExpression)
    case call(String, [BoardExpression])

    struct Failure: Error, CustomStringConvertible {
        let description: String
    }

    /// Where an expression finds its names.
    typealias Scope = (String) throws -> BoardValue?

    // MARK: Parsing

    static func parse(_ text: String) throws -> BoardExpression {
        var parser = Parser(tokens: try tokens(of: text), text: text)
        let expression = try parser.expression()
        guard parser.atEnd else { throw Failure(description: "unexpected \"\(parser.peek!.text)\" in \"\(text)\"") }
        return expression
    }

    private struct Token {
        enum Kind { case number, string, name, symbol }
        let kind: Kind
        let text: String
        var number: Double = 0
    }

    private static func tokens(of text: String) throws -> [Token] {
        var tokens: [Token] = []
        let characters = Array(text)
        var index = 0
        func at(_ offset: Int = 0) -> Character? {
            index + offset < characters.count ? characters[index + offset] : nil
        }
        while let c = at() {
            if c.isWhitespace {
                index += 1
            } else if c.isNumber || (c == "." && at(1)?.isNumber == true) {
                var digits = ""
                while let d = at(), d.isNumber || d == "." {
                    digits.append(d)
                    index += 1
                }
                guard var number = Double(digits) else { throw Failure(description: "bad number \"\(digits)\" in \"\(text)\"") }
                var unit = ""
                while let u = at(), u.isLetter {
                    unit.append(u)
                    index += 1
                }
                switch unit {
                case "": break
                case "mm": break
                case "in": number *= 25.4
                case "mil": number *= 0.0254
                default: throw Failure(description: "unknown unit \"\(unit)\" in \"\(text)\"")
                }
                tokens.append(Token(kind: .number, text: digits + unit, number: number))
            } else if c == "'" {
                index += 1
                var word = ""
                while let w = at(), w != "'" {
                    word.append(w)
                    index += 1
                }
                guard at() == "'" else { throw Failure(description: "unclosed quote in \"\(text)\"") }
                index += 1
                tokens.append(Token(kind: .string, text: word))
            } else if c.isLetter || c == "_" {
                var name = ""
                while let n = at(), n.isLetter || n.isNumber || n == "_" {
                    name.append(n)
                    index += 1
                }
                tokens.append(Token(kind: .name, text: name))
            } else {
                let pair = at(1).map { String([c, $0]) } ?? ""
                if ["<=", ">=", "==", "!=", "&&", "||"].contains(pair) {
                    tokens.append(Token(kind: .symbol, text: pair))
                    index += 2
                } else if "+-*/%<>!?:(),[]".contains(c) {
                    tokens.append(Token(kind: .symbol, text: String(c)))
                    index += 1
                } else {
                    throw Failure(description: "unexpected \"\(c)\" in \"\(text)\"")
                }
            }
        }
        return tokens
    }

    private struct Parser {
        let tokens: [Token]
        let text: String
        var index = 0

        var atEnd: Bool { index >= tokens.count }
        var peek: Token? { atEnd ? nil : tokens[index] }

        mutating func take(_ symbol: String) -> Bool {
            guard let token = peek, token.kind == .symbol || token.kind == .name, token.text == symbol else { return false }
            index += 1
            return true
        }

        mutating func expect(_ symbol: String) throws {
            guard take(symbol) else { throw Failure(description: "expected \"\(symbol)\" in \"\(text)\"") }
        }

        mutating func expression() throws -> BoardExpression {
            let condition = try binary(0)
            guard take("?") else { return condition }
            let yes = try expression()
            try expect(":")
            return .choice(condition, yes, try expression())
        }

        static let levels: [[String]] = [["||"], ["&&"], ["==", "!="], ["<", "<=", ">", ">=", "in"], ["+", "-"],
                                         ["*", "/", "%"]]

        mutating func binary(_ level: Int) throws -> BoardExpression {
            guard level < Self.levels.count else { return try unary() }
            var left = try binary(level + 1)
            while let token = peek, Self.levels[level].contains(token.text),
                  token.kind == .symbol || token.text == "in" {
                index += 1
                if token.text == "in" {
                    try expect("[")
                    var items: [BoardExpression] = []
                    if !take("]") {
                        repeat { items.append(try expression()) } while take(",")
                        try expect("]")
                    }
                    left = .member(left, items)
                } else {
                    left = .binary(token.text, left, try binary(level + 1))
                }
            }
            return left
        }

        mutating func unary() throws -> BoardExpression {
            if take("-") { return .unary("-", try unary()) }
            if take("!") { return .unary("!", try unary()) }
            return try primary()
        }

        mutating func primary() throws -> BoardExpression {
            guard let token = peek else { throw Failure(description: "\"\(text)\" ends early") }
            index += 1
            switch token.kind {
            case .number: return .value(.number(token.number))
            case .string: return .value(.string(token.text))
            case .name:
                if token.text == "true" { return .value(.bool(true)) }
                if token.text == "false" { return .value(.bool(false)) }
                if take("(") {
                    var arguments: [BoardExpression] = []
                    if !take(")") {
                        repeat { arguments.append(try expression()) } while take(",")
                        try expect(")")
                    }
                    guard BoardExpression.functions.contains(token.text) else {
                        throw Failure(description: "unknown function \"\(token.text)\" in \"\(text)\"")
                    }
                    return .call(token.text, arguments)
                }
                return .name(token.text)
            case .symbol:
                guard token.text == "(" else { throw Failure(description: "unexpected \"\(token.text)\" in \"\(text)\"") }
                let inner = try expression()
                try expect(")")
                return inner
            }
        }
    }

    static let functions: Set<String> = ["min", "max", "abs", "floor", "ceil", "round", "sqrt", "sin", "cos", "interp"]

    // MARK: Evaluating

    func evaluate(_ scope: Scope) throws -> BoardValue {
        switch self {
        case .value(let value):
            return value
        case .name(let name):
            guard let value = try scope(name) else { throw Failure(description: "unknown name \"\(name)\"") }
            return value
        case .unary(let op, let operand):
            let value = try operand.evaluate(scope)
            return op == "-" ? .number(-(try Self.number(value, op))) : .bool(!(try Self.flag(value, op)))
        case .binary(let op, let left, let right):
            switch op {
            case "&&": return .bool(try Self.flag(left.evaluate(scope), op) ? Self.flag(right.evaluate(scope), op) : false)
            case "||": return .bool(try Self.flag(left.evaluate(scope), op) ? true : Self.flag(right.evaluate(scope), op))
            case "==": return .bool(try left.evaluate(scope) == right.evaluate(scope))
            case "!=": return .bool(try left.evaluate(scope) != right.evaluate(scope))
            default: break
            }
            let a = try Self.number(left.evaluate(scope), op), b = try Self.number(right.evaluate(scope), op)
            switch op {
            case "+": return .number(a + b)
            case "-": return .number(a - b)
            case "*": return .number(a * b)
            case "/": return .number(a / b)
            case "%": return .number(a.truncatingRemainder(dividingBy: b))
            case "<": return .bool(a < b)
            case "<=": return .bool(a <= b)
            case ">": return .bool(a > b)
            default: return .bool(a >= b)
            }
        case .member(let item, let list):
            let value = try item.evaluate(scope)
            return .bool(try list.contains { try $0.evaluate(scope) == value })
        case .choice(let condition, let yes, let no):
            return try Self.flag(condition.evaluate(scope), "?") ? yes.evaluate(scope) : no.evaluate(scope)
        case .call(let name, let arguments):
            let values = try arguments.map { try Self.number($0.evaluate(scope), name) }
            return .number(try Self.call(name, values))
        }
    }

    func number(_ scope: Scope) throws -> Double { try Self.number(evaluate(scope), "a number") }

    func flag(_ scope: Scope) throws -> Bool { try Self.flag(evaluate(scope), "a condition") }

    private static func number(_ value: BoardValue, _ use: String) throws -> Double {
        guard case .number(let number) = value else { throw Failure(description: "\(value.quoted) is not a number (\(use))") }
        return number
    }

    private static func flag(_ value: BoardValue, _ use: String) throws -> Bool {
        guard case .bool(let flag) = value else { throw Failure(description: "\(value.quoted) is not true or false (\(use))") }
        return flag
    }

    private static func call(_ name: String, _ values: [Double]) throws -> Double {
        func one() throws -> Double {
            guard values.count == 1 else { throw Failure(description: "\(name) takes 1 argument") }
            return values[0]
        }
        switch name {
        case "min", "max":
            guard !values.isEmpty else { throw Failure(description: "\(name) needs an argument") }
            return name == "min" ? values.min()! : values.max()!
        case "abs": return abs(try one())
        case "floor": return (try one()).rounded(.down)
        case "ceil": return (try one()).rounded(.up)
        case "round": return (try one()).rounded()
        case "sqrt": return (try one()).squareRoot()
        case "sin": return sin(try one() * .pi / 180)
        case "cos": return cos(try one() * .pi / 180)
        default:
            // interp(x, x0, y0, x1, y1, …)
            guard values.count >= 3, values.count % 2 == 1 else {
                throw Failure(description: "interp takes x and pairs of points")
            }
            let x = values[0]
            let points = stride(from: 1, to: values.count, by: 2).map { (values[$0], values[$0 + 1]) }
            for (a, b) in zip(points, points.dropFirst()) where x >= a.0 && x <= b.0 {
                return b.0 == a.0 ? a.1 : a.1 + (b.1 - a.1) * (x - a.0) / (b.0 - a.0)
            }
            return x < points[0].0 ? points[0].1 : points[points.count - 1].1
        }
    }
}

/// A field that is an expression: a number, a flag, or text parsed as one.
nonisolated struct BoardFormula: Hashable, Sendable, Decodable {
    let expression: BoardExpression

    init(_ expression: BoardExpression) { self.expression = expression }

    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()
        switch try container.decode(BoardValue.self) {
        case .string(let text):
            do {
                expression = try BoardExpression.parse(text)
            } catch let failure as BoardExpression.Failure {
                throw DecodingError.dataCorruptedError(in: container, debugDescription: failure.description)
            }
        case let value:
            expression = .value(value)
        }
    }

    func number(_ scope: BoardExpression.Scope) throws -> Double { try expression.number(scope) }
    func flag(_ scope: BoardExpression.Scope) throws -> Bool { try expression.flag(scope) }
    func evaluate(_ scope: BoardExpression.Scope) throws -> BoardValue { try expression.evaluate(scope) }
}
