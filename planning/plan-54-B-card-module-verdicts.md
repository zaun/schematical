# plan-54-B: Memory / SIM card sockets and module sockets — ordering tables, datasheet status, pin names and facts, the final pick

Last updated: 2026-11-22
Effort: large (3h–1d)
Depends on: plan-54-A

B reads the datasheets A placed for the **memory / SIM card sockets** (134
candidate rows: Same Sky 74, Amphenol ICC 32, GCT 28) and the **module
sockets** (122: TE Connectivity 75, Amphenol ICC 43, Molex 4) and decides, row by
row, which parts are added. It writes the ordering-code scheme of every
(maker, series, PDF) in `specs.tsv`, reads each datasheet's status, writes
each row's pin names and facts, checks each part's facts against the
datasheet, fills the final pick (`add`), records the skips and the obsolete
PDFs, deletes the PDFs no `add` row uses, and writes `footprints-needed.txt`
for E. The behavioral outcome: `candidates.tsv` has one `add` row per card or
module socket the library will get, every other row of those two families is
`skip` or `out` with a reason, and `specs.tsv` has every column filled on
their 194 rows.

References:

- plan-54-A § 3 (the inclusion rule), § 4 (placement), `specs.tsv` skeleton.
- `AGENTS.md` (§ Datasheets); `datasheets/skipped-parts.md`,
  `datasheets/obsolete-not-included.md`.
