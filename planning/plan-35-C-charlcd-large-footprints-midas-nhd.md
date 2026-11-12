# plan-35-C: 20x4 / 40x2 / 40x4 LCDs — the 10 Midas and 7 Newhaven module footprints

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-35-B

C draws the module footprints of the 13 Midas and 22 Newhaven parts as
ready objects in `planning/plan-35-data/footprints-C.json`. plan-35-E
merges them with the parts. The pad rules, coordinates and checks are
plan-35-B § 3's; this file lists only what differs.

Outcome of C: `footprints-C.json` holds the 17 objects of § 2;
`fp-load.sh` on B's and C's files together reports only extra/unused lines
for exactly the 33 new ids.

References:

- plan-35-A § Prerequisites, plan-35-B § 3 (pad rules, gate).
- `planning/plan-35-data/footprints-needed.txt`.
- `planning/completed/plan-11-C-charlcd-parts.md` § 2 (how Newhaven
  drawings print pins and holes).

## Prerequisites

The gate is plan-35-A § Prerequisites. C adds:

| Must be true | Command | Status |
|---|---|---|
| plan-35-B complete | `grep -c '^Commit: —' planning/plan-35-B-charlcd-large-footprints-focus.md` → 0 (or in `planning/completed/`) | NOT MET (2026-11-11) |
| The Midas and Newhaven PDFs are in the tree | `awk -F'\t' '($1=="Midas Displays" \|\| $1=="Newhaven Display Intl") && $4=="add"{print "datasheets/module-disp-char/"$13}' planning/plan-35-data/candidates.tsv \| xargs ls \| wc -l` → their `add` count (35 at planning) | MET (2026-11-11: 35) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. **If you stop, report the current status of
> *all* prerequisites.**

## 1. Goal

- `planning/plan-35-data/census/fp-load.sh planning/plan-35-data/footprints-B.json planning/plan-35-data/footprints-C.json`
  → 33 appended (16 + 17); failures only extra/unused for those ids.

### Non-goals (explicit constraints)

- As plan-35-B: no `footprints.json`, part file or golden change, and no
  connector footprint.

## 2. Current State

| Id | Outline | Pins | Header (drawing) | Drawing | Parts |
|---|---|---|---|---|---|
| LCM-MIDAS-MC42004 | 77 × 47 | 16 | 1x16, drawing rotated 90° | MC42004A6W-FPTLW-V2.pdf p.2 | MC42004A6W-FPTLW-V2 |
| LCM-MIDAS-MC42004-SPR | 77 × 47 | 16 | 1x16 (rotated) | MC42004A6W-SPR-V2.pdf p.2 | MC42004A6W-SPR-V2 |
| LCM-MIDAS-MD42004 | 77 × 47 | 18 | 1x18 (rotated) | MD42004A6W-FPTLRGB.pdf p.6 | MD42004A6W-FPTLRGB |
| LCM-MIDAS-MC42005 | 98 × 60 | 16 | 1x16 (rotated) | MC42005A6W-FPTLW-V2.pdf p.2 | MC42005A6W-FPTLW-V2, MC42005A6W-SPTLY-V2 |
| LCM-MIDAS-MD42005C | 98 × 60 | 18 | 1x18 (rotated) | MD42005C6W-FPTLRGB.pdf p.6 | MD42005C6W-FPTLRGB |
| LCM-MIDAS-MC42008 | 146 × 62.5 | 16 | 1x16 (rotated) | MC42008A6W-FPTLW.pdf p.2 | MC42008A6W-FPTLW, MC42008A6W-SPTLY |
| LCM-MIDAS-MD24005 | 182 × 33.5 | 18 | 2x7 interface + pads A, K(R), K(G), K(B) (rotated) | MD24005A6W-FPTLRGB.pdf p.6 | MD24005A6W-FPTLRGB |
| LCM-MIDAS-MC44005 | 190 × 54 | 18 | 2x9 (rotated) | MC44005A6W-BNMLW-V2.pdf p.6 | MC44005A6W-BNMLW-V2, MC44005A6W-FPTLW-V2 |
| LCM-MIDAS-MC44005-SPTLY | 190 × 54 | 18 | 2x9 (rotated) | MC44005A6W-SPTLY-V2.pdf p.2 | MC44005A6W-SPTLY-V2 |
| LCM-MIDAS-MD44005 | 190 × 54 | 20 | 2x10 (rotated) | MD44005A6W-FPTLRGB.pdf p.6 | MD44005A6W-FPTLRGB |
| LCM-NHD-0420AZ | 77 × 47 | 16 | 1x16 top-left | NHD-0420AZ-FL-GBW-33V3.pdf p.4 | NHD-0420AZ-FL-GBW-33V3, -FL-YBW-33V3, -FSW-GBW-33V3 |
| LCM-NHD-0420DZ | 98 × 60 | 16 | 1x16 top-left | NHD-0420DZ-FL-GBW.pdf p.3 | NHD-0420DZ-FL-GBW, -FL-YBW, -FL-YBW-33V3, -FSW-FBW, -NSW-BBW |
| LCM-NHD-0420E2Z | 146 × 62.5 | 16 | 1x16 top-left | NHD-0420E2Z-FL-GBW.pdf p.3 | NHD-0420E2Z-FL-GBW, -FL-YBW, -FSW-GBW, -NSW-BBW |
| LCM-NHD-0420H1Z | 79 × 36 | 16 | two 1x8 columns: 1–8 left edge, 9–16 right edge | NHD-0420H1Z-FSW-GBW.pdf p.3 | NHD-0420H1Z-FL-GBW-33V3, -FSW-GBW, -FSW-GBW-33V3 |
| LCM-NHD-0240AZ | 182 × 33.5 | 16 | 2x8 left | NHD-0240AZ-FL-GBW.pdf p.4 | NHD-0240AZ-FL-GBW, -FL-YBW, -FSW-FBW, -NSW-BBW |
| LCM-NHD-0440AZ | 190 × 54 | 18 | 2x9 left | NHD-0440AZ-RN-FBW.pdf p.4 | NHD-0440AZ-RN-FBW |
| LCM-NHD-0440WH | 190 × 54 | 18 | 2x9 left | NHD-0440WH-ATFH-JT.pdf p.3 | NHD-0440WH-ATFH-JT#, NHD-0440WH-ATMI-JT# |

