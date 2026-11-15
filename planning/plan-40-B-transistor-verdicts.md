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
| plan-40-A complete | `ls planning/completed/plan-40-A-*.md` → exists | MET (2026-11-15: `planning/completed/plan-40-A-transistor-data.md`) |
| Every picked row from a cut piece is stock-checked | `awk -F'\t' '$24=="candidate" && $25==1 && $26!=1' planning/plan-40-data/candidates.tsv \| wc -l` → 0 | MET (2026-11-15: 0) |
| The PDFs are placed | `ls datasheets/transistor/datasheets.tsv` → exists; `ls planning/plan-40-data/specs.tsv` → exists | MET (2026-11-15: both exist; 3983 PDFs, 4279 spec rows) |

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

- [x] `census/scheme-check.py`, `census/pinout-check.py` (§ 3.1) and
      `census/specs-fill.py` (merges a hand-written maker file into
      `specs.tsv`). With `census/specs.py` (what they share),
      `census/need-groups.py` and `census/mark-unread.py` (Corrections 1),
      `census/dead-merge.py` (Phase 4), `census/verdicts.py` and `select.py
      --verdicts --carry` (Phase 4). Smoke run on the skeleton:
      `scheme-check.py --maker Nexperia` → `232 eligible rows without a
      scheme`; `pinout-check.py --maker Nexperia` → `326 pinout errors` (every
      row still empty).
