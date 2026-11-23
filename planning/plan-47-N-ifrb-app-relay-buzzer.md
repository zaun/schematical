# plan-47-N: The app — relay and buzzer symbols, the relay's play state, the buzzer's sound

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-M

N draws relays and buzzers and brings them to life in play mode. A relay
draws as a coil (or, for a solid-state relay, an LED) beside its poles, each
pole an arm from its common pin to its normally-closed pin; while the relay
is operated the arms swing to the normally-open pins. A buzzer draws as the
standard half-disc sounder; while it is on it shows sound waves and the app
plays a tone: an indicator's rated frequency, or, for a transducer, the
frequency its `+` pin is being driven at. The behavioral outcome: a gate
output on a relay's coil closes its normally-open contact, which lights an
LED and sounds an indicator buzzer; with the gate's input flipped both stop;
a relay wired to interrupt its own coil toggles every millisecond and gives
a transducer on its contact a 500 Hz voice.

References:

- `schematical/schematical/Simulation/Simulation.swift` (`lights: [UUID:
  [Double]]` `:268–271`, the snapshot handler `:446–475`: brightness from
  `lightTimes`, `togglingNets` from `netChanges`; `SchematicNetlist.connections`
  `:22–23`), `SimulationEngine.swift` (`SimulationSnapshot.netChanges`,
  `lightTimes` `:60–63`, built for every part whose model has the lights
  hooks, `Simulation.swift:351`).
- `Canvas/SchematicModel.swift` (`PartSymbol`, `ChipLayout`),
  `Canvas/TransistorSymbolView.swift` and `DiodeSymbolView.swift` (symbol
  views with play-mode state), `Canvas/TactileSwitchSymbolView.swift` (a
  contact drawn open or closed), `SchematicCanvasView.swift`,
  `SchematicExportScene.swift`, `ReferenceLabel.swift`.
- `Preferences/PreferencesView.swift` (Settings), `Help/KeyboardHelp.swift`.
- plan-47-J § 3.2 (one light per relay channel: operated), § 3.4 (a
  buzzer's light: on); plan-47-M § 3 (the `Details` cases N reads).
- `scripts/harness/transistor-app.swift`, `build.sh`;
  `planning/completed/plan-40-H-transistor-app.md`.

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-M complete | `ls planning/completed/plan-47-M-*.md` → exists | NOT MET as written (2026-11-22: `ls planning/completed/plan-47-M-*.md` → no such file; M's work is committed on this worktree's branch at 15270503 and N is worked on top of it, § Corrections) |
| `Details.relay` and `.buzzer` exist | `grep -c 'case relay(\|case buzzer(' schematical/schematical/Backend/PartCatalog.swift` → 2 | MET (2026-11-22: 2, added by M) |
| The app has no audio engine yet | `grep -rln 'AVAudioEngine\|AVAudioSourceNode' schematical/schematical --include='*.swift' \| wc -l` → 0 | MET at the start (2026-11-22: 0 at HEAD, `git grep -l … HEAD` → 0; `NSSound.beep()` the only sound); after N: 1 file, `Simulation/BuzzerSound.swift` |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-22, P-47 worktree, `-derivedDataPath /tmp/p47-dd-n`: `** BUILD SUCCEEDED **` before and after the changes) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `** BUILD SUCCEEDED **`, and `xcodebuild … test
  -only-testing:schematicalTests/RelayBuzzerTests
  -only-testing:schematicalTests/DrawerPerformanceTests` passes.
- The harness (`scripts/harness/relay-buzzer-app.swift`) prints and checks:
  `symbol(for:)` `.relay` for every relay part and `.buzzer` for every
  buzzer part of the catalog; the layouts of an SPDT, a DPDT, a dual-coil
  latching and a two-channel solid-state relay; the play-mode circuit of the
  outcome above (operated, LED lit, voice at the buzzer's `hz`; then at
  rest, dark, no voice); and the transducer voice within 2 % of 500 Hz.

