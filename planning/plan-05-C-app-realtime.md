# plan-05-C: The app runs circuits at real time, off the UI thread

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-05-B

Today the app advances 10 µs of circuit time per 16 ms frame, on the main
thread: circuits run at 1/1600 of real time, and a heavy step freezes the UI.
C replaces that with a background simulation thread paced by the wall
clock, a speed picker (1× down to 1/1,000,000), a readout of the speed
actually reached when the engine can't keep up, wires drawn as "toggling"
when they changed more than once in a frame, and LEDs/segments drawn with
brightness equal to their on-time in the frame.

**Outcome of C:** opening `test.schematical` (SiT1534 at 32 768 Hz driving
an LED) and pressing Run: after 2 s the circuit time is 2 s ± 10 %, the
reached speed reads 1×, the LED is drawn at brightness 0.5 ± 0.05, and the
oscillator's wire is drawn toggling. At speed 1/1,000,000 the circuit time
after 2 s is 2 µs ± 10 %.

References:

- plan-05-B (`circuit_net_changes`, `circuit_part_light_time`).
- `schematical/schematical/Simulation/Simulation.swift`
  (`SimulationController`), `ContentView.swift` (toolbar),
  `Canvas/SchematicCanvasView.swift` (`WireLayer`, `RoutedWire`, LED and
  7-segment call sites), `Canvas/DiodeSymbolView.swift`,
  `Canvas/SevenSegmentSymbolView.swift`, `Memory/MemoryWindow.swift`.
- Headless runtime-proof approach: `planning/completed/plan-03-D-pcb-tab.md`
  § Runtime proof and § Corrections in `plan-03-A-footprint-format.md`.

## Prerequisites

The plan-05 gate is in plan-05-A § Prerequisites. C adds:

| Must be true | Command | Status |
|---|---|---|
| plan-05-B complete | `ls planning/plan-05-B-* 2>/dev/null` → no output; `ls planning/completed/plan-05-B-accuracy-readback.md` → exists | NOT MET (2026-11-01) |
| New read-backs exist | `grep -c 'circuit_net_changes\|circuit_part_light_time' backend/include/circuit.h` → 2 | NOT MET (2026-11-01) |
| App builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-p05-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-01, at 5aa1fee, 33 s) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- Circuit time follows wall time × chosen speed while the engine keeps up;
  when it can't, the simulation runs flat out, never builds a backlog, and
  the toolbar shows the reached speed.
- The main thread never runs `circuit_advance`.
- Wires: a net that changed ≥ 2 times since the last frame is drawn
  toggling; otherwise as today (value color, dashed when floating, red when
  shorted).
- LEDs and 7-segment segments are drawn with brightness = on-time ÷ frame
  length (0…1).

### Non-goals (explicit constraints)

- No backend changes (all needed calls exist after B).
- No file-format change: speed is an app preference, not saved in the
  `.schematical` file.
- Edit locking in play mode, switch toggling, memory windows and error
  banners behave as today.
- No change to PCB page drawing.

## 2. Current State

- `SimulationController` (`Simulation.swift`) is `@MainActor @Observable`.
  `start` builds the circuit and runs a `Task` loop: `step()` →
  `circuit_advance(circuit, 10_000_000)` (`picosecondsPerTick`), `refresh()`,
  `Task.sleep(16 ms)` (`tickInterval`). Wall time per frame = 16 ms + step
  time, so circuit time also drifts.
- `refresh()` reads every net value, the shorted net, `circuit_part_lights`
  for every part into `lights: [UUID: [Bool]]`, the error, then
  `stateVersion &+= 1`.
- `setSwitch`, `memorySize(of:)`, `readMemory(of:start:count:)` call the
  circuit directly; `MemoryWindow` reloads on `stateVersion` and
  highlights words changed in the last `highlightSteps` versions.
- Drawing: `SchematicCanvasView.worldLayer` builds `RoutedWire(color:
  wireColor, dashed: wireIsFloating)`; `WireLayer` strokes solid 1.5 or
  dashed `[4, 3]`. LEDs: `DiodeSymbolView(isLit: lights[id]?.first)`, glow
  ellipse at opacity 0.3, body/arrows in `light` when lit. 7-segment:
  `SevenSegmentSymbolView(glow: [Bool])`, per LED `isOn ? light :
  white.opacity(0.1)`.
- Toolbar (`ContentView.swift`): Run/Stop button (⌘R) in `.navigation`.
- Build settings: project-level Debug `GCC_OPTIMIZATION_LEVEL = 0`
  (`project.pbxproj` line 181); `MACOSX_DEPLOYMENT_TARGET = 15.8`;
  `SWIFT_DEFAULT_ACTOR_ISOLATION = MainActor`; Swift 5 mode.

