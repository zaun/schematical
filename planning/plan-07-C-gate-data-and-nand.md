# plan-07-C: Voltage data for AND and NAND gate parts

Last updated: 2026-11-01
Effort: large (3h–1d)
Depends on: plan-07-B

Fills in `vcc` (and `inputMax` where inputs are tolerant) for every part of
the 30 AND and NAND devices in `gate.json`. The values come from each part's
datasheet, Recommended Operating Conditions. This sub-plan also sets the
data method that plan-07-D and plan-07-E repeat.

Outcome: every one of the 1404 AND/NAND parts lists a `|vcc=` field.
Spot values match their datasheets:

- SN74HC00N `vcc=2000-6000,in=6000`
- SN74LVC00AD `vcc=1650-3600,in=5500`
- SN54LS00J `vcc=4500-5500,in=5500`
- CD74AC00E `vcc=1500-5500,in=5500`

References:

- plan-07-A §4.1–4.2 (keys, resolution), `docs/parts-format.md` § `gate`
- `datasheets/gate-and/` (83 PDFs), `datasheets/gate-nand/` (88 PDFs)
- `AGENTS.md` § Datasheets

## Prerequisites

The same as plan-07-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-07-B complete | `ls planning/plan-07-B-* 2>/dev/null` → no output | NOT MET (2026-11-01) |
| `pdftotext` available | `which pdftotext` → `/Users/justinzaun/local/brew/bin/pdftotext` | MET (2026-11-01) |

## 1. Goal

- Every part of every gate device with `"function"` `AND` or `NAND` resolves
  to a `vcc` range: through its family timing entry, or through its own
  `vcc` where its datasheet row differs from the entry's.
- Families with 5 V-tolerant or 3.6 V-tolerant inputs also get
  `inputMax`, taken from the VI row of the datasheet.

### Non-goals (explicit constraints)

- No timing, pinout, footprint or flag change. `traces.txt` stays
  byte-identical.
- Only the `vcc` field changes in `parts.txt`, and only on AND/NAND lines.
- No other gate functions (that is D), and no loader change.

## 2. Current State

- The census was run 2026-11-01 (`python3 -I` over `gate.json`).
- AND has 13 devices and 54 (device, family) pairs. NAND has 17 devices
  and 69 pairs. Together that is 123 timing entries and 1404 parts.
- The parts in one (device, family) pair can come from different
  datasheets with different ranges. Measured with `pdftotext -layout`:
  - **AC.** `cd74ac00` is `1.5 5.5` and `sn74ac00` is `2 6`. Some -Q1
    sheets give `1.5 6`.
  - **LVC quad.** `sn74lvc00a` gives SN54 `2 3.6` and SN74 `1.65 3.6`.
    The -Q1 and -EP sheets give `2 3.6`.
  - **LS/S.** SN54 is `4.5 5.5` and SN74 is `4.75 5.25` (`sn74ls00` p.3).
  - **No 54/74 split** was found for HC, HCS, HCT, AHC, AHCT, ACT, ALS, AS
    or F.
- Each family's usual range, as a cross-check only (the source is always
  the part's own sheet):

  | Family | Usual range |
  |---|---|
  | HC, HCS | 2–6 |
  | HCT, ACT, AHCT | 4.5–5.5 |
  | AHC | 2–5.5, inputs 5.5 |
  | LV-A | 2–5.5, inputs 5.5 |
  | LVC 1G/2G | 1.65–5.5, inputs 5.5 |
  | ALVC | 1.65–3.6, inputs 3.6 |
  | AUC | 0.8–2.7, inputs 3.6 |
  | AUP | 0.8–3.6, inputs 3.6 |
  | CD4000 | 3–18 |

- How to find a pair's datasheet:
  - Gate device `note`s name a datasheet stem (51 of 65 do; for example
    `sn74hc10 p.3`).
  - Otherwise search the text: `grep -l <base number> /tmp/p07/txt/*`.
  - All 305 logic pairs were traced to at least one PDF this way.
- Some datasheets have no text layer. The command below found no
  supply-voltage text in these, so their table has to be read from the
  page image (`Read` with `pages`):
  - gate-and (4): `cd4081b`, `sn74ls08`, `sn74ls11`, `sn74ls21`
  - gate-nand (5): `cd4011b`, `cd4068b`, `sn74ls30`, `sn74s10`, `sn74s20`

  The command was
  `for f in */*.pdf; do pdftotext -layout "$f" - | grep -ci "supply voltage\|VCC\|VDD"; done`
  (0 hits = scanned).

### Measured populations

| What | Count | Command |
|---|---|---|
| AND+NAND devices / parts | 30 / 1404 | python over `gate.json`, functions AND/NAND → `(1404, 30)` |
| AND / NAND (device, family) pairs | 54 / 69 | python census → `{'NAND': 69, 'AND': 54}` |
| PDFs | 83 + 88 | `ls datasheets/gate-and/*.pdf \| wc -l` → 83; gate-nand → 88 |
| scanned PDFs among them | 4 + 5 | the command above |

