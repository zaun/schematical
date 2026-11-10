//
//  SimulationEngine.swift
//  schematical
//
//  Runs one circuit on its own thread, paced by the wall clock: circuit time
//  follows wall time × speed while the engine keeps up. When it can't, the
//  engine runs flat out for a frame's budget and re-anchors instead of
//  building a backlog. Once per frame it publishes a snapshot of the state
//  the canvas draws. Every circuit call goes through one lock.
//

import Foundation

/// One character LCD's dot image (`LcdState.dots`, plan-11-B §4.7): row-major
/// over `rows × 8` dot rows by `columns × 5` dot columns, each
/// `LCD_DOT_OFF`, `LCD_DOT_ON` or `LCD_DOT_X`.
nonisolated struct LCDFrame: Equatable, Sendable {
    let columns: Int
    let rows: Int
    let dots: [UInt8]

    var width: Int { columns * Int(LCD_STATE_CELL_WIDTH) }

    /// The dot at dot row `y`, dot column `x`.
    func dot(x: Int, y: Int) -> UInt8 {
        let index = y * width + x
        return index >= 0 && index < dots.count ? dots[index] : UInt8(LCD_DOT_OFF)
    }

    /// Copies the shown area out of the backend's state.
    init(_ state: LcdState) {
        var copy = state
        let columns = min(Int(state.columns), Int(LCD_STATE_MAX_COLUMNS))
        let rows = min(Int(state.rows), Int(LCD_STATE_MAX_ROWS))
        let stride = Int(LCD_STATE_MAX_COLUMNS) * Int(LCD_STATE_CELL_WIDTH)
        let width = columns * Int(LCD_STATE_CELL_WIDTH)
        let height = rows * Int(LCD_STATE_CELL_HEIGHT)
        var dots: [UInt8] = []
        dots.reserveCapacity(width * height)
        withUnsafeBytes(of: &copy.dots) { raw in
            for y in 0..<height {
                for x in 0..<width {
                    dots.append(raw[y * stride + x])
                }
            }
        }
        self.columns = columns
        self.rows = rows
        self.dots = dots
    }
}

/// The circuit state the canvas needs, read in one go under the engine lock.
nonisolated struct SimulationSnapshot: Sendable {
    /// `ProcessInfo.systemUptime` when the snapshot was taken.
    let wallTime: TimeInterval
    /// Circuit time, picoseconds.
    let circuitTime: UInt64
    let netValues: [CircuitValue]
    /// Per net: value changes since start (`circuit_net_changes`).
    let netChanges: [UInt64]
    /// Part index → cumulative on-time per light, picoseconds (parts with lights).
    let lightTimes: [Int: [UInt64]]
    /// Part index → dot image (character LCDs).
    let lcdFrames: [Int: LCDFrame]
    let shortedNet: Int?
    let error: String?
}