### Verified properties

- **Debug C optimization costs 3.2×.** Prototype engine, same bench:
  `-O0` → 0.99x real time (1 MHz, 400 idle) and 0.028x (100 MHz, 100
  idle); `-O2` → 3.2x and 0.09x. The app's backend is only C files, so
  setting the C optimization level for Debug affects only the backend.
- **The proof circuit's numbers.** `test.schematical`: U2 is
  `OSC-SIT1534-1508`, `defaultHz` 32768 → half period
  (10¹² + 32768) / 65536 = 15 258 789 ps; 50 % duty, so the LED (U3) on-time
  per frame is 0.5 of the frame to within one half period (15 µs of a
  16.7 ms frame = 0.1 %).
- **Swift isolation.** With default MainActor isolation, a type used on the
  sim thread must be declared `nonisolated`. `OpaquePointer` is not
  `Sendable`; the engine class is `@unchecked Sendable` with every circuit
  call under its `NSLock`.

## 3. Design Overview

Three pieces:

1. **`SimulationEngine`** (new, `Simulation/SimulationEngine.swift`): owns
   the `Circuit` pointer and a dedicated `Thread`; every circuit call goes
   through one `NSLock`. Its loop paces circuit time to the wall clock and
   publishes a `SimulationSnapshot` to the main actor once per frame.
2. **`SimulationController`** (existing, main actor): builds the circuit as
   today, hands it to the engine, turns snapshots into what views draw (net
   values, toggling set, light levels, reached speed), forwards
   switch/memory calls to the engine.
3. **Views**: speed picker and reached-speed text in the toolbar; toggling
   wire style; brightness for LEDs and segments.

Risk concentrates in (1): thread shutdown (Stop while a chunk runs) and lock
coverage (memory window reads during a run). Both are covered by the
runtime proof's Stop/Run cycles and memory-window step.

**Correctness gate:** behavior, not byte-identity — the runtime proof's
measured circuit time, reached speed, LED level and toggling flag.

**Rejected alternatives**

- *Swift `actor` for the engine.* The loop blocks in C for up to the frame
  budget; a blocking loop on the cooperative pool starves other tasks. A
  dedicated `Thread` is the right tool.
- *`Synchronization.Mutex`.* Available on macOS 15, but its `withLock`
  closure plus a non-`Sendable` `OpaquePointer` adds friction for no gain
  over `NSLock` here.
- *Catching up after falling behind.* Would make the circuit jump forward
  in bursts; re-anchoring keeps motion smooth and the readout honest.
- *Debug `-O2` instead of `-Os`.* `-Os` matches what Release uses by
  default; either removes the 3.2× `-O0` penalty.

## 4. Detailed Design

### 4.1 `SimulationEngine`

```swift
nonisolated struct SimulationSnapshot: Sendable {
    let wallTime: TimeInterval          // ProcessInfo.systemUptime at publish
    let circuitTime: UInt64             // ps, circuit_time
    let netValues: [CircuitValue]
    let netChanges: [UInt64]
    let lightTimes: [Int: [UInt64]]     // part index → cumulative on-ps (parts with lights)
    let shortedNet: Int?
    let error: String?
}

nonisolated final class SimulationEngine: @unchecked Sendable {
    init(circuit: OpaquePointer, netCount: Int, lightParts: [Int], speed: Double,
         publish: @escaping @Sendable (SimulationSnapshot) -> Void)
    func start()                  // spawns the thread
    func stopAndWait()            // sets cancelled, joins (DispatchSemaphore signalled on exit), destroys the circuit
    func setSpeed(_ speed: Double)  // under lock; re-anchors
    func setSwitch(part: Int, position: Int, on: Bool)
    func memorySize(part: Int) -> MemorySize?
    func readMemory(part: Int, start: Int, count: Int) -> [MemoryWord]
    func snapshotNow() -> SimulationSnapshot   // for the first frame after start
    static var liveThreads: Int { get }        // threads started minus exited, under a static lock
}
```

`lightParts` = part indexes whose `circuit_part_light_time` returns > 0,
found once at start.

Loop (constants: `frame = 1/60 s`, `budget = 0.012 s`,
`chunk = 10_000_000 ps`):

```
anchorWall = uptime; anchorSim = circuit_time
while !cancelled:
    frameStart = uptime
    target = anchorSim + UInt64((frameStart - anchorWall) * speed * 1e12)
    loop:
        lock; t = circuit_time                              // circuit_advance call site has
                                                            // dispatchPrecondition(condition: .notOnQueue(.main))
        if t >= target or cancelled { unlock; break }
        ok = circuit_advance(min(chunk, target - t)) != 0
        unlock
        if !ok { publish(snapshot); return }               // error stops the run
        if uptime - frameStart > budget {                   // can't keep up: re-anchor
            anchorWall = uptime; anchorSim = circuit_time (under lock); break }
    publish(snapshot under lock)
    sleep until frameStart + frame
```

