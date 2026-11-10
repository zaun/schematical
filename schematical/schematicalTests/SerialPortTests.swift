import CoreGraphics
import Foundation
import XCTest
@testable import schematical

/// USB-to-UART bridges: the drawer's host label and the terminal's port
/// settings (SerialPort), kept per channel across runs.
@MainActor
final class SerialPortTests: XCTestCase {
    /// Simulations made by tests, kept like TestSupport's documents.
    private static var keptSimulations: [SimulationController] = []

    func testBridgeShowsUSBInTheDrawer() throws {
        let part = try TestSupport.part("XR21V1412IL32-F")
        guard case let .serial(model, channels, _, host) = part.details else {
            return XCTFail("not a serial part")
        }
        XCTAssertEqual(model, "usbuart")
        XCTAssertEqual(channels, 2)
        XCTAssertEqual(host, "usb")
        XCTAssertEqual(part.detailLabels, ["USB", "2 channels"])
    }

    func testFormatNames() {
        XCTAssertEqual(SerialPort().formatName, "8N1")
        XCTAssertEqual(SerialPort.formats.map(\.formatName), ["8N1", "8E1", "8O1", "8N2", "7E1", "7O1", "7N2"])
        var port = SerialPort()
        port.baud = 115200
        port.rtsCts = true
        let changed = port.withFormat(SerialPort.formats[4])
        XCTAssertEqual(changed.formatName, "7E1")
        XCTAssertEqual(changed.baud, 115200, "a format change keeps the baud")
        XCTAssertTrue(changed.rtsCts, "and the flow control")
    }

    func testPortIsAppliedBeforeAndDuringRuns() throws {
        let doc = TestSupport.newDocument()
        let chip = try TestSupport.place(doc, "XR21V1410IL16-F", at: CGPoint(x: 200, y: 200))
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.keptSimulations.append(simulation)

        var port = SerialPort()
        port.baud = 115200
        XCTAssertNil(simulation.setSerialPort(port, of: chip.id, channel: 0), "stored while stopped")
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        XCTAssertEqual(simulation.serialPort(of: chip.id, channel: 0).baud, 115200)

        port = port.withFormat(SerialPort.formats[4])
        XCTAssertNil(simulation.setSerialPort(port, of: chip.id, channel: 0), "taken while running")
        simulation.stop()
        XCTAssertEqual(simulation.serialPort(of: chip.id, channel: 0).formatName, "7E1",
                       "kept after the run")
    }
}
