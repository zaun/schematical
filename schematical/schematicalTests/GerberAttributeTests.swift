import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// One drawn Gerber object and the attributes in force when it was made.
struct GerberRecord: Equatable {
    var kind: String          // "flash", "draw" or "region"
    var function: String?     // .AperFunction of its aperture (or, for a region, in force at G36)
    var net: String?          // .N, nil when none in force
    var pin: [String]?        // .P: reference, number
    var dark: Bool
    var point: String?        // "X…Y…" of a flash or a draw's end
}

/// Reads a Gerber file's attribute commands the way the spec scopes them (§ 5.3–5.5): an aperture takes the
/// `.AperFunction` in force at its `%ADD`, a region the one in force at `G36`; object attributes are in force
/// until deleted. `problems` lists anything the spec forbids (an attribute command inside a region).
enum GerberAttributeReader {
    static func read(_ text: String) -> (objects: [GerberRecord], problems: [String]) {
        var objects: [GerberRecord] = []
        var problems: [String] = []
        var pendingFunction: String?
        var functionOfCode: [Int: String?] = [:]
        var current: Int?
        var net: String?, pin: [String]?
        var dark = true
        var inRegion = false
        for line in text.split(separator: "\n").map(String.init) {
            if inRegion, line.hasPrefix("%") { problems.append("attribute inside a region: \(line)") }
            if line.hasPrefix("%TA.AperFunction,") {
                // A function is several fields (`SMDPad,CuDef`); its commas are not escaped.
                pendingFunction = String(line.dropFirst("%TA.AperFunction,".count).dropLast(2))
            } else if line == "%TD.AperFunction*%" {
                pendingFunction = nil
            } else if line == "%TD*%" {
                pendingFunction = nil; net = nil; pin = nil
            } else if line.hasPrefix("%ADD") {
                let digits = line.dropFirst(4).prefix { $0.isNumber }
                if let code = Int(digits) { functionOfCode[code] = pendingFunction }
            } else if line.hasPrefix("%TO.N,") {
                net = unescape(String(line.dropFirst("%TO.N,".count).dropLast(2)))
            } else if line.hasPrefix("%TO.P,") {
                pin = line.dropFirst("%TO.P,".count).dropLast(2).split(separator: ",", omittingEmptySubsequences: false)
                    .map { unescape(String($0)) }
            } else if line == "%TD.N*%" {
                net = nil
            } else if line == "%TD.P*%" {
                pin = nil
            } else if line == "%LPD*%" {
                dark = true
            } else if line == "%LPC*%" {
                dark = false
            } else if line.hasPrefix("D"), line.hasSuffix("*"), let code = Int(line.dropFirst().dropLast()), code >= 10 {
                current = code
            } else if line == "G36*" {
                inRegion = true
                objects.append(GerberRecord(kind: "region", function: pendingFunction, net: net, pin: pin, dark: dark,
                                            point: nil))
            } else if line == "G37*" {
                inRegion = false
            } else if !inRegion, line.hasSuffix("D03*") {
                objects.append(GerberRecord(kind: "flash", function: current.flatMap { functionOfCode[$0] ?? nil },
                                            net: net, pin: pin, dark: dark, point: String(line.dropLast(4))))
            } else if !inRegion, line.hasSuffix("D01*") {
                objects.append(GerberRecord(kind: "draw", function: current.flatMap { functionOfCode[$0] ?? nil },
                                            net: net, pin: pin, dark: dark, point: String(line.dropLast(4))))
            }
        }
        return (objects, problems)
    }

    private static func unescape(_ text: String) -> String {
        text.replacingOccurrences(of: "\\u002C", with: ",").replacingOccurrences(of: "\\u002A", with: "*")
            .replacingOccurrences(of: "\\u0025", with: "%").replacingOccurrences(of: "\\u005C", with: "\\")
    }
}

/// plan-50-D: aperture functions and net / pin attributes on Gerber copper.
@MainActor
final class GerberAttributeTests: XCTestCase {
    private let rules = DesignRules.pcbway