`setSpeed` re-anchors (`anchorWall = uptime`, `anchorSim = circuit_time`)
under the lock.

### 4.2 `SimulationController` changes

- Remove `picosecondsPerTick`, `tickInterval`, `task`, `step()`, the
  `refresh()` body that reads the circuit.
- New observable state: `speed: Double` (bound to
  `@AppStorage("simulationSpeed")` in `ContentView`, default 1),
  `reachedSpeed: Double?`, `togglingNets: Set<Int>`,
  `lights: [UUID: [Double]]`.
- `start(document:)` becomes `NetlistBuilder.build` + a new
  `start(netlist: SchematicNetlist)` that does the rest, so the runtime
  proof can start a netlist built in code.
- `start(netlist:)`: build as today; create the engine with `publish: { snap in
  Task { @MainActor in self?.apply(snap) } }`; `apply(engine.snapshotNow())`;
  `engine.start()`.
- `apply(snapshot)`: `netValues`, `shortedNets`, `errorMessage` as today;
  `togglingNets` = nets with `netChanges[i] - previousChanges[i] ≥ 2`;
  for each light part, level = `Double(on - prevOn) / Double(t - prevT)`
  clamped to 0…1 (keep the previous levels when `t == prevT`); the first
  snapshot after start only records the baseline and sets every level to 0;
  `reachedSpeed` = Δcircuit ÷ Δwall over the snapshots of the last 0.5 s;
  `stateVersion &+= 1`.
- `stop()`: `engine?.stopAndWait()`; clear state as today.
- `setSwitch`, `memorySize`, `readMemory`: forward to the engine.
- `setSpeed(_:)`: store, forward to the engine if running.
- `wireStyle(_ wireID) -> WireStyle` replacing `wireColor` +
  `wireIsFloating`:
  `enum WireStyle { case normal, shorted, toggling, floating, value(Color) }`
  in that precedence (shorted beats toggling beats floating).

### 4.3 Views

- `RoutedWire`: `style: WireStyle` replaces `color` and `dashed`.
  `WireLayer` strokes: `.normal` → `.primary` solid; `.value(c)` → `c`
  solid; `.floating` → `.secondary` dashed `[4, 3]` (as today);
  `.shorted` → `.red` solid; `.toggling` → `.secondary` solid, then
  `.green` dashed `[4, 3]` on top.
- `DiodeSymbolView`: `isLit: Bool` → `level: Double` (0…1). Glow ellipse
  opacity `0.3 * level` when `level > 0`; triangle and arrows: draw in
  `outline`, then over it in `light.opacity(level)`.
- `SevenSegmentSymbolView`: `glow: [Bool]` → `levels: [Double]`; per LED
  draw `white.opacity(0.1)`, then `light.opacity(level)` on top when
  `level > 0`.
- `SchematicCanvasView`: pass `simulation.lights[chip.id]?.first ?? 0` and
  `simulation.lights[chip.id] ?? []`; build `RoutedWire(style:
  simulation.wireStyle(wire.id))`.
- `ContentView` toolbar, right after Run/Stop in `.navigation`:
  - `Menu` labelled with the current speed, items `1×`, `1/10`, `1/100`,
    `1/1,000`, `1/10,000`, `1/100,000`, `1/1,000,000`; enabled while
    running or stopped; selection writes `@AppStorage("simulationSpeed")`
    and calls `simulation.setSpeed`.
  - `Text` shown while running when `reachedSpeed < 0.9 × speed`:
    `"running at \(formatted)×"` (two significant digits, e.g. `0.078×`),
    `.secondary`, with help text "The circuit is too busy to run at the
    chosen speed."

### 4.4 Build setting

`schematical.xcodeproj/project.pbxproj`, project-level Debug configuration
(line 181): `GCC_OPTIMIZATION_LEVEL = 0;` → `GCC_OPTIMIZATION_LEVEL = s;`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; moot tasks struck through with
> evidence; fill `Commit:` the moment a phase lands. **An unticked box means
> NOT DONE.**

### Phase 1 — Background engine with wall-clock pacing

Same drawing as today (bool lights from levels > 0, wire colors from
values), but real-time and off the main thread.

