//
//  BuzzerSound.swift
//  schematical
//
//  Plays the voices of the sounding buzzers (plan-47-N § 3.3): one audio
//  engine with one source node that sums a square wave per voice, at most
//  `maxVoices` of them, each at -24 dBFS, with a 10 ms ramp when a voice
//  starts, stops or jumps to another pitch. The engine runs while a voice
//  sounds and for a second after the last one, and stops at once when play
//  stops. The pitch is the circuit's: it does not follow the simulation's
//  speed.
//
//  The audio thread reads the voices through `ToneRenderer`, which allocates
//  nothing and never waits for a lock.
//

import AVFoundation
import Foundation
import os

/// The setting (Settings ▸ General Defaults ▸ Play buzzer sounds) and the
/// switches that keep a process quiet.
enum BuzzerSoundSetting {
    static let key = PreferenceKey.playBuzzerSounds

    /// "Play buzzer sounds": on unless the user turned it off.
    static var isOn: Bool {
        UserDefaults.standard.object(forKey: key) as? Bool ?? true
    }

    /// Set by a process that must never make a sound (the harness).
    nonisolated(unsafe) static var isSilenced = false

    /// The tests run in a process that has XCTest loaded.
    static var isUnderTest: Bool { NSClassFromString("XCTestCase") != nil }

    /// Whether the app may open the audio device now.
    static var mayPlay: Bool { isOn && !isSilenced && !isUnderTest }

    /// `mayPlay`, as a function a simulation asks every frame.
    static func mayPlayNow() -> Bool { mayPlay }
}

/// The audio thread's side: a square wave per slot, read from a table the
/// main thread writes. No allocation, no blocking: a table that is being
/// written is skipped and the last one is used for that buffer.
nonisolated final class ToneRenderer: @unchecked Sendable {
    static let maxVoices = 8
    /// -24 dBFS per voice.
    static let amplitude: Float = 0.0630957
    /// The ramp when a voice starts or stops, seconds.
    static let ramp = 0.010
    /// A new pitch within this ratio of the old one glides, anything else
    /// ramps down and up again.
    static let glideRatio = 0.1

    private let sampleRate: Double
    /// Applied to the sum. The engine's mixer takes 3 dB off a mono source
    /// (centre pan law), so `BuzzerSound` asks for `monoMixerCompensation`
    /// to put each voice at -24 dBFS at the device.
    private let outputGain: Float
    static let monoMixerCompensation: Float = 1.4142135
    private let lock = UnsafeMutablePointer<os_unfair_lock>.allocate(capacity: 1)
    /// Written by the main thread under `lock`.
    private let table = UnsafeMutablePointer<Double>.allocate(capacity: maxVoices)
    /// The audio thread's own state.
    private let wanted = UnsafeMutablePointer<Double>.allocate(capacity: maxVoices)
    private let pitch = UnsafeMutablePointer<Double>.allocate(capacity: maxVoices)
    private let phase = UnsafeMutablePointer<Double>.allocate(capacity: maxVoices)
    private let gain = UnsafeMutablePointer<Double>.allocate(capacity: maxVoices)

    init(sampleRate: Double, gain level: Float = 1) {
        self.sampleRate = sampleRate
        self.outputGain = level
        lock.initialize(to: os_unfair_lock())
        for pointer in [table, wanted, pitch, phase, gain] {
            pointer.initialize(repeating: 0, count: Self.maxVoices)
        }
    }

    deinit {
        lock.deallocate()
        for pointer in [table, wanted, pitch, phase, gain] { pointer.deallocate() }
    }

    /// Sets the pitch of each slot (0 = silent). Main thread.
    func set(_ pitches: [Double]) {
        os_unfair_lock_lock(lock)
        for slot in 0..<Self.maxVoices {
            table[slot] = slot < pitches.count ? pitches[slot] : 0
        }
        os_unfair_lock_unlock(lock)
    }

    /// Fills `frames` mono samples. Audio thread.
    func render(into samples: UnsafeMutablePointer<Float>, frames: Int) {
        if os_unfair_lock_trylock(lock) {
            for slot in 0..<Self.maxVoices { wanted[slot] = table[slot] }
            os_unfair_lock_unlock(lock)
        }
        let step = 1 / (Self.ramp * sampleRate)
        for frame in 0..<frames {
            var sum: Float = 0
            for slot in 0..<Self.maxVoices {
                let want = wanted[slot]
                var target = want > 0 ? 1.0 : 0.0
                if want != pitch[slot] {
                    if want > 0, pitch[slot] > 0, abs(want - pitch[slot]) <= Self.glideRatio * pitch[slot] {
                        pitch[slot] = want
                    } else if gain[slot] <= 0 {
                        pitch[slot] = want
                    } else {
                        target = 0
                    }
                }
                if gain[slot] < target {
                    gain[slot] = min(target, gain[slot] + step)
                } else if gain[slot] > target {
                    gain[slot] = max(target, gain[slot] - step)
                }
                guard gain[slot] > 0, pitch[slot] > 0 else { continue }
                phase[slot] += pitch[slot] / sampleRate
                if phase[slot] >= 1 { phase[slot] -= 1 }
                sum += (phase[slot] < 0.5 ? 1 : -1) * Float(gain[slot]) * Self.amplitude
            }
            samples[frame] = sum * outputGain
        }
    }
}

