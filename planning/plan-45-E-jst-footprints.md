# plan-45-E: Footprints — JST board headers

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-45-D

E draws the land patterns of the JST headers of the final pick — 19 variants
at planning time: the series XH, PH, EH, ZH, VH (through hole, top entry and
side entry), and GH, SH/SR, PH, XH, ZH (surface mount, top entry and side
entry) — into `scripts/terminal-footprints.json` and
`planning/plan-45-data/footprints-E.json`. E extends the generator with the
surface-mount entry (signal pads plus two mounting tabs). The behavioral
outcome: every E line of `footprints-needed.txt` has an object for each of
its position counts, and the objects load through `footprint_check`.

References:

- plan-45-C § 3 (table, generator, rules) and plan-45-D § 3 (`explicit`,
  `flange`); `docs/parts-format.md` § Footprints (`explicit`, unnumbered
  pads).
- `datasheets/connector-jst/` (A): the series PDFs (`PC board layout and
  Assembly layout` pages).
- `planning/plan-45-data/footprints-needed.txt` (the E lines).

## Prerequisites

plan-45-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-45-D complete | `ls planning/completed/plan-45-D-*.md` → exists | NOT MET (2026-11-16: not started) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="E"' planning/plan-45-data/footprints-needed.txt \| wc -l` → the E lines (19 at planning time, before B's pick) | NOT MET (2026-11-16: 19 from the candidates) |
| The generator has `explicit` and `flange` | `grep -c 'flange' scripts/gen-terminals.js` ≥ 1 | NOT MET (2026-11-16: D writes it) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-45-data/footprints-E.json` has one object per (variant,
  position count) of the E lines (184 at planning time: `awk -F'\t' '!/^#/ &&
  $1=="E"{n+=gsub(/,/,",",$8)+1} END{print n}' planning/plan-45-data/footprints-needed.txt`
  → 184), and for `JST|XH|2.50|V|tht` and `JST|PH|2.00|V|tht` (the generic
  representatives, B) all position counts 2 to 12.
- The generator writes surface-mount `explicit` entries (§ 3) and passes its
  tests.
- The objects pass `census/fp-load.sh`.

### Non-goals (explicit constraints)

- No change to `footprints.json`, `Parts/*.json`, the goldens, the backend or
  the app. C's and D's entries and files are not touched.
- A variant whose layout needs a pad on the bottom layer or a slotted hole is
  made `skip` (`no footprint`) and listed in `datasheets/skipped-parts.md`,
  recorded under § Corrections.

## 2. Current State

The generator (C, D) writes `header` and `explicit`-with-`flange` objects, all
through hole. A JST through-hole layout is one row of holes (a boss or peg the
figure shows is a `flange` entry); a surface-mount layout is one row of solder
pads and, where the figure shows them, two mounting tabs (reinforcement pads)
beyond the ends of the row (a `tab` entry; a figure without tabs has none).
JST's layout figures are viewed from the
connector mounting surface (`eXH.pdf` note 1, line 107: `The PC board layout
figure shown is viewed from the connector mounting surface.`), that is from
below.

### Measured populations

| What | Count | Command |
|---|---|---|
| E lines (variants) | **19**: through hole 10, surface mount 9 | `awk -F'\t' '!/^#/ && $1=="E"{n[$7]++} END{for(k in n) print k, n[k]}' planning/plan-45-data/footprints-needed.txt` |
| E footprints (variant × position count) | **184** | the formula in § 1 |
| Parts on them | 192 | `awk -F'\t' '!/^#/ && $1=="E"{n+=$9} END{print n}' planning/plan-45-data/footprints-needed.txt` |
| By series (variants) | EH 2, GH 2, PH 4, SH/SR 2, VH 2, XH 3, ZH 4 | `awk -F'\t' '!/^#/ && $1=="E"{n[$4]++} END{for(k in n) print k, n[k]}' …` |
| By pitch (mm) | 1.00: 2, 1.25: 2, 1.50: 4, 2.00: 4, 2.50: 5, 3.96: 2 | `$5` |
| By orientation | vertical (top entry) 9, horizontal (side entry) 10 | `$6` |

### Verified properties

