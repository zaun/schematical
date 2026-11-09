//
//  SerialPort.swift
//  schematical
//
//  The port of a USB-to-UART bridge's channel, as the PC sets it: baud,
//  character format and flow control (circuit_serial_configure). Other
//  serial parts set their own baud and format from their registers.
//

import Foundation

nonisolated struct SerialPort: Hashable, Sendable {
    enum Parity: UInt32, CaseIterable, Sendable {
        case none = 0, odd, even, mark, space

        /// The letter of "8N1".
        var letter: String {
            switch self {
            case .none: "N"
            case .odd: "O"
            case .even: "E"
            case .mark: "M"
            case .space: "S"
            }
        }
    }

    var baud: UInt32 = 9600
    var dataBits: UInt32 = 8
    var parity: Parity = .none
    /// 1 or 2.
    var stopBits: UInt32 = 1
    var rtsCts = false

    /// The baud rates the terminal offers.
    static let bauds: [UInt32] = [300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200,
                                  230400, 460800, 921600]

    /// The formats the terminal offers.
    static let formats: [SerialPort] = [
        SerialPort(dataBits: 8, parity: .none, stopBits: 1),
        SerialPort(dataBits: 8, parity: .even, stopBits: 1),
        SerialPort(dataBits: 8, parity: .odd, stopBits: 1),
        SerialPort(dataBits: 8, parity: .none, stopBits: 2),
        SerialPort(dataBits: 7, parity: .even, stopBits: 1),
        SerialPort(dataBits: 7, parity: .odd, stopBits: 1),
        SerialPort(dataBits: 7, parity: .none, stopBits: 2),
    ]

    /// "8N1".
    var formatName: String { "\(dataBits)\(parity.letter)\(stopBits)" }

    /// This port with `format`'s data bits, parity and stop bits.
    func withFormat(_ format: SerialPort) -> SerialPort {
        var port = self
        port.dataBits = format.dataBits
        port.parity = format.parity
        port.stopBits = format.stopBits
        return port
    }

    /// Sets the channel's port in a circuit: true when taken.
    func apply(to circuit: OpaquePointer, part: Int, channel: Int) -> Bool {
        circuit_serial_configure(circuit, part, UInt32(channel), baud, dataBits, parity.rawValue,
                                 stopBits * 2, rtsCts ? 1 : 0) == 0
    }
}
