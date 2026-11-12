# plan-35-F: 20x4 / 40x2 / 40x4 LCDs — the 32 Focus parts and goldens

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-35-E

F adds the 32 Focus LCDs parts the same way E added Newhaven and Midas:
one device per datasheet, `specs.tsv` rows, `gen-parts.py`, the 16
footprints of `footprints-B.json` merged into `footprints.json`, and
regenerated goldens. E's every-part check covers them with no new test
code.

Outcome of F: parts.txt E + 32 lines, traces.txt E + 32, footprints.txt
E + 16, all added lines only; `ctest` passes; the every-part check has 72
PASS lines.

References:

- plan-35-A § Prerequisites; plan-35-E § 3 (the device rules, generator,
  `specs.tsv`), § 4.1 (the every-part check).
- `planning/plan-35-data/candidates.tsv`, `footprints-B.json`.

## Prerequisites

The gate is plan-35-A § Prerequisites. F adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-E complete | `grep -c '^Commit: —' planning/plan-35-E-charlcd-large-parts-nhd-midas.md` → 0 (or in `planning/completed/`) | MET (2026-11-11, with E Phase 3's hash recorded in this commit: 0) |
| The generator exists | `ls planning/plan-35-data/census/gen-parts.py planning/plan-35-data/specs.tsv` → both | MET (2026-11-11: both) |
| footprints-B.json loads | `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-B.json` → only extra/unused for its ids | MET (2026-11-11: B + C → "34 new footprints: load ok" on the live backend; with C now merged, B alone is the new set) |
| E's line counts recorded | `wc -l < backend/tests/golden/parts.txt` and `footprints.txt` at E's last commit (E_P, E_F) | recorded at 25938c8: E_P = 26764, E_F = 1230 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `grep -c '|charlcd|' backend/tests/golden/parts.txt` → 40 + 32 = 72.
- `wc -l < backend/tests/golden/parts.txt` → E_P + 32; traces.txt the same;
  `footprints.txt` → E_F + 16.
- `git diff backend/tests/golden | grep -c '^-[^-]'` → 0.
- `ctest --test-dir /tmp/logic-p35` → `100% tests passed`.

### Non-goals (explicit constraints)

- No change to any Newhaven, Midas or 16 × 2 part, and no app change.

## 2. Current State

The 32 Focus parts (`awk -F'\t' '$1=="Focus LCDs" && $4=="add"' planning/plan-35-data/candidates.tsv`):

| Group | Parts | Size | Pinout | Panel / backlight |
|---|---|---|---|---|
| C204A-BW-LW25 | 1 | 20x4 | p16 | blue-negative / White |
| C204ABB, C204ADB, C204ALB-98, C204AXB | C204ABBSYSY6WT55PAB, C204ADBFGSW6WT55PAB, C204ALBSYLY6WM55PAA, C204AXBSYLY6WT, C204AXBSGN06WR50XAA | 20x4 | p16 (AXBSGN06: p16-nc) | yellow-green, white, yellow-green, yellow-green, gray / Yellow-green, White, Yellow-green, Yellow-green, none |
| C204BLB / C204DLB 146 × 62.5 | C204BLBFWSW6WT33XAA, C204BLBSBSW6WN33PAB, C204BLBSYLY6WT33PAB, C204DLBSBSW6WN55XAA | 20x4 | p16 | white, blue-negative, yellow-green, blue-negative |
| C204ALB / C204BLB 155.5 × 73 | C204ALBFWSW6WT33XAA, C204ALBSBSW6WN33XAA, C204ALBSYLY6WT33XAA, C204BLBSBSW6WN55XAA, C204BLBSYLY6WT55XAA | 20x4 | p16 | white, blue-negative, yellow-green, blue-negative, yellow-green |
| C204CXB | C204CXBSGSW6WT33XAA, C204CXBSGSW6WT55XAA | 20x4 | p16 | gray / White |
| C204HLB | C204HLBSYLY6WM | 20x4 | p20-fgnd | yellow-green |
| C402 | C402A3-BW-LW65, C402ALBFKSW6WT55XAA, C402ALBFWSW6WT33XAA, C402ALBSBSW6WN33XAA, C402ALBSBSW6WN55XAA, C402ALBSGN06WT50PAB, C402ALBSYLY6WT33XAA, C402ALBSYLY6WT55XAA, C402ASBSYLY6WT55PAB | 40x2 | p16 (ALBSGN06: p16-nc) | per `candidates.tsv` |
| C404ALB | C404ALBFKSW6WT55PAB, C404ALBFWSW6WT33PAB, C404ALBSBSW6WF33PAB, C404ALBSBSW6WN55PAB, C404ALBSYLY6WT33PAB | 40x4 | p18-dual | white ×2, blue-negative ×2, yellow-green |

The exact panel and backlight of every row is in `candidates.tsv`
(`panel`, `backlight`); the table above is a reading aid. DigiKey lists the
SYLY backlights as "LED - Yellow"; the PDFs say yellow-green
(C204ABBSYSY6WT55PAB p.1 "Yellow/Green", p.2 "Backlight Type SIDE Y-G"),
and the devices use the PDFs' color.

### Verified properties

- **Every Focus PDF prints its own timing and instruction sections.**
  `grep -c -i -E 'instruction|clear display'` ≥ 1 in all 32; a "Timing
  Characteristics" or "AC Characteristics" section in all 32. The section
  is text in most of them (C204A-BW-LW25 "Enable Cycle Time tc 1200",
  C402A3-BW-LW65 and C204ADBFGSW6WT55PAB tc 500). C204ABBSYSY6WT55PAB's
  tables are images: pdftotext finds the headings only, so they are read
  from a 300 dpi render.
- **Focus controllers** as printed: ST7066U, ST7066U-0A, ST7066 or
  SPLC780D (DigiKey's column shows the same set). The C404ALB notes say
  "ST7066U or SPLC780D".
- **Focus electrical tables print VDD min/typ/max**, e.g.
  C204ALBFWSW6WT33XAA "Input voltage VDD 3.1 3.3 3.5".

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — specs

- [x] One `specs.tsv` row per Focus part, by plan-35-E § 3 (pages
      recorded; images rendered at 300 dpi). Manufacturer "Focus LCDs".
      — 32 rows appended (pages in each `note`; C204ABB's timing and
      instruction tables and C204ADB's read table read from 300 dpi
      renders). Nine timing sets and six exec sets, each as its PDF prints
      it (e.g. 500/300/100/10/60/10/90/20 on the 20 ALB/BLB/DLB/HLB/C402A3
      parts; 1200/140/100/10/40/10/100/10 on four C404ALB). Controllers:
      ST7066U 18, SPLC780D 8, ST7066U-0A 4, ST7066-0A 1, S6A0069 1.

Check: `python3 -I planning/plan-35-data/census/gen-parts.py --check planning/plan-35-data/specs.tsv planning/plan-35-data/candidates.tsv`
→ 67 rows, all keys present, pin counts match (est. 1 min). — 2026-11-11:
"67 rows ok".

Commit: 9827d8a

### Phase 2 — parts, footprints, goldens

- [x] Paste the 32 Focus devices into `charlcd.json`; merge
      `footprints-B.json` with `fp-merge.py`; regenerate the three goldens;
      hand-check four new lines (p16, p16-nc, p18-dual, p20-fgnd) against
      their PDFs. — `gen-parts.py --maker "Focus LCDs"` → 32 devices
      appended (drawer `["Displays", "Character"]`, plan-35-E Corrections 9);
      `fp-merge.py` → "16 footprints appended". Hand-checked: C204A-BW-LW25
      (p16: 15 A, 16 K p.5; VDD 4.8–5.2 p.6), C204AXBSGN06WR50XAA (p16-nc:
      15/16 NC, no backlight), C404ALBFKSW6WT55PAB (p18-dual: 9 E1, 15 E2,
      `enables=2`, `bus=11,10,9,15,…`), C204HLBSYLY6WM (p20-fgnd: 15 VSS,
      18 FGND ground, 19/20 LED) — each matches its pin table.

Check: § 1 counts; `ctest --test-dir /tmp/logic-p35` → `100% tests passed`;
`/tmp/logic-p35/charlcd_test | grep -c '^FAIL'` → 0 with 72 every-part
PASS lines (est. 5 min). — 2026-11-11: parts.txt 26796 (E_P 26764 + 32),
traces.txt 26796, footprints.txt 1246 (E_F 1230 + 16), `grep -c
'|charlcd|'` → 72, `git diff backend/tests/golden | grep -c '^-[^-]'` → 0;
ctest "100% tests passed, 0 tests failed out of 30"; charlcd_test 0 FAIL,
72 every-part PASS lines plus "72 charlcd parts run".

Commit: —

## Validation Plan

Per-phase checks. The landing tier is H.

## Decisions

- **Manufacturer "Focus LCDs"** (author): DigiKey's name; no earlier Focus
  part (`grep -c 'Focus LCDs' schematical/schematical/Parts/*.json` → 0 at
  656b6d1).
- As plan-35-E § Decisions otherwise.

## Corrections

Phase 1 (2026-11-11, evidence = the PDFs' pages, in each device note):

1. **§ 2's controller set misses two names and three parts are
   different from DigiKey's column:** C204HLB prints ST7066-0A and
   C204AXBSYLY6WT only "S6A0069 or equivalent" (p.2); C402A3-BW-LW65
   prints ST7066U-0A (pp.1–3, DigiKey: ST7066); C402ASBSYLY6WT55PAB
   prints SPLC780D (p.1; DigiKey: ST7066U); C204ADBFGSW6WT55PAB prints
   "ST7066U-OA or Eqv." (p.2). The devices use the PDFs.
2. **The C404ALB devices are SPLC780D**, the "Controller=" entry on p.1 of
   all five; "ST7066U or SPLC780D" is in two of their drawing notes and
   "ST7066U or equivalent" on p.2. § 2's "The C404ALB notes say ST7066U or
   SPLC780D" stands; the device notes quote the alternatives.
3. **C402ALBSGN06WT50PAB contradicts itself:** p.1 says SPLC780D and
   transflective, pp.2–3 say ST7066U and the drawing "GRAY/Positive/
   Reflective". With no backlight the device is reflective, controller as
   p.1; pins 15–16 print "-" in the table and NC on the drawing (NC used).
4. **C204ABBSYSY6WT55PAB's VDD is 3.0–5.5** (p.4 "Logic Supply Voltage
   VDD 3.0 5.0 5.5"), not DigiKey's 5 V; its read timing is on p.9, not
   with the write table on p.8.
5. **Init waits**: 25 Focus PDFs print no initialization flow; their
   `powerOn` is 40 ms from `ST7066U.pdf` p.26 (noted). C402ASB prints
   ">15 ms after VDD rises to 0.9 VDD" (p.5) and uses 15 ms; C204ADB
   prints >30 ms (4.5 V) and >40 ms (2.7 V) and uses 40 ms.
6. **Only-2.7 V timing on C204CXBSGSW6WT33XAA** (p.6): tpw 460 write / 480
   read, 480 used (as plan-35-E's 3 V Newhaven parts).
7. **Page-1 size typos** (no effect; the drawings and footprints agree
   with § 2): C404ALBSBSW6WF33PAB "40x4(183x47)" vs 190 × 54 on p.2;
   C204ALBSYLY6WM55PAA and C204ADB "93x55" vs 98 × 60.

## Summary

The same work as E, for one maker. The every-part check and the
added-lines-only goldens cover it.
