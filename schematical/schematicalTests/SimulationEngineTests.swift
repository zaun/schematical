import Foundation
import XCTest
@testable import schematical

/// plan-68-W: the simulation engine driven directly on a circuit the test
/// builds — every read and switch while it runs, what each gives for a part
/// that isn't of its kind or a bad argument, and that every one gives
/// nothing once the circuit is destroyed; the thread's start, speed and stop;
/// an LCD frame's dots.
@MainActor
final class SimulationEngineTests: XCTestCase {
    private let library = TestSupport.catalog.library

    /// 5V and GND, an SRAM (part 2), a gate (part 3) and a character LCD (part 4), started.
    private func circuit() throws -> OpaquePointer {
        let circuit = try XCTUnwrap(circuit_create(library.pointer))
        var index = 0
        for name in ["5V", "GND", "AS6C6264-55PCN", "SN74HC00N", "NHD-0216BZ-FL-YBW"] {
            XCTAssertEqual(circuit_add_part(circuit, name, &index), 0, name)
        }
        var net = 0
        XCTAssertEqual(circuit_add_net(circuit, &net), 0)
        XCTAssertEqual(circuit_start(circuit), 0, circuit_error(circuit).map { String(cString: $0) } ?? "")
        return circuit
    }

    private nonisolated final class Box: @unchecked Sendable {
        private let lock = NSLock()
        private var snapshots: [SimulationSnapshot] = []
        func add(_ snapshot: SimulationSnapshot) { lock.lock(); snapshots.append(snapshot); lock.unlock() }
        var count: Int { lock.lock(); defer { lock.unlock() }; return snapshots.count }
        var last: SimulationSnapshot? { lock.lock(); defer { lock.unlock() }; return snapshots.last }
    }

    func testReadsWhileRunningAndNothingOnceDestroyed() throws {
        let engine = SimulationEngine(circuit: try circuit(), netCount: 1, lightParts: [3], lcdParts: [3, 4], speed: 1,
                                      publish: { _ in })
        XCTAssertEqual(engine.memorySize(part: 2), MemorySize(words: 8192, bits: 8))
        XCTAssertNil(engine.memorySize(part: 3), "a gate is no memory")
        XCTAssertEqual(engine.readMemory(part: 2, start: 0, count: 4).count, 4)
        XCTAssertEqual(engine.readMemory(part: 2, start: -1, count: 4), [])
        XCTAssertEqual(engine.readMemory(part: 2, start: 0, count: 0), [])
        XCTAssertEqual(engine.serialChannels(part: 3), 0)
        XCTAssertEqual(engine.serialSentCount(part: 3, channel: -1), 0)
        XCTAssertEqual(engine.serialSentCount(part: 3, channel: 0), 0)
        XCTAssertEqual(engine.serialSent(part: 3, channel: 0, from: -1, count: 1), [])
        XCTAssertEqual(engine.serialSent(part: 3, channel: 0, from: 0, count: 4), [])
        XCTAssertEqual(engine.serialSend(part: 3, channel: -1, bytes: [1]), "No such channel.")
        XCTAssertNotNil(engine.serialSend(part: 3, channel: 0, bytes: [1]), "a gate takes no bytes")
        XCTAssertEqual(engine.serialConfigure(part: 3, channel: -1, port: SerialPort.formats[0]), "No such channel.")
        XCTAssertNotNil(engine.serialConfigure(part: 3, channel: 0, port: SerialPort.formats[0]))
        XCTAssertNil(engine.cpuState(part: 3))
        XCTAssertEqual(engine.timerSections(part: 3, count: 2), [])
        engine.setSwitch(part: 3, position: 1, on: true)
        engine.setSlide(part: 3, state: 1)
        let snapshot = engine.snapshotNow()
        XCTAssertEqual(snapshot.netValues.count, 1)
        XCTAssertEqual(snapshot.netChanges.count, 1)
        XCTAssertNotNil(snapshot.lightTimes[3])
        XCTAssertNil(snapshot.lcdFrames[3], "a gate has no dots")
        let frame = try XCTUnwrap(snapshot.lcdFrames[4], "the LCD's dots")
        XCTAssertEqual([frame.columns, frame.rows], [16, 2])

        engine.stopAndWait()
        engine.stopAndWait()
        XCTAssertNil(engine.memorySize(part: 2))
        XCTAssertEqual(engine.readMemory(part: 2, start: 0, count: 4), [])
        XCTAssertEqual(engine.serialChannels(part: 3), 0)
        XCTAssertEqual(engine.serialSentCount(part: 3, channel: 0), 0)
        XCTAssertEqual(engine.serialSent(part: 3, channel: 0, from: 0, count: 4), [])
        XCTAssertEqual(engine.serialSend(part: 3, channel: 0, bytes: [1]), "The simulation is not running.")
        XCTAssertEqual(engine.serialConfigure(part: 3, channel: 0, port: SerialPort.formats[0]), "The simulation is not running.")
        XCTAssertNil(engine.cpuState(part: 3))
        XCTAssertNil(engine.registerState(part: 3, rtc: false))
        XCTAssertEqual(engine.timerSections(part: 3, count: 1), [])
        engine.setSwitch(part: 3, position: 1, on: false)
        engine.setSlide(part: 3, state: 1)
        engine.setSpeed(0.5)
        let dead = engine.snapshotNow()
        XCTAssertEqual(dead.circuitTime, 0)
        XCTAssertEqual(dead.netValues, [])
        XCTAssertNil(dead.error)
    }

