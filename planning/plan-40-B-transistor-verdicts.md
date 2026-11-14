# plan-40-B: Transistors — ordering tables, pinouts, verdicts, the final pick, skipped parts

Last updated: 2026-11-14
Effort: large (3h–1d)
Depends on: plan-40-A

B reads the datasheets A downloaded and decides, for every eligible row of
`planning/plan-40-data/candidates.tsv`, whether the part is `add` (in the
catalog), `skip` (a still-made part the catalog cannot take: no maker
datasheet, not in the datasheet's ordering table) or `out` (obsolete by the
datasheet, or any rule of plan-40-A § 3). It writes each datasheet's
ordering-code scheme and each package's pinout and elements into `specs.tsv`,
fills skipped slots with the next part of the same (family, maker), lists the
skips and the obsolete PDFs, and regenerates `footprints-needed.txt` from the
final pick. The behavioral outcome: `candidates.tsv` is final (every picked
row `add`), `specs.tsv` is complete, and the Phase 5 check commands print
their expected values.

References:

- plan-40-A § 3 (the selection rules 5–7 B applies), § 4, § 6.
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`
  (formats), `AGENTS.md` (§ Datasheets).
- `planning/completed/plan-34-B-resistor-picks.md` and
  `planning/plan-34-data/census/` (`scheme.py`, `scheme-check.py`,
  `specs-fill.py`, `verdicts.py`: the scripts B adapts).
- `docs/parts-format.md` § Pinouts and pins (pin names, roles).

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-A complete | `ls planning/completed/plan-40-A-*.md` → exists | NOT MET (A not started) |
| Every picked row from a cut piece is stock-checked | `awk -F'\t' '$24=="candidate" && $25==1 && $26!=1' planning/plan-40-data/candidates.tsv \| wc -l` → 0 | NOT MET (A not started) |
| The PDFs are placed | `ls datasheets/transistor/datasheets.tsv` → exists; `ls planning/plan-40-data/specs.tsv` → exists | NOT MET (A not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `planning/plan-40-data/specs.tsv` has a `scheme`, `label`, `pinout` and
  `elements` on every row (one row per (maker, series, PDF, DigiKey Package /
  Case)); `census/scheme-check.py` prints `0 eligible rows without a scheme`.
- `planning/plan-40-data/verdicts.tsv` has one row per eligible row (3840 at
  planning time) with `verdict` `add`, `skip` or `out` and its reason;
  `candidates.tsv` is rewritten by `select.py --stock … --verdicts …` with
  `reason` `add` / `skip` / `out:<reason>`; the number of `add` rows per
  (family, maker) equals the share of plan-40-A § 3 rule 4 or, where the
  maker has fewer eligible parts, all of them.
- Every `skip` row is in `datasheets/skipped-parts.md`
  (`census/verdicts.py --verify-skipped` → `0 skip rows missing from
  skipped-parts.md`); every PDF whose rows are all `out` for datasheet status
  is deleted and in `datasheets/obsolete-not-included.md`.
- `footprints-needed.txt` is regenerated from the `add` rows; the lines per
  letter C, D, E are recorded in § Corrections against the planning-time 28,
  63 and 73.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part editor of datasheet tables beyond `specs.tsv`: a pinout or scheme
  that a PDF does not give is a `skip`, never a guess.
- No part with a pin name outside the kind's pin names (plan-40-F § 3.1):
  `B`, `C`, `E`, `G`, `D`, `S`, each with a digit suffix for the elements of
  an array.

## 2. Current State

After A: the PDFs of the eligible rows are under `datasheets/transistor/`,
`candidates.tsv` has `pdf` and `hit` filled, `specs.tsv` is a skeleton with
one row per (maker, series, PDF, DigiKey Package / Case) and its `scheme`,
`label`, `pinout` and `elements` columns empty. The planning-time numbers B
works from: 3840 eligible rows (`awk -F'\t' '$24=="candidate" ||
$24=="duplicate" || $24=="below quota"' planning/plan-40-data/candidates.tsv
\| wc -l`), 1287 picked, hosts and distinct URLs as plan-40-A § 2.

### Verified properties

- **Pin names fit the kind.** The kind's pin names (plan-40-F § 3.1) are
  `base`/`collector`/`emitter` and `gate`/`drain`/`source` per element, which
  the pinout names `B`/`C`/`E` and `G`/`D`/`S` (suffix `1`, `2`, … for an
  array's elements). A tab on the collector or drain carries the same name as
  its lead (`D` twice for a SOT-223), which the kind ties
  (plan-40-F § 3.1, the diode kind's tied pins as precedent:
  `diode.json` device `fast-1`, pinout `p3-a-a-k`).
- **Pin orders differ between datasheets of one package, and between
  packages.** onsemi's `2N3904` TO-92 is 1 Emitter, 2 Base, 3 Collector
  (`pdftotext -layout` of `https://www.onsemi.com/pdf/datasheet/2n3904-d.pdf`,
  lines 24–26); its BC546/547/548/550 TO-92 is 1 Collector, 2 Base, 3 Emitter
  (`BC550-D.PDF`, line 22); a SOT-23 `MMBT3904LT1G` is 1 Base, 2 Emitter, 3
  Collector (`mmbt3904lt1-d.pdf`, CASE 318 style 6). So one device per pinout,
  as the diode plan's `p3-a-k-a`, `p3-k-a-k` (`diode.json`).

