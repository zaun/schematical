# plan-20-F: Dual-supply parts and the `translator` model

Last updated: 2026-11-07
Effort: large (3h–1d)
Depends on: plan-20-E

F lets a part have two supply domains (VCCA side, VCCB side) — in the part
format, the `Part` struct, the voltage check and the listing — and adds the
`buffer` model `translator` for the level-translating transceivers and
fixed-direction translators.

Outcome of F: a dual-supply fixture at VCCA = 1.8 V, VCCB = 5 V reports no
voltage problem when its A pins meet 1.8 V logic and its B pins 5 V logic,
and reports an input-over problem when a 5 V output drives an A pin;
`buffer_test` passes the translator cases; every existing `voltage_test`
case, golden line and trace is unchanged.

References: plan-20-A §3.3; `backend/include/parts.h:67` (`PartVoltage`),
`:132` (`Part.vcc`); `backend/src/voltage.c` (header comment rules 1–3;
`:254-268` the per-part power level); `backend/tests/voltage_test.c`,
`tests/support/voltage_fixture.h` (fixture pattern from plan-10-A);
`backend/tests/support/listing.c:203` (the `|vcc=` field);
`planning/completed/plan-07-B-voltage-check.md`.

## Prerequisites

plan-20-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-20-E complete | `ls planning/completed/plan-20-E-*.md` → exists | MET (2026-11-07: `planning/completed/plan-20-E-registered.md`) |

## 1. Goal

- `voltage_test` new cases PASS (§3); all existing cases still PASS.
- `buffer_test` translator cases PASS.
- `git diff --stat backend/tests/golden` → empty (no dual-supply part exists yet; H adds them).

### Non-goals

- Every existing part has `domain_count == 0` and its voltage result is
  computed by the existing code path unchanged.