- JST's PDFs give both layouts in one place: `eXH.pdf` has `PC board layout
  and Assembly layout` (line 49 of `pdftotext -layout`) for top entry and, at
  line 117, for the radial-tape type; the other series' PDFs have the same
  section (A § 2).
- The loader takes unnumbered (`"pin": 0`) copper pads without a drill
  (`docs/parts-format.md` § `explicit`), which is what a mounting tab is.

## 3. Design

- **Ids** `<stem>-<NN>` (`JST-XH-2.50-VT-03`, `JST-GH-1.25-HS-07`): `T` through
  hole, `S` surface mount, `V` top entry, `H` side entry.
- **Through hole**: `header` entries as C (pad R5-T, round, pin 1 square),
  with `flange` (D) when the layout shows holes beside the row. Top entry:
  pads centred on the housing; side entry: the pad row above the housing,
  the mating face pointing down (C's orientation rule).
- **Surface mount**: `"style": "explicit"`, `"smd": {"pad": {"width",
  "height"}, "tab": {"dx", "y", "width", "height"}}` (`tab` optional): the
  generator writes pads 1…n (rect, no drill) at x = (c − (n − 1)/2)·pitch on
  y = `firstRowY`, and, with `tab`, two copper tabs (`"pin": 0`, rect, no drill) at x = ±((n − 1)/2·pitch +
  `tab.dx`) on y = `firstRowY` + `tab.y`. Pad and tab sizes are the layout
  figure's land sizes (R1), not the lead sizes.
- **Mirror.** JST's layout figures are the view from below: the table's
  numbers are read from the figure mirrored left to right, so the footprint
  is the top view; pin 1 is the housing's pin 1. For a symmetric layout the
  mirror changes nothing; for a side entry with an off-centre tab it does,
  and the `note` says "mirrored".
- **Generic representatives** `JST|XH|2.50|V|tht` and `JST|PH|2.00|V|tht`:
  entries with `positions` 2…12 (B).
- **Position count.** The `NN` in an id is the number of poles
  (`B7B-XH-A` → 07); the datasheet's dimension formula (`A = (n − 1) ×
  pitch`) is used for n at 2 and at the largest n of the line.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The surface-mount entries

- [ ] `scripts/gen-terminals.js`: `smd` entries (§ 3).
- [ ] `scripts/gen-terminals.test.js`: a 1.25 mm entry for n = 3 and n = 12
      gives n rect pads without drill and two `pin: 0` tabs at the computed
      x.

Acceptance: the surface-mount entries work.
  Check: `node --test scripts/gen-terminals.test.js` → all tests pass (est. 1 min).
Commit: —

### Phase 2 — Through-hole JST (10 variants)

- [ ] Table entries for EH, PH, VH, XH, ZH, top and side entry, from
      `datasheets/connector-jst/eEH.pdf`, `ePH.pdf`, `eVH.pdf`, `eXH.pdf`,
      `eZH.pdf` (the layout pages), mirrored per § 3; `positions` from the
      final lines, and 2…12 for XH and PH top entry.
- [ ] `node scripts/gen-terminals.js --letter E --out planning/plan-45-data/footprints-E.json`.

Acceptance: the through-hole E lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py E` → the through-hole lines absent from the missing list (est. 2 h).
Commit: —

### Phase 3 — Surface-mount JST (9 variants), load and hand-check

- [ ] Table entries for GH, PH, SH/SR, XH, ZH surface mount, top and side
      entry, from `eGH.pdf`, `ePH.pdf`, `eSH.pdf`, `eSR_SZ.pdf`, `eXH.pdf`,
      `eZH.pdf`.
- [ ] `census/fp-load.sh planning/plan-45-data/footprints-E.json`: no load
      error; only `extra footprint` / `unused footprint` for the new ids.
- [ ] For each variant, check its smallest and largest position counts
      against the layout figure.
- [ ] `docs/parts-format.md` § Footprints: the `TB-` paragraph becomes
      "Terminal-block and JST footprints (`TB-`, `JST-`)" and names the `smd`
      entries.

Acceptance: every E line has objects that load and match their drawings.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py E` → `<n> lines, 0 without a footprint`, then `planning/plan-45-data/census/fp-load.sh planning/plan-45-data/footprints-E.json` → exits 0 (est. 3 h; the load builds a temp backend, 10 min).
Commit: —

## Validation Plan

- Tests: `scripts/gen-terminals.test.js`, `fp-load.sh`.
- Doc sync: `docs/parts-format.md` § Footprints.
- Final gate: none; the landing tier is I's.

## Decisions

- Author: **Land sizes, not lead sizes, for surface-mount pads**: JST prints
  the PC board layout (the land pattern the datasheet gives), which R1 takes first.
- Author: **Mirror the layout figures** (they are viewed from below), so the
  footprint is the component-side view the app draws.

## Corrections

## Summary

E is the JST half of the footprint work: 19 variants from nine PDFs, with the
mirror as the one thing that is easy to get wrong. G merges everything.
