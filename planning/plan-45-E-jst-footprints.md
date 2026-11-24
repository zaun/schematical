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
| plan-45-D complete | `ls planning/completed/plan-45-D-*.md` → exists | MET (2026-11-24: not archived, every D box ticked: `grep -c '\[ \]' planning/plan-45-D-terminal-footprints-pluggable.md` → 0; D's commit fdb2f35f) |
| `footprints-needed.txt` is final | `awk -F'\t' '!/^#/ && $1=="E"' planning/plan-45-data/footprints-needed.txt \| wc -l` → the E lines (19 at planning time, before B's pick) | MET (2026-11-24: 19 lines from B's pick (184 footprints, 192 → 191 parts after A's stock check); after E's two splits (Corrections) **21 lines, 193 footprints, 191 parts**) |
| The generator has `explicit` and `flange` | `grep -c 'flange' scripts/gen-terminals.js` ≥ 1 | MET (2026-11-24: 9 lines; C wrote `explicit`, D `flange`) |

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

- [x] `scripts/gen-terminals.js`: `smd` entries (§ 3).
      `smd: {pad: {width, height}, tab: {dx, dy, width, height}}` (the tab's centre `dx` outward from the end pad and `dy` from the pad row; the pad row is `firstRowY`, there is no drill), and `pegs` (single holes at pole 1's or the last pole's end: the XH boss and the VH shroud peg).
- [x] `scripts/gen-terminals.test.js`: a 1.25 mm entry for n = 3 and n = 12
      gives n rect pads without drill and two `pin: 0` tabs at the computed
      x.
      New test: a 1.25 mm entry at n = 3 and 12 (rect pads without a drill, two `pin: 0` tabs at ±(end pad + 1.85), body 3 x 1.25 + 3.25) and a peg entry (non-plated `pin: 0` hole 1.6 beyond pole 1).

Acceptance: the surface-mount entries work.
  Check: `node --test scripts/gen-terminals.test.js` → all tests pass (est. 1 min). Result: `ℹ tests 10`, `ℹ pass 10`, `ℹ fail 0`.
Commit: this commit (hash in the next one)

### Phase 2 — Through-hole JST (10 variants)

- [x] Table entries for EH, PH, VH, XH, ZH, top and side entry, from
      `datasheets/connector-jst/eEH.pdf`, `ePH.pdf`, `eVH.pdf`, `eXH.pdf`,
      `eZH.pdf` (the layout pages), mirrored per § 3; `positions` from the
      final lines, and 2…12 for XH and PH top entry.
      Twelve through-hole entries (EH, PH, ZH, VH, XH × top and side entry, XH-M and VH-FB split off): holes Ø0.7 to 1.7, rows and body from the layout pages and the tables' `A`, `B` columns (B = n x pitch + k). The XH top entry and PH top entry entries carry all of 2…12 (B's generics).
- [x] `node scripts/gen-terminals.js --letter E --out planning/plan-45-data/footprints-E.json`.
      Run once all entries were in the table.

Acceptance: the through-hole E lines have objects.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py E` → the through-hole lines absent from the missing list (est. 2 h). Result: `21 lines, 0 without a footprint`.
Commit: this commit (hash in the next one)

### Phase 3 — Surface-mount JST (9 variants), load and hand-check

- [x] Table entries for GH, PH, SH/SR, XH, ZH surface mount, top and side
      entry, from `eGH.pdf`, `ePH.pdf`, `eSH.pdf`, `eSR_SZ.pdf`, `eXH.pdf`,
      `eZH.pdf`.
      Nine entries (GH, PH, ZH, SH/SR × top and side entry, XH side entry): land sizes read from the layout figures enlarged to 400 to 500 dpi (about 0.05 mm per pixel): pads, tabs, their offsets from the outline centre; the SH/SR sheets `eSH.pdf` and `eSR_SZ.pdf` print the same figure (and so does `eSR_SZ-new.pdf` for the top entry).
- [x] `census/fp-load.sh planning/plan-45-data/footprints-E.json`: no load
      error; only `extra footprint` / `unused footprint` for the new ids.
      `193 footprints appended`, `193 new footprints: load ok`.
- [x] For each variant, check its smallest and largest position counts
      against the layout figure.
      `census/fp-widths.py E --ends`: the body width equals the tables' `B` column on all 176 rows that print one (0 mismatches; 9 against a "widest" outline), the smallest and largest positions per variant are in the run (e.g. GH side entry: 3 pos 7, 12 pos 18.25; XH SMT: 3 pos 12.5, 6 pos 20). The pad and tab positions come from the figures' printed dimensions where they print them (pitch, tab widths and heights, the offsets 1.35 / 0.7 / 1.3 / 1.4 from pole 1) and from the enlarged figure elsewhere (Corrections).
- [x] `docs/parts-format.md` § Footprints: the `TB-` paragraph becomes
      "Terminal-block and JST footprints (`TB-`, `JST-`)" and names the `smd`
      entries.
      Heading "Terminal-block and JST footprints (`TB-`, `JST-`)": the `JST-` ids, `smd` pads and tabs, `pegs`, the mirror, and the pad rule at 1.5 mm pitch.

Acceptance: every E line has objects that load and match their drawings.
  Check: `python3 -I planning/plan-45-data/census/fp-check.py E` → `<n> lines, 0 without a footprint`, then `planning/plan-45-data/census/fp-load.sh planning/plan-45-data/footprints-E.json` → exits 0 (est. 3 h; the load builds a temp backend, 10 min). Result: `21 lines, 0 without a footprint`, `193 footprints, 0 missing`; exit 0, `193 new footprints: load ok`.
Commit: this commit (hash in the next one)

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

- **2026-11-24, the E lines.** B's pick gave 19 lines / 184 footprints / 191 parts (the plan's 192 parts were planning-time; A's stock check took `S11B-XH-A`). Two housings share a pad pattern with their series' other parts but not the layout: the XH header with boss (`B3B-XH-AM`, a Ø1.1 +0.2 boss hole 1.6 beyond pole 1 and 2.0 off the row) and the VH shrouded header (`B*P-VH-FB-B`, 8 rows: a Ø1.4 peg 1.5 beyond pole 1 and 3.4 off the row). `census/split-variant.py` split them into `JST|XH-M|2.50|V|tht` (position 3) and `JST|VH-FB|3.96|V|tht` (2, 3, 4, 5, 6, 7, 9, 10): **21 lines, 193 footprints** (`awk`: `21 E lines, 193 E footprints, 191 parts`). The XH and VH ordering schemes of `specs.tsv` are disjoint again (`B*B-XH-A` / `B*B-XH-AM`, `B*P-VH(-B)` / `B*P-VH-FB-B`; `scheme-check.py` → `jst: 191 rows, 0 unmatched, 0 ambiguous`).
- **2026-11-24, `split-variant.py` and a PDF that serves several variants.** The script moved a variant's `specs.tsv` row with the rows, which left the old variant without one when the sheet is shared (one JST PDF per series). It now syncs `specs.tsv` from `candidates.tsv`: a (maker, variant, PDF) a row needs gets a copy of a sibling spec row, one no row needs is dropped (`--sync-specs` runs only that step; 2 copied, 12 stale rows dropped, among them the four pruned MHS rows and the old Weidmüller rows of D's splits).
- **2026-11-24, orientation and pin 1.** § 3's mirror was applied as "the figure viewed from below, mirrored left to right"; pin 1 is the left pad of the footprint for every variant (D's Decision), whichever side of the figure the `No. 1 circuit` mark is on (the SH, GH and PH side entry figures have it on the left, so their pin-1 end is mirrored). Vertical offsets keep the figure's up and down. The side entry through-hole headers (EH, PH, ZH, VH, XH) draw the housing from 0.5 mm behind the hole row to the figure's front distance (EH 6.7, PH 6.25, ZH 6.0, XH 9.2, VH 7.2: the part overhangs the board edge, so the footprint outline is the on-board part); XH's two C versions (9.2 and 7.6) are one `widest` outline.
- **2026-11-24, hole and pad sizes.** The holes are the layout figures' (Ø0.9 to 1.65 with their +0.1: the middle; XH 1.0 covers the 2-circuit Ø1 and the 3+ circuit Ø0.9 +0.1). R5-T (pad = drill + 0.7 at most pitch - 0.8) leaves nothing at ZH's 1.5 mm pitch (0.7 mm drill, 0.7 mm cap), so ZH uses a pad of drill + 0.3 = 1.0 (0.5 mm of space between pads). The surface-mount pads and tabs are the figures' land sizes (R1), measured on the enlarged layout where the figure prints no number for them (pad lengths, tab positions: about ±0.1 mm).
- **2026-11-24, XH side entry SMT is a single row, not staggered** (JST eXH.pdf p.3): 0.9 x 4.4 pads at 2.5 mm and two 1.8 x 3.9 tabs; the figure's long outline (11 mm) is the on-board area from the board edge line to the pad ends.

## Summary

E is the JST half of the footprint work: 19 variants from nine PDFs, with the
mirror as the one thing that is easy to get wrong. G merges everything.