## 3. Design

### 3.1 `specs.tsv`

One row per (maker, series, PDF, DigiKey Package / Case):

| Column | Value |
|---|---|
| `maker`, `series`, `pdf`, `package`, `rows`, `example` | from A's skeleton (`package` = the DigiKey Package / Case string) |
| `scheme` | a regular expression that full-matches every orderable part number of the datasheet's ordering table for that package (one alternation per ordering-table row; `pdftotext -layout` of the PDF's ordering page is the source) |
| `label` | the part's `"package"` value: the package family, no pin count (`TO-92`, `SOT-23`, `SC-70`, `DPAK`, `D2PAK`, `TO-220`, `SOIC`, `PowerPAK SO-8`, …), per docs/parts-format.md § Parts |
| `pinout` | the pin names in pin order, comma separated (`E,B,C`; `G,D,S`; `B1,E1,C2,B2,E2,C1`; `G,D,S,D` for a tab as pin 4), from the datasheet's pin diagram or pin table; a ball package (BGA, WLCSP, LGA) writes `<ball>=<name>` in the datasheet's ball table order, row-major when the table has none (`A1=G,B1=S,A2=D`; ball names as docs/parts-format.md § Pinouts and pins) |
| `elements` | `;`-separated `type:control,a,b` using the pinout's names: `npn:B,C,E`, `pmos:G,D,S`, `npn:B1,C1,E1;pnp:B2,C2,E2` |

`scheme-check.py` prints, per maker, the eligible rows matched by their row's
scheme (or printed verbatim: `hit`), and `0 eligible rows without a scheme`
at the end; `pinout-check.py` checks every row: each name of `elements` is in
`pinout`; the pinout has one name per pin of the package (the pin count of
the DigiKey Package / Case, e.g. `-3`, `-6`, `8-`; tab packages count the
tab); every `pinout` name is used by an element.

### 3.2 Verdicts

`verdicts.py` writes `verdicts.tsv` (`mpn`, `verdict`, `reason`) from
`candidates.tsv` and `specs.tsv`:

- `skip`, `no datasheet on the maker's site` — `pdf` empty (plan-40-A § 3
  rule 5);
- `skip`, `not in the datasheet's ordering table` — `hit` not 1 and the
  scheme does not match;
- `skip`, `no pinout in the datasheet` — the row's package has no `pinout`;
- `out`, `datasheet status` — `dead` = 1 (the datasheet marks the part
  obsolete, discontinued, NRND or end of life);
- else `add`.

