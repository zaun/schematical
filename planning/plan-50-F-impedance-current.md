# plan-50-F: Impedance and Current Estimates, and the Width Calculator

Last updated: 2026-11-16
Effort: medium (1h–2h)
Depends on: plan-50-E (complete)

With the stackup in the board (A) and editable (B), the app can finally say what a trace is: the characteristic
impedance it has on its layer, the differential impedance when its net is in a pair, and how much current it
carries for a given temperature rise. This sub-plan adds those estimates as readouts in the Trace sidebar and on
each net class, and a **Calculate Width…** sheet that solves the other way: the width that gives a target
impedance, differential impedance or current.

The single outcome: selecting a trace shows its impedance and current capacity from the board's stackup; the
numbers match `planning/plan-50-data/reference_formulas.py` (§ 3) to 0.01 Ω / 0.0001 A; and the sheet turns a
target into a width, applies it as an undoable edit to the trace or the net class, and says plainly when the
width needed is outside what a trace can be (5–100 mil).

References:

- plan-50-A (prerequisites, the model), plan-50-B (the Stackup section the temperature rise lives in)
- `planning/plan-50-data/README.md` (formula accuracy against a field solver, and fixtures F1–F16) and
  `reference_formulas.py`, `fd_solver.py`
- `schematical/schematical/Canvas/PCBSidebarView.swift` (`TracePanel`, `NetAndClassRows`),
  `Canvas/NetsSidebar.swift` (the class row, `setNetClassWidth`), `Canvas/NetRules.swift` (`NetGroup`)

## Prerequisites

The shared gate is in plan-50-A. This sub-plan adds:

| Must be true | Command | Status |
|---|---|---|
| plan-50-E is complete | `ls planning/completed \| grep -c '^plan-50-E'` → 1 | MET (2026-11-16: 1) |

Everything below is written against the world where this holds.

> **NOTE — the Status column is a snapshot; the Command column is the truth.** Re-run every command and update
> every status before you continue, and again before you decide to stop. If you stop, report the current status
> of *all* prerequisites.

## 1. Goal

- `StackupAnalysis` (new, `Canvas/StackupAnalysis.swift`) gives, for a board and a copper layer: the trace
  geometry it sits in (§ 3), single-ended impedance, differential impedance for a gap, current capacity for a
  width, and the width for a target of each kind.