The drawing pages are the PDFs as read on 2026-11-11 (DigiKey copies for
Midas; 10 of the 13 own-site files are byte-identical and the other 3 have
the same text, plan-35-A § 2). After A Phase 2, each page is checked again
in the own-site file, and any change is recorded in Corrections.

### Measured populations

| What | Count | Command |
|---|---|---|
| Midas footprints | 10 | `grep -c '^LCM-MIDAS' planning/plan-35-data/footprints-needed.txt` |
| Newhaven footprints | 7 | `grep -c '^LCM-NHD' planning/plan-35-data/footprints-needed.txt` |
| Parts served | 35 | `awk -F'\t' '($1=="Midas Displays" \|\| $1=="Newhaven Display Intl") && $4=="add"' planning/plan-35-data/candidates.tsv \| wc -l` |

### Verified properties

- **Newhaven: one footprint per model code.** Newhaven's part number is
  model-glass-backlight (`NHD-0420DZ-FL-YBW`), and the model names the
  PCB. The PDFs whose drawings carry vector text agree:
  - NHD-0420DZ-FL-YBW and NHD-0420DZ-NSW-BBW print the same outer chain
    (49.0 55.0 60.0 70.4 77.0 93.0 96.8 98.0);
  - both NHD-0440WH sheets print the same set (`pdftotext -raw` numbers
    ≥ 40).
  Some members' sheets are later redraws, so C still checks every member
  (Phase 2).
- **Newhaven drawings print pad sizes.** NHD-0440WH p.3: "18-Ø1.0 PTH,
  18-Ø1.8 PAD", "4-Ø3.5 PTH, 4-Ø6.5 PAD". Where a sheet prints them, the
  printed size wins over B's defaults.
- **Midas drawings are rotated 90°** (long side vertical) on every sheet;
  the footprint is drawn landscape: x along the long side, pin 1 on the
  left as on the other modules.
- **Midas MD24005 backlight pads** are named, not numbered (p.4 pin table:
  "A, K(R), K(G), K(B)"). The device numbers them 15–18 in that order
  (plan-35-E § Decisions), and the footprint uses the same numbers.
- **NHD-0420H1Z pins** run 1–8 down the left edge and 9–16 down the right
  edge (p.3; "P2.54*7=17.78" on each column).

## Phases

> Tick `- [x]` in the same commit as the work; `- [~]` for partial with one
> line of what remains. Fill each `Commit:` line when the phase lands. **An
> unticked box means NOT DONE.**

### Phase 1 — the 10 Midas footprints

- [ ] Render each Midas sheet at 300 dpi (`magick -rotate 90` to read it
      landscape), write the 10 objects into
      `planning/plan-35-data/footprints-C.json` per plan-35-B § 3. MD24005:
      pads 1–14 on the 2x7 interface, 15–18 on the A, K(R), K(G), K(B)
      pads where the drawing places them.
- [ ] Member check: MC42005 (2), MC42008 (2), MC44005 (2), as in B.

Check: `fp-load.sh planning/plan-35-data/footprints-C.json` → 10 appended,
only extra/unused for them (est. 3 min).

Commit: —

### Phase 2 — the 7 Newhaven footprints

- [ ] As Phase 1, for the seven `LCM-NHD-*` rows, using the printed pad
      sizes.
- [ ] Member check for every member of 0420AZ, 0420DZ, 0420E2Z, 0420H1Z,
      0240AZ and 0440WH (18 sheets beyond the representatives).

Check: `fp-load.sh planning/plan-35-data/footprints-B.json planning/plan-35-data/footprints-C.json`
→ 33 appended; only extra/unused for the 33 ids (est. 3 min).

Commit: —

## Validation Plan

Per-phase `fp-load.sh` runs. The landing tier is H.

## Decisions

- **Newhaven grouped by model code; Midas by drawing sheet** (author,
  § Verified properties): Midas's code (MC42005) also names the PCB, but
  its reflective (SPR) and RGB variants have their own sheets and pin
  counts, so they are kept apart.
- **Printed pad sizes win** over plan-35-B's defaults (author).

## Corrections

(none yet)

## Summary

Same risk as B: misreading a dimension. Every member of each group is
checked against its own sheet.