    func testAPartThatHasNoRegistersGivesNone() throws {
        let engine = SimulationEngine(circuit: try circuit(), netCount: 1, lightParts: [], speed: 1, publish: { _ in })
        defer { engine.stopAndWait() }
        XCTAssertNil(engine.registerState(part: 2, rtc: false))
    }

    /// The thread publishes a snapshot a frame, follows the speed, starts
    /// once, and is gone after the stop.
    func testTheThreadRunsAtItsSpeedAndStops() throws {
        let box = Box()
        let before = SimulationEngine.liveThreads
        let engine = SimulationEngine(circuit: try circuit(), netCount: 1, lightParts: [], speed: 1e-3,
                                      publish: { box.add($0) })
        engine.start()
        engine.start()
        XCTAssertEqual(SimulationEngine.liveThreads, before + 1, "one thread, however often it is started")
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        XCTAssertGreaterThan(box.count, 3, "a snapshot each frame")
        let slow = try XCTUnwrap(box.last).circuitTime
        engine.setSpeed(1)
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        let fast = try XCTUnwrap(box.last).circuitTime
        XCTAssertGreaterThan(fast - slow, 100 * slow, "real time runs about a thousand times faster")
        engine.stopAndWait()
        XCTAssertEqual(SimulationEngine.liveThreads, before)
    }

    /// A switch closed across the supply while running stops the circuit:
    /// the thread publishes why and ends by itself.
    func testACircuitThatStopsEndsItsThread() throws {
        let circuit = try XCTUnwrap(circuit_create(library.pointer))
        var index = 0
        for name in ["5V", "GND", "EDGSC0 LFG"] { XCTAssertEqual(circuit_add_part(circuit, name, &index), 0, name) }
        var net = 0
        XCTAssertEqual(circuit_add_net(circuit, &net), 0)
        XCTAssertEqual(circuit_add_net(circuit, &net), 0)
        XCTAssertEqual(circuit_connect(circuit, 0, "1", 0), 0)
        XCTAssertEqual(circuit_connect(circuit, 2, "1", 0), 0)
        XCTAssertEqual(circuit_connect(circuit, 1, "1", 1), 0)
        XCTAssertEqual(circuit_connect(circuit, 2, "2", 1), 0)
        XCTAssertEqual(circuit_start(circuit), 0, circuit_error(circuit).map { String(cString: $0) } ?? "")
        let box = Box()
        let before = SimulationEngine.liveThreads
        let engine = SimulationEngine(circuit: circuit, netCount: 2, lightParts: [], speed: 1, publish: { box.add($0) })
        defer { engine.stopAndWait() }
        engine.start()
        RunLoop.main.run(until: Date().addingTimeInterval(0.1))
        engine.setSwitch(part: 2, position: 1, on: true)
        let end = Date().addingTimeInterval(3)
        while SimulationEngine.liveThreads > before, Date() < end { RunLoop.main.run(until: Date().addingTimeInterval(0.05)) }
        XCTAssertEqual(SimulationEngine.liveThreads, before, "the thread ended")
        let last = try XCTUnwrap(box.last)
        XCTAssertTrue(last.shortedNet != nil || last.error != nil, "it says why")
    }

    /// An engine let go without a stop destroys its circuit itself.
    func testAnEngineLetGoFreesItsCircuit() throws {
        weak var gone: SimulationEngine?
        do {
            let engine = SimulationEngine(circuit: try circuit(), netCount: 1, lightParts: [], speed: 1, publish: { _ in })
            gone = engine
            XCTAssertNotNil(engine.memorySize(part: 2))
        }
        XCTAssertNil(gone)
    }

    func testAnLCDFramesDots() {
        var state = LcdState()
        state.columns = 2
        state.rows = 1
        withUnsafeMutableBytes(of: &state.dots) { raw in
            raw[0] = UInt8(LCD_DOT_ON)
            raw[Int(LCD_STATE_CELL_WIDTH) + 1] = UInt8(LCD_DOT_X)
        }
        let frame = LCDFrame(state)
        XCTAssertEqual([frame.columns, frame.rows, frame.width], [2, 1, 2 * Int(LCD_STATE_CELL_WIDTH)])
        XCTAssertEqual(frame.dots.count, frame.width * Int(LCD_STATE_CELL_HEIGHT))
        XCTAssertEqual(frame.dot(x: 0, y: 0), UInt8(LCD_DOT_ON))
        XCTAssertEqual(frame.dot(x: Int(LCD_STATE_CELL_WIDTH) + 1, y: 0), UInt8(LCD_DOT_X))
        XCTAssertEqual(frame.dot(x: 1, y: 0), UInt8(LCD_DOT_OFF))
        XCTAssertEqual(frame.dot(x: -1, y: 0), UInt8(LCD_DOT_OFF), "outside: off")
        XCTAssertEqual(frame.dot(x: 0, y: 99), UInt8(LCD_DOT_OFF))
    }
}