- The Trace sidebar shows, under Width and Layer, an **Estimates** section: **Impedance** (Ω), **Differential**
  (Ω, only when the trace's net is in a differential-pair group, at that group's gap) and **Current** (A, at the
  stackup's temperature rise), each with a help text; an unavailable value shows "—" with the reason as help.
- Each net class row in the Nets sidebar shows the same Impedance and Current for the class width on its first
  layer.
- A **Calculate Width…** button on the Trace panel and on each net class opens a sheet with Goal (Impedance,
  Differential impedance, Current), target, layer, gap (differential) or temperature rise (current), the result in
  mil and in board units, the impedance or current actually reached at the rounded width, and **Apply** and
  **Cancel**. Apply sets the trace's width (`setTraceWidth`) or the class's (`setNetClassWidth`), one undo step.

### Non-goals (explicit constraints)

- The 100 mil / 5 mil width limits (`BoardTrace.widthRange`) do not change. A width outside them is reported, and
  Apply is disabled.
- The estimates never change a trace, flag a DRC problem or block anything.
- No loss, no frequency dependence, no solder-mask effect, no via or connector effect, no coplanar waveguide, no
  broadside-coupled pairs: the formulas are the ones in § 3.
- Net classes and groups are not changed by the sheet (a differential target solves a width at the gap; the
  group's own gap stays).
- Settings, the file format and every export are untouched. The temperature rise is plan-50-A's field.

## 2. Current State

- The Trace panel (`Canvas/PCBSidebarView.swift:809`): Width (Int mil, `BoardTrace.widthRange` 5…100, stepper),
  Layer, Lock, Net and Class rows (`NetAndClassRows`, `:888`, which gets a net with
  `document.net(ofTrace:in:)`), Connects.
- A class's Width row is at `Canvas/NetsSidebar.swift:238`–`:243` (`document.setNetClassWidth(id, _)`, an Int mil
  clamped to the same range).
- `NetGroup` (`Canvas/NetRules.swift:54`): `mode` (`.diffPair`), `members: [PlaneNet]`, `gap` (mm, edge to edge;
  default 0.2). `BoardConnectivity.planeNet(ofNet:)` gives a net's `PlaneNet`.
- `BoardSettings.style(of:).type == .plane` says a layer is a plane; `traceLayers` are the non-plane layers
  (`Canvas/BoardSettings.swift:271`). `BoardStackup` (plan-50-A) gives copper thickness per layer, the dielectric
  between each two, `finish` and `temperatureRise`.

### Verified properties

- **The formulas were checked against a field solver before this plan was written**
  (`planning/plan-50-data/README.md`, table and `fd_solver.py`; the solver matches scikit-rf's microstrip model
  to 2.2% on 5 cases, and its odd-mode and stripline cases are its own Laplace solution on a 0.0063–0.0132 mm
  grid): Hammerstad–Jensen microstrip within ±1.9% (one −7.2% at w/h 0.71), differential microstrip within
  −3.5%, centred Cohn stripline within +5.9% (thick copper close to a plane) and ±2.8% otherwise, offset stripline
  within +9.7% / ±6.3%, differential stripline within ±3.2%. So the estimates are labelled "about ±10%".
- **A 2-layer 1.6 mm board cannot reach 50 Ω within the width limit**: F11, 110.12 mil needed; the limit is 100.
  The sheet's "outside the limit" message is therefore reachable on the standard board, not a corner case.

## 3. Design Overview

**Geometry of a layer** (`StackupAnalysis.geometry(of:)`; `layers = board.copperLayers`, `dielectrics[k]` lies
between `layers[k]` and `layers[k+1]`; a layer is a plane when `board.style(of:).type == .plane`):

- A 1-layer board: none ("A 1-layer board has no reference plane").
- A plane layer: none ("A plane layer carries no traces").
- **Top or Bottom** (outer): walk inward from the layer to the nearest plane layer `p`. Between them: the
  dielectrics, and the copper of any non-plane layers in between, add up to the height `h`; `εr` is the
  thickness-weighted mean of those dielectrics. Result `microstrip(h, εr)`. On a 2-layer board the other outer
  layer is the reference, assumed solid ("assumes a solid ground on Bottom"), `assumed: true`. On a board of 4
  layers or more with no plane before the far outer layer: none ("No plane layer under Top").
- **An inner non-plane layer**: the nearest plane above and the nearest plane below, each as above, giving
  `stripline(hAbove, hBelow, εr)` with `εr` the thickness-weighted mean over both sides. Without a plane on both
  sides: none ("Needs a plane layer on both sides").

`t` is the layer's copper thickness. Every `h` is the dielectric height between the copper surfaces.

**The formulas** (all in `reference_formulas.py`; widths and heights in mm; `t` copper thickness):

- Microstrip Z0 — Hammerstad–Jensen with the thickness correction: `u = w/h`, `t̃ = t/h`,
  `Δu₁ = (t̃/π)·ln(1 + 4e·tanh²(√(6.517u))/t̃)`, `Δu_r = Δu₁·(1 + sech(√(εr−1)))/2`, `u_r = u + Δu_r`;
  `Z_l(u) = (η₀/2π)·ln(F/u + √(1 + (2/u)²))`, `F = 6 + (2π−6)·exp(−(30.666/u)^0.7528)`, `η₀ = 376.730313668`;
  `a(u) = 1 + ln((u⁴ + (u/52)²)/(u⁴ + 0.432))/49 + ln(1 + (u/18.1)³)/18.7`,
  `b = 0.564((εr−0.9)/(εr+3))^0.053`, `ε_eff(u) = (εr+1)/2 + (εr−1)/2·(1 + 10/u)^(−a·b)`;
  `Z0 = Z_l(u_r)/√ε_eff(u_r)`.
- Differential microstrip: `Zdiff = 2·Z0·(1 − 0.48·exp(−0.96·s/h))`, `s` the edge-to-edge gap.
- Centred stripline, plane spacing `b`: `w_e = w + ½(t/π)(1 + ln(4πw/t))`, `k = tanh(π·w_e/2b)`,
  `Z = (30π/√εr)·K(√(1−k²))/K(k)` (`K` the complete elliptic integral of the first kind, parameter `m = k²`
  in the usual notation; compute it with the arithmetic–geometric mean).
- Offset stripline: `Z = 2·Za·Zb/(Za + Zb)` where `Zx` is the centred value with `b = 2·hx + t` for
  `hx = hAbove, hBelow`.
- Differential stripline: `Zodd` is the centred odd-mode value with `ko = tanh(π·w_e/2b)/tanh(π(w+s)/2b)` in place
  of `k`; `Zdiff = 2·Zodd`; offset: the same harmonic combination of the two centred `2·Zodd`.
- Current (IPC-2221): `I = k·ΔT^0.44·A^0.725`, `A = width_mil × thickness_mil` (`thickness_mil = t/0.0254`),
  `k = 0.048` on Top or Bottom and `0.024` on an inner layer; its inverse is the width for a current.

**Solving for width.** Impedance falls as width rises (single-ended, and differential at a fixed gap), so
`width(forImpedance:)` is a bisection on width in mil over `1…1000` to 0.001 mil: if the impedance at 1 mil is
below the target the result is `.tooHigh` ("no width gives that high an impedance"), if at 1000 mil it is above it
`.tooLow`; otherwise `.mil(x)`. The sheet rounds to a whole mil (traces are `Int` mil), reports the impedance at
the rounded width, and disables Apply with a message when the rounded width is outside 5…100 ("needs 110 mil; the
limit is 100").

**Where each piece is.** `Canvas/StackupAnalysis.swift`: the geometry, formulas and solvers, as `nonisolated`
pure functions. `Canvas/WidthCalculator.swift`: `WidthGoal`, `WidthCalculation` (inputs → `Result`: exact mil,
rounded mil, achieved value, message, `canApply`) and `WidthCalculatorSheet: View`. `PCBSidebarView.swift` and
`NetsSidebar.swift`: the readouts and buttons.

**Risk.** Correctness of the numbers, which is why every formula has a recorded expected value (fixtures F1–F16)
and the geometry rule has its own cases. This is a new feature with no existing behaviour to preserve, so the
gate is tests against the reference values, not byte-identity.

**Rejected.** *IPC-2141's microstrip and stripline formulas*: off by up to 11–17% against the field solver
(README). *A broadside / coplanar option*: no way to describe them on this board. *Auto-warning when a width
misses 50 Ω*: the owner asked for readouts and a calculator, not checks.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work. `- [~]` for
> partial, with one line on what remains. A moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill the phase's
> `Commit:` line the moment it lands. An unticked box means NOT DONE.

### Phase 1 — The calculators

- [x] New `Canvas/StackupAnalysis.swift` with the geometry rule, the formulas, `currentCapacity`, the inverse
      current width and the bisection solvers of § 3, `Reason` strings for each "none".
- [x] Tests: new `schematicalTests/StackupAnalysisTests.swift`, tolerance 0.01 Ω / 0.0001 A / 0.01 mil: F1 51.711,
      F2 47.958, F3 46.455, F4 115.783 (the pure functions, given the numbers), F5 84.142, F6 51.723, F7 91.547;
      F8a 0.8855, F8b 0.2418; F9 11.826, F10 14.650, F11 110.12, F16 70.841; and on boards built with the
      standard stackups: the 2-layer Top is `microstrip(1.51, 4.5)` with `assumed` true and F2 follows at 3.0 mm;
      the 4-layer Top is `microstrip(0.2104, 4.4)` and F1 follows at 0.35 mm, Bottom `microstrip(0.2104, 4.4)`;
      the 6-layer Top is `microstrip(0.0994, 4.1)` (F3, F14 at 50 Ω, 6.881 mil); the 4-layer Top's 100 Ω
      differential width at 0.15 mm is F15 8.418 mil; the 6-layer with Layer 3 set to a Wire layer
      (`setLayerType(.inner(3), .wire)`) gives Layer 3 `stripline(0.55, 0.1164, 4.5231)` and F12 53.659 at 0.15 mm,
      F13 92.364 differential at w 0.12 / s 0.12; Layer 3 with Layer 4 also a Wire layer takes its lower plane from Layer 5 (`hBelow` = 0.1164 + 0.0152 + 0.55 = 0.6816, `εr` the thickness-weighted mean of 4.16 and 4.6), and with Layers 4 and 5 both Wire layers has no geometry ("Needs a plane layer on both sides" reason text); a 1-layer board and a plane layer have none; a 4-layer
      board with both inner layers Wire has no geometry for Top; an unreachable target gives `.tooLow` (1 Ω on
      the 4-layer Top) and `.tooHigh` (10 000 Ω).

Acceptance: every number equals the reference to the tolerance.
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/StackupAnalysisTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 5 min). Observed: `Executed 10 tests, with 0 failures`.
  And: `$VENV/bin/python planning/plan-50-data/reference_formulas.py` prints the same numbers as the table in `README.md` for F1–F11 (est. 1 min). Observed: F1 51.711, F2 47.958, F3 46.455, F4 115.783, F5 84.142, F6 51.723, F7 91.547, F8 0.8855 / 0.2418, F9 11.826, F10 14.650, F11 110.12: the numbers the Swift tests assert.
Commit: THIS

### Phase 2 — The readouts

- [x] `Canvas/PCBSidebarView.swift` `TracePanel`: an **Estimates** section (Impedance, Differential when
      `document.board.netGroups` has a `.diffPair` whose `members` contains the trace's net's `PlaneNet`, using
      that group's `gap`; Current), numbers formatted to 1 decimal Ω / 2 decimals A, "—" and a help with the reason
      when none; a **Calculate Width…** button (opens the sheet of Phase 3, wired there).
- [x] `Canvas/NetsSidebar.swift`: in each class's `DisclosureGroup`, an Impedance and Current row for the class width on
      the class's first layer (its layer rule's first allowed trace layer, else the board's first trace layer) and
      a **Calculate Width…** button.
- [x] Tests: in `StackupAnalysisTests`: the readout strings for the standard 4-layer Top at 14 mil ("…Ω", "…A")
      from a pure `EstimateText.rows(…)` helper both views call; a trace whose net is in a pair group at gap 0.15
      gets the Differential row with that gap and one not in a group does not; a 1-layer board shows "—" with the
      1-layer reason; the views build for 1, 2, 4 and 6 layers (`ImageRenderer(content: …).cgImage != nil`).

Acceptance: the readouts show the Phase 1 numbers for the selected trace and class.
  Check: `xcodebuild … test -only-testing:schematicalTests/StackupAnalysisTests -only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/NetRulesTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min). Observed with Phase 3's suites in one run (`StackupAnalysisTests`, `NetClassTests`, `NetRulesTests`, `TraceToolTests`): `Executed 66 tests, with 0 failures`, `** TEST SUCCEEDED **`.
Commit: THIS

### Phase 3 — The Calculate Width sheet

- [x] New `Canvas/WidthCalculator.swift`: `WidthGoal` (`.impedance(Double)`, `.differential(Double, gapMM: Double)`,
      `.current(Double, rise: Double)`), `WidthCalculation.result(goal:layer:board:)` (§ 3, the rounding and
      `canApply` rule), and `WidthCalculatorSheet`: Goal picker, target field, Layer menu (the board's trace layers;
      initial: the trace's layer / the class's first layer), gap field (differential; initial the trace's group gap
      or `NetGroup.defaultGap`), rise field (current; initial `stackup.temperatureRise`), the result lines
      (exact width in mil and in board units, rounded mil, the value reached, and the message when it can't
      apply), **Apply** (disabled unless `canApply`) and **Cancel**.
- [x] `PCBSidebarView.swift` / `NetsSidebar.swift`: the buttons present the sheet; Apply on the Trace panel calls
      `setTraceWidth(trace.id, mil)`, on a class `setNetClassWidth(id, mil)`; the sheet closes.
- [x] Tests, in `StackupAnalysisTests`: `WidthCalculation` for 50 Ω on the 4-layer Top gives exact 14.650 mil,
      rounded 15, achieved ≈ F10's neighbour (impedance at 15 mil from the reference formula), `canApply`; on the
      2-layer Top 50 Ω gives 110.12 mil, rounded 110, `canApply` false with a message naming 110 and 100; a
      current of 1 A on the 4-layer Top at 10 °C gives 11.826 → 12 mil; a trace goal sets the trace's width in one
      undo step named "Change Trace Width" (`doc.undoName(on: .pcb)`) and a class goal sets the class width;
      Cancel changes nothing (`undoName` unchanged); the sheet builds.

Acceptance: a target becomes an applied width, or an explained refusal.
  Check: `xcodebuild … test -only-testing:schematicalTests/StackupAnalysisTests -only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/NetClassTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 7 min).
Commit: THIS

## Validation Plan

- Tests: `StackupAnalysisTests` (new), plus the trace, class and group suites named in the phases.
- Coverage check: each fixture test fails if its formula's constant is perturbed (run once with `0.48` → `0.5`
  in the differential microstrip, F4 must fail, then revert).
