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
| plan-35-E complete | `grep -c '^Commit: —' planning/plan-35-E-charlcd-large-parts-nhd-midas.md` → 0 (or in `planning/completed/`) | NOT MET (2026-11-11) |
| The generator exists | `ls planning/plan-35-data/census/gen-parts.py planning/plan-35-data/specs.tsv` → both | NOT MET (E Phase 1) |
| footprints-B.json loads | `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-B.json` → only extra/unused for its ids | NOT MET (B) |
| E's line counts recorded | `wc -l < backend/tests/golden/parts.txt` and `footprints.txt` at E's last commit (E_P, E_F) | record at start |

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

- [ ] One `specs.tsv` row per Focus part, by plan-35-E § 3 (pages
      recorded; images rendered at 300 dpi). Manufacturer "Focus LCDs".

Check: `python3 -I planning/plan-35-data/census/gen-parts.py --check planning/plan-35-data/specs.tsv planning/plan-35-data/candidates.tsv`
→ 67 rows, all keys present, pin counts match (est. 1 min).

Commit: —

### Phase 2 — parts, footprints, goldens

- [ ] Paste the 32 Focus devices into `charlcd.json`; merge
      `footprints-B.json` with `fp-merge.py`; regenerate the three goldens;
      hand-check four new lines (p16, p16-nc, p18-dual, p20-fgnd) against
      their PDFs.

Check: § 1 counts; `ctest --test-dir /tmp/logic-p35` → `100% tests passed`;
`/tmp/logic-p35/charlcd_test | grep -c '^FAIL'` → 0 with 72 every-part
PASS lines (est. 5 min).

Commit: —

## Validation Plan

Per-phase checks. The landing tier is H.

## Decisions

- **Manufacturer "Focus LCDs"** (author): DigiKey's name; no earlier Focus
  part (`grep -c 'Focus LCDs' schematical/schematical/Parts/*.json` → 0 at
  656b6d1).
- As plan-35-E § Decisions otherwise.

## Corrections

(none yet)

## Summary

The same work as E, for one maker. The every-part check and the
added-lines-only goldens cover it.
