# plan-47-B: Inductors, fuses and PTCs — ordering tables, datasheet status, pinouts and facts

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-A

B reads the datasheets A placed for the inductor, fuse and PTC rows of the
download set and gives each row its verdicts: is the part in the
datasheet's ordering table, does the datasheet say it is obsolete, what are
its pins, and do DigiKey's facts agree with the datasheet. It writes the
ordering-code scheme, the pinout and the fact columns of the matching
`specs.tsv` rows. The behavioral outcome: every inductor, fuse and PTC row
of the download set is `ok`, `skip` or `out` with a reason, and their
`specs.tsv` rows have every column filled. The final pick (`add`) is C's,
after the relay and buzzer verdicts.

References:

- plan-47-A § 3 (the inclusion rule), § 4 (placement), the `specs.tsv`
  skeleton; `planning/plan-47-data/candidates.tsv` columns (`README.md`).
- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` § Parts (`package`,
  `flags`).
- `planning/completed/plan-40-B-transistor-verdicts.md` and
  `planning/plan-40-data/census/` (`scheme-check.py`, `verdicts.py`,
  `pinout-check.py`: the scripts B adapts); `planning/completed/plan-44-B-crystal-verdicts.md`
  (the same method on two-terminal parts).
- plan-47-I § 3.1 (the `inductor` and `fuse` kinds' keys: the facts B must
  deliver).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-A complete | `ls planning/completed/plan-47-A-*.md` → exists | NOT MET (2026-11-16: not started) |
| Every download-set row is stock-checked | `awk -F'\t' 'NR>1 && ($23=="candidate" \|\| $23=="reserve") && $29==""' planning/plan-47-data/candidates.tsv \| wc -l` → 0 | NOT MET (2026-11-16: 3522, no row checked yet) |
| The PDFs are placed | `python3 -I planning/plan-47-data/census/place.py --verify` → `0 missing, 0 unused` | NOT MET (2026-11-16: no PDF placed yet) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-47-data/census/scheme-check.py inductor fuse ptc`
  prints `<n> rows: <k> ok, 0 unchecked` with `<n>` = the inductor, fuse and
  PTC rows of the download set after A (2213 at planning time: 750 + 753 +
  710); every row not `ok` is `skip` or `out` with a `reason`.
- `specs.tsv` has `scheme`, `label`, `page`, `pinout`, `terminals` and the
  fact columns of § 3.4 filled on every inductor, fuse and PTC row (777 at
  planning time: inductor 418, fuse 237, PTC 122; `python3` over
  `candidates.tsv`, the (type, maker, series, url, package) keys of the
  download set).
- No verdict is `add` yet and no PDF is deleted (C Phase 3 and 4).

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No DigiKey call: a row that was not stock-checked by A is not judged.
- No datasheet is downloaded here; a row without a PDF is `skip`, reason `no
  datasheet on the maker's site`.
- A fact DigiKey gives and the datasheet contradicts is not corrected by
  hand: the row is `skip` (§ 3.4).

## 2. Current State

After A, `candidates.tsv` has `pdf`, `hit` and `dead` on every download-set
row and `specs.tsv` has the skeleton rows. Nothing has a verdict.

### Measured populations

| What | Count | Command |
|---|---|---|
| Download-set rows of B | 2213: inductor 750, fuse 753, PTC 710 | `awk -F'\t' 'NR>1 && ($23=="candidate" \|\| $23=="reserve"){print $3}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` |
| `specs.tsv` rows of B | 777: inductor 418 (Bourns 116, Murata 52, Würth 250), fuse 237 (Littelfuse 116, Eaton 71, SCHURTER 50), PTC 122 (Littelfuse 68, Bourns 30, Bel Fuse 24) | the distinct (type, company, series, url, package) of the download set |
| Distinct PDFs | inductor 416, fuse 204, PTC 101 | the distinct `url` per type of the download set |
| One-PDF-per-part rows | Würth 250 rows, 250 PDFs | `awk -F'\t' 'NR>1 && $2=="wurth" && ($23=="candidate" \|\| $23=="reserve"){print $22}' planning/plan-47-data/candidates.tsv \| sort -u \| wc -l` → 250 |
| DigiKey packages, inductors | Nonstandard 200, 0402 150, 0201 63, 0603 54, radial vertical cylinder 45, 1008 37, axial 33, 0805 25, radial vertical (open) 22, 0806 18, radial horizontal cylinder (open) 15, 2-SMD 13, 1210 12, the rest under 12 each | `awk -F'\t' 'NR>1 && $3=="inductor" && ($23=="candidate" \|\| $23=="reserve"){print $10}' … \| sort \| uniq -c \| sort -rn` |
| …fuses | 5 × 20 mm 164, 2-SMD square end block 109, 3AB/3AG 100, 5 × 20 mm axial 73, 1206 66, 3AB/3AG axial 50, radial box 44, 0603 38, 2-SMD J-lead 33, radial can 23, non-standard axial cartridge 16, axial 13, the rest under 10 each | same, `$3=="fuse"` |
| …PTCs | radial disc 317, 1206 73, 1812 64, 1210 57, 2920 47, 0805 44, 0603 43, 2-SMD 35, 2016 12, 0402 8, 2018 5, the rest under 5 each | same, `$3=="ptc"` |
| Fuse response times (DigiKey) | fast 388, slow 321, medium 27, none given 17 | `awk -F'\t' 'NR>1 && $3=="fuse" && ($23=="candidate" \|\| $23=="reserve"){print $9}' … \| sort \| uniq -c` |
| Inductor shielding (DigiKey) | unshielded 425, shielded 300, semi-shielded 25 | same, `$3=="inductor"`, `$15` |

