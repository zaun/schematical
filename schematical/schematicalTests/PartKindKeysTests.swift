import Foundation
import XCTest
@testable import schematical

/// The Part Editor's form has a control for every key the backend accepts
/// for a kind (plan-32-F … J, the drift test): the `KEYS[] = { … }` arrays of
/// the kind's model sources, against `PartDraft.formKeys(kind)`.
final class PartKindKeysTests: XCTestCase {
    /// Each kind's model sources under backend/src/models: its own files, the
    /// shared helpers its loader calls (seq.c for the sequential kinds; the
    /// converter loaders for rtc).
    static let files: [String: [String]] = [
        "gate": ["gate.c"],
        "dff": ["dff.c"], "jkff": ["jkff.c"],
        "mux": ["mux.c", "mux_stored.c", "seq.c"], "decoder": ["decoder.c", "seq.c"],
        "latch": ["latch.c", "seq.c"], "buffer": ["buffer.c", "buffer_reg.c", "seq.c"],
        "arith": ["arith.c", "arith_alu.c", "seq.c"],
        "counter": ["counter.c", "counter_div.c", "seq.c"], "shift": ["shift.c", "shift_special.c", "seq.c"],
        "sram": ["sram.c"], "eeprom": ["eeprom.c"], "switch": ["switch.c"], "led": ["led.c"],
        "7seg": ["display7.c"], "oscillator": ["oscillator.c"], "supply": ["supply.c"],
        "nixie": ["nixie.c"],
        "dekatron": ["dekatron.c"],
        "lcd": ["lcd.c"],
        "powerswitch": ["powerswitch.c"],
        "serialmem": ["serialmem.c"],
        "regulator": ["regulator.c"],
        "crystal": ["crystal.c"],
        // plan-32-J: `rg -l 'KEYS\[\] = '` over the kinds' files (Corrections there).
        "cpu": ["cpu.c"], "pio": ["pio.c", "pio_pia6521.c", "pio_via6522.c"],
        "serial": ["serial.c", "serial_acia6551.c", "serial_ay1013.c", "serial_com78c80x.c", "serial_hs82c52.c",
                   "serial_i8251.c", "serial_i82510.c", "serial_max3100.c", "serial_max310x.c",
                   "serial_sc18is604.c", "serial_target.c", "serial_usbuart.c",
                   "ns16550.c", "ns16550_bridge.c", "ns16550_exar.c"],
        "charlcd": ["charlcd.c"], "connector": ["connector.c"],
        "converter": ["converter.c", "converter_load.c"], "rtc": ["rtc.c", "converter_load.c"],
        // plan-56-G: the display drivers share the converter loader too.
        "leddriver": ["leddriver.c", "converter_load.c"], "lcddriver": ["lcddriver.c", "converter_load.c"],
        "diode": ["diode.c"], "header": ["header.c"],
        "register": ["register.c", "seq.c"],
        // plan-46-C: the last kinds to get a form.
        "resistor": ["resistor.c"], "capacitor": ["capacitor.c"], "transistor": ["transistor.c"],
        "mcu": ["mcu.c"], "pld": ["pld.c"], "generic": ["generic.c"],
    ]

    static var models: URL {
        URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
            .deletingLastPathComponent().appendingPathComponent("backend/src/models")
    }

    /// Loader functions a kind's device loader never calls, by file: their
    /// arrays are not the kind's keys. rtc and converter share
    /// converter_load.c's `cv_load_device(ctx, json, rtc)`: with `rtc` set it
    /// reads `clock` (`load_clock`) and never `dac`, `codeFormat` or
    /// `conversion` (rtc.c's `DEVICE_KEYS` has no "conversion"), so
    /// `load_dac`, `load_choice` (dac's `clearCode` and `codeFormat`),
    /// `load_conversion` and `load_time` (conversion's `time`) are converter
    /// only; without it, it never calls `load_clock`.
    ///
    /// plan-56-G: the `display` section's own arrays (`load_display_scan`,
    /// `_static`, `_ports`, `_lcd`) are read only for the display drivers
    /// (converter and rtc reject `display`). For leddriver and lcddriver they
    /// are the keys *inside* `display`, which the Part Editor's form edits as
    /// one JSON field (Editor/LeddriverDraft.swift), not as controls of their
    /// own: they are skipped here, and `testDisplayDriversEditDisplayAsJSON`
    /// checks the form has the `display` key itself (no `KEYS[]` array names
    /// it: `DEVICE_KEYS` in leddriver.c / lcddriver.c does). A display driver
    /// has no conversion, clock or `ready`, so those loaders are skipped too.
    static let displayLoaders: Set<String> = [
        "load_display_scan", "load_display_static", "load_display_ports", "load_display_lcd",
        "load_display_clock", "clk_cell",
    ]
    static let notCalled: [String: [String: Set<String>]] = [
        "converter": ["converter_load.c": Set(["load_clock"]).union(displayLoaders)],
        "rtc": ["converter_load.c": Set(["load_choice", "load_time", "load_conversion", "load_dac"]).union(displayLoaders)],
        "leddriver": ["converter_load.c": displayLoaders.union(displayDriverSkipped)],
        "lcddriver": ["converter_load.c": displayLoaders.union(displayDriverSkipped)],
    ]
    private static let displayDriverSkipped: Set<String> = [
        "load_clock", "load_choice", "load_time", "load_conversion", "load_dac", "load_ready",
    ]

