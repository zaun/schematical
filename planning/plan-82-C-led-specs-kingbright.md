# plan-82-C: Indicator LEDs: the spec rules; Kingbright specs and footprints

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-82-B

C fixes the rules every maker letter (C, D, E) follows to fill `specs.tsv` and draw footprints,
writes the two shared tools (`fp-match.py`, `fp-merge.py`), and does Kingbright: up to 300 `add`
rows, one PDF each.

References: plan-82-A § Prerequisites; plan-82-B (`specs.tsv` columns);
`docs/parts-format.md` § Footprints (§ Common keys, § `dual`, § `quad`, § `explicit`), § `led`;
`schematical/schematical/Parts/footprints.json` (the 11 generic `LED-` footprints, lines 5–15);
`planning/plan-74-D-ethernet-footprints-ic.md` (a footprint letter's precedent).

## Prerequisites

plan-82-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-82-B complete | `ls planning/completed/plan-82-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25) |
| Footprint baseline | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` | 3993 at planning; C records its own F1 |

## 1. Goal

- Every Kingbright row of `specs.tsv` has its pin map, dies, colours, facts and a footprint id.
- `planning/plan-82-data/footprints-kingbright.json` holds every Kingbright footprint not already
  in `footprints.json`; `fp-merge.py` has merged it.
- `fp-match.py` and `fp-merge.py` exist and are used by D and E unchanged.

### Non-goals

- No change to `Parts/led.json`, `backend/` or the app. Footprints are the only shipped file C
  changes (the footprints golden moves in H, when parts use them; `footprints.txt` lists every
  footprint, so C regenerates it, § 4 Phase 3).

## 2. Current State

The 11 `LED-` footprints are generic (`"note":"generic; plan-03-A §4.5"`), e.g. `LED-0603`: dual,
2 pads 0.7 × 1.0 mm, span 1.2 mm; `LED-RADIAL-5MM`: 2.54 mm pitch, 0.9 mm drill. None comes from a
maker drawing. A Kingbright PDF (`APT1608EC.pdf`, 4 pages, fetched at planning) has its package
drawing and a "Recommended Soldering Pattern" (`pdftotext … | grep -ci 'recommended\|soldering
pattern\|pad'` → 5).

## 3. Design: the spec rules (C, D and E)

**Pins and dies.** `pins` lists the package's pins as `number:name`, the name `A`/`K` for a single
die, else `A1`, `K1`, `A2`, … per die, a common pin named `A` (common anode) or `K` (common
cathode). `dies` lists one die per colour as `anode>cathode:colour`, e.g. a common-anode RGB
`A>K1:Red;A>K2:Green;A>K3:Blue`; a bidirectional pair `1>2:Red;2>1:Green` (two dies on the same
two pins, reversed); a single die `A>K:Red`. Die order: the datasheet's colour order.

**Colours** (the vocabulary the app's `lightColor(named:)` gets in G): `Red`, `Orange`, `Amber`,
`Yellow`, `Yellow-green`, `Green`, `Blue-green`, `Blue`, `Violet`, `Pink`, `White`, `Warm white`.
DigiKey names map as: Red-Orange / Orange-Red → Orange; Amber-Yellow → Amber; Green-Yellow /
Yellow-Green → Yellow-green; Cyan / Blue-Green / Ice Blue → Blue-green; Purple → Violet; White,
White Cool, White Neutral → White; White Warm → Warm white.

**Facts** (per die, `/`-separated in die order; the datasheet's typical values at its test
current): `wavelength` dominant nm (white: empty), `mcd` luminous intensity mcd, `vf` forward
voltage V, `if` test current mA (one value), `viewingAngle` 2θ½ degrees (one value), `lens` one of
`water clear`, `diffused`, `white diffused`, `tinted`, `tinted diffused`.

**Footprints.** From the drawing's recommended land pattern (SMD) or lead pitch and diameter (TH).
`fp-match.py` compares it with the 11 generic `LED-` footprints: same pad count, every pad centre
and size within 0.05 mm, same drill within 0.05 mm → reuse the generic id. Otherwise a new id
`LED-KB-<package>` (Broadcom `LED-AV-`, SunLED `LED-SL-`, QTB `LED-QTB-`), `<package>` the maker's
package code (`APT1608` → `LED-KB-1608`, a 3 mm radial with 2.54 mm pitch → `LED-KB-T1-2.54`). Two
drawings of one maker with identical pads share one id. Pad 1 is the drawing's pin 1; through-hole
pin 1 is square (`pin1Shape`). Styles: `dual` for 2- and 4-pad chip/PLCC packages, `explicit` for
anything else (right-angle and bottom-entry pads, odd pin layouts). Body = the package outline;
the note cites `<pdf> p.<page>`.

## 4. Phases

### Phase 1: the tools

- [ ] `census/fp-match.py` (§ 3) and `census/fp-merge.py` (adds the ids of a
      `footprints-<maker>.json` absent from `footprints.json`, sorted with the existing `LED-`
      block; refuses a present id with other geometry).

Acceptance: `python3 -I planning/plan-82-data/census/fp-match.py --self-test` matches each generic
`LED-` footprint to itself and `LED-0603` shifted 0.06 mm to nothing — 1 min.

Commit:

### Phase 2: Kingbright specs

- [ ] Fill every Kingbright row of `specs.tsv` by § 3 (`pdftotext -layout` for the tables,
      `pdftoppm -r 150` of the drawing page for the pin map).

Acceptance: `python3 -I planning/plan-82-data/census/specs-check.py kingbright` (written here:
every row has pins, dies whose pins exist in `pins`, colours from the vocabulary, numeric facts)
→ `rows N ok N` — ~3 h of reading for 300 PDFs.

Commit:

### Phase 3: Kingbright footprints

- [ ] Write `footprints-kingbright.json`; run `fp-merge.py`; rebuild, then
      `/tmp/logic-p82/footprint_check --print > backend/tests/golden/footprints.txt` and
      hand-check the new lines against the drawings (`docs/backend.md` § `footprint_check`).

Acceptance: `ctest --test-dir /tmp/logic-p82 -R footprint_check` passes and `wc -l <
backend/tests/golden/footprints.txt` = F1 + the new id count — 2 min.

Commit:

## 5. Validation Plan

C's gate is Phase 3's footprint test. The landing tier is J's.
