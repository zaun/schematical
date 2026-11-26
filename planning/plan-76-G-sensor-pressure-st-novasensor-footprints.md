# plan-76-G: Pressure specs, ST and NovaSensor; the pressure footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-F

G fills the `st` and `novasensor` rows of `specs-pressure.tsv` (ST ≤ 104, NovaSensor ≤ 93; plan-76-A
table) by plan-76-C § 3 and plan-76-F § 3, then draws and merges every new pressure footprint
(all four makers).

References:

- plan-76-C § 3, plan-76-D § 3.2 (`fp-merge76.py`), plan-76-F § 3 (pressure facts, units, one
  device per series and package).
- `docs/parts-format.md` § Footprints (`explicit` pads for DIP and SIP modules with ports).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-F complete | `ls planning/completed/plan-76-F-*.md` → exists, or every F box ticked | NOT MET (2026-11-25: F not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every row of `specs-pressure.tsv` passes `specs-check76.py --family pressure`.
- Every new pressure footprint id is drawn in `footprints-pressure.json`, merged, and its golden
  line added; no existing footprint line changes.

### Non-goals (explicit constraints)

- No port or tube geometry in a footprint beyond the land pattern and courtyard (a top-port
  part's port is above the board; a bottom-port part's board hole is drawn only when the
  datasheet's recommended PCB layout has one, as a non-plated `explicit` hole).

## 2. Current State

Parts seen at planning (`b512`'s first 100 rows): ST 42 rows — `LPS25HB`, `LPS27HHTW`, `LPS28DFW`,
`ILPS28QSW` (HLGA / ceramic LGA), and NXP-heritage `MPX2010`, `MPX2050`, `MPX2102`, `MPX5010`,
`MPX5050`, `MPX5100`, `MPX5500`, `MPX5700`, `MPXAZ6115`, `MPXH6115`, `MP3V5004`, `MP3V5050`,
`MPVZ5004`, `MPL3115A2` (unibody, SSOP-8, LGA-8); 36 of the 42 link `www.nxp.com` datasheets.
NovaSensor 1 row seen: `NPA` surface-mount series (`AAS-920-477J-NovaSensor-NPA-SurfaceMnt`, a
1,098,253-byte PDF in the probe).

## 3. Design

- Specs: plan-76-F § 3 (decoders in `pressure-codes.py` for the MPX and NPA codes).
- Footprints: plan-76-D § 3.2. The MPX unibody and the Honeywell / All Sensors DIP and SIP modules
  are `explicit` footprints from each drawing's lead positions; SMD parts with a printed land
  pattern use it.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — ST and NovaSensor specs

- [ ] Decoders for ST's MPX / MP3V / MPVZ / MPXH codes and NovaSensor's NPA / NPC codes in
      `pressure-codes.py`; fill every `st` and `novasensor` row and its pinout files.

Acceptance: the whole pressure specs file passes.
  Check: `python3 -I planning/plan-76-data/census/pressure-codes.py --verify st novasensor` → `<n> parts decoded, 0 unknown`; `python3 -I planning/plan-76-data/census/specs-check76.py --family pressure` → `<n> rows ok, 0 errors` (est. < 1 min).
  Result:
Commit:

### Phase 2 — Pressure footprints

- [ ] Draw `footprints-pressure.json`; merge; regenerate `footprints.txt`; hand-check each added
      line.

Acceptance: every pressure footprint id exists, and the golden only grew.
  Check: `python3 -I planning/plan-76-data/census/fp-merge76.py --check pressure` → `0 missing`; `ctest --test-dir /tmp/logic-p76 -R footprint_check` → passed; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0 (est. 3 min).
  Result:
Commit:

## Validation Plan

- G's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **ST's NXP-heritage pressure parts are ST's**, read from the NXP datasheets DigiKey
  links: DigiKey lists ST as the maker, and the datasheets are the current ones.
- Author: **all pressure footprints in one phase**, after all four makers' specs, because the
  Honeywell, All Sensors and MPX DIP/SIP modules share lead pitches and some outlines.

## Corrections

## Summary

G finishes the pressure specs and lands every pressure footprint.