    /// The string keys of every `KEYS[] = { … }` array in a source file (its
    /// comments removed: a comment may quote a string), with the function
    /// each array is in (nil at file scope).
    static func keyArrays(in file: String) throws -> [(function: String?, keys: [String])] {
        let source = try String(contentsOf: models.appendingPathComponent(file), encoding: .utf8)
        let comments = try NSRegularExpression(pattern: #"/\*.*?\*/|//[^\n]*"#, options: [.dotMatchesLineSeparators])
        let text = comments.stringByReplacingMatches(in: source, range: NSRange(source.startIndex..., in: source),
                                                     withTemplate: "")
        let array = try NSRegularExpression(pattern: #"KEYS\[\]\s*=\s*\{(.*?)\}"#, options: [.dotMatchesLineSeparators])
        let literal = try NSRegularExpression(pattern: #""([^"]*)""#)
        let function = try NSRegularExpression(pattern: #"(?m)^[A-Za-z][^\n;]*?\b([a-z_0-9]+)\([^;{]*\)\s*\{"#)
        let starts = function.matches(in: text, range: NSRange(text.startIndex..., in: text)).map {
            ($0.range.location, String(text[Range($0.range(at: 1), in: text)!]))
        }
        return array.matches(in: text, range: NSRange(text.startIndex..., in: text)).map { match in
            let body = String(text[Range(match.range(at: 1), in: text)!])
            let keys = literal.matches(in: body, range: NSRange(body.startIndex..., in: body)).map {
                String(body[Range($0.range(at: 1), in: body)!])
            }
            // Inside a function when it starts after the function's line and
            // is indented (file-scope arrays start at column 0).
            let enclosing = starts.last { $0.0 < match.range.location }?.1
            let lineStart = (text as NSString).lineRange(for: NSRange(location: match.range.location, length: 0)).location
            let indented = match.range.location > lineStart
                && (text as NSString).substring(with: NSRange(location: lineStart, length: 1)) == " "
            return (indented ? enclosing : nil, keys)
        }
    }

    /// Keys of a shared helper in seq.c that only that helper reads: a kind
    /// whose own sources never call the helper (in that mode) does not accept
    /// them, so the form needs no control for them. Derived from the sources.
    static func helperOnlyKeys(kind: String) throws -> Set<String> {
        let own = try Self.files[kind]!.filter { $0 != "seq.c" }.map {
            try String(contentsOf: models.appendingPathComponent($0), encoding: .utf8)
        }.joined(separator: "\n")
        guard Self.files[kind]!.contains("seq.c") else { return [] }
        var exempt = Set<String>()
        if !own.contains("seq_gate_name(") { exempt.insert("op") }
        if !own.contains("seq_clock_name(") { exempt.insert("edge") }
        // "when" only with allow_when set: seq_control_name(ctx, json, key, required, 1, …)
        // or seq_control_object(ctx, item, 1, …).
        let when = try NSRegularExpression(
            pattern: #"seq_control_name\([^;]*?"[A-Za-z]+",\s*[01],\s*1\s*,|seq_control_object\(\s*ctx,\s*\w+,\s*1\s*,"#)
        if when.firstMatch(in: own, range: NSRange(own.startIndex..., in: own)) == nil { exempt.insert("when") }
        return exempt
    }

    /// The loader reads `display` straight from the device object, so no
    /// `KEYS[]` array lists it; the two display drivers' forms must still
    /// have it, required, as a JSON field.
    func testDisplayDriversEditDisplayAsJSON() throws {
        for kind in ["leddriver", "lcddriver"] {
            let schema = try XCTUnwrap(PartKinds.schemas[kind], kind)
            let field = try XCTUnwrap(schema.device.first { $0.key == "display" }, "\(kind): no display field")
            XCTAssertTrue(field.required, kind)
            if case .json = field.type {} else { XCTFail("\(kind): display is not a JSON field") }
            // Device keys of the converter and rtc kinds that the loader rejects here.
            for gone in ["clock", "ready", "conversion", "type", "bits", "channels", "codeFormat", "dac"] {
                XCTAssertFalse(schema.device.contains { $0.key == gone }, "\(kind) form has \(gone), the loader rejects it")
            }
        }
    }

    func testFormKeysCoverTheBackendKeys() throws {
        var missing: [String: [String]] = [:]
        for kind in Self.files.keys.sorted() {
            var keys = Set<String>()
            var arrays = 0
            for file in Self.files[kind]! {
                let skip = Self.notCalled[kind]?[file] ?? []
                let all = try Self.keyArrays(in: file)
                for name in skip {
                    XCTAssertTrue(all.contains { $0.function == name }, "\(file) has no KEYS array in \(name)")
                }
                let found = all.filter { $0.function.map { !skip.contains($0) } ?? true }
                arrays += found.count
                keys.formUnion(found.flatMap(\.keys))
            }
            let exempt = try Self.helperOnlyKeys(kind: kind)
            print("\(kind): \(arrays) KEYS arrays, \(keys.count) keys, seq.c helper keys not read: \(exempt.sorted())")
            let lacking = keys.subtracting(PartDraft.formKeys(kind)).subtracting(exempt).sorted()
            if !lacking.isEmpty { missing[kind] = lacking }
        }
        XCTAssertEqual(missing, [:], "keys the form has no control for: \(missing)")
    }
}
