# plan-71-D: Line transceivers — RS-232 specs, the final pick, skipped parts, obsolete PDFs

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-C

D fills the RS-232 rows of `specs.tsv` (ADI with Maxim, TI, Renesas: at most
501 candidates, A § 2). It then fixes the final pick for all four protocols,
writes the skipped parts and the all-obsolete PDFs to their two files, and
writes `footprints-needed.txt` for E and F.

References:

- plan-71-B (§ 3, § 4 the tools), plan-71-A (§ 4 the verdict rules).
- `planning/plan-48-data/census/final.py`, `footprint-lines.py`,
  `fp-work.py` (the scripts D adapts).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-C complete | `ls planning/completed/plan-71-C-*.md` → exists, or every C box ticked | NOT MET (2026-11-25: C not started) |
| CAN, LIN and RS-485 rows check clean | `python3 -I planning/plan-71-data/census/spec-check.py can lin rs485` → `0 errors` | NOT MET (B, C) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `specs.tsv` row with protocol `rs232` is filled, and
  `spec-check.py can lin rs485 rs232` prints `<n> rows, 0 errors`.
- `candidates.tsv` `state` is `add` for exactly the rows with verdict `add`
  and a spec row with a pinout.
- `datasheets/skipped-parts.md` has a `## Line transceivers (plan-71)`
  section naming every `skip` and `out` row with its reason.
  `datasheets/obsolete-not-included.md` has one row per PDF whose listed
  parts are all obsolete, and no such PDF is left in
  `datasheets/drivers-receivers-transceivers/`.
- `footprints-needed.txt` lists one line per (package label, pin count,
  datasheet drawing) of the final `add` rows. Each line has its `add`-row
  count, the existing footprint of that family and pin count to check for
  reuse (or `-`), and its letter, `E` (leaded) or `F` (leadless).

### Non-goals (explicit constraints)

- As plan-71-B § 1.

## 2. Current State

The RS-232 rows cover charge-pump transceivers (`MAX232`, `MAX3232`,
`MAX3221`, `MAX3243`, `ICL3232`, `TRS3232`, `HIN232`, `ADM3202`) and
drivers or receivers only (`MAX3188`, `SN75C1406`, `GD75232`). Some have
±12 V supply pins and no charge pump (`SN75188`, `SN75189`). Some have
shutdown, enable, auto-shutdown and `INVALID` pins.

## 3. RS-232 rules

- Pin roles:
  - `T1IN` … `TnIN`, `EN`, `SHDN` / `SHDN#`, `FORCEON`, `FORCEOFF#` are
    `input`;
  - `T1OUT` … `TnOUT` are `output`;
  - `R1IN` … `RnIN` are `input`;
  - `R1OUT` … `RnOUT`, `R2OUTB` (always-active receiver outputs) and
    `INVALID#` are `output`;
  - `C1+`, `C1-`, `C2+`, `C2-`, `V+`, `V-` are `inout` and listed under
    `analog`;
  - `VCC` is `power`, and a ±12 V part's `VDD` / `VSS` are `power` and
    listed under `analog` (the app has no negative supply; plan-48-F's note).
- `drivers`: one entry per driver, `{"input": "T1IN", "outputs": ["T1OUT"], "enable": …}`.
  The enable is the shutdown pin (`SHDN#` active 1 = on; `FORCEOFF#` active
  1). An auto-shutdown part (`FORCEON` / `FORCEOFF#`) is written with
  `FORCEOFF#` as its driver enable, and `FORCEON` and `INVALID#` go under
  `analog` (G § 3.5: auto-shutdown is not modeled).
- `receivers`: `{"inputs": ["R1IN"], "output": "R1OUT", "enable": {"pin": "EN#", "active": 0}}`.
  `"disabled"` is `"z"`. An always-active output (`R2OUTB`) is its own
  receiver entry with no enable, on the same input.
- Delays: tPHL/tPLH max of driver and receiver at VCC 3.3 V or 5 V. When the
  PDF gives only a data rate, the driver delay is 1/(2 × the rate), and the
  `note` says so.

## 4. The final pick

- `final.py` sets `state` = `add` for rows with verdict `add` and a spec row
  whose `pinout` is not `-`. Rows whose package has no pinout become
  `skip: no pinout`, and a reserve row of the piece moves up (A Phases 2–4
  for that row, then B–D's fill for its spec row if it is new). This repeats
  until no piece changes.
- `skipped.py` writes the `datasheets/skipped-parts.md` section (part, maker,
  protocol, reason; sorted by protocol, maker, part).
- `obsolete.py` finds PDFs whose every ordering-table row is marked
  obsolete. It deletes them and writes their rows to
  `obsolete-not-included.md` (PDF name, Interface / Transceiver / protocol).
- `footprint-lines.py --add` writes `footprints-needed.txt`.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — RS-232 rows

- [ ] Fill every `rs232` row per § 3 (ADI, TI, Renesas).

Acceptance: all rows check clean.
  Check: `python3 -I planning/plan-71-data/census/spec-check.py can lin rs485 rs232` → `<n> rows, 0 errors`; `awk -F'\t' '$10==""' planning/plan-71-data/specs.tsv | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

### Phase 2 — The final pick and its records

- [ ] `final.py`, `skipped.py`, `obsolete.py` per § 4.
- [ ] `footprint-lines.py --add` → `footprints-needed.txt`.

Acceptance: every `add` row has a pinout and a package line, and every
non-`add` row is recorded.
  Check: `python3 -I planning/plan-71-data/census/final.py --check` → `<n> add rows, 0 without a pinout, 0 without a package line, 0 skip/out rows missing from skipped-parts.md`; `awk -F'\t' '!/^#/ && $8==0' planning/plan-71-data/footprints-needed.txt | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

## Corrections

(none yet)