/// Which slot of the renderer each sounding chip plays in: the
/// `ToneRenderer.maxVoices` lowest chip ids, each kept in its slot while it
/// sounds so a change in the others does not move it.
struct VoiceSlots {
    private(set) var chips: [UUID?] = Array(repeating: nil, count: ToneRenderer.maxVoices)

    /// The pitch of each slot (0 = silent) for the voices (chip → hertz).
    mutating func pitches(for voices: [UUID: Double]) -> [Double] {
        let chosen = voices.keys.sorted { $0.uuidString < $1.uuidString }.prefix(ToneRenderer.maxVoices)
        for slot in chips.indices {
            if let chip = chips[slot], !chosen.contains(chip) { chips[slot] = nil }
        }
        for chip in chosen where !chips.contains(chip) {
            if let free = chips.firstIndex(where: { $0 == nil }) { chips[free] = chip }
        }
        return chips.map { chip in chip.flatMap { voices[$0] } ?? 0 }
    }

    mutating func reset() {
        chips = Array(repeating: nil, count: ToneRenderer.maxVoices)
    }
}

/// Owns the audio engine of one simulation controller.
@MainActor
final class BuzzerSound {
    /// As `SchematicDocument`'s: an isolated deinit aborts the runtime when
    /// a closing document frees this.
    nonisolated deinit {}

    private let makeEngine: () -> AVAudioEngine
    private var engine: AVAudioEngine?
    private var renderer: ToneRenderer?
    private var slots = VoiceSlots()
    private var idleStop: Task<Void, Never>?
    /// The audio device could not be opened; not tried again until `stop`.
    private(set) var failed = false
    /// How long the engine runs on after the last voice stops.
    var idleDelay: Duration = .seconds(1)

    /// Plays on the system's audio device.
    convenience init() {
        self.init(makeEngine: AVAudioEngine.init)
    }

    /// Plays through the engines `makeEngine` makes (a test's renders offline).
    init(makeEngine: @escaping () -> AVAudioEngine) {
        self.makeEngine = makeEngine
    }

    /// Whether the audio engine is running.
    var isRunning: Bool { engine?.isRunning ?? false }

    /// The running engine, nil when there is none.
    var runningEngine: AVAudioEngine? { engine }

    /// Plays `voices` (chip → hertz), at most `ToneRenderer.maxVoices` of
    /// them (the lowest chip ids). An empty table silences everything and
    /// stops the engine `idleDelay` later.
    func update(_ voices: [UUID: Double]) {
        let pitches = slots.pitches(for: voices)
        if pitches.contains(where: { $0 > 0 }) {
            idleStop?.cancel()
            idleStop = nil
            startEngineIfNeeded()
            renderer?.set(pitches)
        } else {
            renderer?.set(pitches)
            scheduleIdleStop()
        }
    }

    /// Silences everything and stops the engine at once (play stopped).
    func stop() {
        idleStop?.cancel()
        idleStop = nil
        slots.reset()
        failed = false
        renderer?.set([])
        engine?.stop()
        engine = nil
        renderer = nil
    }

    private func scheduleIdleStop() {
        guard engine != nil, idleStop == nil else { return }
        let delay = idleDelay
        idleStop = Task { [weak self] in
            try? await Task.sleep(for: delay)
            guard !Task.isCancelled, let self else { return }
            self.engine?.stop()
            self.engine = nil
            self.renderer = nil
            self.idleStop = nil
        }
    }

    private func startEngineIfNeeded() {
        guard engine == nil, !failed else { return }
        let engine = makeEngine()
        if let renderer = Self.start(engine) {
            self.engine = engine
            self.renderer = renderer
        } else {
            failed = true
        }
    }

    /// Attaches a source node to `engine` and starts it; the node's renderer,
    /// or nil when the engine has no output or won't start.
    static func start(_ engine: AVAudioEngine) -> ToneRenderer? {
        let sampleRate = engine.outputNode.outputFormat(forBus: 0).sampleRate
        guard sampleRate > 0, let format = AVAudioFormat(standardFormatWithSampleRate: sampleRate, channels: 1) else { return nil }
        let renderer = ToneRenderer(sampleRate: sampleRate, gain: ToneRenderer.monoMixerCompensation)
        let node = makeNode(renderer: renderer, format: format)
        engine.attach(node)
        engine.connect(node, to: engine.mainMixerNode, format: format)
        engine.prepare()
        do { try engine.start() } catch { return nil }
        return renderer
    }

    /// The source node, made outside the main actor: its render block runs
    /// on the audio thread.
    nonisolated static func makeNode(renderer: ToneRenderer, format: AVAudioFormat) -> AVAudioSourceNode {
        AVAudioSourceNode(format: format) { _, _, frameCount, audioBufferList in
            let buffers = UnsafeMutableAudioBufferListPointer(audioBufferList)
            for buffer in buffers {
                guard let data = buffer.mData?.assumingMemoryBound(to: Float.self) else { continue }
                renderer.render(into: data, frames: Int(frameCount))
            }
            return noErr
        }
    }
}