- [x] Read each onsemi PDF's ordering table and pin diagram (`pdftotext
      -layout`; one `specs.tsv` row per (series, PDF, package)); fill
      `scheme`, `label`, `pinout`, `elements`. Fairchild-branded PDFs are
      onsemi's after the rewrite of plan-40-A § 4 rule 3. Done for every
      onsemi spec row the pick can reach (Corrections 1): `specs-onsemi-1.tsv`
      282 rows, `specs-onsemi-2.tsv` 258 rows, seven waves.
- [x] Record each distinct pinout checked by hand against the PDF's pin
      diagram (one per distinct pinout) in § Corrections (Corrections 7).

Acceptance: every onsemi eligible row has a scheme or is a skip.
  Check: `python3 -I planning/plan-40-data/census/scheme-check.py --maker onsemi && python3 -I planning/plan-40-data/census/pinout-check.py --maker onsemi` → `0 eligible rows without a scheme`; `0 pinout errors` (est. 1 min; the reading is the work).
  Result (2026-11-15, on the pre-verdict view, before the final pick):
  `onsemi: 2678 rows with a PDF; 794 verbatim, 3 by scheme, 43 in neither, 1838 not read (n/r)`,
  `0 eligible rows without a scheme`; `pinout-check.py` → `0 pinout errors`.
Commit: 7d17c1e0 (wave 1), d27a13fb, dde82394 (waves 2–6), (this commit) (wave 7)

### Phase 2 — Nexperia and Diodes Incorporated

- [x] As Phase 1 for the Nexperia PDFs (type-number family datasheets, e.g.
      `BC847_BC547_SER`: one scheme per series, the `,215`/`,235` packing
      suffixes of DigiKey's part numbers are in the table) and the Diodes
      Incorporated PDFs (ordering information tables with `-7`, `-13` reel
      suffixes). `specs-nexperia.tsv` 126 rows, `specs-diodes.tsv` 232 rows.

Acceptance: as Phase 1 for both makers.
  Check: `scheme-check.py --maker Nexperia`, `--maker "Diodes Incorporated"`, the two `pinout-check.py` runs → `0 eligible rows without a scheme`; `0 pinout errors` (est. 1 min).
  Result (2026-11-15): `Nexperia: 232 rows with a PDF; 0 verbatim, 190 by scheme, 2 in neither, 40 not read (n/r)`;
  `Diodes Incorporated: 504 rows with a PDF; 296 verbatim, 3 by scheme, 34 in neither, 171 not read (n/r)`;
  `0 eligible rows without a scheme`; `0 pinout errors`.
Commit: d7c5fba3 (Nexperia wave 1), 7d17c1e0, d27a13fb, dde82394, (this commit)

### Phase 3 — Infineon and Vishay Siliconix

- [x] As Phase 1 for the Infineon PDFs (PG-TO packages; pins in the package
      outline table) and the Vishay Siliconix PDFs (`doc?NNNNN` and `docs/…`
      data sheets; the part-marking / ordering table gives `-E3`, `-GE3`,
      `-T1-GE3` suffixes). `specs-infineon.tsv` 277 rows, `specs-vishay.tsv` 254 rows.

Acceptance: as Phase 1 for both makers.
  Check: the four runs as Phase 2 for `--maker "Infineon Technologies"` and `--maker "Vishay Siliconix"` → `0 eligible rows without a scheme`; `0 pinout errors` (est. 1 min).
  Result (2026-11-15): `Infineon Technologies: 1494 rows with a PDF; 55 verbatim, 198 by scheme, 34 in neither, 1207 not read (n/r)`;
  `Vishay Siliconix: 1592 rows with a PDF; 308 verbatim, 1 by scheme, 13 in neither, 1270 not read (n/r)`;
  `0 eligible rows without a scheme`; `0 pinout errors`.
Commit: 7d17c1e0, d27a13fb, dde82394, (this commit)

### Phase 4 — Datasheet status, verdicts, the final pick

- [x] Read every eligible row's PDF for the datasheet's own status wording
      (`obsolete`, `discontinued`, `end of life`, `not recommended for new
      designs`, `NRND`, last-time-buy notes in the ordering table):
      `candidates.tsv` `dead` = 1 for such a part; record each PDF whose
      eligible rows are all `dead`. Every row of a read spec row, and every
      text-scan dead flag at or above its floor, was read by hand
      (`dead-*.tsv`, merged by `census/dead-merge.py`; Corrections 6): 8 PDFs
      are all dead by hand (Phase 5's obsolete list).
- [x] `census/verdicts.py` (§ 3.2): `verdicts.tsv`; `select.py`'s
      `--verdicts` option; re-run the pick to the final
      `candidates.tsv` (`reason` `add` / `skip` / `out:<reason>`).
- [x] Hand-check one part per distinct pinout of each maker against its PDF's
      pin diagram (the first onsemi, Nexperia, Diodes, Infineon and Vishay
      lines of each distinct `pinout` of `specs.tsv`), recorded in
      § Corrections: row, PDF page, result (Corrections 7).

Acceptance: the verdicts and the pick hold, and the owner's named parts are
in (or their absence is explained).
  Check: `awk -F'\t' '$24=="add"' planning/plan-40-data/candidates.tsv \| wc -l` → N (recorded here; at most 1287); `python3 -I planning/plan-40-data/census/select.py /tmp/p40/raw /tmp/p40/cand.tsv --stock planning/plan-40-data/stock-check.tsv --verdicts planning/plan-40-data/verdicts.tsv --report` → exits 0 and the `add` per (family, maker) line; `for n in 2N3904 2N3906 BC547B BC557B 2N7000 BS170 2N7002; do echo $n $(awk -F'\t' -v n=$n '$24=="add" && index($1,n)==1' planning/plan-40-data/candidates.tsv \| wc -l); done` → each at least 1; a name with 0 is written in § Corrections with the verdict reasons of its rows (est. 3 min).
  Result (2026-11-15): N = **1247**; `select.py … --report` exits 0: `bjt: 364 picked` (Nexperia 62,
  onsemi 152, Diodes 150), `bjt-array: 51` (onsemi 20, Diodes 20, Nexperia 11), `bjt-prebiased: 109`
  (Nexperia 29, onsemi 40, Diodes 40), `bjt-prebiased-array: 42` (14 each), `mosfet: 601` (onsemi 201,
  Infineon 200, Vishay 200), `mosfet-array: 80` (onsemi 27, Vishay 27, Infineon 26); `not read: 3988
  rows; at or above their floor: 0; in a short share: 0`; `dead by the text scan only: 94 rows; at or
  above their floor: 0`; `picked from a cut piece, not stock-checked: 0`. Named parts: 2N3904 2,
  2N3906 0, BC547B 2, BC557B 0, 2N7000 2, BS170 2, 2N7002 6 (the two 0s: Corrections 8).
Commit: (this commit)

### Phase 5 — Skips, obsolete PDFs, footprint lines

- [x] `datasheets/skipped-parts.md`: a `## Transistors (plan-40)` section with
      one table row per (device or series, reason) group of the `skip` rows,
      in the file's four-column format (`Part(s) | Device | Would have gone
      into | Why skipped`, the "Would have gone into" `transistor` /
      `Discrete → Transistors → BJT` or `… MOSFET`). `verdicts.py --write-skipped`
      → `713 skip rows in 56 table rows` (the skip rows at or above their
      (family, maker)'s floor).
- [x] `datasheets/obsolete-not-included.md`: one row per PDF whose eligible
      rows are all `out` for datasheet status; delete those PDFs and their
      `datasheets/transistor/datasheets.tsv` lines. 8 rows (ZTX550, FDMA7628,
      NSS60200SMT, NSS60201SMT, BCV71, EFC6605R, MBT35200MT1, MJE182: each
      dead by hand, and every part its table lists dead); 69 more PDFs are
      all dead by the text scan only, below the pick: not listed as obsolete
      (unverified), and not kept (no `add` row uses them). The PDFs leave the
      tree in the prune task below.
- [x] `census/footprint-lines.py`: take rows whose `reason` is `add` (it takes
      `candidate` today, `if r['reason'] != 'candidate'`); regenerate
      `planning/plan-40-data/footprints-needed.txt` with `python3 -I
      planning/plan-40-data/census/footprint-lines.py
      planning/plan-40-data/candidates.tsv >
      planning/plan-40-data/footprints-needed.txt`; record the lines per
      letter in § Corrections (Corrections 9: 140 lines, C 20, D 56, E 64).
- [ ] Prune the PDFs to those an `add` row or a footprint note cites
      (`place.py --prune`), then commit them with `datasheets.tsv`
      (plan-40-A Corrections 6). Waits for C–E's final footprint notes.

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

1. **Reading waves: only the spec rows the pick can reach are read.** After
   the re-listing (plan-40-A Corrections 3) there are 6247 eligible rows and
   4279 spec rows over 3983 PDFs, against the plan's 3840 rows. The pick
   takes at most 1287 parts, so most spec rows serve rows far below their
   (family, maker)'s floor. `census/need-groups.py` runs the pick
   optimistically (a row whose spec row is not read counts as passing) and
   prints the spec rows a picked row, or one of the next 10 rows of its
   (family, maker), uses: wave 1 is `1236 picked; 1296 spec rows to read`
   (onsemi 471, Vishay Siliconix 247, Infineon 243, Diodes 211, Nexperia
   124). Each wave is read and merged; the waves end when need-groups.py
   prints nothing. Then `mark-unread.py` sets the remaining spec rows'
   `scheme` to `n/r`, `verdicts.py` makes their rows `out`, `below the pick
   (not read)`, and `select.py … --verdicts … --report` proves the shortcut
   changed nothing: its `not read: …; at or above their floor: 0; in a short
   share: 0` line (a not-read row below its share's lowest `add` stock cannot
   be picked whatever its verdict). `scheme-check.py` counts them as `not
   read (n/r)`, not as rows without a scheme. The reading is split into six
   work lists (onsemi in two halves by PDF), each read by a separate agent
   from the brief `/tmp/p40/b/BRIEF.md` into `specs-<list>.tsv` and
   `dead-<list>.tsv`.
2. **Conventions of `specs.tsv`.** `scheme` `=`: every row of the group is
   printed verbatim, so no regex is needed (a hit is in the ordering table
   by rule 6); `-`: the PDF has no ordering table (only a hit counts);
   `n/r`: not read (Corrections 1). Pinout name `NC` for a not-connected pin
   (an `nc` pin in G). A pinout may have one pin more than DigiKey's count
   when the datasheet numbers the tab on its own and the last pin repeats a
   channel name (`G,D,S,D` on a DPAK DigiKey counts as `-3`).
   Added during the reading: `SD` (with an element digit) names a half
   bridge's switch node, the source of one element and the drain of the
   other (`nmos:G1,D1,SD;nmos:G2,SD,S2`; Infineon BSC09xx / BSG08xx /
   ISG0614 / IRFH4257D / IRFI4212H, Vishay PowerPAIR, onsemi NTMFD /
   FDPC / NTLGD / NTTFD), and several numbered pads past DigiKey's lead
   count are allowed when each is on a channel (ThinPAK pad 5, TOLL pad 9,
   Nexperia DFN2020-6 pads 7–8). `gen-parts.py` reads `1=G,2=S,…` pinouts
   (Vishay MICRO FOOT bumps numbered 1–4) as numbered pins. Pinouts that
   still cannot be written stay `-` and their parts are skipped (`no
   pinout in the datasheet`): a pin that is one element's collector and
   the other's base (Diodes UMC4N/UMC5N, onsemi EMC2DXV5T1), a common drain
   with no terminal (onsemi EFC parts), a MOSFET with a separate Schottky
   (FDFMA2P853, FDFME3N311ZT, FDZ3N513ZT, NTLGF3402P, NTLJF3117P), DirectFET
   MN/ME/L8 (pads not numbered), unnumbered PHASE pads (FDMS3606AS,
   FDMC7208S), PDFs with no pin numbers (FDN337N, FQPF3N80C, FCMT199N60,
   FCPF600N60Z).
3. **Verdict overrides.** A part printed verbatim in a PDF that is not its
   datasheet passes rule 6 by the text match alone, so the readers listed
   such parts and `planning/plan-40-data/verdict-overrides.tsv` (read by
   `verdicts.py` and `need-groups.py`) gives them their verdict: MMBT3904SL
   (named only as the NPN complement in `mmbt3906sl-d.pdf`), BCX71J (only
   the publication number `BCX71J/D`), BC846A / BC847A / BC848A (the SC-70
   datasheet), DTC143Z (the SMD datasheet), TIP32 (`mjd112-d.pdf`: "similar
   to"), all `skip`, `not in the datasheet's ordering table`; KSP2907ACTA
   (the centre-collector E-C-B version), `skip`, `no pinout in the
   datasheet`.
