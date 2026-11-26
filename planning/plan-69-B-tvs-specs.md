# plan-69-B: TVS specs — ordering schemes, pinouts, the fact check, skipped and obsolete lists

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-69-A

B reads every (datasheet, package) line of `specs.tsv`: the page of the ordering
table, the pinout of each package, and the electrical-table line of each `add`
part, and checks DigiKey's facts against it. A row the datasheet does not bear out
becomes `skip`. B ends the pick: after B no row changes its verdict, and C, D, F and
G work from the `add` rows.

References:

- plan-69-A § 3, § 4 (verdict rules), `planning/plan-69-data/README.md` (columns).
- `planning/completed/plan-47-B-ifrb-verdicts-inductor-fuse.md` § 3.3–3.4 (facts from
  DigiKey's parameters, checked against the datasheet: the method B follows) and
  `planning/plan-40-data/census/scheme-check.py`, `pinout-check.py`.
- `planning/completed/plan-48-B-timer-opamp-specs.md` (the pinout columns,
  `final.py`, the skipped-parts section).
- plan-69-E § 3.1 (the kind's keys: what a pinout must name).

## Prerequisites

plan-69-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-69-A complete | `ls planning/completed/plan-69-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| Every picked row has a verdict | `awk -F'\t' 'NR>1 && $24=="candidate"' planning/plan-69-data/candidates.tsv \| wc -l` → 0 after A's refill (every picked row is `add`, `skip` or `out`) | NOT MET (2026-11-25: A not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `specs.tsv` row has `scheme`, `page`, `pinout`, `pins` and `pads` filled;
  `python3 -I planning/plan-69-data/census/tvs-pinout-check.py` → `<n> specs rows, 0
  errors`.
- Every `add` row's facts agree with its datasheet line:
  `python3 -I planning/plan-69-data/census/tvs-fact-check.py --facts` → `<n> rows: <n>
  ok, 0 unchecked`.
- `datasheets/skipped-parts.md` has a "TVS diodes (plan-69)" section listing each
  `skip` row; `obsolete-not-included.md` has a row for each PDF whose every row is
  `out` by status.
- `footprints-needed.txt` is rewritten from the final `add` rows
  (`tvs-footprint-lines.py --add`).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No new listing; no row comes back from `out`.

## 2. Current State

After A, `specs.tsv` has one row per (type, company, pdf, package,
supplier_package, uni, bi) with its B columns empty, and `candidates.tsv` carries
DigiKey's text for the facts (`vrwm` … `cj`, columns 14–19).

## 3. Design

### 3.1 Pinouts (the `pinout` column)

A `pinout` is a comma list `number=name:role` in the format E's loader takes
(plan-69-E § 3.1):

- **Unidirectional, one diode, two terminals**: `1=K:inout,2=A:inout` (pin 1 the
  cathode band, as `diode.json`'s `two-pin`). A three-lead package with one diode
  (SOT-23-3): the datasheet's numbers, the unused lead `nc`.
- **Bidirectional, one diode, two terminals**: `1=1:inout,2=2:inout` (no cathode).
- **Multi-channel Zener-type arrays** (`uni` or `bi` above 1, no rail pin): one pin
  per line named as the datasheet names it (`IO1`, `CH1`, `I/O 1`), the common
  terminal `GND` (role `inout`: a diode-type device lists it in its diodes), every
  other lead `nc`.
- **Steering arrays**: the lines as the datasheet names them (role `inout`), the
  rail pin (`VBUS`, `VCC`, `VP`, `V+`: role `inout`), the ground pins (`GND`, `VN`:
  role `ground`), an unnumbered exposed pad left out (the footprint's `centre`),
  `nc` leads `nc`.
- `pins`: the pinout's pin count; `pads`: the land pattern's pad count, which C and
  D check against the footprint.
- `elements`: the kind's structure in E's terms: `diodes=K>A` / `diodes=1~2`
  (anode>cathode, `~` bidirectional) for the diode type, `lines=IO1,IO2,IO3,IO4
  rail=VBUS` for a steering array.

`census/tvs-pinout-check.py` (from plan-40's `pinout-check.py`) checks: pin numbers
1…N in order; the roles of § 3.1; every `inout` pin named by `elements`; a
steering row has at least one `ground` pin; `uni` and `bi` agree with the count of
`>` and `~` entries (diode type) or the line count (steering).

### 3.2 Facts (the `facts` column and `tvs-fact-check.py`)

The part keys of plan-69-E § 3.1 come from the row's DigiKey parameters, checked
against the part's line of the datasheet's electrical table:

| Fact (part key) | DigiKey parameter | Read as | Check against the datasheet |
|---|---|---|---|
| `vrwm` (V) | Voltage - Reverse Standoff (Typ) (`761`) | the first number (`5V (Max)` → 5) | the number stands on the part's table line |
| `vbr` (V) | Voltage - Breakdown (Min) (`1731`) | the first number | on the part's line, or the line's min/max pair brackets it |
| `vc` (V) | Voltage - Clamping (Max) @ Ipp (`1732`) | the first number | on the part's line |
| `ipp` (A), `ippWave` | Current - Peak Pulse (10/1000µs) (`714`) | the number; the waveform in brackets (`8/20µs` → `8/20`, none → `10/1000`) | on the part's line, or in the series' table header with that waveform |
| `ppk` (W) | Power - Peak Pulse (`1733`) | the number (`kW` × 1000) | in the series' ratings table |
| `cj` (pF) | Capacitance @ Frequency (`566`) | the number before `pF` | on the part's line or in the series' characteristics table; left out when DigiKey gives `-` |

`vrwm` is required (a row DigiKey gives `-` is `skip`, `fact not given`); a value
not found as the table says → `skip`, `fact not in datasheet`. The part's table
line is the text line holding its base number (the number without the packing
suffix of A § 4 rule 2); `page` records the table's page for the device `note`
(F, G). `tvs-fact-check.py --sample` prints 20 random `ok` rows per (type, company)
for a check by hand.

### 3.3 Skipped and obsolete lists

`census/tvs-final.py` (from plan-48's `final.py`) writes the "TVS diodes
(plan-69)" section of `datasheets/skipped-parts.md` from `verdicts.tsv` (`mpn`,
company, reason), the obsolete-PDF rows, removes the PDFs no `add` row cites
(`tvs-place.py --prune`), and reruns `tvs-specs-skeleton.py` and
`tvs-footprint-lines.py --add` for the final rows; `--verify-skipped` checks every
`skip` row is listed.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools

- [ ] `census/tvs-pinout-check.py` and `census/tvs-fact-check.py` per § 3.1 and
      § 3.2; a unit check of the number readers on the planning sample row
      (`5V (Max)` → 5, `6A (8/20µs)` → 6 and `8/20`, `15pF @ 1MHz` → 15, `1.5kW` → 1500).

Acceptance: the readers parse DigiKey's forms.
  Check: `python3 -I planning/plan-69-data/census/tvs-fact-check.py --selftest` → `ok` (est. < 1 min).
Commit:

### Phase 2 — Zener-type specs (Littelfuse, Vishay, YAGEO)

- [ ] Fill `scheme`, `page`, `pinout`, `pins`, `pads`, `elements` on every Zener-type
      `specs.tsv` row from its placed PDF (`pdftotext -layout`; the package drawing
      for the pinout and pad count).
- [ ] `tvs-fact-check.py zener --facts`; each failing row `skip` with its reason in
      `verdicts.tsv`; read the `--sample` rows against their PDFs.

Acceptance: the Zener-type rows are checked.
  Check: `python3 -I planning/plan-69-data/census/tvs-pinout-check.py zener` → `0 errors`; `tvs-fact-check.py zener --facts` → `0 unchecked` (est. 10 min of runs).
Commit:

### Phase 3 — Steering specs (Semtech, Littelfuse, Diodes Inc.)

- [ ] The same columns for the steering rows; each line, rail and ground pin read
      from the datasheet's pin table.
- [ ] `tvs-fact-check.py steering --facts`; failing rows `skip`.

Acceptance: the steering rows are checked.
  Check: `python3 -I planning/plan-69-data/census/tvs-pinout-check.py steering` → `0 errors`; `tvs-fact-check.py steering --facts` → `0 unchecked` (est. 5 min).
Commit:

### Phase 4 — Final lists

- [ ] `census/tvs-final.py` per § 3.3: skipped-parts section, obsolete rows, prune,
      `specs.tsv` and `footprints-needed.txt` rewritten.

Acceptance: the final pick is written down.
  Check: `python3 -I planning/plan-69-data/census/tvs-final.py --verify-skipped` → `<n> skip rows, 0 missing from skipped-parts.md`; `python3 -I planning/plan-69-data/census/tvs-place.py --verify planning/plan-69-data/candidates.tsv` → `0 missing, 0 unused` (est. 2 min).
Commit:

## Validation Plan

- Tests: none (no product code); the acceptance commands above.
- Coverage check: `git status --short backend schematical` → empty after B.
- Doc sync: `planning/plan-69-data/README.md` (the new columns).
- Final gate: plan-69-I.

## Decisions

- Author: **facts come from DigiKey's parameters, used only when the datasheet's
  table agrees** (plan-47-B's rule); the table is the authority, DigiKey the
  reader.
- Author: **a bidirectional two-terminal TVS names its pins `1` and `2`**; the app
  draws pins `K`/`A` as the unidirectional symbol and `1`/`2` as the bidirectional
  one (plan-69-H).
- Author: **a steering array's rail pin is `inout`**, not `power`: the part has no
  supply range, and the voltage check rates it by `vrwm` like every other pin
  (plan-69-E § 3.2).

## Corrections

## Summary

A reading letter: one pass over each series datasheet for its table page, pinout
and pad count, with the fact check deciding the rows the datasheet does not bear
out.