## 3. Design

### 3.1 Ordering-code schemes

`specs.tsv`'s `scheme` is a regex over the part number, written from the
datasheet's ordering or part-numbering table: the literal series, then one
group per option with the characters the table allows (the value code, the
tolerance letter, the packing code). `census/scheme-check.py` (adapted from
plan-40's; it takes the types to check) runs every download-set row against
its specs row: a row printed verbatim in its PDF (`hit` 1) is `ok` whatever
the scheme says; else it must match fully; else `skip`, reason `not in the
datasheet's ordering table`. The scheme of a one-part PDF (every Würth
datasheet) is the literal word `verbatim`: the row must be a `hit`.

A value group is checked where the part number carries the value in a
standard code: an inductor's three-character code (`100` = 10 µH, `4R7` =
4.7 µH, `R47` = 0.47 µH, `10N` = 10 nH; group name `uh` or `nh`), a fuse's
current code (`1.25`, `01.6`, `.500`; group name `amps`), a PTC's hold
current in hundredths of an amp (`050` = 0.50 A; group name `hold`). The
parsed value must equal the row's `value`. A series that codes its value
another way has a `valuemap` of its datasheet's table in the `scheme` column
(`<regex>;<code>=<value>,…`).

### 3.2 Status

A `dead` = 1 row is read in its PDF: text that says obsolete, discontinued,
NRND, "not recommended for new designs" or end of life → `out`, reason
`obsolete`; a `dead` row the datasheet clears gets `dead` 0. The deletion of
all-obsolete PDFs is C's (it needs every type's verdicts).

### 3.3 Pinouts, terminals and labels

`pinout` is the whole pin list in pad order, `<designator>=<name>` joined by
commas; `terminals` is always `1,2` (the two terminal pin names of plan-47-I
§ 3.1).

- a two-pad or two-lead part: `1=1,2=2`;
- an inductor with four pads of which two are the winding and two are
  mechanical or shield pads: `1=1,2=NC,3=2,4=NC` in the datasheet's pad
  order (role `nc`);
- an inductor with two pads per terminal: `1=1,2=1,3=2,4=2` in the
  datasheet's pad order;
- a part whose datasheet shows more than two electrical terminals (a coupled
  or tapped inductor, a three-terminal fuse) → `out`, reason `more than two
  terminals`;
- a cartridge fuse: `1=1,2=2` (the two end caps; the footprint is the clip
  pair of plan-47-A § 6);
- a polarized part does not exist in these three types: pin 1 is the pad the
  datasheet numbers 1, or the left pad of its land pattern when it numbers
  none.

