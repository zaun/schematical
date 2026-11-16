# plan-44-B: Crystals — ordering tables, datasheet status, pinouts and facts, the final pick

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-44-A

B reads the datasheets A placed and decides, row by row of the download set,
which parts are added. It writes the ordering-code scheme of every (maker,
series, PDF) in `specs.tsv`, reads each datasheet's status, writes each
(series, package)'s pinout and terminals, checks each part's facts against the
datasheet, fills the final pick (`add`), records the skips and the obsolete
PDFs, deletes the PDFs no `add` row uses, and regenerates
`footprints-needed.txt` from the `add` rows. The behavioral outcome:
`candidates.tsv` has one `add` row per part the library will get, every other
download-set row is `skip` or `out` with a reason, and `specs.tsv` has every
column filled.

References:

- plan-44-A § 3 (the inclusion rule), § 4 (placement), `specs.tsv` skeleton.
- `AGENTS.md` (§ Datasheets); `datasheets/skipped-parts.md`,
  `datasheets/obsolete-not-included.md`.
- `planning/completed/plan-40-B-transistor-verdicts.md` and
  `planning/plan-40-data/census/` (`scheme-check.py`, `verdicts.py`,
  `pinout-check.py`: the scripts B adapts).

## Prerequisites

plan-44-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-44-A complete | `ls planning/completed/plan-44-A-*.md` → exists | NOT MET (2026-11-16: not started) |
| Every download-set row is stock-checked | `awk -F'\t' '$25!="out" && NR>1 && $31==""' planning/plan-44-data/candidates.tsv \| wc -l` → 0 | NOT MET (2026-11-16: 744, no row checked yet) |
| The PDFs are placed | `python3 -I planning/plan-44-data/census/place.py --verify` → `0 missing, 0 unused` | NOT MET (2026-11-16: no PDF placed yet) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-44-data/census/scheme-check.py` prints `<n> rows:
  <k> ok, 0 unchecked` with `<n>` = the rows of the download set after A (744 at
  planning time); every row not `ok` is `skip` or `out` with a `reason`.
- `specs.tsv` has `scheme`, `label`, `pinout`, `terminals` and `capacitors`
  filled on every row (196 rows at planning time: Abracon 82, ECS 102,
  Suntsu 12, `python3` over `candidates.tsv`, the (maker, series, url,
  package) keys of the download set).
- `candidates.tsv` has `state` `add` on the final pick: per (maker, class) the
  first `quota` passing rows in rank order (candidate rows first, then
  reserve rows), quotas of A § 3 rule 5; a shortfall is a number in § Corrections.
- `footprints-needed.txt` is regenerated from the `add` rows
  (`python3 -I planning/plan-44-data/census/footprint-lines.py planning/plan-44-data/candidates.tsv`);
  C and D draw from it.
- `datasheets/skipped-parts.md` has a `## Crystals and resonators (plan-44)`
  section for the skips; every PDF whose rows are all obsolete is deleted and
  listed in `datasheets/obsolete-not-included.md`; every PDF under
  `datasheets/crystal/` is used by at least one `add` row.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No new DigiKey listing and no part added beyond the download set.
- No footprint drawing (C and D) and no part JSON (F).

## 2. Current State

A leaves `candidates.tsv` with `pdf`, `hit` and `dead` filled and `specs.tsv`
as a skeleton. The `scheme` of a row is a Python regular expression that must
`re.fullmatch` a part number the datasheet's ordering table allows.

### Measured populations

| What | Count | Command |
|---|---|---|
| Download-set rows | 744 (candidate 600, reserve 144) | `awk -F'\t' '$25!="out" && NR>1' planning/plan-44-data/candidates.tsv \| wc -l` |
| `specs.tsv` rows (maker, series, URL, package) | 196: Abracon 82, ECS 102, Suntsu 12; 180 series, 191 URLs | the planning-time keys of the download set (`python3 -I` over `candidates.tsv`, `(maker, series, url, package)`) |
| Package / Case of the crystals in the download set | 4-SMD, No Lead 343; 2-SMD, No Lead 154; HC-49/US 73; 4-SOJ, 5.50mm pitch 37; Cylindrical Can, Radial 26; HC-49/U 8; HC-49/US - 3 Lead 6; 2-SMD, Flat Lead 6; HC-49/US - 4 Lead 4; 4-SMD, Flat Lead, 3 Leads 3; HC-46/X 3; 3-SMD, No Lead 2; 4-SOJ, 9.40mm pitch 2; `-` 1 | `awk -F'\t' '$25!="out" && NR>1 && $3!="resonator"{print $14}' planning/plan-44-data/candidates.tsv \| sort \| uniq -c \| sort -rn` |
| Ceramic resonators in the download set | 76 (Abracon 50, ECS 26) | `awk -F'\t' '$25!="out" && NR>1 && $3=="resonator"' … \| wc -l` |
| Distinct load capacitances among the picked rows | 17 values, plus 68 rows with none (series-resonant parts) | `awk -F'\t' '$25=="candidate"{print $8}' … \| sort -u \| wc -l` |