4. **One datasheet row, both polarities.** A spec row can hold the NPN and
   the PNP part of one datasheet (MJD31/MJD32, TIP31/32, D44H/D45H,
   MJD41C/42C, 2SA1416 …): the pin roles are written once and
   `gen-parts.py` takes a single element's npn/pnp (nmos/pmos) from the
   part's own DigiKey type (`polarity()`); arrays keep `specs.tsv`'s
   element types (DigiKey types FMBA56 and DTA115TD as single, the
   datasheets show duals). Array device ids carry the element types.
5. **Named parts and DigiKey's base.** DigiKey's base product number lumps
   gain grades (`BC547` for BC547A/B/C), so rule 3 made BC547BBU/BC547BTF
   duplicates of a BC547C part and the owner's named BC547B fell out. A
   named part is now a duplicate only of the same named part (`named_of()`
   in `select.py`, the same key in `need-groups.py` and the report's
   proof): BC547BBU and BC547BTF are `add`.
6. **Dead flags the text scan set.** The 300-character window flags parts
   that sit next to an obsolete one (the onsemi tables' "DISCONTINUED"
   heading, a neighbouring NRND row, the "NRND = …" legend): of the flags
   the readers checked, 112 were false (`dead-merge.py`: `1 -> 0`) and 33
   true, and 17 more were found only as page-image watermarks
   (`pdftotext` drops them; the readers looked at page 1 of each PDF and
   onsemi-1 scanned pages 1–2 for grey pixels). A dead row never enters the
   reading waves, so every text-only flag at or above its (family,
   maker)'s floor was read by hand (68 rows, 41 PDFs: 64 false); the report
   proves the rest are below the pick: `dead by the text scan only: 94
   rows; at or above their floor: 0`.