    private func file(_ files: [GerberExport.File], _ suffix: String) throws -> String {
        try XCTUnwrap(files.first { $0.name.hasSuffix(suffix) }, "no \(suffix)").text
    }

    // MARK: The writer

    func testAFileWithNoAttributesIsUnchanged() {
        var writer = GerberWriter(attributes: ["FileFunction,Copper,L1,Top"], top: 10)
        writer.fill(.circle(centre: CGPoint(x: 1, y: 2), diameter: 0.5))
        writer.stroke([CGPoint(x: 0, y: 0), CGPoint(x: 5, y: 0)], width: 0.25)
        writer.region([CGPoint(x: 0, y: 0), CGPoint(x: 1, y: 0), CGPoint(x: 1, y: 1)])
        let golden = """
        G04 Schematical Gerber export*
        %TF.FileFunction,Copper,L1,Top*%
        %FSLAX46Y46*%
        %MOMM*%
        %LPD*%
        G01*
        G75*
        %ADD10C,0.500000*%
        %ADD11C,0.250000*%
        D10*
        X1000000Y8000000D03*
        D11*
        X0Y10000000D02*
        X5000000Y10000000D01*
        G36*
        X0Y10000000D02*
        X1000000Y10000000D01*
        X1000000Y9000000D01*
        X0Y10000000D01*
        G37*
        M02*

        """
        XCTAssertEqual(String(decoding: writer.data(), as: UTF8.self), golden)
    }

    func testOneFunctionOneAperture() {
        var writer = GerberWriter(attributes: [], top: 10)
        writer.setFunction("ViaPad")
        writer.fill(.circle(centre: CGPoint(x: 1, y: 1), diameter: 0.8))
        writer.setFunction("ComponentPad")
        writer.fill(.circle(centre: CGPoint(x: 2, y: 1), diameter: 0.8))
        writer.fill(.circle(centre: CGPoint(x: 3, y: 1), diameter: 0.8))
        writer.setFunction(nil)
        writer.fill(.circle(centre: CGPoint(x: 4, y: 1), diameter: 0.8))
        let text = String(decoding: writer.data(), as: UTF8.self)
        XCTAssertTrue(text.contains("%TA.AperFunction,ViaPad*%\n%ADD10C,0.800000*%\n%TD.AperFunction*%"), text)
        XCTAssertTrue(text.contains("%TA.AperFunction,ComponentPad*%\n%ADD11C,0.800000*%\n%TD.AperFunction*%"), text)
        XCTAssertTrue(text.contains("\n%ADD12C,0.800000*%\n"), "no function, no attribute lines around it")
        let objects = GerberAttributeReader.read(text).objects
        XCTAssertEqual(objects.map(\.function), ["ViaPad", "ComponentPad", "ComponentPad", nil])
    }

    func testObjectAttributesFollowTheFlashesAndAreDeletedWhenDropped() {
        var writer = GerberWriter(attributes: [], top: 10)
        writer.setFunction("SMDPad,CuDef")
        writer.setObject(.init(net: "GND", pin: .init(reference: "U1", number: "7")))
        writer.fill(.rect(CGRect(x: 0, y: 0, width: 1, height: 1)))
        writer.setObject(.init(net: "GND", pin: .init(reference: "U1", number: "8")))
        writer.fill(.rect(CGRect(x: 2, y: 0, width: 1, height: 1)))
        writer.setObject(.init(net: ""))
        writer.fill(.rect(CGRect(x: 4, y: 0, width: 1, height: 1)))
        writer.setObject(nil)
        writer.fill(.rect(CGRect(x: 6, y: 0, width: 1, height: 1)))
        let text = String(decoding: writer.data(), as: UTF8.self)
        // GND is written once for the two pads; the second pad changes only the pin.
        XCTAssertEqual(text.components(separatedBy: "%TO.N,GND*%").count - 1, 1)
        let objects = GerberAttributeReader.read(text).objects
        XCTAssertEqual(objects.map(\.net), ["GND", "GND", "", nil])
        XCTAssertEqual(objects.map(\.pin), [["U1", "7"], ["U1", "8"], nil, nil])
        XCTAssertTrue(objects.allSatisfy { $0.function == "SMDPad,CuDef" })
        XCTAssertTrue(text.contains("%TO.N,*%"), "an empty net is `%TO.N,*%`")
    }

