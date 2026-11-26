# plan-71-C: Line transceivers — pinouts and facts per datasheet, RS-485 / RS-422

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-B

C fills the RS-485 rows of `specs.tsv` with B's tools and B's § 3 rules,
plus the RS-485-only keys below. It covers ADI (with Maxim), TI and
MaxLinear: at most 511 candidates (A § 2: 200 + 200 + 111).

References:

- plan-71-B (§ 3 what a row records, § 4 the tools); plan-71-G § 3.1, § 3.3.
- `backend/src/models/serial_max3100.c:33-41`: the MAX3140 transceiver
  reading, which G's RS-485 receiver matches (a `failSafe` part reads an
  idle bus as 1).

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-B complete | `ls planning/completed/plan-71-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |
| The tools exist | `python3 -I planning/plan-71-data/census/spec-check.py --self-test` → `0 missed` | NOT MET (written by B) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `specs.tsv` row with protocol `rs485` has `scheme`, `pinout`,
  `label` and `facts` filled (or `pinout` `-`).
- `python3 -I planning/plan-71-data/census/spec-check.py rs485` prints
  `<n> rows, 0 errors`.

### Non-goals (explicit constraints)

- As plan-71-B § 1. In addition, slew-rate pins (`SRL`, `SLR`, `RS`), the
  half/full-duplex select pin (`H/F`) and polarity pins (`TXP`, `RXP`) are
  `input` pins listed under `analog`. The model does not use them (G § 3.5).

## 2. Current State

The RS-485 rows are B's skeleton rows with protocol `rs485`. They cover
RS-485 half-duplex transceivers (`MAX485`, `SP3485`, `THVD1450`),
full-duplex transceivers (`MAX491`, `SN65HVD33`), RS-422 quad drivers and
receivers (`AM26LS31`, `AM26LS32`, `AM26C31`, `AM26C32`) and isolated
transceivers (`ADM2587E`, `ADM2682E`, `ISO3082`).

## 3. RS-485 rules

- Pin roles:
  - half duplex: `A`, `B` are `inout`;
  - full duplex: driver `Y`, `Z` are `output` and receiver `A`, `B` are
    `input`;
  - `DI`, `DE`, `RE` / `RE#` are `input`, and `RO` is `output`;
  - a quad driver's `1A` … `4A` inputs are `input` and its `1Y`/`1Z` …
    outputs `output`;
  - the driver enable pair `G` / `G#` are `input`.
- `drivers`: `{"input": "DI", "outputs": ["A", "B"], "enable": {"pin": "DE", "active": 1}}`.
  A full-duplex part names `Y`, `Z`. A quad driver has four entries sharing
  one enable array (`G` active 1 and `G#` active 0 are *either*, not both).
  Such a part's enable is written as the datasheet's truth table resolved to
  the one control that the datasheet's application circuit ties, with the
  other pin listed under `analog`, and the `note` says so.
- `receivers`: `{"inputs": ["A", "B"], "output": "RO", "enable": {"pin": "RE", "active": 0}}`.
  `"disabled"` is `"z"` (every RS-485 part's RO is 3-state).
- `failSafe` (required on every RS-485 row): `true` when the datasheet
  promises RO high for an open, shorted or idle bus ("true fail-safe",
  "fail-safe receiver", a −200…−50 mV threshold). `false` when it promises
  only open-input fail-safe or nothing (the classic ±200 mV threshold, as on
  `MAX485`, `SN75176B`, `AM26LS32`): an idle bus then reads X.
- Isolated parts: `domains` = logic side (`VCC1` / `VDD1` / `VIO`, with
  `DI`, `DE`, `RE`, `RO`) then bus side (`VCC2` / `VISOOUT` / `VISOIN`, with
  `A`, `B`, `Y`, `Z`). The isolated supply's own pins (`VISOOUT`, `VSEL`) go
  under `analog`. The facts get `"isolated": true`.
- Delays: driver tPLH/tPHL max and receiver tPLH/tPHL max (the larger of
  each pair) at the 5 V row, or at the 3.3 V row for a 3.3 V-only part.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — ADI rows

- [ ] Fill every `rs485` ADI row (the 505 and 175 labels) per § 3.

Acceptance: the ADI rows check clean.
  Check: `python3 -I planning/plan-71-data/census/spec-check.py rs485 --company adi` → `<n> rows, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — TI and MaxLinear rows

- [ ] Fill every `rs485` TI and MaxLinear row per § 3.

Acceptance: all RS-485 rows check clean.
  Check: `python3 -I planning/plan-71-data/census/spec-check.py rs485` → `<n> rows, 0 errors`; `awk -F'\t' '$1=="rs485" && $10==""' planning/plan-71-data/specs.tsv | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

## Corrections

(none yet)