`select.py --stock <stock-check.tsv> --verdicts <verdicts.tsv>` (B adds the
option) takes each row's verdict, then applies the duplicate rule (plan-40-A
§ 3 rule 3) and the quota (rule 4) again to the rows whose verdict is `add`:
a skipped winner is replaced by its duplicate (the same base and package with
another packing suffix) and an unfilled share by the next `below quota` row
that is `add`; a share that cannot be filled stays short (the maker has no
more). Example from planning: onsemi's `BC547B` (the most-stocked of its
key) links a Rochester copy and is not in the BC546/547 datasheet's ordering
table, while `BC547BTA`, `BC547BTF` and `BC547BBU` are (`pdftotext -layout`
of `https://www.onsemi.com/pub/Collateral/BC550-D.PDF` lines 194, 195, 208:
`BC547BBU`, `BC547BTF`, `BC547BTA` in the ordering table, pin line 22
`1. Collector 2. Base 3. Emitter`), so the duplicate takes the slot.

### 3.3 Where the risk is

Risk concentrates in the pinouts: a wrong pin order puts a part with the
collector where the emitter is. The mitigation is `pinout-check.py` (every
name once, the right count) and a hand check of one part per distinct pinout
of each maker against its PDF's pin diagram (Phase 4), recorded in
§ Corrections.

Rejected alternatives: (a) one pinout per DigiKey package (`TO-92` is E-B-C
for one datasheet and C-B-E for the next, so a package does not fix a
pinout); (b) taking pinouts from DigiKey's description text (it has none).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — onsemi (with Fairchild)

- [ ] `census/scheme-check.py`, `census/pinout-check.py` (§ 3.1) and
      `census/specs-fill.py` (merges a hand-written maker file into
      `specs.tsv`).