`label` is the part's `"package"` (the family; `docs/parts-format.md` § Parts
forbids pin counts and sizes other than the chip code): the chip code for a
chip package (`0201` … `2920`, from DigiKey's Package / Case), `SMD` for
every other surface-mount package (`Nonstandard`, `2-SMD`, square end block,
J-lead), `Radial` for radial leads (cylinder, box, can, disc), `Axial` for
axial leads (including cartridges with axial leads), `Cartridge` for a 5 ×
20 mm or 6.3 × 32 mm fuse on clips.

`census/pinout-check.py` (adapted from plan-40's) checks: exactly the two
terminal names, every other pin `NC`, the pin count equal to the pad count
of the datasheet's land pattern or lead drawing (the `pads` column B fills).

### 3.4 Facts

The kinds' facts (plan-47-I § 3.1) come from the row's DigiKey parameters
(`raw/*.jsonl.gz`), checked against the series' datasheet:

| Type | Fact (part key) | DigiKey parameter | Check against the datasheet (`specs.tsv` column) |
|---|---|---|---|
| inductor | `henries` | Inductance | in the series' value list or range (`values`) |
| inductor | `tolerance` | Tolerance | one of the series' tolerances (`tols`) |
| inductor | `amps`, `isat` | Current Rating (Amps), Current - Saturation (Isat) | not above the table's largest (`amps_max`); left out when DigiKey gives `-` |
| inductor | `dcr` | DC Resistance (DCR), the "Max" figure | not above the table's largest (`dcr_max`) |
| inductor | `shielding` | Shielding | the series' construction (`shield`) |
| fuse | `amps` | Current Rating (Amps) | in the series' rating list (`values`) |
| fuse | `speed` | Response Time (Fast / Medium / Slow Blow) | the series' characteristic (`speed`); a row DigiKey gives no response time takes the datasheet's (F, FF, M → `fast` / `medium`; T, TT → `slow`) |
| fuse | `voltsAc`, `voltsDc` | Voltage Rating - AC, - DC | the series' ratings for that current (`vac`, `vdc`) |
| fuse | `breaking` | Breaking Capacity @ Rated Voltage (the first figure) | not above the table's largest (`break_max`); left out when DigiKey gives none |
| PTC | `hold`, `trip` | Current - Hold (Ih) (Max), Current - Trip (It) | the part's line of the electrical table (`values` = the hold list) |
| PTC | `volts`, `imax` | Voltage - Max, Current - Max | the part's line (`vmax`, `imax`) |

`scheme-check.py --facts` compares each row; a value outside the datasheet's
→ `skip`, reason `fact not in datasheet`; a required fact DigiKey leaves
blank (2 fuse rows and 2 PTC rows of the planning pick have no current) →
`skip`, reason `fact not given`. The page of the table goes in
`page` and into the device `note` (plan-47-K). A PTC's "240V (265V Int)"
style voltage is its first number.

Risk: the ordering regex of a series with many options (Bourns SRR / SRN
families, Littelfuse 0451 / 0452 with their packing suffixes, PolySwitch
names with `/` and `-2` suffixes); a regex that is too loose adds a part the
datasheet does not list, one that is too tight skips a real one. The checks
are the verbatim `hit` rows, the `--sample` list (20 random `ok` rows per
maker and type printed with their regex for a hand check) and the value
group.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools, and the inductors

- [ ] Adapt `census/scheme-check.py` and `census/pinout-check.py` from
      `planning/plan-40-data/census/` to this plan's `candidates.tsv`
      columns and to `specs.tsv`'s (§ 3.1, § 3.3, § 3.4); both take type
      names.
- [ ] Write `scheme`, `page`, `label`, `pinout`, `terminals`, `pads` and the
      inductor fact columns on the 418 inductor rows of `specs.tsv` (Würth:
      `verbatim`) from the placed PDFs (`pdftotext -layout`).
- [ ] Run `scheme-check.py inductor`, `--facts` and `pinout-check.py
      inductor`; set `skip` / `out` rows with their reasons; print
      `--sample` and compare 20 rows per maker with their PDF by hand; read
      every `dead` = 1 inductor row (§ 3.2).

Acceptance: every inductor row of the download set has its verdicts.
  Check: `python3 -I planning/plan-47-data/census/scheme-check.py inductor` → `<n> rows: <k> ok, 0 unchecked`; `python3 -I planning/plan-47-data/census/pinout-check.py inductor` → `<m> specs rows: 0 errors`; `python3 -I planning/plan-47-data/census/scheme-check.py inductor --sample \| wc -l` → 60 (est. 5 min).
Commit: —

### Phase 2 — The fuses

- [ ] The 237 fuse rows of `specs.tsv` as Phase 1 (§ 3.1, § 3.3, § 3.4),
      then the three checks, the sample and the `dead` rows.

Acceptance: every fuse row of the download set has its verdicts.
  Check: `python3 -I planning/plan-47-data/census/scheme-check.py fuse` → `<n> rows: <k> ok, 0 unchecked`; `…/pinout-check.py fuse` → `0 errors` (est. 3 min).
Commit: —

### Phase 3 — The PTCs

- [ ] The 122 PTC rows of `specs.tsv` as Phase 1, then the three checks, the
      sample and the `dead` rows.

Acceptance: every PTC row of the download set has its verdicts.
  Check: `python3 -I planning/plan-47-data/census/scheme-check.py ptc` → `<n> rows: <k> ok, 0 unchecked`; `…/pinout-check.py ptc` → `0 errors`; and for the letter `python3 -I planning/plan-47-data/census/scheme-check.py inductor fuse ptc` → `<n> rows: <k> ok, 0 unchecked` with `<n>` the three types' download-set rows (est. 3 min).
Commit: —

## Validation Plan

- Tests: none (data only); `scheme-check.py` and `pinout-check.py` are the
  checks, with the hand-compared samples.
- Coverage check: `scheme-check.py` prints `0 unchecked`: every download-set
  row of the three types met a specs row.
- Runtime proof: n/a.
- Doc sync: none (the skip and obsolete files are C's).
- Final gate: plan-47-O.

## Decisions

- **A verbatim hit beats the regex**, and a one-part PDF needs no regex
  (`verbatim`).
- **DigiKey's facts are used only when the datasheet agrees** (§ 3.4); a
  disagreement skips the row instead of picking a side.
- **AEC-Q200 rows are judged like the others**; plan-47-K writes their
  `"flags": ["automotive"]`.
- **`label`s** (§ 3.3): chip codes as the resistor and capacitor parts use
  them, `SMD`, `Radial`, `Axial`, and `Cartridge` (new).
- **The reserve rows get verdicts too**, so C can refill a skipped slot
  without coming back.

## Corrections

## Summary

B's risk is a wrong ordering regex or a mis-read fact table across 777
(series, datasheet) rows; the verbatim rule, the value group and the
per-maker samples bound it. Untouched: every part file, the footprints, the
backend, the app and DigiKey.
