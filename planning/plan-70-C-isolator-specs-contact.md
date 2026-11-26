# plan-70-C: Specs: transistor-, Darlington- and triac-output optocouplers

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-70-B

C fills `specs.tsv` for the `transistor` and `triac` rows: the pinout
(designator → pin name) and the part facts of each (PDF, package, channels)
line, read from the PDF. It also decides the `pending` rows of those two
types against their datasheets' ordering keys. It writes the shared spec
tools D reuses.

References:

- plan-70-B (`specs.tsv` skeleton, `candidates.tsv` verdicts, the PDFs).
- plan-70-G § 3.2 (pin names and roles), § 3.5 (part keys). The spec
  columns are written in G's format so H's generator copies them.
- `planning/plan-58-data/census/specs-check.py`, `scheme-check.py`,
  `merge-specs.py` and `planning/plan-48-data/census/pinout-check.py`,
  `slice-specs.py`: the tools C copies.

## Prerequisites

plan-70-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-70-B complete | `ls planning/completed/plan-70-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |
| The skeleton exists | `awk -F'\t' 'NR>1 && ($1=="transistor" \|\| $1=="triac")' planning/plan-70-data/specs.tsv \| wc -l` → at least 1 | NOT MET (2026-11-25: no file) |

Everything below is written against the world where these hold.

> **NOTE: the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `transistor` and `triac` row of `specs.tsv` has `pinout` and
  `facts` filled. `python3 -I planning/plan-70-data/census/specs-check.py transistor triac`
  → `<n> rows, 0 errors`.
- No `transistor` or `triac` row of `candidates.tsv` is `pending`. Each is
  `add` with `scheme` set (the ordering-key rule that matched), or `skip`,
  `not in the ordering key`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint (E maps the packages).
- Facts that G § 3.5 does not name are not collected.

## 2. Current State

The skeleton rows carry `type maker pdf package supplier_package channels
rows example`. The pinout and fact columns are empty (B § 3.3).
Transistor-output parts come in 4-pin DIP / SMD / SOP (one LED, C, E), 6-pin
DIP (A, K, NC, E, C, B), 8- and 16-pin DIP / SOP (2 and 4 channels), and
AC-input variants with two antiparallel LEDs (the H11AA rows, B § 2).
Triac-output parts are 6-pin DIP / SMD with A, K, NC, MT2, NC/substrate,
MT1, plus 4-pin SOP parts. Some have a zero-cross circuit, which is a
separate device (MOC302x random phase, MOC304x/306x zero cross).

## 3. Design

### 3.1 `pinout`

`designator=name;…` for every pin of the package, in pin order. Names
follow G § 3.2: `A`, `K` (LED anode, cathode); `C`, `E`, `B` (collector,
emitter, base); `MT1`, `MT2` (triac main terminals); `NC` for no-connect
and the triac's substrate pin. With several channels, `A1`, `K1`, `C1`,
`E1`, … numbered by the datasheet's channel order. With an AC input, the
two LED pins are `A` and `K` as the datasheet's first diode reads; the
`facts` column carries `ac`. A pin the datasheet names `C` for the cathode
(Vishay 4N25 pin 2) is renamed `K`, and the `note` column records the
rename (G § 3.2: names are unique per role).

### 3.2 `facts`

`key=value;…` with G § 3.5's part keys:

- transistor: `viso` (V rms, the isolation test voltage), `ctrMin`,
  `ctrMax` (%, at the datasheet's test current; `ctrMax` only when given),
  `vceo` (V), `ifMax` (mA, LED forward current); device facts `darlington`
  (Output Type Darlington), `ac` (AC input), `base` (Transistor with Base).
- triac: `viso`, `vdrm` (V, peak off-state), `ift` (mA, maximum LED trigger
  current), `itrms` (mA, on-state RMS current, Triac Power rows); device
  fact `zeroCross` (`true` for a zero-cross part).

One spec row covers every part of its line. A fact that differs between the
parts of a line (CTR ranks such as `-1`, `-2`, `-3`; trigger-current grades
`MOC3021`/`MOC3022`/`MOC3023`) is written as `key@mpn-regex=value` overrides
after the line's default (`ctrMin@-2$=63`). H's generator applies the first
override whose regex matches.

### 3.3 Ordering keys (`census/scheme-check.py`)

For every `pending` row, `scheme` is an anchored regex over the part
number, read from the datasheet's ordering key: base, CTR rank, lead form
(`S`, `SM`, `X`, `W`, `G`), tape (`T&R`, `TR`, `TA1`, `-X007T`), VDE option.
`scheme-check.py <type>` applies every `scheme` of the line's PDF to the
line's `pending` rows. A match is `add`, a non-match `skip`, `not in the
ordering key`. The rows' lead form must equal the line's `package` (a `SM`
suffix on a DIP line is a mismatch, `skip`, `package differs`).

### 3.4 Work split

The lines are read in slices of 5 (`census/slice-specs.py`, from
plan-48's), one slice per subagent brief. Every brief has § 3.1–3.3 and
G § 3.2 / § 3.5. `census/merge-specs.py` merges the slices. `specs-check.py`
then checks the rows:

- each pin of the package is named once;
- the names are from § 3.1's set;
- each channel has an LED pair and an output (`C`+`E`, or `MT1`+`MT2`);
- the fact values are in G § 3.5's ranges.

## Phases

> **NOTE: keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1: Tools

- [ ] `census/specs-check.py`, `scheme-check.py`, `slice-specs.py`,
      `merge-specs.py` per § 3; `specs-check.py --selftest` (a duplicate
      name, a missing LED pin, a channel without an output, a CTR out of
      range) prints `ok`.

Acceptance: the selftest passes.
  Check: `python3 -I planning/plan-70-data/census/specs-check.py --selftest`
  → `ok`. Takes under 1 min.
Commit:

### Phase 2: Transistor and Darlington lines

- [ ] Fill `pinout` and `facts` of every `transistor` row (Isocom, Vishay,
      onsemi); merge.
- [ ] `scheme-check.py transistor` decides the `pending` rows.

Acceptance: the transistor rows are complete and decided.
  Check: `python3 -I planning/plan-70-data/census/specs-check.py transistor`
  → `0 errors`; `awk -F'\t' 'NR>1 && $1=="transistor" && $23=="pending"' planning/plan-70-data/candidates.tsv | wc -l`
  → 0. Takes about 2 min.
Commit:

### Phase 3: Triac lines

- [ ] Fill `pinout` and `facts` of every `triac` row (onsemi, Vishay,
      Würth); merge; `scheme-check.py triac`.
- [ ] Re-run B's refill for the rows C made `skip`
      (`build.py --refill` → `fetch.py` → `textscan.py` → `decide.py`). Any
      new `pending` rows go through Phase 2 or 3's steps.

Acceptance: the triac rows are complete. No transistor or triac row is
`pending`.
  Check: `specs-check.py transistor triac` → `0 errors`;
  `awk -F'\t' 'NR>1 && ($1=="transistor" || $1=="triac") && $23=="pending"' planning/plan-70-data/candidates.tsv | wc -l`
  → 0. Takes about 2 min.
Commit:

## Validation Plan

- Tests: `specs-check.py --selftest`; the acceptance checks.
- Coverage check: `git status --short` lists only `planning/plan-70-data/`.
- Doc sync: `planning/plan-70-data/README.md` (the spec columns and tools).
- Final gate: plan-70-J.

## Decisions

- Author: **pin names are unique per role** (`K` for the LED cathode, `C`
  for the collector). Kind keys name pins, and a name on two pins means
  both (`docs/parts-format.md` § Pinouts), so the datasheets' shared `C`
  cannot stay.
- Author: **CTR and trigger-current grades are part-level overrides** of
  one spec line, not separate lines. They share a pinout and a footprint.
- Author: **the substrate pin of a triac driver is `NC`**. The datasheets
  say not to connect it.

## Corrections

## Summary

Pinouts are short (4 to 16 pins). The work is in the ordering keys and the
CTR-rank overrides, and the rule that catches mistakes is `specs-check.py`.