### Verified properties

- **The DigiKey facts are columns of `candidates.tsv`**: `hz` (7), `load_pf` (8,
  blank when DigiKey's Load Capacitance is "Series"), `tol_ppm` (9),
  `stab_ppm` (10), `esr_ohm` (11) (for a resonator: Impedance). They come
  from DigiKey's `Parameters` (`Frequency`, `Load Capacitance`, `Frequency
  Tolerance`, `Frequency Stability`, `ESR`), `common.read_raw`.
- **Pad counts come from the package text**: DigiKey's "2-SMD", "4-SMD",
  "3-SMD", "HC-49/US - 3 Lead", "HC-49/US - 4 Lead" name the pads; an
  `HC-49/U` or `HC-49/US` with no count is 2 leads, `Cylindrical Can,
  Radial` is 2 leads, a 4-SOJ is 4 pads. The datasheet's pin table decides.

## 3. Design

### 3.1 The scheme

`specs.tsv`'s `scheme` is a regex over the part number, written from the
datasheet's ordering / part-numbering table: literal series, then groups for
each option with the characters the table allows (`(?P<hz>\d+\.\d+MHZ)`, the
load-capacitance code, the tolerance / stability code, the packing code).
A group named `hz`, when the part number carries the frequency as number +
`K`/`KHZ`/`M`/`MHZ`, is parsed (`18.432MHZ` → 18432000 Hz) and must equal the
row's `hz`; a part number with another frequency spelling (an Abracon
`AWSCR-800CV` style code) has a `hzmap` of its datasheet's frequency table
instead, in `specs.tsv`'s `scheme` column as `<regex>;<code>=<Hz>,…`.
`scheme-check.py` (adapted from plan-40's) runs every download-set row against
its specs row: a row printed verbatim in its PDF (`hit` 1) is `ok` whatever
the scheme says; else it must match; else `skip`, reason `not in the
datasheet's ordering table`.

### 3.2 Status

A `dead` = 1 row is read in its PDF: text that says obsolete, discontinued,
NRND, "not recommended for new designs", end of life → `out`, reason
`obsolete`. The rule of `AGENTS.md`: a PDF whose every download-set row is
`out` for status is deleted and listed in `datasheets/obsolete-not-included.md`
(PDF name, family `Crystal` / `Ceramic resonator`, group); a PDF with some
obsolete rows keeps them out (the final pick never adds an obsolete part, so
no `"flags": ["obsolete"]` is written).

### 3.3 Pinouts and terminals

A crystal part has two terminals named `X1` and `X2` (role `inout`). The
`pinout` column is the whole pin list in pad order, `<designator>=<name>`
joined by commas, e.g. `1=X1,2=NC,3=X2,4=NC`. Rules (the same for every
maker, the datasheet decides which pad is which):

- a two-pad part: `1=X1,2=X2`;
- a four-pad SMD crystal: the two crystal pads are `X1` / `X2`, the other two
  `NC` (the datasheet may say "GND or NC"; the pinout still says `NC`, role
  `nc`, and the part `note` carries the datasheet's remark);
- a three-pad ceramic resonator with built-in capacitors, and a three-lead
  HC-49/US: `1=X1,2=GND,3=X2` (the middle pad is the common / ground pad; role
  `nc`, name `GND`), `capacitors` = 1 only for a resonator whose datasheet
  says it has built-in load capacitors;
- a two-pin ceramic resonator without capacitors: `1=X1,2=X2`;
- a four-lead HC-49/US and a SOJ-4: as the four-pad SMD crystal.

`label` is the part's `"package"` (the package family the drawer shows;
`docs/parts-format.md` forbids sizes and pin counts in it): from DigiKey's
Package / Case, `SMD` for every `2-SMD`, `3-SMD`, `4-SMD`, `1512 (3731
Metric)` and flat-lead package, `HC-49/US` for `HC-49/US` and its 3- and
4-lead versions, `HC-49/U`, `HC-46/X`, `SOJ` for `4-SOJ`, `Cylinder` for
`Cylindrical Can, Radial`; the one `-` row gets the datasheet's package
family.

`terminals` is always `X1,X2`; it exists as a column because the backend kind
takes it from the device (plan-44-E), and `pinout-check.py` (adapted from
plan-40's) checks: exactly two `X1` / `X2` pins, every other pin `NC` or one
`GND`, the pin count equals the pad count of the package text or the datasheet's
pin table.

### 3.4 Facts

The kind's facts (plan-44-E) come from the row: `hz` (DigiKey's `hz`),
`loadPf`, `tolerancePpm`, `stabilityPpm`, `esrOhms`. For each (series, fact)
`scheme-check.py --facts` compares the row's value with the set or range the
datasheet gives for the series: `specs.tsv` columns `hz_range` (`min-max`
Hz), `loads` (comma list of pF, `-` for series resonance), `tols`, `stabs`
(ppm lists), `esr_max` (ohms; the largest the table gives for the part's
frequency). A row whose value is outside → `skip`, reason `fact not in
datasheet`. The page of the table goes in `specs.tsv` `page` and into the
part's `note` (plan-44-F).

Risk: the ordering regex of a series with many options (Abracon ABM / ABS
families, ECS CSM / ECX families); a regex that is too loose adds a part the
datasheet does not list, one that is too tight skips a real one. The check is
the verbatim `hit` rows (a row printed in the PDF is always `ok`), the
`--sample` list (20 random `ok` rows per maker printed with their regex for
a hand check) and the DigiKey part number itself.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Ordering-code schemes and facts

- [ ] Write `scheme` (and `hzmap`, `page`, `hz_range`, `loads`, `tols`,
      `stabs`, `esr_max`) on every `specs.tsv` row from the placed PDF
      (`pdftotext -layout`); adapt `census/scheme-check.py` from
      `planning/plan-40-data/census/scheme-check.py` to the columns of
      `candidates.tsv` (§ 3.1, § 3.4).
- [ ] Run `scheme-check.py`; every row that fails is `skip` (`reason` as
      § 3.1 / § 3.4). Print the `--sample` and compare 20 rows per maker with
      their PDF by hand.

Acceptance: every download-set row has a verdict on the ordering table and the facts.
  Check: `python3 -I planning/plan-44-data/census/scheme-check.py` → `<n> rows: <k> ok, 0 unchecked`, then `python3 -I planning/plan-44-data/census/scheme-check.py --sample \| wc -l` → 60 (est. 5 min).
Commit: —

### Phase 2 — Datasheet status

- [ ] Read every `dead` = 1 row of `candidates.tsv` in its PDF (§ 3.2); set
      `out` / `obsolete` rows.
- [ ] Delete each PDF whose every download-set row is `out`, remove it from
      `datasheets/crystal/datasheets.tsv`, add it to
      `datasheets/obsolete-not-included.md` (PDF name, `Crystal` or `Ceramic
      resonator`, group `MHz` / `kHz` / `Ceramic`).

Acceptance: no `add`-eligible row is obsolete; the obsolete PDFs are deleted.
  Check: `awk -F'\t' '$30==1 && $25!="out"' planning/plan-44-data/candidates.tsv \| wc -l` → 0 once each `dead` row has its verdict (a `dead` row the datasheet clears gets `dead` 0 after B has read it), and `grep -c 'plan-44' datasheets/obsolete-not-included.md` → the PDFs deleted (est. 20 min).
Commit: —

### Phase 3 — Pinouts and terminals

- [ ] Write `pinout`, `terminals`, `capacitors` on every `specs.tsv` row (§
      3.3) from the PDF's pin table; adapt `census/pinout-check.py` from
      plan-40's.
- [ ] Run `pinout-check.py`.

Acceptance: every specs row has a pinout the kind will accept.
  Check: `python3 -I planning/plan-44-data/census/pinout-check.py` → `<n> rows, 0 bad` with `<n>` = the `specs.tsv` rows (est. 5 min).
Commit: —

### Phase 4 — The final pick, the skips, the prune

- [ ] `census/final.py`: per (maker, class), the passing rows in rank order,
      candidate rows then reserve rows, become `add` up to the quota; the rest
      stay `out` (reason `below quota`). The footprints of plan-44-F's generic
      parts (§ Decisions) are pinned: before the quota is applied, the
      most-stocked passing row of each pinned label (`footprint-lines.py`
      `label()`) is `add`, and counts against its (maker, class) quota. Print
      the shortfall per (maker, class) and record it in § Corrections.
- [ ] Write `datasheets/skipped-parts.md` § `Crystals and resonators (plan-44)`:
      one row per `skip` row group (part numbers, device, "would have gone
      into" `crystal` / Crystal → kHz | MHz | Ceramic resonator, reason: `no
      datasheet on the maker's site` with the host, `not in the datasheet's
      ordering table`, `fact not in datasheet`).
- [ ] Delete every PDF no `add` row uses (`place.py --prune`); rewrite
      `datasheets/crystal/datasheets.tsv`.
- [ ] `census/final.py --verify-skipped` (prints `<n> skip rows, 0 missing
      from skipped-parts.md`) is the check that every `skip` row is listed.
- [ ] Regenerate `footprints-needed.txt` from the `add` rows.

Acceptance: the pick is final, consistent and pruned.
  Check: `awk -F'\t' '$25=="add"' planning/plan-44-data/candidates.tsv \| wc -l` → the final pick (≤ 600, shortfall recorded); `python3 -I planning/plan-44-data/census/place.py --verify` → `0 missing, 0 unused`; `python3 -I planning/plan-44-data/census/final.py --verify-skipped` → `0 missing from skipped-parts.md`; `awk -F'\t' '!/^#/' planning/plan-44-data/footprints-needed.txt \| awk -F'\t' '{s+=$8} END{print s}'` → the same number (est. 5 min).
Commit: —

## Validation Plan

- Tests: none (data only); the Phase checks above are the gates.
- Coverage check: Phase 1 covers every download-set row, Phase 4's counts tie
  the pick to `footprints-needed.txt`.
- Runtime proof: n/a.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: plan-44-H.

## Decisions

- **The facts of a part are DigiKey's, checked against the datasheet**
  (frequency, load capacitance, tolerance, stability, ESR); a mismatch skips
  the row rather than overriding a value. The datasheet's table is the
  authority, DigiKey's parameter the carrier; a row the table does not cover
  is not a part the library can vouch for.
- **A 4-pad crystal's two other pads are `NC`**, whatever the datasheet's
  grounding advice: the schematic symbol draws two terminals (plan-44-G), the
  footprint keeps all four pads, and the PCB's net lists decide the case
  ground.
- **An AEC-Q200 row is added as a normal part** with `"flags": ["automotive"]`
  written by F (the owner put the grade in scope); it is not a separate group.
- **A three-pad resonator's middle pad is `GND`, role `nc`**, drawn on the
  symbol (plan-44-G), so a schematic can ground it.
- **Twelve footprint labels are pinned** because plan-44-F's generic parts
  choose among them: `XTAL-3215-2`, `XTAL-2012-2`, `XTAL-1610-2`,
  `XTAL-CYL-2X6-2` (32.768 kHz); `XTAL-HC49US-2`, `XTAL-3225-2`,
  `XTAL-5032-2`, `XTAL-6035-2` (8 and 16 MHz); `CRES-3731-3`, `CRES-6030-3`,
  `CRES-3213-3`, `CRES-2520-3` (the resonator). Each has at least one
  download-set line at planning time (`awk -F'\t' '!/^#/{print $12}'
  planning/plan-44-data/footprints-needed.txt | sort -u`).
- **B writes no `"flags": ["obsolete"]`**: obsolete parts are never added.
- **No part is added for a series whose datasheet is missing**; its rows are
  `skip`, reason `no datasheet on the maker's site`.

## Corrections

## Summary

B's risk is the hand-written ordering regexes (a loose one adds parts the
table does not list). The verbatim-hit rule, the sample check and the
fact-range check bound it. Untouched: every file outside `planning/`,
`datasheets/crystal/` and the two datasheet ledgers.
