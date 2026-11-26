# plan-76-F: Pressure specs, Amphenol All Sensors and Honeywell

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-E

F fills the `allsensors` and `honeywell` rows of `specs-pressure.tsv` by plan-76-C § 3's rules and
the pressure facts of § 3. Each maker is capped at 300 (All Sensors 411, Honeywell 406 at
planning; plan-76-A table). Both makers sell series whose part numbers encode range, unit,
pressure type, port and output in fixed fields, so most rows are decoded from the ordering-code
table rather than read one by one. G does ST and NovaSensor and draws every pressure footprint.

References:

- plan-76-C § 3 (rules); plan-76-L § 3.1 (`pressure` in kPa, `"pressureType"`, output `bridge`).
- Each series' "ordering information" / "nomenclature" page in its datasheet.

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-E complete | `ls planning/completed/plan-76-E-*.md` → exists, or every E box ticked | NOT MET (2026-11-25: E not started) |
| The skeleton has the two makers | `awk -F'\t' 'NR>1 && ($1=="allsensors" \|\| $1=="honeywell")' planning/plan-76-data/specs-pressure.tsv \| wc -l` → above 0 | NOT MET (written by B) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `allsensors` and `honeywell` row of `specs-pressure.tsv` passes
  `specs-check76.py --family pressure --maker <m>`, with `pressureType` in `extra` and a
  per-part `ranges` column where the series' parts differ by range.
- Each series decoder used is recorded in `census/pressure-codes.py` with the datasheet page it
  reads.

### Non-goals (explicit constraints)

- No footprint drawn here (G).
- No port, media or tube fact: the kind has no key for them (plan-76-L § 3.1); the row's `note`
  records the port style.

## 2. Current State

Parts seen at planning (the census page `b512`'s first 100 rows, read at planning and not kept):
Honeywell 25 rows — `ABP` / `ABP2`, `HSC`, `SSC`, `MPR`, `RSC` series (e.g. `ABPDANT005PGAA5`,
`HSCDRRN001ND2A3`, `MPRLS0025PA00002A`, `SSCDANN015PGAA5`); All Sensors 3 rows — `AUAV-L30D-…`,
`1 INCH-D-4V`, `15 PSI-A-4V-ASCX`. Pressure packages (`show.py raw/b512.json "Package / Case"`):
6-SMD no lead top port 121, 4-SIP module 95, 8-SMD J-lead dual ports same side 77, 8-DIP top port
76, 6-SMD no lead 49, 8-DIP dual ports same side 43.

All Sensors' links (plan-76-data/plan-76.md probe): the HubSpot copy served a PDF; a
`www.allsensors.com/datasheets/…` link was a 404 (B's fallback handles it).

## 3. Design

### 3.1 Pressure facts

- `measures`: `pressure`, plus `temperature` when the part reports it.
- `output`: `bridge` (mV/V, no `vcc`), `analog` (an amplified voltage output), `i2c`, `spi`.
- `ranges`: pressure in kPa, converted from the datasheet's unit with 1 psi = 6.894757 kPa,
  1 inH2O = 0.249089 kPa (at 4 °C, Honeywell's and All Sensors' convention as their tables print
  it), 1 mbar = 0.1 kPa, 1 bar = 100 kPa, 1 Pa = 0.001 kPa; rounded to 3 decimals. A differential
  or bidirectional range is `[-p, p]`; gauge `[0, p]`; absolute `[0, p]` (or the printed lower
  bound).
- `extra`: `pressureType=absolute|gauge|differential` from the code's type letter (Honeywell: `A`
  absolute, `G` gauge, `D` differential; All Sensors: `-A-`, `-G-`, `-D-`).
- `accuracy`: Honeywell's Total Error Band (±%FSS) is not an absolute accuracy in kPa: converted
  to kPa as `TEB × span / 100` and recorded in the row's `note` with the TEB; All Sensors likewise.

### 3.2 One device per (series, package, output)

A series' parts that share a package and pin table share one pinout file and one specs row with
`parts` and a per-part `ranges` map (`part=min..max;…`), which M turns into each part's
`"ranges"` (plan-76-L § 3.1: a part's ranges replace the device's).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Honeywell

- [ ] `planning/plan-76-data/census/pressure-codes.py` with the Honeywell series decoders (each
      with its datasheet page); fill every `honeywell` row and its pinout files.

Acceptance: every Honeywell row passes, and every part's code decodes.
  Check: `python3 -I planning/plan-76-data/census/pressure-codes.py --verify honeywell` → `<n> parts decoded, 0 unknown`; `python3 -I planning/plan-76-data/census/specs-check76.py --family pressure --maker honeywell` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — Amphenol All Sensors

- [ ] The All Sensors decoders; fill every `allsensors` row and its pinout files.

Acceptance: the same for All Sensors.
  Check: `python3 -I planning/plan-76-data/census/pressure-codes.py --verify allsensors` → `<n> parts decoded, 0 unknown`; `python3 -I planning/plan-76-data/census/specs-check76.py --family pressure --maker allsensors` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

## Validation Plan

- F's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **ranges in kPa for every pressure part**, converted with the fixed factors of § 3.1, so
  the facts compare across makers whatever unit a datasheet uses.
- Author: **Total Error Band goes to the note, converted**: it is the makers' headline accuracy
  figure, and the kPa value fits the kind's `accuracy`; the note keeps the original.
- Author: **decoders, not hand entry**, for the coded series: 600 part numbers in a handful of
  series are decoded from the ordering tables, checked by `--verify` against every row.

## Corrections

## Summary

F fills the pressure specs of the two biggest pressure makers by decoding their ordering codes.