7. **Hand checks of the pinouts.** For every distinct (maker, pinout,
   element types) of the `add` rows — 136 — one row was checked against
   the drawing as a page image (`pdftoppm`), its note in
   `specs-<list>.tsv` saying `visual p.N ok` (132 by the readers; the
   reader reports list each pinout with its PDF and page). The main
   session checked the other four: `ZTX951.pdf` p.1 (E-Line, leads drawn
   C B E with no numbers: see below), `irfhs8242pbf.pdf` p.1 (PQFN 2×2 top
   view 1 D, 2 D, 3 G, 4 S, 5 D, 6 D: ok), `PQMD12.pdf` p.2 Table 2 (pins
   1–8 GND1, I1, O2, GND2, I2, O1, O1, O2: ok), `ntjd4152p-d.pdf` STYLE 26
   (S1, G1, D2, S2, G2, D1: ok). One fault found: the Diodes E-Line rows
   numbered the leads E, B, C while plan-40-C's `E-LINE-3-DIODES` numbers
   its pads C, B, E (the drawn order; neither PDF numbers the leads), which
   would have put the emitter on the collector's pad; the five E-Line rows
   now read `C,B,E`.
8. **Named parts.** 2N3904 2 (`2N3904BU`, `2N3904TA`), BC547B 2
   (`BC547BBU`, `BC547BTF`), 2N7000 2, BS170 2, 2N7002 6. **2N3906: 0** —
   `2N3906BU`, `TA`, `TF`, `TFR`, `TAR` are `skip`, `not in the
   datasheet's ordering table` (onsemi's `2n3906-d.pdf` lists only the
   RL1 / RLRA / RLRM / RLRP packings), `2N3906ZL1` links a Rochester copy.
   **BC557B: 0** — `BC557BTF` and `BC557BTA` are `out`, `datasheet status`
   (listed under DISCONTINUED in `BC556-D`'s ordering table).
9. **The final pick: 1247 parts.** The waves read 1390 spec rows (wave 1
   1296, then 76, 8, 1, 6 + 1, and 39 after the dead-flag reading); 4043
   are `n/r`. The final `footprints-needed.txt` has 140 lines (C 20, D 56,
   E 64), against the planning-time 28, 63, 73.
10. **The Infineon FET piece was re-listed.** The skips lowered Infineon's
   MOSFET floor to 11082, below the 300th stock (13354) of its cut piece
   `278-448 all` (`NEEDS DEEPER LISTING`), so `recheck.sh` got a fourth
   line and re-listed it (106 calls; 2144 parts), then A Phase 3 ran again
   for the new rows (1127 more URLs, 1123 PDFs) before the last waves. No
   picked row comes from a cut piece now (`candidates from a cut piece: 0
   of 1247`).

## Summary

B is judgment work on PDFs: schemes and pinouts. Its risk is a wrong pinout,
caught by the structure check and a hand check per distinct pinout. After B
nothing in the tree changes except `planning/plan-40-data/`,
`datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` and the
placed PDFs.