- No supply-off behavior (the engine has no supply level per part: a
  translator's outputs do not float when one side is unpowered).

## 2. Design

### 2.1 Format and loader

- Device key `"domains"` (any kind may use it; only `buffer` does in plan-20):
  exactly 2 objects `{"power": <pin name>, "pins": [<pin names>]}`; the
  power pin must have role `PIN_POWER`; every other non-ground, non-NC pin of
  every pinout is in exactly one domain (load error otherwise:
  `"domains": pin "1B3" is in no domain`).
- Timing entries and parts accept `vccB`, `inputMaxB` (domain B), required
  when `domains` is present, rejected otherwise. Domain A uses the existing
  `vcc`/`inputMax`.
- `parts.h`: `typedef struct { size_t power_pin; PartVoltage vcc; const
  size_t *pins; size_t pin_count; } PartSupplyDomain;` and `Part` gains
  `const PartSupplyDomain *domains; size_t domain_count;` (appended after
  `pio`, before `internal`). `Part.vcc` = domain A's range when domains exist.

### 2.2 Voltage check

In `voltage.c`, the per-part step: when `domain_count > 0`, compute one
power level per domain from its own power pin, and judge each connection's
pin against its domain's level, range and input max; outputs in a domain
drive that domain's level. The single-domain path (every existing part) is
the unchanged code. The header comment gains rule 3a for domains.

### 2.3 Listing

`listing.c`: after the `|vcc=` field, a part with domains appends
`|vccb=<min>-<max>,in=<inmax>`. Lines of parts without domains are
byte-identical.

### 2.4 Model `translator`

Keys of `transceiver` (DIR + OE, per unit or per bit: 1T45 has DIR only, no
OE; 2T45 one DIR; 8T245 DIR + OE; 16T245 two units) or of `tristate` for
fixed-direction parts (TXU/TXV with OE, CD4504 with no enable, CD4010
buffer/converter), plus `domains`. Logic is the transceiver/tristate logic;
the domain only affects the voltage check (§2.2) and which supply's timing
applies (one pair per family, plan-20-A §4).

## 3. Tests

| File | Case |
|---|---|
| `voltage_test` | fixture `VFX-T45` (`buffer`, translator, SOT23-6: 1 VCCA, 2 GND, 3 A, 4 B, 5 DIR, 6 VCCB; domains A = {A, DIR}, B = {B}; `vcc` [1.65, 5.5], `vccB` [1.65, 5.5]): VCCA 1.8 V, VCCB 5 V, A driven by a 1.8 V part, B into a 5 V HC input → no problem; B driven by a 5 V output with DIR low (B→A) and A into a 1.8 V-max input → `INPUT_OVER` on that input; a 5 V output into pin A directly → `INPUT_OVER` on the translator's A pin (domain A input max) |
| `voltage_test` | every existing case unchanged (the file's existing PASS lines) |
| `library_check` | `domains` with a pin in no domain; `vccB` without `domains`; `domains` with a non-power `power` pin — each rejected with its message |
| `buffer_test` | 1T45 DIR both ways; 8T245 OE; TXU0104 fixed direction with OE |

## Phases

### Phase 1 — Format, `Part`, listing

- [x] §2.1 loader (buffer kind: `domains` device key, `vccB`/`inputMaxB` timing and part keys, per-pinout domain binding) + `parts.h` (`PartSupplyDomain`, `Part.domains`/`domain_count`, Corrections 1–2); §2.3 listing (`|vccb=`); loader cases in `buffer_test` (pin in no domain, pin in two domains, non-power `power`, `vccB` on a part / in timing without `domains`, `domains` on a model without them, translator without `domains`, timing without `vccB`, `inputMaxB` below `vccB` max — all `PASS`); docs `parts-format.md` § Two supply domains, `backend.md` (Voltages, source map). `git diff --stat backend/tests/golden` → empty.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 + 1 of T0 + 1; golden diff empty (seconds).
Commit: b0dbb70 (Phases 1 and 2 together)

### Phase 2 — Voltage check and `translator`

- [x] §2.2 in `voltage.c` (step 3a and the per-domain checks; header rule 3a); `voltage_test` fixture `VFX-T45` (A side 1.65–3.6 V, B side 1.65–5.5 V, Corrections 3) and 6 cases: `PASS: translator 1.8 V / 5 V: 1.8 V logic on A, 5 V logic on B: no problem`, `… B to A: 5 V on B, A into a 1.8 V-side input: no problem`, `… a 5 V output into pin A is INPUT_OVER (A side input max 3.6 V)`, `… 5 V into pin B at VCCB 3.3 V is within the B side's 5.5 V`, `… 5 V on VCCA is out of the A side's range; 5 V on VCCB is not`, `… 12 V on VCCB is out of the B side's 1.65-5.5 V range`; every existing case still `PASS` (`voltage_test: all passed`, 36 PASS lines). `translator` model (transceiver or tristate logic by the units' keys) and `buffer_test` cases: 1T45 domains bound and DIR both ways, 8T245 OE, TXU0104 fixed direction with OE. Dry run: the 27 real translator specs (232 parts) load. Docs § Model `translator`, `backend.md` § Voltages.

Acceptance: `ctest --test-dir /tmp/logic-p20` → T0 + 1 of T0 + 1; `voltage_test` prints the new PASS lines; golden diff empty (seconds).
Commit: b0dbb70

## Validation Plan

F's gate: Phase 2 (`voltage_test` covers every voltage rule; goldens prove
no existing line moved). Landing tier at the end of I.

## Decisions

- Domains are a general part-format feature, not buffer-only, so a later
  dual-supply part of any kind reuses them (author).
- No supply-off modeling (non-goal; the engine has no per-part supply level).

## Corrections

1. **A domain has a list of power pins**, not one `power_pin`: the '4245 and TXV0108 have two pins named VCCB (plan-20-A reports G06, G03). `PartSupplyDomain` is `{power_pins, power_pin_count, vcc, pins, pin_count}`; `"power"` names every pin of that name.
2. **`domains` is parsed by the buffer kind**, not the library core: the domains are bound per pinout (pin indices differ between pinouts) and the B range comes from the kind's timing entries, so the kind's `bind_pinout`/`load_part` is the place. The `Part` fields, the voltage check and the listing are kind-independent, so another kind can fill them the same way (the Decision's intent).
3. **§3's second voltage case was mis-stated**: with DIR low a 5 V output on B and A into a 1.8 V-max input is *not* a problem — the translator drives A at its A-side level, which is the point of the part. The fixture's A side is 1.65–3.6 V so that "a 5 V output into pin A directly → `INPUT_OVER`" holds (with both sides 1.65–5.5 V, 5 V is within A's input max and the case cannot fail).

## Summary

F carries plan-20's widest blast radius — the voltage check every circuit
runs — behind a path split that leaves every existing part on the old code.
