# plan-40-E: Footprints — power and leadless surface-mount transistor packages

Last updated: 2026-11-14
Effort: large (3h–1d)
Depends on: plan-40-D

E draws the land patterns of the power and multi-pin surface-mount packages
of the final pick — the tab packages (DPAK / TO-252, D2PAK / TO-263 in 3 and 7
leads, SOT-223, TO-277, PowerDI 5, LFPAK / SOT-669), the SO-8 family (SOIC-8,
TSSOP-8, TSOP-8, VSSOP-8, SOIC-16, SuperSOT-6, PowerPAK SO-8, 1212-8, 0806,
SC-70-6, SC-75-6, ChipFET), and the power leadless ones (PQFN / TDFN / WDFN /
VDFN, flat-lead SMD, DirectFET) — from the datasheets' package drawings,
reusing a library footprint when its pads equal the drawing's. It writes
`planning/plan-40-data/footprints-E.json` and the E rows of
`footprint-map.tsv`. The behavioral outcome: every E line of
`footprints-needed.txt` has a footprint id in `footprint-map.tsv`, and the
new objects load through `footprint_check`.

References:

- plan-40-C § 3.1 (ids, reuse, shared ids, ready objects) and its tools
  (`census/fp-load.sh`, `fp-merge.py`, `fp-check.py`); plan-40-D (the small
  packages E's tab and chip-scale cases follow).
- `docs/parts-format.md` § Footprints (`dual`, `quad`, `explicit`, `slots`,
  `centre`; R1–R9, mid-values, no overlapping pads).
- Library precedents: `DPAK-3`, `TO-252-2-TAB`, `TO-263-2-TAB-ONSEMI`,
  `D2PAK-3`, `TO-277-3`, `LFPAK4-5`, `SOIC-8`, `WDFN8-5`, `DFN33A-5` (the
  diode plans' power packages; their pad numbering is the diode
  datasheets').

## Prerequisites

plan-40-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-40-D complete | `ls planning/completed/plan-40-D-*.md` → exists | NOT MET (D not started) |
| `footprints-needed.txt` is final | `grep -c '^E' planning/plan-40-data/footprints-needed.txt` → the E lines (73 at planning time) | NOT MET (B not started) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-40-data/census/fp-check.py E` prints `<n> lines,
  0 without a footprint`, `<n>` = `awk -F'\t' '$1=="E"' planning/plan-40-data/footprints-needed.txt | wc -l`
  (73 at planning time).
- `planning/plan-40-data/census/fp-load.sh planning/plan-40-data/footprints-C.json planning/plan-40-data/footprints-D.json planning/plan-40-data/footprints-E.json`
  prints `<k> new footprints: load ok` (E has at most 56 at planning time,
  the lines with no same-outline footprint).
- `footprint-map.tsv` has, for every E line, the footprint id and the
  pinouts it serves.

### Non-goals (explicit constraints)

- No change to `Parts/footprints.json`, the part files, `backend/` or the app.
- No edit of an existing footprint.
- No footprint for a package no `add` row uses.

## 2. Current State

The library's tab and power-leadless footprints come from the diode plans and
number their pads as the diode datasheets do (tab last: `DPAK-3` pads 1, 2 =
leads, 3 = tab); a transistor datasheet numbers a DPAK 1 gate, 2 drain (the
tab, the middle lead removed), 3 source, so the same outline is not reused
until its pads are compared (plan-40-C § 2).

### Measured populations

| What | Count | Command |
|---|---|---|
| E lines (maker, DigiKey Package / Case), picked rows | 73 (539 parts), 54 distinct packages | `awk -F'\t' '!/^#/ && $1=="E"' planning/plan-40-data/footprints-needed.txt \| wc -l` |
| …with no existing footprint of the same outline | 56 | the E lines with an empty 10th column |
| …tab packages (Phase 1) | 21 (9 with no existing footprint): TO-252 / DPAK 5, TO-261 / SOT-223 5 + SOT-223-8 1, TO-263-3 3, TO-263-7 4, PowerDI 5 1, SOT-669 1, TO-277 1 | the E lines whose package matches `TO-252\|TO-263\|TO-261\|SOT-223\|TO-277\|PowerDI\|SOT-669\|SC-100\|TO-26` |
| …SO-8 family (Phase 2) | 22 (17 with none): SOIC-8 3, SOIC-16 1, TSSOP 2, TSOP 1, VFSOP 1, SuperSOT-6 1, PowerPAK 13 | the lines matching `SOIC\|TSSOP\|TSOP\|VFSOP\|SSOT\|PowerPAK\|ChipFET\|ChipFet` |
| …leadless power (Phase 3) | 30 (30 with none): TDFN / WDFN / VDFN / SFN / UDFN / TSFN / VFQFN 19, flat-lead SMD 3, DirectFET 8 | the remaining E lines |

### Verified properties

- **The tab's pin number is the datasheet's, and differs by package.**
  Infineon's `IPB017N10N5LF` (TO-263-7) is pin 1 gate, pins 2, 3, 5, 6, 7
  source and "Pin 4, tab" drain (`pdftotext -layout` of
  `https://www.infineon.com/assets/row/public/documents/24/49/infineon-ipb017n10n5lf-datasheet-en.pdf`,
  lines 21–28): the tab is pad 4 and B writes `G,S,S,D,S,S,S`. Infineon's
  `IPN60R2K1CE` (SOT-223) gives pin 1 gate, pin 2 drain, pin 3 source (same
  method, `infineon-ipn60r2k1ce-datasheet-en.pdf` lines 18–26) and B writes
  `G,D,S`, or `G,D,S,D` when a datasheet numbers the tab 4. A tab that is its
  own pin is a pad of that number and its pin name is shared with the drain
  lead, which the kind ties (plan-40-F § 3.1), so the footprint has a pad per
  pinout position whatever the numbering.
- **DirectFET and some PQFN have no pin numbers.** The datasheet names the
  pads (G, S, D, S1, S2, …); B numbers them in the datasheet's table order and
  E's pads follow that numbering (a `centre` or `explicit` pad per name).

## 3. Design

Ids, reuse, shared ids and ready objects are plan-40-C § 3.1: `<outline>-<MAKER>`
(`DPAK-3-ONSEMI`, `DPAK-3-INFINEON`, `D2PAK-7-VISHAY`, `SOT223-4-NEXPERIA`,
`SO8-DIODES`, `POWERPAK-SO8-VISHAY`, `PQFN-5X6-INFINEON`,
`DIRECTFET-MX-INFINEON`), variants appended. Styles: tab packages `explicit`
(the tab as one pad, the leads as in the drawing's recommended land pattern),
SO-8 / TSSOP / VSSOP `dual`, PowerPAK and PQFN `explicit` or `dual` with a
`centre` exposed pad (the drain), DirectFET `explicit`. Pads come from the
datasheet's recommended land pattern when it prints one (rule R1), else
the outline by R2–R6, mid-values
throughout; the `note` names the drawing, the PDF under
`datasheets/transistor/` and the page.

Risk: the tab and thermal pads (large, many pins on one pad), and pad numbering
that disagrees between the land-pattern drawing and the pin table; the `note`
records which numbering was used and the hand check below covers one object
per family.

Rejected alternative: reusing the diode plans' tab footprints by renumbering
them (rejected: an existing footprint is never edited, and a second id with the
same pads is what the geometry rule produces when the numbering differs).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tab packages (21 lines)

- [ ] One object per distinct land pattern for the TO-252 / DPAK, TO-263-3,
      TO-263-7, TO-261 / SOT-223 (and SOT-223-8), TO-277, PowerDI 5 and SOT-669 /
      LFPAK lines, from each line's example part's datasheet; the `footprint-map.tsv`
      rows; reuse where an existing footprint's pads (designator, centre, size)
      equal the drawing's within 0.05 mm.

Acceptance: the tab-package objects load and their lines are covered.
  Check: `planning/plan-40-data/census/fp-load.sh planning/plan-40-data/footprints-C.json planning/plan-40-data/footprints-D.json planning/plan-40-data/footprints-E.json` → only `extra footprint` / `unused footprint` failures, for exactly the new ids; no load error (est. 4 min: copies `backend/`, builds `footprint_check`).
Commit: —

### Phase 2 — SO-8 family and PowerPAK (22 lines)

- [ ] One object per distinct land pattern for the SOIC-8 / SOIC-16, TSSOP-8,
      TSOP-8, VSSOP-8, SuperSOT-6 and the PowerPAK SO-8, SO-8 Dual, 1212-8
      (8S, 8SH, Dual), 0806, SC-70-6 (Single, Dual), SC-75-6 and ChipFET
      lines; reuse as Phase 1 (`SOIC-8` has the TI drawing: it is reused where
      a maker's SO-8 drawing equals it).

Acceptance: as Phase 1 for the Phase 2 lines.
  Check: `fp-load.sh footprints-C.json footprints-D.json footprints-E.json` as Phase 1 (est. 4 min).
Commit: —

### Phase 3 — Power leadless packages and DirectFET (30 lines)

- [ ] One object per distinct land pattern for the TDFN / WDFN / VDFN / SFN /
      UDFN / TSFN / VFQFN, flat-lead SMD, 9- and 12-pad PowerWDFN and the eight
      DirectFET lines; pad numbering by B's `pinout` for the line.
- [ ] Hand-check one object per family (a tab package, an SO-8, a PowerPAK, a
      PQFN, a DirectFET): pad centres and sizes against the drawing, recorded in
      § Corrections.

Acceptance: every E line has a footprint and the objects load.
  Check: `python3 -I planning/plan-40-data/census/fp-check.py E` → `<n> lines, 0 without a footprint` (n = the E lines); `fp-load.sh footprints-C.json footprints-D.json footprints-E.json` as Phase 1 (est. 6 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` through `fp-load.sh`.
- Coverage check: `fp-check.py E` joins every E line.
- Runtime proof: n/a here (G's goldens and H's harness render the parts).
- Doc sync: none.
- Final gate: plan-40-I.

## Decisions

- **Objects stay in `planning/plan-40-data/footprints-E.json` until G** —
  author: `footprint_check` fails on a footprint no part uses.
- **A tab is a pad of its own pin number** — author: B's pinouts give the tab
  a pin name and number (§ 2).
- **Reuse by pad geometry** — author: plan-40-C § 2.
- Owner decisions: plan-40-A § Decisions.

## Corrections

(none yet)

## Summary

E is the plan's heaviest drawing letter (73 lines, up to 56 new objects) and
the one with the large-pad cases. Nothing in the tree changes except
`planning/plan-40-data/`.