### Non-goals (explicit constraints)

- No new field in the schematic file; no saved relay or buzzer state.
- No backend change: N reads `lights` and `netChanges`, which the engine
  already reports.
- No waveform fidelity: one square-wave voice per sounding buzzer at a fixed
  level; no sound-pressure, resonance, attack or "siren" patterns.
- Sound only in play mode, only in the app's process: no audio in exports,
  in the automation API's results, or under tests.
- No relay click sound.

## 2. Current State

- A relay or buzzer part draws as a block (`symbol(for:)`'s default); M
  gave both kinds their `Details`, facts and references.
- `Simulation.lights` already holds a brightness per light for every part
  whose model has the lights hooks (`Simulation.swift:351, 465–475`): after
  J, a relay has one entry per channel (operated time over frame time) and a
  buzzer one (on time over frame time). No view reads them for these kinds.
- `Simulation` knows each net's change count per frame (`netChanges`,
  `togglingNets`) and each connected pin's net (`SchematicNetlist.connections`:
  part index, pin designator, net index).
- The app has no audio output besides `NSSound.beep()`
  (`Memory/MemoryWindow.swift:293`, `Document/EditCommands.swift:108`).

### Measured populations

| What | Count | Command |
|---|---|---|
| Relay arrangements the layout must draw (planning pick) | 16 contact forms, up to 5 poles; non-latching 481, single-coil latching 17, dual-coil latching 31, solid-state (an LED input) 44; 17 of them multi-channel | plan-47-J § 2 |
| Buzzers by driver (planning pick) | transducer 331, indicator 222 | plan-47-L § 2 |
| Buzzer frequencies (planning pick) | 4 kHz 89, 2.7 kHz 60, 2.4 kHz 39, 2.73 kHz 39, 2.048 kHz 33, 3.5 kHz 24, 3 kHz 23, 4.1 kHz 22, 2.5 kHz 21, 2.3 kHz 19, 2 kHz 16; 13 give a range (`150Hz ~ 4kHz`) | `awk -F'\t' '$23=="candidate" && $3=="buzzer"{print $8}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c \| sort -rn` |
| Exhaustive `PartSymbol` switch sites | 4 files | plan-47-M § 2 |

### Verified properties

- **Brightness is on-time over circuit time** (`Simulation.swift:462–475`),
  so a relay that is operated for the whole frame reads 1.0 and a buzzer on
  a 50 % square wave reads 0.5.
- **`netChanges` counts every value change of a net since start**
  (`circuit.h:65–66`), so a net driven by a square wave of frequency f gains
  2 f changes per second of circuit time.

## 3. Design

### 3.1 The relay symbol

`PartSymbol.relay`; `symbol(for:)` returns it for every `PART_RELAY` part.
`ChipLayout.relay(part)` builds from `RelayInfo` and the pins' names (the
names plan-47-C § 3.2 fixed):

