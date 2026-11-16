# plan-46-M: Part Wizard — curated guides for 12 simple kinds and blurbs for all 60 models

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-46-L

M writes the content L's framework carries: curated guides for the
remaining twelve simple kinds (buffer, dff, jkff, latch, mux, decoder,
led, diode, switch, oscillator, header, 7seg; gate is L's), and a
one-line plain-language blurb for every model of every kind with a variant
(60 values over 13 kinds), which the "Which chip does yours behave like?"
screen shows. Complex kinds (serial, converter, rtc, cpu, pio, counter,
shift, arith, register, sram, eeprom, charlcd, connector, supply,
resistor, capacitor, transistor, mcu, generic) keep their derived steps;
those that have variants get blurbs here.

Behavioral outcome: each of the 13 simple kinds walks as named,
explained screens with a Find button on every step; every model row on
every variant screen has a sentence and example parts; `KindGuideTests`
passes for all 13 guides.

References:

- plan-46-L (`FieldGuide`, `KindGuide`, `SharedGuides`, `KindGuideTests`),
  plan-46-I (`variantBlurbs`, `examplesByVariant`).
- `docs/parts-format.md` § Kinds — the per-kind sections (`led` :813,
  `diode` :829, `oscillator` :1047, `gate` :1066, `dff` :1106, `jkff`
  :1139, `switch` :1152, `header` :1262, and the latch / mux / decoder /
  buffer / 7seg sections): the meaning of every key the guides explain.
- The kinds' `…Draft.swift` files (the keys, below).

## Prerequisites

plan-46-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-46-L complete | `ls planning/completed/plan-46-L-*.md` → exists | MET (2026-11-16: `planning/completed/plan-46-L-wizard-guidance-framework.md`) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**

## 1. Goal

- `Editor/Wizard/Guides/<Kind>Guide.swift` for the twelve kinds,
  registered in `PartKinds.guides`.
- `Editor/Wizard/Guides/VariantBlurbs.swift`: `PartKinds.variantBlurbs`
  with all 60 values.
- `KindGuideTests` gains: (6) every value of every variant key has a
  non-empty blurb (60); (7) every required field of the 13 curated kinds
  resolves to a `FieldGuide` (kind or shared) whose `whereToFind` is
  non-empty.

### Non-goals (explicit constraints)

- **No schema changes** — guides only point at keys.
- **No guides for the complex kinds** (owner: simple kinds first).

## 2. The keys each guide covers

Top-level keys per kind (`*` = required), from each generated schema's
`device:` / `part:` arrays (planning probe over the `…Draft.swift` files;
144 keys in all):

| Kind | Device | Part |
|---|---|---|
| buffer (6+3) | domains, invert, model*, output, timing*, units* | family*, inputMax, vcc |
| dff (7+2) | clearActiveHigh*, clockEdge*, output, outputEnable, presetActiveHigh*, timing*, units* | family*, vcc |
| jkff (6+2) | clearActiveHigh*, clockEdge*, kInverted*, presetActiveHigh*, timing*, units* | family*, vcc |
| latch (31+2) | a, address, b1, b2, blank, both, clear, clearActive, data, enable, enableActive, glyphs, inputs, inputsActive, invert, lampTest, latch, latchEnables, model*, output, outputEnable, outputEnableActive, outputEnables, outputs, polarity, presetActive, presetWins, segments, select, timing*, units | family*, vcc |
| mux (14+2) | bits, dataClock, dataLatch, disabled, files, invert, model*, oes, output, outputClock, selectLatch, timing*, units, words | family*, vcc |
| decoder (26+2) | blank, code, codeActive, disabled, enableIn, enableOut, glyphs, groupSelect, idle, inputs, inputsActive, lampTest, latchActive, model*, monitor, offset, output, outputsActive, phase, rbi, rbo, segments, segmentsActive, timing*, units, unselected | family*, vcc |
| led (3+0) | anode*, cathode*, colors* | — |
| diode (2+4) | diodes*, type* | io, pd, vr, vz (`io` and `vr` for every type; `pd` and `vz` for `zener` only, § Corrections 2) |
| switch (7+0) | actuator*, bodySize, contacts*, joined, momentary, normallyClosed, ratedVolts* | — |
| oscillator (6+4) | family*, inputMax, maxHz*, minHz*, outputs*, vcc* | defaultHz*, enablePin, inputMax, vcc |
| header (2+0) | genders*, rows* | — |
| 7seg (13+0) | chip*, colonAfter*, color*, commonAnode*, commonPins*, commonSide*, digits*, dotAfter*, leds*, segmentPins*, segmentSide*, size*, vf* | — |

## 3. Step layout

Fixed layouts for the kinds without variants:

- **dff** — Supply voltage (`part.vcc`); Flip-flops (`units`, `clockEdge`,
  `clearActiveHigh`, `presetActiveHigh`, `output`, `outputEnable`);
  Logic family and timing (`part.family`, `timing`).
- **jkff** — Supply voltage; Flip-flops (`units`, `clockEdge`, `kInverted`,
  `clearActiveHigh`, `presetActiveHigh`); Logic family and timing.
- **led** — The LED (`anode`, `cathode`, `colors`).
- **switch** — The switch (`actuator`, `contacts`, `momentary`,
  `normallyClosed`, `joined`); Ratings and size (`ratedVolts`, `bodySize`).
- **oscillator** — Supply voltage (device and part `vcc`, `inputMax`);
  Frequency (`minHz`, `maxHz`, `part.defaultHz`); Outputs (`outputs`,
  `part.enablePin`); Logic family (`family`).
- **header** — The header (`rows`, `genders`).
- **7seg** — The display (`digits`, `size`, `color`, `chip`, `vf`);
  Wiring (`commonAnode`, `commonPins`, `segmentPins`, `leds`,
  `commonSide`, `segmentSide`); Dots and colon (`dotAfter`, `colonAfter`).

Kinds with variants (buffer, latch, mux, decoder, diode) put the variant
step first, then sort the chosen variant's keys by one mechanical rule, so
no key needs a judgment call:

1. Supply voltage — `vcc`, `inputMax` (either side).
2. Pins it uses — every key whose field type is `.pin`, `.pins`, or an
   object/list whose fields are all `.pin`/`.pins` (e.g. latch `enable`,
   `data`, `outputs`; mux `oes`).
3. Active levels — keys ending `Active`, `ActiveHigh`, `Inverted`, and
   `invert`, `polarity`.
4. Display — `glyphs`, `segments`, `segmentsActive`, `blank`, `lampTest`,
   `rbi`, `rbo` (the seg7 models).
5. Logic family and timing — `part.family`, `timing`.
6. Options — every key the rules above did not take.
   (diode: Diodes = `diodes`; Ratings = part `io`, `vr`, `pd`, `vz`.)

A step with no keys for the variant is omitted. Test (1) of
`KindGuideTests` proves the rule covers every key of every variant once.

## 4. Variant blurbs

One sentence each, plain words, naming what the chip does and one
familiar part family when there is one, e.g. buffer `transceiver` —
"Bidirectional bus buffer with a direction pin, like the 74HC245." The 60
values:

| Kind | Values |
|---|---|
| arith (5) | adder, alu181, compare, parity, wallace |
| buffer (4) | registered, transceiver, translator, tristate |
| capacitor (6) | aluminum, aluminum-polymer, ceramic, niobium-oxide, tantalum, tantalum-polymer |
| converter (2) | adc, dac |
| counter (6) | divn, johnson, rate, ripple, seg7, sync |
| decoder (3) | binary, priority, seg7 |
| diode (6) | bridge, fast, rectifier, schottky, switching, zener |
| latch (5) | addressable, bcd7seg, bus-mux, sr, transparent |
| mux (3) | regfile, select, stored |
| pio (2) | pia6521, via6522 |
| register (1) | parallel |
| serial (11) | acia6551, ay1013, com78c80x, hs82c52, i8251, i82510, max3100, max310x, ns16550, sc18is604, usbuart |
| shift (6) | bus-register, fifo, ls673, serial-in, shift, tap64 |

Each blurb is checked against the kind's section of
`docs/parts-format.md` (the model's own description there) while it is
written.

## Phases

> **NOTE — keep the checkboxes current as you go.** **An unticked box means NOT DONE.**

### Phase 1 — Kinds without variants (dff, jkff, led, switch, oscillator, header, 7seg)

- [x] The seven guide files per § 3, with intros, search terms and field
      guides (label, help, whereToFind, alsoCalled, example) for every key.
      (`DffGuide`, `JkffGuide`, `OscillatorGuide`, `SevenSegGuide` in files
      of their own; `LedGuide`, `SwitchGuide`, `HeaderGuide` together in
      `LedGuide.swift`; shared pieces in `GuideParts.swift`; § Corrections 1.)
- [x] `KindGuideTests` (7) for these seven
      (`testRequiredFieldsOfCuratedKindsSayWhereToFindThem`, over all 13;
      also `testEveryTopLevelKeyOfACuratedKindHasAGuide`).

Acceptance: seven guides pass the drift test.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p46-dd test -only-testing:schematicalTests/KindGuideTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 6 min).
Commit: 1c0608df (one commit for the three phases: the guide registry names all twelve)

### Phase 2 — Kinds with variants (buffer, latch, mux, decoder, diode)

- [x] The five guide files: the § 3 rule as a shared helper
      (`GuideLayout.byRule(schema:, variant:)`) plus each kind's field guides.
      (`GuideLayout.swift`; the five guides in `VariantKindGuides.swift`,
      § Corrections 1; `testLayoutByRule`.)
- [x] `KindGuideTests` (7) for these five. (`checked 101 required fields of
      13 curated kinds`; `xcodebuild … test
      -only-testing:schematicalTests/KindGuideTests` → `Executed 12 tests,
      with 0 failures`, `** TEST SUCCEEDED **`.)

Acceptance: five more guides pass, for every variant.
  Check: the same command (est. 6 min).
Commit: 1c0608df

### Phase 3 — Blurbs

- [x] `VariantBlurbs.swift` (60); `KindGuideTests` (6)
      (`testEveryModelHasABlurb`: 60 blurbs over 13 kinds, and no blurb for a
      model a kind lacks).
- [x] Runtime check (run skill): New Part… → Diode → six models with
      blurbs and examples → Zener → the Ratings step shows `vz`; New Part…
      → Latch → `transparent` → the steps are Supply voltage, Pins it
      uses, Active levels, Logic family and timing, Options. Pointer to
      (0, 0).
      (2026-11-16, this plan's Debug build: Diode → step 5 `What kind of
      diode is it?`, rows such as `switching, A small, fast signal diode,
      like the 1N4148., In the library: 1N4148, 1N4148 A0G`; Zener → steps
      `What kind of diode is it?; Diodes; Ratings`; Ratings shows `Forward
      current`, `Power rating`, `Reverse voltage`, `Zener voltage` (`Usually
      under: Electrical Characteristics (also: Characteristics)`, `Also
      called: VZ, Nominal Zener voltage`, key `vz`). Latch → `transparent`
      → steps `5. Which chip does yours behave like?; 6. Supply voltage; 7.
      Pins it uses; 8. Active levels; 9. Logic family and timing; 10.
      Options`; the model step's five rows each carry a blurb and library
      parts (`addressable, Eight single-bit latches, one picked by an
      address, like the 74HC259., In the library: …`). Pointer left at
      (0, 0).)

Acceptance: every model has a blurb; the variant kinds read as § 3 lays out.
  Check: the same test command (est. 6 min) and the walk (est. 10 min).
Commit: 9f0cc929 (code in Phase 1's commit)

## Validation Plan

- Tests: `KindGuideTests` (1)–(7) over all 13 curated kinds.
- Runtime proof: Phase 3's walk.
- Doc sync: none (content lives in code; N documents how to add a guide).
- Final gate: plan-46-N.

## Decisions

plan-46-A § Decisions (the 13 simple kinds; blurbs for every variant kind).
Author's call: the variant kinds use one mechanical layout rule so a new
key lands in a step without a decision.

## Corrections

1. **The guides are grouped in fewer files than § 1 names.** § 1 says one
   `<Kind>Guide.swift` per kind. The three one-screen kinds (led, switch,
   header) share `LedGuide.swift`, and the five kinds with models (buffer,
   latch, mux, decoder, diode) share `VariantKindGuides.swift`; each still
   has its own `<Kind>Guide` type, registered in `PartKinds.guides`, which
   is what the wizard and the tests go by.
2. **A Zener's Ratings step has four keys, not two.** The diode schema
   marks only `vz` and `pd` as Zener-only (`PART_VARIANT_KEYS` in
   `scripts/part-editor-schema.py`); `io` and `vr` belong to every type, a
   Zener included, as far as the editor's schema goes (the backend refuses
   them on a Zener when given; the form leaves an empty optional key out).
   So the step shows `io, pd, vr, vz` for a Zener and `io, vr` for the
   others; `testLayoutByRule` asserts both. The walk's "Zener → the Ratings
   step shows `vz`" holds.
3. **Two guide paths the first draft named do not exist**, and test (2)
   caught them: `device.units.invert` on buffer (the editor's schema has
   `invert` at the top level and inside `ab`/`ba`, not on a unit) and a
   top-level `device.latch` on decoder (its `latch` is a unit key). Both
   were removed.
4. **Nested control keys are explained once.** `pin`, `active` and `edge`
   (the keys of a control or a clock object, which the five model kinds
   use in dozens of places) get shared guides (`SharedGuides.controls`),
   so every required nested key of the curated kinds has a guide: test (7)
   walks nested required keys too, 101 in all.
5. **In the rule, a key the Pins rule fits is not offered to a later
   rule.** `polarity` (a pin), `segments`, `rbi` and `rbo` (pins) are
   listed in § 3's rules 3 and 4 but land in "Pins it uses" by rule 2,
   since the first rule that fits takes the key; `segmentsActive` lands in
   "Active levels". The Display step then holds `glyphs`, `blank` and
   `lampTest`. Test (1) proves every key of every model is in one step.

## Summary

Content work under a drift test; the risk is wrong advice, limited by
checking each blurb and field guide against the kind's section of
`docs/parts-format.md`.