nonisolated final class SimulationEngine: @unchecked Sendable {
    /// Wall time per frame, and how much of it the circuit may use.
    static let frame: TimeInterval = 1.0 / 60.0
    static let budget: TimeInterval = 0.012
    /// Longest single `circuit_advance`, picoseconds, so the lock is never
    /// held for long.
    static let chunk: UInt64 = 10_000_000

    private let lock = NSLock()
    private let circuit: OpaquePointer
    private let netCount: Int
    private let lightParts: [Int]
    private let lcdParts: [Int]
    private let publish: @Sendable (SimulationSnapshot) -> Void

    // Shared with the main thread; under `lock`.
    private var speed: Double
    private var anchorWall: TimeInterval = 0
    private var anchorSim: UInt64 = 0
    private var cancelled = false
    private var destroyed = false

    private var started = false
    /// Signalled when the thread exits.
    private let exited = DispatchSemaphore(value: 0)
    /// Signalled by `stopAndWait` to cut a frame's sleep short.
    private let wake = DispatchSemaphore(value: 0)

    private static let liveLock = NSLock()
    nonisolated(unsafe) private static var live = 0

    /// Engine threads started and not yet exited.
    static var liveThreads: Int {
        liveLock.lock()
        defer { liveLock.unlock() }
        return live
    }

    /// Takes ownership of `circuit` (started, not yet advanced); it is
    /// destroyed by `stopAndWait`.
    init(circuit: OpaquePointer, netCount: Int, lightParts: [Int], lcdParts: [Int] = [], speed: Double,
         publish: @escaping @Sendable (SimulationSnapshot) -> Void) {
        self.circuit = circuit
        self.netCount = netCount
        self.lightParts = lightParts
        self.lcdParts = lcdParts
        self.speed = speed
        self.publish = publish
    }

    deinit {
        if !destroyed {
            circuit_destroy(circuit)
        }
    }

    /// Starts the simulation thread.
    func start() {
        guard !started else { return }
        started = true
        Self.liveLock.lock()
        Self.live += 1
        Self.liveLock.unlock()
        let thread = Thread { [self] in
            self.run()
            Self.liveLock.lock()
            Self.live -= 1
            Self.liveLock.unlock()
            self.exited.signal()
        }
        thread.name = "Simulation"
        thread.qualityOfService = .userInitiated
        thread.start()
    }

    /// Stops the thread, waits for it to exit and destroys the circuit.
    func stopAndWait() {
        lock.lock()
        cancelled = true
        lock.unlock()
        wake.signal()
        if started {
            exited.wait()
            started = false
        }
        lock.lock()
        if !destroyed {
            destroyed = true
            circuit_destroy(circuit)
        }
        lock.unlock()
    }

    /// Changes the speed. Circuit time keeps going from where it is.
    func setSpeed(_ newSpeed: Double) {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return }
        speed = newSpeed
        anchorWall = ProcessInfo.processInfo.systemUptime
        anchorSim = circuit_time(circuit)
    }

    // MARK: Simulation thread

    private func run() {
        lock.lock()
        anchorWall = ProcessInfo.processInfo.systemUptime
        anchorSim = circuit_time(circuit)
        lock.unlock()

        while true {
            let frameStart = ProcessInfo.processInfo.systemUptime

            lock.lock()
            if cancelled {
                lock.unlock()
                return
            }
            let elapsed = max(0, frameStart - anchorWall)
            let target = anchorSim &+ UInt64(elapsed * speed * 1e12)
            lock.unlock()

            while true {
                lock.lock()
                let now = circuit_time(circuit)
                if now >= target || cancelled {
                    lock.unlock()
                    break
                }
                dispatchPrecondition(condition: .notOnQueue(.main))
                let ok = circuit_advance(circuit, min(Self.chunk, target - now)) != 0
                lock.unlock()

                if !ok {
                    // The circuit stopped (short, or did not settle): show why.
                    publish(snapshotNow())
                    return
                }
                if ProcessInfo.processInfo.systemUptime - frameStart > Self.budget {
                    // Can't keep up: drop the backlog and go on from here.
                    lock.lock()
                    anchorWall = ProcessInfo.processInfo.systemUptime
                    anchorSim = circuit_time(circuit)
                    lock.unlock()
                    break
                }
            }

            publish(snapshotNow())

            let sleep = frameStart + Self.frame - ProcessInfo.processInfo.systemUptime
            if sleep > 0 {
                _ = wake.wait(timeout: .now() + sleep)
            }
        }
    }

    // MARK: Reads and switches (any thread)

    /// The current state, read under the lock.
    func snapshotNow() -> SimulationSnapshot {
        lock.lock()
        defer { lock.unlock() }
        let wall = ProcessInfo.processInfo.systemUptime
        guard !destroyed else {
            return SimulationSnapshot(wallTime: wall, circuitTime: 0, netValues: [], netChanges: [],
                                      lightTimes: [:], lcdFrames: [:], shortedNet: nil, error: nil)
        }
        let values = (0..<netCount).map { circuit_net_value(circuit, $0) }
        let changes = (0..<netCount).map { circuit_net_changes(circuit, $0) }
        var lightTimes: [Int: [UInt64]] = [:]
        var buffer = [UInt64](repeating: 0, count: 64)
        for part in lightParts {
            let count = circuit_part_light_time(circuit, part, &buffer, buffer.count)
            lightTimes[part] = Array(buffer.prefix(min(count, buffer.count)))
        }
        var lcdFrames: [Int: LCDFrame] = [:]
        for part in lcdParts {
            var state = LcdState()
            if circuit_lcd_state(circuit, part, &state) == 0 {
                lcdFrames[part] = LCDFrame(state)
            }
        }
        var net = 0
        let shorted = circuit_shorted_net(circuit, &net) != 0 ? net : nil
        let error = circuit_error(circuit).map { String(cString: $0) }
        return SimulationSnapshot(wallTime: wall, circuitTime: circuit_time(circuit),
                                  netValues: values, netChanges: changes, lightTimes: lightTimes,
                                  lcdFrames: lcdFrames, shortedNet: shorted, error: error)
    }

    /// Turns a switch position (1-based) on or off.
    func setSwitch(part: Int, position: Int, on: Bool) {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return }
        circuit_set_switch(circuit, part, UInt32(position), on ? 1 : 0)
    }

    func memorySize(part: Int) -> MemorySize? {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return nil }
        var words: UInt32 = 0
        var bits: UInt32 = 0
        guard circuit_memory_size(circuit, part, &words, &bits) != 0 else { return nil }
        return MemorySize(words: Int(words), bits: Int(bits))
    }

    func readMemory(part: Int, start: Int, count: Int) -> [MemoryWord] {
        guard start >= 0, count > 0, start <= Int(UInt32.max) else { return [] }
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return [] }
        var values = [UInt32](repeating: 0, count: count)
        var known = [UInt32](repeating: 0, count: count)
        let read = circuit_memory_read(circuit, part, UInt32(start), count, &values, &known)
        return (0..<read).map { MemoryWord(value: values[$0], known: known[$0]) }
    }

    // MARK: Serial terminal (plan-12-K)

    /// A part's serial channels (0 for any other part).
    func serialChannels(part: Int) -> Int {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return 0 }
        return circuit_serial_channels(circuit, part)
    }

    /// Frames a channel has sent since start.
    func serialSentCount(part: Int, channel: Int) -> Int {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed, channel >= 0 else { return 0 }
        return circuit_serial_sent_count(circuit, part, UInt32(channel))
    }

    /// Frames [from, from + count) a channel has sent.
    func serialSent(part: Int, channel: Int, from: Int, count: Int) -> [UInt16] {
        guard from >= 0, count > 0, channel >= 0 else { return [] }
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return [] }
        var frames = [UInt16](repeating: 0, count: count)
        let read = circuit_serial_sent(circuit, part, UInt32(channel), from, &frames, count)
        return Array(frames.prefix(read))
    }

    /// Types bytes into a channel's RX pin: nil on success, else the reason.
    func serialSend(part: Int, channel: Int, bytes: [UInt8]) -> String? {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return "The simulation is not running." }
        guard channel >= 0 else { return "No such channel." }
        let result = bytes.withUnsafeBufferPointer {
            circuit_serial_send(circuit, part, UInt32(channel), $0.baseAddress, $0.count)
        }
        if result == 0 { return nil }
        return circuit_error(circuit).map { String(cString: $0) } ?? "The send was refused."
    }

    /// Sets a USB bridge channel's port, as the PC would: nil on success, else the reason.
    func serialConfigure(part: Int, channel: Int, port: SerialPort) -> String? {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return "The simulation is not running." }
        guard channel >= 0 else { return "No such channel." }
        if port.apply(to: circuit, part: part, channel: channel) { return nil }
        return circuit_error(circuit).map { String(cString: $0) } ?? "The port settings were refused."
    }

    /// A processor part's state (cpu kind), or nil for any other part.
    func cpuState(part: Int) -> CpuSnapshot? {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return nil }
        var state = CpuState()
        guard circuit_cpu_state(circuit, part, &state) == 0 else { return nil }
        return CpuSnapshot(state)
    }

    /// A converter's or RTC's registers and, when `rtc`, its clock
    /// (plan-24-I); nil for any other part. Call it only for those parts: a
    /// refused query sets the circuit's error, which reads as a stop.
    func registerState(part: Int, rtc: Bool) -> RegisterSnapshot? {
        lock.lock()
        defer { lock.unlock() }
        guard !destroyed else { return nil }
        let count = circuit_converter_registers(circuit, part, nil, 0)
        guard count >= 0 else { return nil }
        var registers = [ConverterRegister](repeating: ConverterRegister(), count: Int(count))
        let read = registers.withUnsafeMutableBufferPointer {
            circuit_converter_registers(circuit, part, $0.baseAddress, $0.count)
        }
        guard read == count else { return nil }
        var time = RtcTime()
        let clock: RtcTime? = rtc && circuit_rtc_time(circuit, part, &time) == 0 ? time : nil
        return RegisterSnapshot(registers, clock: clock)
    }
}
