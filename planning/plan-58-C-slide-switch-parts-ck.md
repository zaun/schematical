# plan-58-C: Slide switches — C&K: schemes, footprints, generator, parts

Last updated: 2026-11-24
Effort: large (3h–1d)
Depends on: plan-58-B

C adds the C&K / Littelfuse slide switches to the library, and writes the
generator that D, E and F reuse. It decides the `pending` rows of C&K against
the datasheets' part-number keys, draws one footprint per distinct recommended
layout, fills C&K's rows of `specs.tsv`, and writes the devices and parts into
`Parts/switch.json` from `candidates.tsv`.

Outcome: `Parts/switch.json` holds one device per (circuit, action, rated
volts, actuator) of the kept C&K rows, each part naming its footprint; the
backend loads them; every kept C&K row of `candidates.tsv` is a part
(`grep -c 'states=' backend/tests/golden/parts.txt` = the number of C&K `add`
rows); the goldens carry exactly those lines, and `ctest` still passes.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` § `switch` (the keys of
  plan-58-B), § Parts, § Footprints, § Datasheets; `docs/backend.md` (goldens).
- `planning/plan-58-B-slide-switch-kind.md` § 4.3 (pin and contact names),
  `planning/plan-58-A-slide-switch-data.md` § 4.1–4.3 (columns, `specs.tsv`).
- `planning/plan-27-data/gen-parts.py` and `planning/plan-27-data/census/`
  (`scheme-check.py`, `specs-check.py`): the model for the generator and the
  checks; copy and adapt, do not call them.
- C&K datasheets: `datasheets/switch/slide/ck/` (13 PDFs, plan-58-A).

## Prerequisites

The gate for the whole feature is in plan-58-A § Prerequisites. This letter adds:

| Must be true | Command | Status |
|---|---|---|
| plan-58-A and plan-58-B are complete | `ls planning/completed/plan-58-A-* planning/completed/plan-58-B-* \| wc -l` → 2 | NOT MET until they land |
| The `switch` kind accepts `"states"` | `grep -c '"states"' backend/src/models/switch.c` → ≥ 1 | NOT MET until B lands |
| C&K's PDFs are placed | `ls datasheets/switch/slide/ck/*.pdf \| wc -l` → 13 (or fewer, when A skipped a PDF's rows) | NOT MET until A lands |
| `Parts/switch.json` has no slide devices | `grep -c '"id": *"slide_' schematical/schematical/Parts/switch.json` → 0 | MET (2026-11-24) |

If plan-58-A or plan-58-B is not complete, this plan cannot start, full stop.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-58-data/schemes.tsv` has a row per C&K series whose `pending`
  rows it decides; no C&K row has `decision` = `pending`.
- `specs.tsv` has, for every C&K row, `mpn_regex`, `pins`, `footprint`, `note`;
  `census/specs-check.py --maker 'C&K'` exits 0.
- `planning/plan-58-data/footprints-C.json` holds the C&K footprints until the
  parts land; then `Parts/footprints.json` has them and `golden/footprints.txt`
  has exactly their lines added.
- `Parts/switch.json` has the C&K devices and parts; `Parts/datasheets.json`
  has an entry per C&K datasheet the parts name; the goldens have exactly the
  new parts' lines added.

### Non-goals (explicit constraints)

- Every existing device, part and golden line is byte-identical (DIP, tactile,
  every other kind). `generator --dry-run` on an unchanged input changes nothing.
- No app code, no backend code (B), no other maker's rows.
- A part that is not in stock at DigiKey, or whose number the datasheet does not
  support, is not added (AGENTS.md); it is `skip` with its reason.

## 2. Current State

`switch.json` has 186 devices: 28 hand-written DIP devices, then the generated
tactile region (`planning/plan-27-data/gen-parts.py` owns it "from the first
`tact_` device to the end of the array"). `footprints.json` holds the DIP and
tactile footprints. `candidates.tsv` (A) has 119 C&K `add`/`pending` rows.

### Measured populations

| What | Count | Command |
|---|---|---|
| C&K rows kept by DigiKey rules (`add` at planning) | 119 | `awk -F'\t' '$1=="C&K" && $18=="add"' planning/plan-58-data/candidates.tsv \| wc -l` |
| …verbatim in the PDF text | 54 | planning-time probe (`pdftotext` + substring); A records the real `hit` column |
| …series / (series, mounting) / (series, mounting, circuit) | 12 / 27 / 51 | `awk -F'\t' '$1=="C&K" && $18=="add"{print $3}' … \| sort -u \| wc -l` → 12; likewise with `$3,$11` → 27 and `$3,$11,$4` → 51 |
| …by series | 1000: 33, L: 17, OS: 14, ES: 11, JS: 10, TS: 8, S: 7, V: 6, GS: 4, SK COMAX: 4, AYZ: 3, SS COMAX: 2 | `awk … \| sort \| uniq -c` |
| …by circuit and action | SPDT On-On 51, DPDT On-On 41, SPDT On-Off-On 9, SPST On-Off 9, DPST On-Off 6, DPDT On-Off-On 3 | `awk -F'\t' '$1=="C&K" && $18=="add"{print $4,$5}' planning/plan-58-data/candidates.tsv \| sort \| uniq -c` |
| Distinct datasheet URLs | 13 (12 on www.littelfuse.com, 1 `mm.digikey.com`) | `awk -F'\t' '$1=="C&K" && $18=="add"{print $16}' … \| sort -u \| wc -l` → 13 |
| Rated volts (DigiKey DC rating, else AC) | 5, 6, 12, 20, 28, 30, 125 | `awk -F'\t' '$1=="C&K" && $18=="add"{print $9}' … \| sort -u` |

### Verified properties

- `candidates.tsv` columns and `decision` values are those of plan-58-A § 4.1;
  `specs.tsv` columns those of § 4.2.
- A footprint is `explicit` when pads are not on a regular grid; every pin
  `1…pins` needs a pad; a `pin: 0` pad is unnumbered (`docs/parts-format.md`
  § Footprints › `explicit`). Mounting tabs and posts of a slide switch are
  `pin: 0` pads.
- `footprint_check` fails on a footprint no part uses (`backend/tests/footprint_check.c:179`,
  `unused footprint: <id>`, counted as a failure) and on one missing from
  `golden/footprints.txt`. So a footprint lands in `Parts/footprints.json` in the
  same step as the first part that names it; until then it is a ready object in
  `planning/plan-58-data/footprints-C.json`.

## 3. Design Overview

Four pieces, in order: (1) decide every `pending` row with the datasheet's
part-number key (`schemes.tsv`); (2) read each PDF's recommended PCB layout (or,
when it shows none, its terminal dimensions) and write the footprints plus the
`specs.tsv` rows that map part-number patterns to pins and footprints; (3) write
`census/gen-parts.py`; (4) run it, regenerate the goldens, check.

Correctness risk is in (2): a footprint with a wrong pad position is valid
JSON and passes every test. The checks that catch it are the pad count against
the pinout (the loader), `specs-check.py` (every kept row has exactly one spec
whose footprint has its pad count), and a rendered image of each new footprint
beside its datasheet drawing, read by eye (§ Validation). Byte-identity of
every other golden line is the gate against touching anything else.

Rejected: footprints added to `Parts/footprints.json` before their parts (the
`unused footprint` failure of `footprint_check` forbids it); one footprint per part number (a series' layout repeats
across part numbers); skipping a `pending` row because the PDF does not print it
(the key is the datasheet's statement of the part number).

## 4. Detailed Design

### 4.1 `schemes.tsv` and `scheme-check.py`

`schemes.tsv`: `maker`, `series`, `regex`, `source` (PDF file and page of the
key). The regex is an anchored Python expression over the part number built
from the key's positions and allowed letters (e.g. `OS102011M[AS]2Q[NS]1C?`).
`census/scheme-check.py --maker 'C&K'` turns each `pending` row of the maker into
`add` when its part number fullmatches its series' regex, else `skip`, reason
`number does not fit the datasheet's part-number key`; it rewrites
`candidates.tsv` and the "Slide switches (plan-58)" section of
`datasheets/skipped-parts.md`. `--selftest`: three inline cases.

### 4.2 `specs.tsv` rows and `specs-check.py`

One row per (series, mounting, circuit) of the kept rows (`add` after 4.1):
`mpn_regex` (anchored, selects exactly that row group: usually the mounting and
circuit digits of the key), `pins` (`designator=name;…`, names of plan-58-B
§ 4.3, designators as the datasheet numbers the terminals), `footprint` (an
id of `footprints.json`), `note` (drawing source: PDF file, page, and which
terminal is `A` / `B`: the one the common joins in the position the datasheet
lists first). `census/specs-check.py --maker M`: every `add` row of the maker
matches exactly one spec row whose `pins` has the circuit's pin names and whose
footprint's numbered pads are exactly the designators in `pins`.

### 4.3 Footprints

`style: explicit`; id `SW-SLIDE-<MAKER>-<SERIES>-<M>-<N>P` with `<MAKER>` =
`CK`, `NKK`, `ESW`, `CIT`; `<SERIES>` upper-case, non-alphanumerics removed;
`<M>` = `THT` / `THTRA` (right angle) / `THTRAV` (right angle, vertical) / `SMD` /
`SMDRA`; `<N>` = numbered pads; a second layout with the same id gets `-B`.
Origin: the body centre; pin 1 at the datasheet's pin 1; mm. Through-hole pads:
`round`, drill from the drawing's hole, pad diameter drill + 0.6 mm unless the
drawing gives a pad. Surface-mount pads: the drawing's land. Mounting tabs,
posts and shield legs: `pin: 0` pads. `note` names the PDF file and page. When
a datasheet shows only the switch's terminal dimensions and no PCB layout, the
terminal positions are the hole centres and the pad is drill + 0.6 mm.

### 4.4 `gen-parts.py`

`python3 -I planning/plan-58-data/gen-parts.py [--maker M]... [--dry-run]` and `--selftest`.
Owns the contiguous run of devices whose `id` starts `slide_`, placed
immediately before the first `tact_` device of `switch.json` (so plan-27's
generator, which owns "from the first `tact_` device to the end", never
touches it, and the hand-written DIP devices before it stay). It rebuilds the
region from `candidates.tsv` (rows `add`) and `specs.tsv` for the named
makers plus every maker whose parts the region already holds; a second run
with the same arguments changes nothing.

- Device = (circuit, action, rated volts, actuator): `id`
  `slide_<circuit>_<onon|onoffon|onoff>_<V>v_<actuator slug>`, `function`
  `<circuit> <action>` (`SPDT On-On`), `description` `<circuit> slide switch, <action>`,
  `drawer ["Switches & Relays","Slide"]`, `actuator` as DigiKey names it,
  `ratedVolts` = DigiKey's DC voltage rating, else its AC rating, integer volts,
  `note` `Rating <V> V: DigiKey Voltage Rating - DC|AC`, `contacts`, `states`,
  `rest` from plan-58-B § 4.3, `pinouts` one per distinct `pins` string
  (`p<N>`, then `p<N>_2`, … in sorted order of the strings; pins `role` `inout`,
  `number` the designator as an integer).
- Part = `{"name": MPN, "manufacturer", "datasheet", "package", "pinout",
  "footprint"}`; `manufacturer` C&K → `Littelfuse`, NKK → `NKK Switches`, CIT →
  `CIT Relay & Switch`, E-Switch → `E-Switch`; `package` from DigiKey's mounting:
  `Through Hole` → `THT`, `Through Hole, Right Angle` and `…, Vertical` → `THT right
  angle`, `Surface Mount` → `SMD`, `Surface Mount, Right Angle` → `SMD right angle`;
  `datasheet` = the placed PDF's file name, lower case, no `.pdf`.
- `datasheets.json`: an entry per datasheet id the parts name (`url` = the
  `maker_url` of `maker-urls.tsv` when set, else the row's DigiKey URL); existing
  entries kept, new ones inserted in `id` order.
- Footprints: for the parts it writes, moves every footprint they name from
  `planning/plan-58-data/footprints-*.json` into `Parts/footprints.json` (one
  line per footprint, the file's layout, appended in id order), so a footprint and
  its first part always land together.
- Refusals (exit 1): a part name already in another `Parts/*.json` or outside the
  region; an `add` row with zero or two `specs.tsv` matches; an unknown footprint
  id; an unreadable rating (no DC and no AC rating); a spec whose circuit does not
  match the row's.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` partial; moot tasks stay with
> evidence; fill each phase's `Commit:` when it lands. An unticked box means NOT DONE.

### Phase 1 — decide the pending rows

- [ ] Read each C&K PDF's part-number key; write `planning/plan-58-data/schemes.tsv`
      (C&K rows, § 4.1) and `census/scheme-check.py` with `--selftest`.
- [ ] `python3 -I census/scheme-check.py --maker 'C&K'`; commit the new
      `candidates.tsv` and the skipped-parts section.

Acceptance: no C&K row is `pending`; each `skip` reason is listed in
`datasheets/skipped-parts.md`.
  Check: `awk -F'\t' '$1=="C&K" && $18=="pending"' planning/plan-58-data/candidates.tsv \| wc -l` → 0 and `python3 -I planning/plan-58-data/census/scheme-check.py --selftest` → `ok` (est. 3 min).
Commit: —

### Phase 2 — footprints and specs

- [ ] For each of C&K's (series, mounting, circuit) lines (51 at planning): read
      the PDF drawing (open the page; the layout is usually a drawing, not text),
      write the footprint (§ 4.3) as a ready object in
      `planning/plan-58-data/footprints-C.json` (`{"footprints":[…]}`) and its
      `specs.tsv` row (§ 4.2). Footprints shared by several lines are written once.
- [ ] Write `census/specs-check.py` (with `--selftest`) and `census/fp-load.sh <footprints-X.json>`,
      then run both on `footprints-C.json`: `fp-load.sh` copies the tree's `Parts/` to a
      temp dir, appends the staged footprints, runs `footprint_check` there; the
      only failures allowed are `unused footprint` for exactly the staged ids, and
      there is no load error.
- [ ] For every new footprint: a rendered image (the app's footprint renderer
      through the harness, as `scripts/harness/tactile-footprints.swift` does for the
      tactile ones) beside the datasheet page; read each pair and fix any mismatch.

Acceptance: every C&K row has a spec whose footprint is staged and has its
pads; the staged footprints load; each image matches its drawing.
  Check: `python3 -I planning/plan-58-data/census/specs-check.py --maker 'C&K' && planning/plan-58-data/census/fp-load.sh planning/plan-58-data/footprints-C.json` → `specs ok`, then only `unused footprint` lines for the staged ids and `load ok` (est. 8 min).
Commit: —

### Phase 3 — generator and parts

- [ ] `planning/plan-58-data/gen-parts.py` (§ 4.4) with `--selftest` (a three-row
      inline `candidates.tsv`/`specs.tsv`: one device per distinct key, pin
      names, the `slide_` region placement before `tact_`, the footprint move, idempotence, each refusal).
- [ ] `python3 -I planning/plan-58-data/gen-parts.py --maker 'C&K'` writes the
      devices, parts and `datasheets.json` entries.
- [ ] Regenerate the goldens (`listing_check --print > parts.txt`, `trace_check --print > traces.txt`,
      `footprint_check --print > footprints.txt`) and review: only added lines, in each file.
- [ ] `backend/tests/slide_test.c`: add a library pass that loads the real
      library and checks, for every device whose `id` starts `slide_`: 2–3 states,
      `rest` in range, contacts and pins as plan-58-B § 4.3 says for its circuit,
      each part's footprint has the pinout's pads (so a generator slip fails a test).
- [ ] `node scripts/check-datasheets.js --host www.littelfuse.com` → exit 0.

Acceptance: `grep -c 'states=' backend/tests/golden/parts.txt` equals the number
of C&K `add` rows; the generator run is idempotent; `ctest` passes.
  Check: `python3 -I planning/plan-58-data/gen-parts.py --selftest && python3 -I planning/plan-58-data/gen-parts.py --maker 'C&K' --dry-run && cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8 -R 'slide_test|listing_check|trace_check|footprint_check|library_check'` → `ok`, no change, `100% tests passed` (est. 12 min).
Commit: —

## Validation Plan

- Tests: `scheme-check.py`, `specs-check.py`, `gen-parts.py` `--selftest`; `fp-load.sh`; `slide_test`'s
  library pass; the three goldens (`footprint_check` also proves every footprint is used).
- Coverage check: `grep -c 'states=' backend/tests/golden/parts.txt` = the C&K
  `add` count (the new parts are in `listing_check`'s and `trace_check`'s denominators:
  both iterate every library part).
- Runtime proof: the footprint images read against the datasheet pages (Phase 2);
  one part's page of the app is G's job.
- Doc sync: `docs/parts-format.md` § `switch` (the generated region note: the
  `slide_` devices are written by `gen-parts.py`; edit the inputs, not the devices);
  `datasheets/skipped-parts.md`.
- Final gate (once, after Phase 3): `cmake --build /tmp/logic-p58 -j8 && ctest --test-dir /tmp/logic-p58 -j8`
  → `100% tests passed, 0 tests failed out of 62` (est. 15 min).

## Decisions

- Slide devices sit before the tactile region and are written by a generator
  that owns only `slide_` devices — the author's call (plan-27's generator owns
  the end of the array). (§ 4.4)
- Footprint ids, device ids, package names and the rating rule are those of § 4.3–4.4:
  the author's call; ratings follow plan-27 (DigiKey's rating).
- A `pending` row is added only when its number fits the datasheet's key — the
  author's call (AGENTS.md: parts come from the datasheet).

## Corrections

(Filled in during execution.)

## Summary

The risk is the footprints: they are drawn from PDF drawings, a wrong pad is valid
data, and only the image comparison catches it. The generator is mechanical and
tested by selftest and the library pass. Untouched: the backend, the app, every
other maker, every existing golden line.