- left side, top to bottom: the coil pins in coil order (`A1`, `A2`; `S1`,
  `S2`, `R1`, `R2`; `S`, `C`, `R`; a solid-state input `A`, `K`; several
  channels' inputs in channel order);
- right side, per pole in contact order: `NO`, `COM`, `NC` rows (a pole
  without `NO` or `NC` leaves that row out), a gap between poles;
- every `NC`-role pin and every extra pin of a doubled terminal is not drawn
  (the pads exist on the PCB; the resistor and crystal rule);
- the box's height is the taller side's; a relay with more than 16 contact
  rows falls back to the block layout (none in the pick: a 4PDT has 12 rows,
  the 4 NO + 1 NC part 10).

`Canvas/RelaySymbolView.swift` draws inside the box: at the left a coil
rectangle between the coil pins (two rectangles for a dual-coil relay,
labelled S and R; an LED glyph for a solid-state input); a dashed line from
the coil to the poles; per pole a contact arm pivoting at `COM`: to the `NC`
terminal at rest, to the `NO` terminal when operated (a Form A pole's arm
rests open, a Form B pole's arm opens when operated). The operated state is
`simulation.lights[chip.id]?[channel] ?? 0 > 0.5`, with `channel` the pole's
coil (0 for a latching relay); outside play mode every arm is at rest.
Static `paint(_:chip:isSelected:isHovered:operated:)` for the export scene
(always at rest); highlight and flip as `TransistorSymbolView`. The coil
voltage ("12 V") sits under the coil.

### 3.2 The buzzer symbol

`PartSymbol.buzzer`; `symbol(for:)` returns it for a `PART_BUZZER` part with
pins named `+` and `-`. `ChipLayout.buzzer(pins)`: `+` left, `-` right on
the diode box's centre line; other pins not drawn.
`Canvas/BuzzerSymbolView.swift`: a half disc on a base line between the two
leads, a small `+` by the positive lead, the frequency under it
(`PartFacts.hertz`); in play mode, while `simulation.lights[chip.id]?[0] ??
0 > 0`, three arcs above the dome. Static `paint` for the export scene (no
arcs).

### 3.3 The buzzer's sound

`Simulation` computes, per snapshot, `buzzerVoices: [UUID: Double]` (chip →
frequency in Hz) with the pure function
`BuzzerTone.voices(netlist:snapshot:previous:parts:)`
(`Simulation/BuzzerTone.swift`):

- an **indicator** whose light gained on-time in the frame sounds at its
  part's `hz`;
- a **transducer** sounds at f = (changes its `+` pin's net gained in the
  frame) ÷ 2 ÷ (circuit seconds of the frame), when its light gained on-time
  and 20 ≤ f ≤ 20000; a transducer held on by a steady level gains no
  changes and is silent, as the real part is;
- a buzzer whose `+` pin has no net, and every buzzer outside play mode, has
  no voice.

`Simulation/BuzzerSound.swift` plays the voices: one `AVAudioEngine` with
one `AVAudioSourceNode` that sums a square wave per voice (at most 8 voices,
the 8 lowest chip ids in a stable order), each at −24 dBFS, with a 10 ms
linear ramp when a voice starts, stops or changes frequency; the engine
starts with the first voice and stops 1 s after the last one ends and at
once when play stops. The frequency is the circuit's: at a simulation speed
other than real time the tone keeps the circuit frequency.

Settings ▸ General Defaults (`PreferencesPage.defaults`) gains **Play buzzer
sounds** (`@AppStorage("playBuzzerSounds")`, default on). `BuzzerSound` is not created when the setting is off or when
the process runs under XCTest (`NSClassFromString("XCTestCase") != nil`), so
the tests and the harness check `buzzerVoices`, never the audio device.

Help: the Simulation section of `Help/KeyboardHelp.swift` gains "Buzzers
play through the Mac's speakers; turn them off in Settings".

Where the risk concentrates: the audio render callback (it runs on the audio
thread: it reads an atomically swapped voice table and allocates nothing)
and the relay layout for the unusual arrangements (dual coil with a common
pin, mixed poles, multi-channel). The tests cover the layouts and
`BuzzerTone`; the audio node is exercised by the runtime proof (Phase 3).

Rejected alternatives: (1) a backend "tone" API (rejected: `netChanges` and
`lights` already carry the frequency and the state; plan-47-J § 3.5); (2)
`NSSound`/system beeps per buzzer (rejected: no frequency control, and a
buzzer held on would need a looping asset per frequency); (3) one symbol per
contact arrangement (rejected: 16 forms; one layout from the device's
contacts draws them all); (4) sine voices (rejected: buzzers are driven by
square waves and a square voice is audibly "a buzzer" at −24 dBFS without
being loud).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The relay symbol and its play state

- [x] (2026-11-22: `PartSymbol.relay`, `.buzzer` after `.fuse`; `ChipLayout.relay(_:)` over `Canvas/RelayShape.swift`; `symbol(for:)` `case PART_RELAY` falls back to `.block` when `RelayShape` is nil, § Corrections) `Canvas/SchematicModel.swift`: `PartSymbol.relay` (only if absent),
      `ChipLayout.relay`; `Backend/PartCatalog.swift` `symbol(for:)`.
- [x] (2026-11-22: `RelaySymbolView`; `SchematicCanvasView` passes `simulation.lights[chip.id]` as `levels`; `SchematicExportScene` (at rest); `ReferenceLabel` anchor) `Canvas/RelaySymbolView.swift` (§ 3.1); the three other switch sites;
      the canvas passes the operated state from `simulation.lights`.
- [x] (2026-11-22: `testEveryRelayDrawsAsARelay`, `testSPDTLayout`, `testDPDTLayout`, `testDualCoilLatchingLayout` (both pinouts), `testSolidStateLayouts` (Form A, Form B, two-channel), `testNoConnectAndDoubledPinsAreNotDrawn`, `testRelaysRender`, `testTheArmSwingsWithTheOperatedState`; parts picked by property) Tests: `schematicalTests/RelayBuzzerTests.swift`: `symbol(for:)` is
      `.relay` for every relay part of the catalog; the layouts of one real
      SPDT, DPDT, dual-coil latching, Form A solid-state and two-channel
      solid-state part (pin sides and order; no `NC` pin drawn); a placed
      relay renders.

Acceptance: relays draw as relays.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO test -only-testing:schematicalTests/RelayBuzzerTests` → `** TEST SUCCEEDED **` (est. 8 min).
  Observed (2026-11-22, `-derivedDataPath /tmp/p47-dd-n`): the relay tests of `RelayBuzzerTests` pass; the class (22 tests) → `Executed 22 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: 68871ba5

### Phase 2 — The buzzer symbol and its voices

- [x] (2026-11-22: `PartSymbol.buzzer`, `ChipLayout.buzzer`, `symbol(for:)`, `BuzzerSymbolView`, the other switch sites and both exporters' `bodyBox`) `PartSymbol.buzzer`, `ChipLayout.buzzer`, `symbol(for:)`;
      `Canvas/BuzzerSymbolView.swift` (§ 3.2); the three other switch sites.
- [x] (2026-11-22: `BuzzerTone.voices(netlist:snapshot:previous:window:)`, `SchematicNetlist.buzzers`, `SimulationController.buzzerVoices`, § Corrections) `Simulation/BuzzerTone.swift` and `Simulation.buzzerVoices` (§ 3.3).
- [x] (2026-11-22: `testEveryBuzzerDrawsAsABuzzer`, `testBuzzersRender`, `testIndicatorVoice`, `testTransducerVoice` (1000 Hz, steady, 30 kHz), `testTransducerPitchUsesTheWindow`, `testABuzzerWithoutAPlusNetHasNoVoice`, `testTheNetlistListsTheBuzzers`) Tests (`RelayBuzzerTests`): `symbol(for:)` `.buzzer` for every buzzer
      part; the layout; `BuzzerTone.voices` on hand-built snapshots: an
      indicator on (its `hz`), off (none); a transducer with 2000 changes in
      1 s of circuit time (1000 Hz), with a steady level (none), at 30 kHz
      (none); no voice without a `+` net.

Acceptance: buzzers draw as buzzers and their voices are computed.
  Check: `xcodebuild … test -only-testing:schematicalTests/RelayBuzzerTests` → `** TEST SUCCEEDED **` (est. 8 min).
  Observed (2026-11-22): `RelayBuzzerTests` → `Executed 22 tests, with 0 failures (0 unexpected)`, `** TEST SUCCEEDED **`.
Commit: 68871ba5

### Phase 3 — The sound, the setting, the harness

- [x] (2026-11-22: `BuzzerSound.swift`, the setting in Settings > General Defaults, the Simulation help line, `KeyboardHelpTests.testSimulationBuzzerLine`; also `testToneRenderer`, `testVoiceSlots`, `testNoAudioUnderTest`, `testTheSourceNodeRendersTheToneOffline`) `Simulation/BuzzerSound.swift` (§ 3.3); `Preferences/PreferencesView.swift`:
      the setting; `Help/KeyboardHelp.swift` and `KeyboardHelpTests`.
- [x] (2026-11-22: written; run result under Acceptance) `scripts/harness/relay-buzzer-app.swift` (from `transistor-app.swift`):
      the checks of § 1 on the app's real catalog, document and
      `Simulation`: `5V`/`GND` → `SN74HC04N` (input on GND) → 1Y → the coil
      `A1` of the most-stocked 5 V SPDT `add` relay (`A2` on GND); `5V` on
      its `COM`; its `NO` → `GENERIC-RESISTOR` → `GENERIC-LED` → GND and →
      the `+` of the most-stocked 5 V indicator `add` buzzer (`-` on GND):
      a 0.3 s run: the relay's light is on, the LED lit,
      `buzzerVoices` holds the buzzer at its `hz`; the same with the gate's
      input on 5V: at rest, dark, no voice. A second circuit, the
      self-interrupting relay of plan-47-J § 6 (p): `5V` on the relay's
      `NC`, its `COM` on its own `A1` and on the `+` of the most-stocked
      transducer `add` buzzer, `A2` and the buzzer's `-` on GND: `COM`
      toggles every 1 ms, so the voice is within 2 % of 500 Hz. Renders a placed
      relay (at rest and operated), a solid-state relay and a buzzer to
      `/tmp`.
- [ ] NOT DONE by the implementing agent (it cannot hear and drove no audio; § Corrections "runtime proof by ear") Runtime proof by ear and eye, once, in the built app
      (`open /tmp/p47-dd/Build/Products/Debug/schematical.app`): build the
      first harness circuit by hand, press play, hear the tone, toggle the
      setting off and hear it stop; move the pointer to (0, 0) afterwards
      (`AGENTS.md` § Testing in the app). Record what was seen and heard in
      § Corrections.

Acceptance: relays and buzzers work in play mode, end to end.
  Check: `rm -rf /tmp/p47-harness && APP=/tmp/p47-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/relay-buzzer-app.swift /tmp/p47-harness && /tmp/p47-harness/H.app/Contents/MacOS/harness` → every line `ok`, exit 0; `xcodebuild … test -only-testing:schematicalTests/RelayBuzzerTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/KeyboardHelpTests` → `** TEST SUCCEEDED **` (est. 15 min, plus 5 min for the hand check).
  Observed (2026-11-22): the harness (built with `APP=/tmp/p47-dd-n/Build/Products/Debug/schematical.app`) printed 13 `ok` lines, `gate low in: relay light 1.0, LED 1.0, voices ["2700.0 Hz"], rated 2700.0 Hz`, `gate high in: relay light 0.0, LED 0.0, voices [:]`, `self-interrupting relay 1558940-2 with transducer AST0440MFD-03TRQ: voice 500.3657369165446 Hz`, `all checks ok`, exit 0; `xcodebuild … test` with RelayBuzzerTests, DrawerPerformanceTests, KeyboardHelpTests and InductorFuseTests → `Executed 52 tests, with 0 failures`, `** TEST SUCCEEDED **`. NOT met: the hand check (ear) above, so the phase's last box stays open. A run of the whole `schematicalTests` target was stopped at RouterFixtureTests (minutes of router work, untouched by N): 933 test cases had passed and none failed up to there.
Commit: 68871ba5

## Validation Plan

- Tests: `RelayBuzzerTests` (layouts, `BuzzerTone`); `DrawerPerformanceTests`,
  `KeyboardHelpTests`.
- Coverage check: the harness runs the app's own catalog and `Simulation`
  with real `add` parts; every relay and buzzer part's symbol is asserted.
- Runtime proof: the harness's two circuits, and the hand check of Phase 3
  (the one thing a test cannot assert: that sound comes out).
- Doc sync: the in-app help line; no file-format change.
- Final gate: plan-47-O.

## Decisions

Owner decisions (asked once, 2026-11-16):

- **A relay switches in play mode** and the symbol shows it.
- **A buzzer sounds in play mode**: the symbol shows sound waves and the app
  plays a tone at the part's rated frequency (an indicator) or at the
  frequency of its input (a transducer).

Author decisions:

- **A setting turns the sound off** (Settings ▸ General Defaults ▸ Play
  buzzer sounds, default on): an app that makes noise needs a switch.
- **Square voices at −24 dBFS, at most 8** (§ 3.3).
- **A transducer on a steady level is silent**, as the real part is; the
  drawer leaf and the sidebar Type say which kind a buzzer is.
- **One relay layout from the device's coils and contacts** (§ 3.1); `NC`
  pins and doubled terminals are not drawn on the schematic.
- **The tone follows circuit time**, not wall time.
- **No audio under XCTest or in the harness**; they assert `buzzerVoices`.

## Corrections

- **2026-11-22, § Prerequisites.** plan-47-M is not in `planning/completed/`
  (the coordinator commits the plans); its work is in the branch at 15270503
  and N ran on top of it.
- **2026-11-22, line numbers moved.** `lights` is at `Simulation.swift:~314`
  (not 268), the snapshot handler `apply` at `:~500` (not 446), `lightTimes`
  in `SimulationEngine.swift:63`; the part files now draw relays under
  `["Switches & Relays","Relay",…]` and buzzers under
  `["Displays & Indicators","Buzzers"]` (the plan's older paths).
- **2026-11-22, § 3.1 pins and layout.** The backend's relay entries
  (coils, contacts) are not in the C part struct the app reads (`PartRelay`
  has only counts), so `Canvas/RelayShape.swift` derives the coils and poles
  from the pin names and roles: coil pins are the `input`-role pins (`A1`/`A2`
  [`A3`/`A4`], `S1`/`S2`/`R1`/`R2`, `S`/`C`/`R`, `A`/`K`, `A<n>`/`K<n>`),
  contacts are the `inout`-role pins named `COM`/`NO`/`NC` plus an optional
  pole number, and a solid-state output `D1`/`D2` or `D<n>A`/`D<n>B` (first
  = common, second = `NO`, or `NC` when the part's `function` says `NC` and not
  `NO`). Role matters: a reed relay's `NC` no-connect pads are named `NC` too.
  A non-latching relay with several coils shares its poles out in order
  (pole i moves with coil `i * coils / poles`; checked against every device of
  `relay.json`: it matches each `"coil"` number). A two-pin coil takes rows
  `3 k` and `3 k + 2` (a free row between its pins gives the coil room), the
  three-terminal `S`, `C`, `R` takes rows 0, 2, 4 with two rectangles sharing
  `C`. The body is 140 wide (`ChipLayout.relayBodyWidth`) and half a row taller
  than its lowest pin row, which holds the coil voltage; the box has the part
  number in a header like the tactile switch. The worst case in the catalog is
  15 rows (the 16-row fallback is `RelayShape.maxRows`); a relay whose shape is
  nil (`RelayShape(part:)`) gets `symbol(for:) == .block`, so "`.relay` for
  every relay" holds for the whole catalog (asserted) and the fallback is
  decided in `symbol(for:)`, not in the layout.
- **2026-11-22, § 3.1 coil glow.** A non-latching relay's coil rectangle is
  tinted while operated, an LED input turns red; a latching relay's two coils
  never tint (which one is energised is not in the lights).
- **2026-11-22, § 3.2.** The buzzer's frequency is under the symbol and the
  reference label sits at the resistor's height (above the sound waves); `+`
  and `-` are drawn left and right at `diodeCentreY`; the KiCad and EasyEDA
  writers' two-terminal `bodyBox` lists gained `.buzzer` (the relay is a box
  there, as the tactile switch is).
- **2026-11-22, § 3.3 names.** `BuzzerTone.voices(netlist:snapshot:previous:window:)`
  (no `parts:`): the part facts the tone needs (chip, part index, driver,
  rated hertz, `+` net) are read once into `SchematicNetlist.buzzers`
  (`BuzzerTone.Buzzer`) by `NetlistBuilder`. A transducer's pitch is counted over a
  100 ms window of circuit time (`Simulation.toneHistory`, oldest snapshot
  within `BuzzerTone.windowPS`), because a 60 Hz frame holds 16 or 17 changes
  of a 500 Hz wave (3 % jitter); when the last frame holds at least 8 changes
  and differs from the window by more than 30 %, the frame's pitch wins (the
  drive has just changed); a frame with fewer changes uses the window. The
  rest is as planned (an indicator at its `hz`; 20 to 20000 Hz; no `+` net, no
  voice; a snapshot with an error, none).
- **2026-11-22, sound.** `Simulation/BuzzerSound.swift`: `ToneRenderer` (the
  audio thread's square-wave mixer: preallocated, `os_unfair_lock_trylock`,
  10 ms ramp, a pitch within 10 % of the old one glides instead of ramping),
  `VoiceSlots` (8 lowest chip ids, each kept in its slot), `BuzzerSound` (the
  engine; stops 1 s after the last voice and at once on stop), `BuzzerSoundSetting`
  (the `playBuzzerSounds` key, default on, `PreferenceKey.playBuzzerSounds`;
  `isSilenced`, which the harness sets; `isUnderTest`). The sound is never
  made under XCTest or in the harness. The engine's mixer takes 3 dB off a
  mono source (measured offline: peak 0.0446 for 0.0631), so `BuzzerSound`
  gives the renderer a gain of sqrt 2 (`monoMixerCompensation`) and each voice
  is -24 dBFS at the device (offline test asserts it). The setting is read each frame, so
  turning it off mid-run stops the sound.
- **2026-11-22, tests.** A test helper named `run(for:)` in an `XCTestCase`
  subclass is called as `run()` by its default argument and resolves to
  `XCTestCase.run()` ("Invalid attempt to start a test run that has already
  been started"); the helper is `spin(for:)`.
- **2026-11-22, parts named.** Nothing is hard-coded: tests and harness pick by
  property, the first by name. At this catalog: SPDT `1393277-4`, DPDT
  `1-1462039-7`, dual-coil latching `DE2A-L2-12V` (S1/S2/R1/R2) and
  `1-1415540-1` (S/C/R), solid-state `AQV210EHAX`, two-channel solid-state
  `AQW210AX`, the circuits' 5 V SPDT `1558940-2`, indicator `AI-1027-TWT-3V-R`
  (2700 Hz), transducer `AST0440MFD-03TRQ`. After the final regeneration of the
  parts these are re-picked by the same properties; the harness prints them.
- **2026-11-22, runtime proof by ear.** Not done by the implementing agent: it
  cannot hear. What was run instead: the harness (the circuits' voices, the
  pictures), `testToneRenderer` (the waveform, ramps, glide, level -24 dBFS)
  and `testTheSourceNodeRendersTheToneOffline` (the real `AVAudioSourceNode`
  from `BuzzerSound.makeNode` in an `AVAudioEngine` in offline manual
  rendering mode). What has not been exercised: the output device, and the
  engine start/idle-stop timing in the running app. The hand check stays open.

## Summary

N's risk is the audio render path (a callback on the audio thread) and the
relay layout's unusual arrangements; `BuzzerTone` and the layouts are unit
tested, the harness runs both kinds in real circuits, and one hand check
confirms sound. Untouched: the backend, the part files, the file format and
the Bill of Materials.