    func testARegionTakesTheFunctionInForceAtG36() {
        var writer = GerberWriter(attributes: [], top: 10)
        writer.setFunction("Conductor")
        writer.setObject(.init(net: "5V"))
        writer.region([CGPoint(x: 0, y: 0), CGPoint(x: 5, y: 0), CGPoint(x: 5, y: 5)])
        writer.setFunction(nil)
        writer.region([CGPoint(x: 0, y: 0), CGPoint(x: 1, y: 0), CGPoint(x: 1, y: 1)])
        let read = GerberAttributeReader.read(String(decoding: writer.data(), as: UTF8.self))
        XCTAssertEqual(read.objects.map(\.function), ["Conductor", nil])
        XCTAssertEqual(read.objects.first?.net, "5V")
        XCTAssertTrue(read.problems.isEmpty, "\(read.problems)")
    }

    func testClearPolarityDropsEveryAttribute() {
        var writer = GerberWriter(attributes: [], top: 10)
        writer.setFunction("Conductor")
        writer.setObject(.init(net: "GND"))
        writer.region([CGPoint(x: 0, y: 0), CGPoint(x: 9, y: 0), CGPoint(x: 9, y: 9)])
        writer.setDark(false)
        writer.fill(.circle(centre: CGPoint(x: 1, y: 1), diameter: 0.5))
        writer.setDark(true)
        writer.fill(.circle(centre: CGPoint(x: 2, y: 1), diameter: 0.5))
        let objects = GerberAttributeReader.read(String(decoding: writer.data(), as: UTF8.self)).objects
        XCTAssertEqual(objects.map(\.dark), [true, false, true])
        XCTAssertEqual(objects.map(\.net), ["GND", nil, nil])
        XCTAssertEqual(objects.map(\.function), ["Conductor", nil, nil])
    }

    func testFieldEscaping() {
        XCTAssertEqual(GerberWriter.fieldEscaped("a,b*c%d\\e"), "a\\u002Cb\\u002Ac\\u0025d\\u005Ce")
        XCTAssertEqual(GerberWriter.fieldEscaped("Net-(U1-A0)"), "Net-(U1-A0)")
        var writer = GerberWriter(attributes: [], top: 10)
        writer.setObject(.init(net: "a,b"))
        writer.fill(.circle(centre: .zero, diameter: 1))
        let text = String(decoding: writer.data(), as: UTF8.self)
        XCTAssertTrue(text.contains("%TO.N,a\\u002Cb*%"))
    }

    // MARK: The export

    func testEveryCopperObjectHasANetAndTheRightFunction() throws {
        let doc = try ExportSamples.vias()
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        for suffix in [".GTL", ".GBL"] {
            let read = GerberAttributeReader.read(try file(files, suffix))
            XCTAssertTrue(read.problems.isEmpty, "\(read.problems)")
            XCTAssertFalse(read.objects.isEmpty, suffix)
            for object in read.objects { XCTAssertNotNil(object.net, "\(suffix) \(object)") }
        }
        let top = GerberAttributeReader.read(try file(files, ".GTL")).objects
        let references = Set(doc.chips.map(\.reference))
        for object in top where object.function == "ComponentPad" || object.function == "SMDPad,CuDef" {
            XCTAssertTrue(references.contains(try XCTUnwrap(object.pin?.first)), "\(object)")
        }
        XCTAssertEqual(top.filter { $0.function == "ViaPad" }.count, 4)
        // The vias sit on the trace between U1 pin 3 and U2 pin 1: that net.
        let traceNet = try XCTUnwrap(top.first { $0.function == "Conductor" }?.net)
        XCTAssertFalse(traceNet.isEmpty)
        XCTAssertEqual(top.filter { $0.function == "ViaPad" }.map(\.net), Array(repeating: traceNet, count: 4))
        let pins = top.filter { $0.function == "ComponentPad" }.filter { $0.net == traceNet }.compactMap(\.pin)
        XCTAssertEqual(Set(pins.map { $0.joined(separator: " ") }), Set(["U1 3", "U2 1"]))
        // The bottom has the through-hole pads and the vias, no trace.
        let bottom = GerberAttributeReader.read(try file(files, ".GBL")).objects
        XCTAssertEqual(bottom.filter { $0.function == "ViaPad" }.count, 4)
        XCTAssertFalse(bottom.contains { $0.function == "Conductor" })
    }

