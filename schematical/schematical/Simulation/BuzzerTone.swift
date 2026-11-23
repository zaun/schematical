//
//  BuzzerTone.swift
//  schematical
//
//  What each buzzer of a running circuit sounds like (plan-47-N § 3.3): a
//  pure function of two simulation snapshots, so tests and the harness check
//  the tone without an audio device.
//

import Foundation

enum BuzzerTone {
    /// One buzzer part of the circuit, read once when the netlist is built.
    struct Buzzer: Hashable, Sendable {
        let chip: UUID
        /// The part's index in the circuit.
        let part: Int
        /// A transducer (external drive) sounds at the frequency it is driven
        /// at; an indicator (built-in driver) at its rated one.
        let isTransducer: Bool
        /// The rated frequency, hertz.
        let hz: Double
        /// The net of the `+` pin; nil when the pin has no wire.
        let plusNet: Int?
    }

    /// The circuit time a transducer's pitch is averaged over, picoseconds.
    static let windowPS: UInt64 = 100_000_000_000

    /// The pitches a transducer's drive is heard at, hertz.
    static let audible: ClosedRange<Double> = 20...20000

    /// The fewest value changes a frame must hold for its own count to be
    /// trusted over the window's average (a frame at a low speed holds few).
    static let minimumFrameChanges: UInt64 = 8

    /// The tone of each buzzer that sounds, by chip, in hertz.
    ///
    /// - An **indicator** whose light gained on-time since `previous` sounds
    ///   at its part's rated frequency.
    /// - A **transducer** whose light gained on-time sounds at (value
    ///   changes its `+` net gained) ÷ 2 ÷ (circuit seconds), when that is
    ///   audible: one the drive holds at a steady level gains no changes and
    ///   is silent, as the real part is. The count is taken over `window`, an
    ///   older snapshot, so the pitch is steady (a square wave's count is
    ///   one change short or long in any short frame); when the last frame's
    ///   own count (at least `minimumFrameChanges`) differs from the
    ///   window's by more than 30 %, the drive has just changed and the
    ///   frame's pitch is used.
    /// - A buzzer whose `+` pin has no net has no voice.
    ///
    /// Empty when there is no `previous` snapshot or no circuit time passed.
    static func voices(netlist: SchematicNetlist, snapshot: SimulationSnapshot, previous: SimulationSnapshot?,
                       window: SimulationSnapshot? = nil) -> [UUID: Double] {
        guard let previous, snapshot.error == nil, snapshot.circuitTime > previous.circuitTime else { return [:] }
        var result: [UUID: Double] = [:]
        for buzzer in netlist.buzzers {
            guard let net = buzzer.plusNet,
                  let after = snapshot.lightTimes[buzzer.part]?.first,
                  let before = previous.lightTimes[buzzer.part]?.first, after > before else { continue }
            guard buzzer.isTransducer else {
                if buzzer.hz > 0 { result[buzzer.chip] = buzzer.hz }
                continue
            }
            guard net < snapshot.netChanges.count, net < previous.netChanges.count else { continue }
            let frameHz = frequency(net: net, from: previous, to: snapshot)
            var hz = frameHz
            if let window, window.circuitTime < snapshot.circuitTime, net < window.netChanges.count {
                let windowHz = frequency(net: net, from: window, to: snapshot)
                let frameChanges = snapshot.netChanges[net] &- previous.netChanges[net]
                hz = frameChanges >= minimumFrameChanges && abs(frameHz - windowHz) > 0.3 * windowHz
                    ? frameHz : windowHz
            }
            if audible.contains(hz) { result[buzzer.chip] = hz }
        }
        return result
    }

    /// Half the value changes `net` gained per circuit second between two snapshots.
    static func frequency(net: Int, from older: SimulationSnapshot, to newer: SimulationSnapshot) -> Double {
        let seconds = Double(newer.circuitTime &- older.circuitTime) / 1e12
        guard seconds > 0 else { return 0 }
        return Double(newer.netChanges[net] &- older.netChanges[net]) / 2 / seconds
    }
}