- Runtime proof: Phase 1's reference-script run (the same numbers from an independent implementation); and the
  accuracy table in `README.md` is the evidence the formulas are close to a field solver.
- Doc sync: `docs/file-format.md` is unchanged (no file change); no doc describes the sidebar (`grep -n "Trace panel" docs/*.md` → no output). `docs/export.md` unchanged.
- Final gate (once, after Phase 3): `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath $DD test -only-testing:schematicalTests/StackupAnalysisTests -only-testing:schematicalTests/BoardStackupTests -only-testing:schematicalTests/StackupSectionTests -only-testing:schematicalTests/TraceToolTests -only-testing:schematicalTests/NetClassTests -only-testing:schematicalTests/NetRulesTests -only-testing:schematicalTests/PCBUndoTests CODE_SIGNING_ALLOWED=NO` → `** TEST SUCCEEDED **` (est. 9 min).

## Decisions

- Readouts plus a width calculator sheet (a button opening a dialog that solves for the width), not readouts only — decided by the owner, 2026-11-16. The sheet applies the solved width to the trace or class. (§ 1, Phase 3)
- Guidance is shown in the Trace sidebar and on each net class; the temperature rise is edited in the Stackup section — decided by the owner, 2026-11-16 (the options given).
- Closed-form formulas, labelled ±10%, not a field solver — decided by the author: they are within 2–10% of a solver in the ranges a PCB uses (README) and run instantly.
- A width the sheet cannot apply is reported, not clamped — decided by the author: a silently wrong width is worse than a refusal.

## Corrections

- `reach(from:step:)` first weighted Er by the whole height (dielectric plus the copper of non-plane layers passed). The plan says "thickness-weighted mean of those dielectrics", and the test for Layer 3 with Layer 4 a wire layer showed the difference (4.5016 vs 4.5579); fixed to weight by the dielectrics' own thickness.
- The third-phase check "`TraceToolTests`" and the class tests are named in the plan; `NetRulesTests` was run too because it builds the same net class rows.
- The sub-plan's final gate was run as part of plan-50's landing tier (below), not on its own.

## Summary

The risk is numerical, and each formula is pinned by a fixture measured before this plan was written. Untouched:
the file format, the exports, the DRC, the width limits, and any existing trace or class until Apply is pressed.