- [ ] Read each onsemi PDF's ordering table and pin diagram (`pdftotext
      -layout`; one `specs.tsv` row per (series, PDF, package)); fill
      `scheme`, `label`, `pinout`, `elements`. Fairchild-branded PDFs are
      onsemi's after the rewrite of plan-40-A § 4 rule 3.
- [ ] Record each distinct pinout checked by hand against the PDF's pin
      diagram (one per distinct pinout) in § Corrections.

Acceptance: every onsemi eligible row has a scheme or is a skip.
  Check: `python3 -I planning/plan-40-data/census/scheme-check.py --maker onsemi && python3 -I planning/plan-40-data/census/pinout-check.py --maker onsemi` → `0 eligible rows without a scheme`; `0 pinout errors` (est. 1 min; the reading is the work).
Commit: —

### Phase 2 — Nexperia and Diodes Incorporated

- [ ] As Phase 1 for the Nexperia PDFs (type-number family datasheets, e.g.
      `BC847_BC547_SER`: one scheme per series, the `,215`/`,235` packing
      suffixes of DigiKey's part numbers are in the table) and the Diodes
      Incorporated PDFs (ordering information tables with `-7`, `-13` reel
      suffixes).

Acceptance: as Phase 1 for both makers.
  Check: `scheme-check.py --maker Nexperia`, `--maker "Diodes Incorporated"`, the two `pinout-check.py` runs → `0 eligible rows without a scheme`; `0 pinout errors` (est. 1 min).
Commit: —

### Phase 3 — Infineon and Vishay Siliconix

- [ ] As Phase 1 for the Infineon PDFs (PG-TO packages; pins in the package
      outline table) and the Vishay Siliconix PDFs (`doc?NNNNN` and `docs/…`
      data sheets; the part-marking / ordering table gives `-E3`, `-GE3`,
      `-T1-GE3` suffixes).

Acceptance: as Phase 1 for both makers.
  Check: the four runs as Phase 2 for `--maker "Infineon Technologies"` and `--maker "Vishay Siliconix"` → `0 eligible rows without a scheme`; `0 pinout errors` (est. 1 min).
Commit: —

### Phase 4 — Datasheet status, verdicts, the final pick

- [ ] Read every eligible row's PDF for the datasheet's own status wording
      (`obsolete`, `discontinued`, `end of life`, `not recommended for new
      designs`, `NRND`, last-time-buy notes in the ordering table):
      `candidates.tsv` `dead` = 1 for such a part; record each PDF whose
      eligible rows are all `dead`.
- [ ] `census/verdicts.py` (§ 3.2): `verdicts.tsv`; `select.py`'s
      `--verdicts` option; re-run the pick to the final
      `candidates.tsv` (`reason` `add` / `skip` / `out:<reason>`).
- [ ] Hand-check one part per distinct pinout of each maker against its PDF's
      pin diagram (the first onsemi, Nexperia, Diodes, Infineon and Vishay
      lines of each distinct `pinout` of `specs.tsv`), recorded in
      § Corrections: row, PDF page, result.

Acceptance: the verdicts and the pick hold, and the owner's named parts are
in (or their absence is explained).
  Check: `awk -F'\t' '$24=="add"' planning/plan-40-data/candidates.tsv \| wc -l` → N (recorded here; at most 1287); `python3 -I planning/plan-40-data/census/select.py /tmp/p40/raw /tmp/p40/cand.tsv --stock planning/plan-40-data/stock-check.tsv --verdicts planning/plan-40-data/verdicts.tsv --report` → exits 0 and the `add` per (family, maker) line; `for n in 2N3904 2N3906 BC547B BC557B 2N7000 BS170 2N7002; do echo $n $(awk -F'\t' -v n=$n '$24=="add" && index($1,n)==1' planning/plan-40-data/candidates.tsv \| wc -l); done` → each at least 1; a name with 0 is written in § Corrections with the verdict reasons of its rows (est. 3 min).
Commit: —

### Phase 5 — Skips, obsolete PDFs, footprint lines

- [ ] `datasheets/skipped-parts.md`: a `## Transistors (plan-40)` section with
      one table row per (device or series, reason) group of the `skip` rows,
      in the file's four-column format (`Part(s) | Device | Would have gone
      into | Why skipped`, the "Would have gone into" `transistor` /
      `Discrete → Transistors → BJT` or `… MOSFET`).
- [ ] `datasheets/obsolete-not-included.md`: one row per PDF whose eligible
      rows are all `out` for datasheet status; delete those PDFs and their
      `datasheets/transistor/datasheets.tsv` lines.
- [ ] `census/footprint-lines.py`: take rows whose `reason` is `add` (it takes
      `candidate` today, `if r['reason'] != 'candidate'`); regenerate
      `planning/plan-40-data/footprints-needed.txt` with `python3 -I
      planning/plan-40-data/census/footprint-lines.py
      planning/plan-40-data/candidates.tsv >
      planning/plan-40-data/footprints-needed.txt`; record the lines per
      letter in § Corrections.

Acceptance: the skip list is complete and the footprint lines are final.
  Check: `python3 -I planning/plan-40-data/census/verdicts.py --verify-skipped` → `0 skip rows missing from skipped-parts.md`; `grep -vc '^#' planning/plan-40-data/footprints-needed.txt` → the final count (est. 1 min).
Commit: —

## Validation Plan

- Tests: `scheme-check.py`, `pinout-check.py`, `verdicts.py --verify-skipped`.
- Coverage check: every eligible row has a verdict (`wc -l <
  planning/plan-40-data/verdicts.tsv` = the eligible rows + 1).
- Runtime proof: n/a (data letter).
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: plan-40-I.

## Decisions

- **A skipped part is a part the catalog cannot take now** — author: a part
  with no maker datasheet, or not in its ordering table, would have no
  verifiable source for its name or pinout (the inclusion rule).
- **Pin names are the function names with a digit suffix per array element** —
  author: the kind refers to pins by name (docs/parts-format.md § Pinouts).
- **Pinouts come from the datasheet's pin diagram only** — author: a wrong pin
  order is the plan's main risk (§ 3.3).
- Owner decisions: plan-40-A § Decisions.

## Corrections

(none yet)

## Summary

B is judgment work on PDFs: schemes and pinouts. Its risk is a wrong pinout,
caught by the structure check and a hand check per distinct pinout. After B
nothing in the tree changes except `planning/plan-40-data/`,
`datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` and the
placed PDFs.