- `planning/completed/plan-44-B-crystal-verdicts.md` and
  `planning/plan-44-data/census/` (`scheme-check.py`, `specs.py`, `spec-rows.py`,
  `assign.py`, `pinout-check.py`, `final.py`, `footprint-lines.py`: the scripts B
  adapts; `planning/plan-44-data/specs-parts/README.md`: the readers' brief).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-A complete | `ls planning/completed/plan-54-A-*.md` → exists | NOT MET (2026-11-22: not started) |
| Every candidate row is stock-checked | `awk -F'\t' '$20=="candidate" && $25==""' planning/plan-54-data/candidates.tsv \| wc -l` → 0 | NOT MET (2026-11-22: 1270 rows unchecked; A Phase 2) |
| The PDFs are placed | `python3 -I planning/plan-54-data/census/place.py --verify planning/plan-54-data/candidates.tsv` → `0 missing, 0 unused` | NOT MET (2026-11-22: A Phase 3) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-54-data/census/scheme-check.py --family card --family module`
  prints `<n> rows: <k> ok, 0 unchecked` with `<n>` = the card and module
  candidate rows after A (256 at planning time); every row not `ok` is `skip`
  or `out` with a `reason`.
- `specs.tsv` has `scheme`, `label`, `card`, `pinout`, `positions`, `pitch` and
  `status` filled on every `card` and `module` row (194 at planning time: card
  119, module 75, `census/report.py` → `specs rows`).
- `candidates.tsv` has `state` `add` on the final pick: every card and module
  row that passes the rules of A § 3 (no quota: owner, plan-54-A § Decisions).
- `footprints-needed.txt` has the lines of the `add` rows of these two
  families (`python3 -I planning/plan-54-data/census/footprint-lines.py planning/plan-54-data/candidates.tsv`);
  E draws from it.
- `datasheets/skipped-parts.md` has a `## Card slots (plan-54)` section for the
  skips; every PDF whose rows are all out for datasheet status is deleted and
  listed in `datasheets/obsolete-not-included.md`; every PDF under
  `datasheets/connector-card-socket/` and `datasheets/connector-memory-module/`
  is used by at least one `add` row.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part of the card-edge family is judged here (C and D).
- No obsolete part is added with a flag: only Active ones (owner).

## 2. Current State

A leaves `candidates.tsv` with the stock check and `pdf` / `hit` / `dead` set
and `specs.tsv` as a skeleton (`family`, `maker`, `series`, `pdf`, `rows`,
`example`; the rest empty). The datasheets are of three kinds: Same Sky and
GCT per-part or per-series PDFs with a "part number key" and a pin table;
Amphenol ICC and TE drawings with a part-number table and a pin / contact
list; Molex sales drawings (one part each).

### Measured populations

| What | Count | Command |
|---|---|---|
| Memory / SIM candidate rows, specs rows | **134**, **119** | `python3 -I planning/plan-54-data/census/report.py planning/plan-54-data/raw planning/plan-54-data/candidates.tsv` → the `card:` line |
| Module candidate rows, specs rows | **122**, **75** | same, the `module:` line |
| Card types of the card rows | Nano SIM 25, Micro SIM 23, Secure Digital – SD 23, microSD 17, SIM Card 15, Secure Digital – microSD 10, Smart Card 10, Mini SIM 6, SIM / SAM 4, microSD + UFS 1 | `awk -F'\t' '$20=="candidate" && $4=="card"{print $13}' planning/plan-54-data/candidates.tsv \| sort \| uniq -c` |
| Styles of the module rows | SODIMM 59, DIMM 54, SIMM 7, MiniDIMM 1, SOCAMM 1 | same with `$4=="module"` |
| Positions of the module rows | 72 to 694 (110 of the 122 above 196) | `awk -F'\t' '$20=="candidate" && $4=="module"{print $8}' planning/plan-54-data/candidates.tsv \| sort -n \| uniq -c` |
| Mounting of the card rows | surface mount right angle 125, through hole right angle 5, mixed 4 | same with `$11` |
| Footprint keys the two families need | **87** card, **30** module | `census/report.py` |

### Verified properties

- DigiKey's Number of Positions of a card socket counts contacts and switch
  contacts as `10 (8 + 2)`; the datasheet's pad list also has shell tabs. The
  facts check therefore compares contacts + switch contacts with DigiKey's
  number, and the shell tabs make one more pin (`SHIELD`).
- DigiKey gives the module sockets no Pitch parameter (`gzip -dc
  planning/plan-54-data/raw/413-17.jsonl.gz | head -1` has no `Pitch` key), so
  the pitch is read from the drawing.
- The pin-name conventions of the library's other connectors: USB pins are
  passive `inout` terminals named by function, `nc` for a pad the datasheet
  marks not connected, one `SHIELD` pin for the shell (`docs/parts-format.md`
  § `connector`).

## 3. Design

### 3.1 The scheme

Per `specs.tsv` row (maker, series, PDF) a `scheme` is a regular expression
over the part numbers the datasheet's ordering key or part-number table allows
(`^SIM8055-6-1-14-00-A$`, `^MSD-[0-9]-[AB](-[A-Z0-9]+)?$`, a table of dash
numbers as an alternation). `scheme-check.py` checks every candidate row of a
specs row against it (a part number that matches no scheme is `skip`, reason
`not in the datasheet's ordering table`) and that no part number matches two
rows' schemes. A row whose PDF prints no ordering key or table for its part
number is `skip`, reason `not in the datasheet`.

### 3.2 Status

Every `dead` = 1 row, and every PDF scanned whole for `obsolete`,
`discontinued`, `not recommended for new designs`, `last time buy`, `end of
life`, `EOL`, is read in context. A part the datasheet marks with any of them is
`skip`, reason `obsolete by the datasheet` (not added with a flag: owner);
a PDF whose every row is skipped for it is deleted and listed in
`datasheets/obsolete-not-included.md`.

### 3.3 Pin names and pinouts

`pinout` is the list `1=NAME,2=NAME,…` of every electrically distinct pad the
datasheet's pin table or drawing shows, in the datasheet's numbering:

- **card sockets**: the signal names as the datasheet prints them (SD: `DAT2`,
  `CD/DAT3`, `CMD`, `VDD`, `CLK`, `VSS`, `DAT0`, `DAT1`; SIM: `VCC`, `RST`,
  `CLK`, `GND`, `VPP`, `I/O`), the card-detect and write-protect contacts as the
  datasheet names them (`CD`, `WP`, `COM`, `DETECT`), and one `SHIELD` pin for the
  shell tabs; a contact the datasheet marks not connected is `NC` (role `nc`);
  every other pin is a passive `inout` terminal. The pin count is DigiKey's
  contacts + switch contacts + the shell pin (when the shell has a pad);
- **module sockets**: the contact designators the drawing prints, pins numbered
  1 … N in the drawing's order (front side then back side), named by their
  designator (`1`, `2`, … `288`), all `inout`; the mounting pegs and
  board-lock holes are footprint pads with no pin.

Pins of a pinout with more than 40 pins carry `"side": "in"` on the first
half (rounded up), so the block symbol has two columns (the default puts every
`inout` pin on the right); the pinout column stores the plain list and J adds
the key.

### 3.4 Facts

`card` is the text the sidebar shows: card sockets — the card type as
DigiKey gives it with `™` dropped and the words of the standard (`SD`,
`microSD`, `SIM`, `Micro SIM`, `Nano SIM`, `Mini SIM`, `Smart card`,
`SIM / SAM`, `microSD / UFS`); module sockets — `<generation> <style>` from
DigiKey's Memory Type and Connector Style (`DDR4 SODIMM`, `DDR5 DIMM`); the style
alone when DigiKey gives no Memory Type. `pitch` (mm) is the contact pitch of
the drawing, read for module sockets only (card sockets carry no pitch). `label`
is the device's short name (`<series> <card>`). Each fact is compared with the
datasheet; a part whose datasheet gives another value is `skip`, reason `fact
not in datasheet`.

**Risk**: the pin tables of the 119 card-socket rows (each series numbers its
contacts differently) and the module drawings that list contacts per side; the
checks of Phase 3 compare each pin count with DigiKey's number and list every
difference for a second reading.

Rejected alternative: naming card-socket pins `1 … N` like the module sockets
(rejected: the datasheets name them by function, and a user wires `CLK` and
`CMD`, not pin 5).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Ordering-code schemes and facts

- [ ] Adapt `census/specs.py`, `scheme-check.py`, `spec-rows.py`, `assign.py` from
      plan-44's to the columns of this plan's `candidates.tsv` and `specs.tsv`
      (`--family card`, `--family module`).
- [ ] Six parallel readers (the brief is `planning/plan-54-data/specs-parts/README.md`:
      the plan-44 brief with § 3.1 – 3.4 here; fragments `card-samesky`,
      `card-amphenol`, `card-gct`, `module-te`, `module-amphenol`, `module-molex`)
      fill `scheme`, `label`, `card`, `pinout`, `positions`, `pitch`, `status` of
      the 194 rows from the placed PDFs (`pdftotext -layout`); `assign.py --merge`.
- [ ] Run `scheme-check.py`; every row that fails is `skip` (`reason` as § 3.1 /
      § 3.4). Print the `--sample` and compare 20 rows per maker with their PDF by
      hand.
- [ ] Apply plan-54-A § 3 rule 9 to the drawings: a row whose land pattern needs a
      pad on the bottom layer, a rotated slot or a keyhole shell slot is `skip`
      (`footprint not expressible: <feature>`); record the number in § Corrections.

Acceptance: every card and module row has a verdict on the ordering table and the facts.
  Check: `python3 -I planning/plan-54-data/census/scheme-check.py --family card --family module` → `<n> rows: <k> ok, 0 unchecked`, then `python3 -I planning/plan-54-data/census/scheme-check.py --family card --family module --sample \| wc -l` → 60 (est. 10 min of scripts; the reading is the work).
Commit: —

### Phase 2 — Datasheet status

- [ ] Read every `dead` = 1 row of the two families in its PDF (§ 3.2); set
      `skip` / `obsolete by the datasheet` rows. Scan every placed PDF whole for
      the status words (`pdftotext`, context printed), since `dead` only looks
      300 characters around a part number; record the PDFs read and kept.
- [ ] Delete each PDF whose every row is `skip` for status, remove it from
      its `datasheets.tsv`, add it to `datasheets/obsolete-not-included.md`
      (PDF name, `connector`, `Card slot → <leaf>`, the page and the words).

Acceptance: no `add`-eligible row is obsolete; the obsolete PDFs are deleted.
  Check: `awk -F'\t' '($4=="card"||$4=="module") && $24==1 && $20!="out" && $20!="skip"' planning/plan-54-data/candidates.tsv \| wc -l` → 0 once each `dead` row has its verdict (a `dead` row the datasheet clears gets `dead` 0), and `grep -c 'plan-54' datasheets/obsolete-not-included.md` → the PDFs deleted (est. 20 min).
Commit: —

### Phase 3 — Pin names and pinouts

- [ ] Adapt `census/pinout-check.py`: for every `specs.tsv` row of the two
      families, the pin names of § 3.3 are unique (a repeated name only for `NC`,
      `GND`, `VSS`, `SHIELD` as USB parts do), the pinout has the pin count the
      row's candidate rows give (card: DigiKey contacts + switch contacts + the
      shell pin when the PDF has a shell pad; module: DigiKey positions), and
      `positions` lists the counts.