## 3. Design Overview

This is data entry, with one rule per part (§4).

**Correctness risk:** a part silently inherits an entry value that is wrong
for it. The usual cases are a SN54 or CD part under a SN74-sourced entry, or
a -Q1/-EP part. §4 step 3 makes the implementer list every part name in a
pair against its datasheet row, so this can't be skipped.

**Gate class:** behavior-neutral. `traces.txt` must not change, and
`parts.txt` changes only by an added `|vcc=` on AND/NAND lines.

**Rejected:** copying the family table above without reading the sheets.
The measured splits (AC, LVC, LS) show it would be wrong.

## 4. Method (per device; plan-07-D and -E repeat it)

1. Dump the text once: `mkdir -p /tmp/p07/txt && for f in datasheets/*/*.pdf
   datasheets/*/*.PDF; do pdftotext -layout "$f" "/tmp/p07/txt/$(basename "${f%.*}").txt"; done`.
   This takes about 63 s for all 471 PDFs.
2. For each family entry in the device's `timing`, open its datasheet: the
   stem named in the device or entry `note`, or the PDF whose text contains
   the parts' base number. Read the **Recommended Operating Conditions**
   `VCC Supply voltage` MIN and MAX. Use the "Operating" row, not "Data
   retention only". Also read the `VI Input voltage` MAX:
   - When it is a number above VCC MAX (5.5 or 3.6), that number is the
     `inputMax`.
   - When it reads `VCC`, there is no `inputMax`.
   - Scanned sheets: open the page with the Read tool.
3. List the entry's part names. Match each to its sheet's row:
   - SN54 vs SN74 columns.
   - CD vs SN sheets.
   - -Q1/-EP sheets.
   - 5962-/JM38510/V62 parts: match by the SMD-to-part table in the sheet's
     ordering addendum.

   Set the entry's `vcc` (and `inputMax`) to the range that most of its
   parts have. Give each other part its own `"vcc"` (and `"inputMax"` if
   that differs too).
4. Add to the entry's `note`: `VCC <min>-<max> V (<stem> p.<n>)`. Add part
   overrides' provenance to the device `note` as `| <part>: VCC … (<stem>)`.
5. Obsolete parts are filled the same way. No part is removed.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the
> same commit as the work. Use `- [~]` for partial and say what remains. A
> moot task is `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` when a
> phase lands. **An unticked box means NOT DONE.**

### Phase 1 — AND devices

- [ ] Run §4 on the 13 AND devices (54 entries) in `gate.json`.
- [ ] Regenerate `parts.txt` (`docs/backend.md` § Tests).

Acceptance: every AND part has `vcc`, and nothing else changed.
  Check: `python3 -I -c "import json;g=json.load(open('schematical/schematical/Parts/gate.json'))['devices'];print(sum(1 for d in g if d['function']=='AND' for p in d['parts'] if 'vcc' not in p and 'vcc' not in d['timing'][p['family']]))"`
  → `0`, and `git diff --quiet backend/tests/golden/traces.txt` → exit 0, and
  `git diff -U0 backend/tests/golden/parts.txt | grep '^+[^+]' | grep -vc '|vcc='`
  → `0` (est. 1 min).
Commit: —

### Phase 2 — NAND devices

- [ ] Run §4 on the 17 NAND devices (69 entries).
- [ ] Regenerate `parts.txt`.

Acceptance: every AND and NAND part has `vcc`, and the spot values match.
  Check: the Phase 1 script with `in ('AND','NAND')` → `0`; then
  `grep -E '^(SN74HC00N|SN74LVC00AD|SN54LS00J|CD74AC00E)\|' backend/tests/golden/parts.txt | grep -o 'vcc=.*'`
  → the four values in the Outcome above (est. 1 min).
Commit: —

## Validation Plan

- Tests: `listing_check` holds every part's value from now on.
  `library_check` is unchanged.
- Coverage check: the Phase 2 script counts parts with no range over all
  1404 parts. `0` means none were skipped.
- Runtime proof: none. The data is checked by plan-07-B's function, which
  is already tested.
- Doc sync: none. The format doc was done in A.
- Final gate (once): `cmake --build /tmp/p07 -j8 && ctest --test-dir /tmp/p07`
  → 12/12 (est. 1 min).

## Decisions

See plan-07-A § Decisions. Author decisions in this sub-plan:

- The entry carries the range most of its parts have, and the other parts
  override it.
- `inputMax` is set only when the VI maximum is a fixed voltage above VCC
  MAX.

## Corrections

## Summary

The risk is a part inheriting the wrong entry value. §4 step 3 checks every
part name against its datasheet row. No behavior change.
