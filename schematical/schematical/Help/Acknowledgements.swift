//
//  Acknowledgements.swift
//  schematical
//
//  The third-party code the app ships, and its licences, in the About panel.
//

import AppKit
import SwiftUI

/// One piece of third-party code in the app. Its licence text is read from
/// the copy of the licence file the bundle carries, so the panel always shows
/// the text that ships with the code.
struct Acknowledgement: Equatable {
    let name: String
    /// What it is, and what the app uses it for.
    let use: String
    /// The licence file's name in the bundle's resources.
    let licenceFile: String

    /// Everything third-party the app ships: the backend's vendored C code
    /// (`backend/src/third_party`) and the Markdown datasheet renderer's
    /// scripts (`Datasheet/Renderer`).
    static let all: [Acknowledgement] = [
        Acknowledgement(
            name: "cJSON",
            use: "JSON parser, by Dave Gamble and the cJSON contributors. Reads the part files.",
            licenceFile: "LICENSE"),
        Acknowledgement(
            name: "floooh/chips",
            use: "6502, 65C02 and Z80 processor cores and disassemblers, by Andre Weissflog. "
                + "Unmodified. https://github.com/floooh/chips",
            licenceFile: "LICENSE-chips"),
        Acknowledgement(
            name: "jsmooch",
            use: "WDC 65816 processor core and disassembler, by raddad772, from jsmooch-emus. "
                + "Modified for this app.",
            licenceFile: "LICENSE-jsmooch"),
        Acknowledgement(
            name: "avr8js",
            use: "AVR microcontroller simulation, by Uri Shaked and contributors. "
                + "Ported to C for this app. https://github.com/wokwi/avr8js",
            licenceFile: "LICENSE-avr8js"),
        Acknowledgement(
            name: "markdown-it",
            use: "Markdown parser, by Vitaly Puzrin and Alex Kocharin. Renders datasheets.",
            licenceFile: "markdown-it-LICENSE.txt"),
        Acknowledgement(
            name: "WaveDrom",
            use: "Timing diagram renderer, by Aliaksei Chapyzhenka. Draws datasheet waveforms.",
            licenceFile: "wavedrom-LICENSE.txt"),
    ]

    /// The licence's text, or nil when the bundle has no such file.
    func licenceText(in bundle: Bundle = .main) -> String? {
        let file = licenceFile as NSString
        let ext = file.pathExtension
        guard let url = bundle.url(forResource: file.deletingPathExtension,
                                   withExtension: ext.isEmpty ? nil : ext),
              let text = try? String(contentsOf: url, encoding: .utf8) else { return nil }
        return text
    }

    /// A licence's paragraphs, with the file's hard line breaks removed so the
    /// text wraps to the panel's width. The words are unchanged. A blank line
    /// ends a paragraph; a copyright line is its own paragraph, and so is each
    /// numbered condition.
    static func paragraphs(of licence: String) -> [String] {
        var paragraphs: [String] = []
        var current: [String] = []
        func end() {
            if !current.isEmpty { paragraphs.append(current.joined(separator: " ")) }
            current = []
        }
        for raw in licence.components(separatedBy: .newlines) {
            let line = raw.trimmingCharacters(in: .whitespaces)
            if line.isEmpty { end(); continue }
            let numbered = line.range(of: #"^\d+\.\s"#, options: .regularExpression) != nil
            let copyright = line.hasPrefix("Copyright")
            if numbered || copyright { end() }
            current.append(line)
            if copyright { end() }
        }
        end()
        return paragraphs
    }

    /// The About panel's credits: each entry's name, use and full licence.
    static func credits(in bundle: Bundle = .main) -> NSAttributedString {
        let size = NSFont.smallSystemFontSize
        let paragraph = NSMutableParagraphStyle()
        paragraph.paragraphSpacing = 6
        let body: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: size),
            .foregroundColor: NSColor.labelColor,
            .paragraphStyle: paragraph,
        ]
        var heading = body
        heading[.font] = NSFont.boldSystemFont(ofSize: size)
        var fine = body
        fine[.font] = NSFont.systemFont(ofSize: size - 1)
        fine[.foregroundColor] = NSColor.secondaryLabelColor

        let credits = NSMutableAttributedString(
            string: "Acknowledgements\n", attributes: heading)
        credits.append(NSAttributedString(
            string: "Schematical includes the following software.\n", attributes: body))
        for entry in all {
            credits.append(NSAttributedString(string: "\n\(entry.name)\n", attributes: heading))
            credits.append(NSAttributedString(string: "\(entry.use)\n", attributes: body))
            let licence = entry.licenceText(in: bundle).map { paragraphs(of: $0) } ?? []
            for text in licence {
                credits.append(NSAttributedString(string: "\(text)\n", attributes: fine))
            }
        }
        return credits
    }
}

/// Schematical › About Schematical: the standard panel, with the
/// acknowledgements as its credits.
struct AboutCommands: Commands {
    var body: some Commands {
        CommandGroup(replacing: .appInfo) {
            Button("About Schematical") {
                NSApp.orderFrontStandardAboutPanel(options: [.credits: Acknowledgement.credits()])
            }
        }
    }
}