- [ ] Run it; list every difference between a pinout and DigiKey's number and
      read it again in the PDF; a difference the datasheet confirms is recorded
      in `specs.tsv` `status` (`pins n, digikey m`) and in § Corrections.

Acceptance: every specs row has a pinout the kind will accept.
  Check: `python3 -I planning/plan-54-data/census/pinout-check.py --family card --family module` → `<n> rows, 0 bad` with `<n>` = the two families' `specs.tsv` rows after Phase 2 (est. 5 min).
Commit: —

### Phase 4 — The final pick, the skips, the prune

- [ ] `census/final.py` (plan-44's, no quota): every row of the two families that
      passes § 3 and A § 3 becomes `add`; the rest stay `skip` / `out`. Print the
      counts per (family, maker).
- [ ] Write `datasheets/skipped-parts.md` § `## Card slots (plan-54)`: one row
      per `skip` row group (part numbers, device, "would have gone into"
      `connector` / Connector → Card slot → …, reason: `no datasheet on the maker's
      site` with the host, `not in the datasheet's ordering table`, `fact not in
      datasheet`, `obsolete by the datasheet`). `final.py --write-skipped`.
- [ ] Delete every PDF no `add` row uses (`place.py --prune`) from
      `datasheets/connector-card-socket/` and `datasheets/connector-memory-module/`;
      rewrite their `datasheets.tsv`.
- [ ] `census/final.py --verify-skipped` prints `<n> skip rows, 0 missing from
      skipped-parts.md`.
- [ ] Write `census/footprint-lines.py` (plan-44's, the two families) and
      `footprints-needed.txt`: one line per variant — letter `E`, family, maker,
      series, mounting, card, pitch, the position counts, parts, example, datasheet,
      the existing footprints of the same pad count (none at planning), the id stem
      (`SOCK-<MAKER>-<series slug>` or `DIMM-<MAKER>-<series slug>`).

Acceptance: the pick is final, consistent and pruned.
  Check: `awk -F'\t' '$20=="add" && ($4=="card"||$4=="module")' planning/plan-54-data/candidates.tsv \| wc -l` → the final pick (≤ 256); `python3 -I planning/plan-54-data/census/place.py --verify planning/plan-54-data/candidates.tsv` → `0 missing, 0 unused`; `python3 -I planning/plan-54-data/census/final.py --verify-skipped` → `0 missing from skipped-parts.md` (est. 20 min).
Commit: —

## Validation Plan

- Tests: the scripts of § Phases (each prints counts and a zero-failure line);
  no code changes.
- Coverage check: every `candidate` row of the two families has a verdict:
  `awk -F'\t' '($4=="card"||$4=="module") && $20=="candidate"' planning/plan-54-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (data only).
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: none; the landing tier is M's.

## Decisions

- Owner (2026-11-22): **only Active card slots** — a part the datasheet marks
  obsolete is skipped, not flagged (§ 3.2).
- Owner (2026-11-22): **no quota and no generic part**; every passing row is added.
- Author: **Card-socket pins are named by function, module pins by designator**
  (§ 3.3), because a datasheet names SD / SIM contacts by function and numbers
  a DIMM's 288 contacts without names. Rejected: numbers for both.
- Author: **One `SHIELD` pin for the shell tabs, no pin for mounting pegs**,
  as the USB connectors (`docs/parts-format.md` § `connector`).

## Corrections

## Summary

B is reading: 194 specs rows, six readers, one pin table each. The risk is a
wrong pin name or count, which `pinout-check.py` and the facts check bound; the
pick is every row that passes, so a shortfall is not a failure.
