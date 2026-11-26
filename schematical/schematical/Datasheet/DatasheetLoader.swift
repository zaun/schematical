import Foundation
import PDFKit

/// Downloads a datasheet PDF (plan-46-G: moved out of the datasheet window so
/// the Part Wizard's pane loads the same way).
enum DatasheetLoader {
    enum Failure: Error, Equatable {
        /// The site answered with a status outside 200–299.
        case status(Int)
        /// The body is not a PDF.
        case notPDF
        /// The request itself failed (no network, a bad host).
        case network(String)

        var message: String {
            switch self {
            case let .status(code):
                "The manufacturer's site answered \(code) "
                    + "(\(HTTPURLResponse.localizedString(forStatusCode: code)))."
            case .notPDF:
                "The manufacturer's site did not send a PDF."
            case let .network(text):
                text
            }
        }
    }

    /// Some manufacturer sites refuse requests with no browser agent.
    static let userAgent = "Mozilla/5.0 (Macintosh) Schematical"

    /// `session` is the shared one in the app; tests pass one with a stub protocol.
    static func load(_ url: URL, session: URLSession = .shared) async -> Result<PDFDocument, Failure> {
        do {
            var request = URLRequest(url: url)
            request.setValue(userAgent, forHTTPHeaderField: "User-Agent")
            let (data, response) = try await session.data(for: request)
            return document(data: data, status: (response as? HTTPURLResponse)?.statusCode)
        } catch {
            return .failure(.network(error.localizedDescription))
        }
    }

    /// The checks on an answer: a 2xx status (when there is one) and a PDF body.
    static func document(data: Data, status: Int?) -> Result<PDFDocument, Failure> {
        if let status, !(200..<300).contains(status) { return .failure(.status(status)) }
        guard let document = PDFDocument(data: data) else { return .failure(.notPDF) }
        return .success(document)
    }
}

/// What the Part Wizard's datasheet step makes of a typed address
/// (plan-46-G § 1; the rules are the loader's, docs/parts-format.md
/// § Datasheets).
enum DatasheetURLRule: Equatable {
    /// An `https://` address the part can keep.
    case ok(String)
    /// An `http://` address: offered as this `https://` one.
    case offerHTTPS(String)
    /// Why the address cannot be used.
    case message(String)
    /// Nothing typed.
    case empty

    static let needsHTTPS = "The address must start with https://"
    static let needsManufacturer = "A datasheet needs a manufacturer"

    static func check(_ text: String, manufacturer: String?) -> DatasheetURLRule {
        let text = text.trimmingCharacters(in: .whitespacesAndNewlines)
        if text.isEmpty { return .empty }
        // The loader refuses a datasheet on a part with no maker, and on a
        // "Generic" one.
        if (manufacturer ?? "").isEmpty || manufacturer == "Generic" { return .message(needsManufacturer) }
        let lower = text.lowercased()
        if lower.hasPrefix("https://"), text.count > "https://".count { return .ok(text) }
        if lower.hasPrefix("http://"), text.count > "http://".count {
            return .offerHTTPS("https://" + text.dropFirst("http://".count))
        }
        return .message(needsHTTPS)
    }
}
