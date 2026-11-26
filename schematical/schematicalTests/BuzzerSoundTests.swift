import AVFoundation
import Foundation
import XCTest
@testable import schematical

/// An engine that renders offline and never opens the audio device.
private func offlineEngine() -> AVAudioEngine {
    let engine = AVAudioEngine()
    let format = AVAudioFormat(standardFormatWithSampleRate: 48_000, channels: 1)!
    try! engine.enableManualRenderingMode(.offline, format: format, maximumFrameCount: 4096)
    return engine
}

/// An offline engine that refuses to start.
private final class RefusingEngine: AVAudioEngine {
    override func start() throws {
        throw NSError(domain: "BuzzerSoundTests", code: 1)
    }
}

/// plan-68-W: the buzzers' player — its engine started for the first voice,
/// stopped after the idle delay and at Stop, a device that won't start, and
/// a running simulation's buzzer reaching it. Every engine renders offline.
@MainActor
final class BuzzerSoundTests: XCTestCase {
    /// Simulations are kept for the life of the process, as the other
    /// simulation tests keep theirs.
    private static var kept: [SimulationController] = []

    override func setUp() {
        super.setUp()
        AppDelegate.quitsAfterLastWindowCloses = false
    }

    private func peak(_ engine: AVAudioEngine) throws -> Float {
        let buffer = try XCTUnwrap(AVAudioPCMBuffer(pcmFormat: engine.manualRenderingFormat, frameCapacity: 4096))
        XCTAssertEqual(try engine.renderOffline(4096, to: buffer), .success)
        let samples = UnsafeBufferPointer(start: try XCTUnwrap(buffer.floatChannelData)[0], count: Int(buffer.frameLength))
        return samples.map(abs).max() ?? 0
    }

    func testAVoiceStartsTheEngineAndSilenceStopsItLater() async throws {
        var made = 0
        let sound = BuzzerSound(makeEngine: { made += 1; return offlineEngine() })
        sound.idleDelay = .milliseconds(50)
        XCTAssertFalse(sound.isRunning)
        sound.update([:])
        XCTAssertEqual(made, 0, "silence opens nothing")

        sound.update([UUID(): 2000])
        XCTAssertTrue(sound.isRunning)
        let engine = try XCTUnwrap(sound.runningEngine)
        XCTAssertEqual(try peak(engine), ToneRenderer.amplitude * ToneRenderer.monoMixerCompensation, accuracy: 0.005,
                       "the tone, mono to mono through the mixer")
        sound.update([UUID(): 1000])
        XCTAssertEqual(made, 1, "one engine while it sounds")

        sound.update([:])
        XCTAssertTrue(sound.isRunning, "on for the idle delay")
        sound.update([:])
        try await Task.sleep(for: .milliseconds(400))
        XCTAssertFalse(sound.isRunning)
        XCTAssertNil(sound.runningEngine)

        sound.update([UUID(): 500])
        XCTAssertEqual(made, 2)
        XCTAssertTrue(sound.isRunning)
        sound.update([:])
        sound.update([UUID(): 500])
        try await Task.sleep(for: .milliseconds(200))
        XCTAssertTrue(sound.isRunning, "a voice within the delay keeps it on")
        sound.stop()
        XCTAssertFalse(sound.isRunning)
        XCTAssertEqual(made, 2)
    }

    func testAnEngineThatWontStartIsNotTriedAgainUntilStop() {
        var made = 0
        let sound = BuzzerSound(makeEngine: {
            made += 1
            let engine = RefusingEngine()
            try? engine.enableManualRenderingMode(.offline, format: AVAudioFormat(standardFormatWithSampleRate: 48_000, channels: 2)!,
                                                  maximumFrameCount: 512)
            return engine
        })
        sound.update([UUID(): 440])
        XCTAssertTrue(sound.failed)
        XCTAssertFalse(sound.isRunning)
        sound.update([UUID(): 440])
        XCTAssertEqual(made, 1)
        sound.stop()
        XCTAssertFalse(sound.failed)
        sound.update([UUID(): 440])
        XCTAssertEqual(made, 2)
    }

    func testTheDefaultPlayerAndSettingUnderTest() {
        XCTAssertFalse(BuzzerSoundSetting.mayPlayNow(), "never under XCTest")
        XCTAssertFalse(BuzzerSound().isRunning)
        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.kept.append(simulation)
        XCTAssertFalse(simulation.soundAllowed())
    }

    /// A powered indicator buzzer in a running simulation sounds through the
    /// player the simulation makes; Stop silences it.
    func testARunningBuzzerReachesThePlayer() throws {
        let part = try XCTUnwrap(TestSupport.allParts.first { $0.buzzer?.driver == "indicator" }, "an indicator buzzer")
        let doc = TestSupport.newDocument()
        let vcc = try TestSupport.place(doc, "5V", at: CGPoint(x: 60, y: 60))
        let gnd = try TestSupport.place(doc, "GND", at: CGPoint(x: 60, y: 400))
        let buzzer = try TestSupport.place(doc, part.name, at: CGPoint(x: 300, y: 200))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(vcc, "VCC")), .pin(try TestSupport.pin(buzzer, "+"))))
        XCTAssertNil(doc.connect(.pin(try TestSupport.pin(buzzer, "-")), .pin(try TestSupport.pin(gnd, "GND"))))

        let simulation = SimulationController(library: TestSupport.catalog.library)
        Self.kept.append(simulation)
        simulation.soundAllowed = { true }
        simulation.makeSound = { BuzzerSound(makeEngine: offlineEngine) }
        simulation.start(document: doc)
        XCTAssertTrue(simulation.isRunning, simulation.errorMessage ?? "")
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        XCTAssertFalse(simulation.buzzerVoices.isEmpty, simulation.errorMessage ?? "no voice")
        let sound = try XCTUnwrap(simulation.buzzerSound)
        XCTAssertTrue(sound.isRunning)

        simulation.soundAllowed = { false }
        RunLoop.main.run(until: Date().addingTimeInterval(0.2))
        XCTAssertNil(simulation.buzzerSound, "the setting turned off: the player goes")
        simulation.stop()
        XCTAssertNil(simulation.buzzerSound)
    }
}