    func testPlanesAndCentrePadsAndConnectors() throws {
        let doc = try ExportSamples.fourLayer()
        let files = GerberExport.files(for: doc, base: "b", rules: rules)
        let ground = GerberAttributeReader.read(try file(files, ".G1"))
        XCTAssertTrue(ground.problems.isEmpty, "\(ground.problems)")
        let plane = try XCTUnwrap(ground.objects.first { $0.kind == "region" })
        XCTAssertEqual(plane.function, "Conductor")
        XCTAssertEqual(plane.net, "GND")
        let clears = ground.objects.filter { !$0.dark }
        XCTAssertFalse(clears.isEmpty)
        XCTAssertTrue(clears.allSatisfy { $0.net == nil && $0.function == nil })
        XCTAssertEqual(try XCTUnwrap(GerberAttributeReader.read(try file(files, ".G2")).objects.first).net, "5V")
        // The VQFN part's centre pad is unnumbered.
        let top = GerberAttributeReader.read(try file(files, ".GTL")).objects
        let centre = try XCTUnwrap(top.first { $0.function == "HeatsinkPad" })
        XCTAssertEqual(centre.pin?.count, 2)
        XCTAssertEqual(centre.pin?.last, "")
    }

    func testCardFingersAndHeaderPads() throws {
        let card = TestSupport.newDocument()
        card.setBoardType(.pci)
        let fingers = GerberAttributeReader.read(try file(GerberExport.files(for: card, base: "b", rules: rules), ".GTL")).objects
        let connector = fingers.filter { $0.function == "ConnectorPad" }
        XCTAssertFalse(connector.isEmpty)
        XCTAssertTrue(connector.allSatisfy { $0.pin?.first == "EDGE1" && ($0.pin?.last.map { !$0.isEmpty } ?? false) })
        let wing = TestSupport.newDocument()
        wing.setBoardType(.featherWing)
        let header = GerberAttributeReader.read(try file(GerberExport.files(for: wing, base: "b", rules: rules), ".GTL")).objects
        let pads = header.filter { $0.function == "ComponentPad" }
        XCTAssertFalse(pads.isEmpty)
        XCTAssertTrue(pads.allSatisfy { $0.pin?.first == "HEADER1" })
    }

    func testEveryFileHasAProjectAndAPart() throws {
        let files = GerberExport.files(for: try ExportSamples.vias(), base: "b", rules: rules)
        let uuid = KiCad.uuid("gerber/b")
        for file in files where !file.name.hasSuffix(".drl") && !file.name.hasSuffix(".gbrjob") {
            XCTAssertTrue(file.text.contains("%TF.ProjectId,b,\(uuid),1*%"), file.name)
            XCTAssertTrue(file.text.contains("%TF.Part,Single*%"), file.name)
        }
        let outline = try file(files, ".GKO")
        XCTAssertTrue(outline.contains("%TA.AperFunction,Profile*%\n%ADD10C,0.100000*%"), outline)
    }

    func testTheExportIsDeterministic() throws {
        let doc = try ExportSamples.fourLayer()
        let first = GerberExport.files(for: doc, base: "b", rules: rules)
        let second = GerberExport.files(for: doc, base: "b", rules: rules)
        XCTAssertEqual(first, second)
    }
}
