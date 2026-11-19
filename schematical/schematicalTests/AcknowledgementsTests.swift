import XCTest
@testable import schematical

/// The About panel names every piece of third-party code the app ships and
/// shows its licence, read from the bundle.
final class AcknowledgementsTests: XCTestCase {
    func testNamesTheThirdPartyCode() {
        XCTAssertEqual(Acknowledgement.all.map(\.name),
                       ["cJSON", "floooh/chips", "jsmooch", "markdown-it", "WaveDrom"])
    }

    func testEveryLicenceIsInTheBundle() {
        let holders = [
            "cJSON": "Copyright (c) 2009-2017 Dave Gamble and cJSON contributors",
            "floooh/chips": "Copyright (c) 2018 Andre Weissflog",
            "jsmooch": "Copyright (c) 2024 raddad772",
            "markdown-it": "Copyright (c) 2014 Vitaly Puzrin, Alex Kocharin.",
            "WaveDrom": "Aliaksei Chapyzhenka",
        ]
        for entry in Acknowledgement.all {
            let text = entry.licenceText()
            XCTAssertNotNil(text, entry.licenceFile)
            XCTAssertTrue(text?.contains(holders[entry.name] ?? "?") ?? false, entry.name)
        }
    }

    /// The credits carry each copyright notice and the whole permission
    /// notice, which is what the MIT and zlib licences ask for.
    func testCreditsCarryTheNotices() {
        let credits = Acknowledgement.credits().string
        for entry in Acknowledgement.all {
            XCTAssertTrue(credits.contains("\n\(entry.name)\n"), entry.name)
        }
        for notice in [
            "Copyright (c) 2009-2017 Dave Gamble and cJSON contributors",
            "Copyright (c) 2018 Andre Weissflog",
            "Copyright (c) 2024 raddad772",
            "The above copyright notice and this permission notice shall be included in "
                + "all copies or substantial portions of the Software.",
            "3. This notice may not be removed or altered from any source distribution.",
        ] {
            XCTAssertTrue(credits.contains(notice), notice)
        }
    }

    /// Unwrapping a licence changes its line breaks only, never its words.
    func testParagraphsKeepEveryWord() {
        func words(_ text: String) -> [String] {
            text.components(separatedBy: .whitespacesAndNewlines).filter { !$0.isEmpty }
        }
        for entry in Acknowledgement.all {
            guard let text = entry.licenceText() else { return XCTFail(entry.licenceFile) }
            let paragraphs = Acknowledgement.paragraphs(of: text)
            XCTAssertEqual(words(paragraphs.joined(separator: "\n")), words(text), entry.name)
        }
    }

    func testParagraphsSplitCopyrightAndConditions() {
        let licence = """
            zlib/libpng license

            Copyright (c) 2018 Someone
            This software is provided
            'as-is'.
                1. First
                condition.
                2. Second.
            """
        XCTAssertEqual(Acknowledgement.paragraphs(of: licence), [
            "zlib/libpng license",
            "Copyright (c) 2018 Someone",
            "This software is provided 'as-is'.",
            "1. First condition.",
            "2. Second.",
        ])
    }
}