- [ ] `project.pbxproj`: §4.4.
- [ ] `Simulation/SimulationEngine.swift`: §4.1.
- [ ] `Simulation.swift`: §4.2 except `wireStyle` and light levels (keep
      `lights: [UUID: [Bool]]` from on-time growth > 0 for this phase);
      `speed` fixed at 1.
- [ ] `MemoryWindow.swift`: no edits; it uses the same controller methods,
      now forwarded to the engine.

Acceptance: app builds; at speed 1 the proof circuit's circuit time tracks
wall time.
  Check: the `xcodebuild` in Prerequisites → `** BUILD SUCCEEDED **` (est. 3 min),
  then runtime proof step 1 (est. 5 min).
Commit: —

### Phase 2 — Speed picker, reached speed, toggling wires, brightness

- [ ] `Simulation.swift`: `setSpeed`, `reachedSpeed`, `togglingNets`,
      `wireStyle`, light levels (§4.2).
- [ ] `SchematicCanvasView.swift`, `DiodeSymbolView.swift`,
      `SevenSegmentSymbolView.swift`: §4.3.
- [ ] `ContentView.swift`: speed `Menu` and reached-speed `Text` (§4.3).

Acceptance: runtime proof steps 2–5.
  Check: the `xcodebuild` → `** BUILD SUCCEEDED **` (est. 3 min), then
  runtime proof steps 2–5 (est. 10 min).
Commit: —

## Validation Plan

- Tests: no Swift test target exists (`find schematical -name '*Tests*'` →
  none, plan-02-B); the behavior proof is the runtime proof. The backend
  calls it relies on are tested in plan-05-B.
- Runtime proof (Debug build; headless harness as plan-03-D if the session
  is locked; values printed from `SimulationController`, PNGs to
  `/tmp/p05c-proof/`):
  1. Open `test.schematical`, Run, wait 2 s: circuit time 2 s ± 10 %;
     the `dispatchPrecondition(.notOnQueue(.main))` at the
     `circuit_advance` call site never fires (a Debug build traps if it does).
  2. Same run: `reachedSpeed` ∈ [0.9, 1.1]; LED level ∈ [0.45, 0.55]; the
     wire on U2 pin 1 (OUT) has style `.toggling`; screenshot.
  3. Speed 1/1,000,000, wait 2 s: circuit time advanced 2 µs ± 10 % since
     the change; LED level is 0 or 1 per frame (the clock half period
     15.26 µs is longer than a frame's 16.7 ns).
  4. The harness builds `sim_bench`'s circuit as a `SchematicNetlist`
     (`LMK6CE10000CDLER`, 16 SN74HC74N stages with LEDs, 100 idle SN74HC00N;
     same connections as `backend/tests/sim_bench.c`) and calls
     `start(netlist:)` at speed 1: after 2 s `reachedSpeed` < 0.9 and the
     toolbar shows "running at …×"; the main run loop keeps servicing
     (a main-queue timer scheduled every 50 ms fires ≥ 30 times in those
     2 s); `stop()` returns in < 50 ms.
  5. A schematic with an AS6C6264-55PCN SRAM: its memory window opens and
     reloads during a run without a crash; Run/Stop 10 times in a row, and
     after each `stop()` `SimulationEngine.liveThreads == 0`.
- Doc sync: none in `docs/` (app behavior); `tmp/sim.md` is untracked
  scratch and is not updated.
- Final gate (plan-05 overall, run once after this phase):
  `rm -rf /tmp/logic-land-p05 && cmake -S backend -B /tmp/logic-land-p05 && cmake --build /tmp/logic-land-p05 -j8 && ctest --test-dir /tmp/logic-land-p05`
  → `100% tests passed, 0 tests failed out of 9` (est. 1 min);
  Release `sim_bench` both cases at their plan-05-A thresholds (est. 15 s);
  the `xcodebuild` (est. 3 min); the runtime proof (est. 15 min).

## Decisions

- Run speed: real time by default, a picker 1× … 1/1,000,000, run flat out
  and show the reached speed when the engine can't keep up — owner,
  2026-11-01.
- Fast wires drawn as "toggling" — owner, 2026-11-01.
- LED and segment brightness = on-time in the frame — owner, 2026-11-01.
- plan-05 before plan-04 — owner, 2026-11-01.
- Toggling = ≥ 2 changes in a frame (a single edge, e.g. a switch flip,
  shows as the new value) — author, 2026-11-01.
- Speed is an app preference (`@AppStorage`), not saved in documents —
  author, 2026-11-01.
- Debug C optimization `-Os` — author, 2026-11-01 (§2, 3.2× measured).

## Corrections

## Summary

Risk: thread lifetime and locking in `SimulationEngine`; covered by the
Stop/Run and memory-window proof steps. The backend, the file format and
the PCB page are untouched.
