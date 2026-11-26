# plan-73-B: USB-UART bridges — datasheets and the six Silicon Labs footprints

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-73-A (letter order; B touches none of A's files and needs none of
A's keys, so it may also land before A finishes)

B puts the eight datasheet PDFs plan-73-C transcribes into the tree. It adds the
six land patterns the Silicon Labs parts need to `footprints.json`, each from its
datasheet's recommended PCB land pattern. The CH340T uses the existing `SSOP-20`
(§ 3).

Outcome of B:

- `datasheets/controller-uart/` holds `cp2102-9.pdf`, `cp2102n.pdf`, `cp2102c.pdf`,
  `cp2104.pdf`, `cp2105.pdf`, `cp2108.pdf`, `cp2110.pdf` and `ch340.pdf`.
- `datasheets.json` has their 8 entries.
- `footprints.json` has 6 more footprints.
- `backend/tests/golden/footprints.txt` has 6 more lines (F0 + 6), and each line
  is hand-checked against its land pattern table.

References: plan-73-A (§ Prerequisites, which B shares);
`planning/plan-73-data/README.md` (the datasheet table: URLs, sizes, the pages
cited below); `docs/parts-format.md` § Datasheets (line 483), § Footprints
(line 601), § `quad` (line 714), § `explicit` (line 822); `docs/backend.md`
§ `footprint_check` (line 924).

## Prerequisites

plan-73-A § Prerequisites, all rows. B adds:

| Must be true | Command | Status |
|---|---|---|
| Silicon Labs and WCH serve the PDFs to Python | `python3 -I planning/plan-73-data/fetch-datasheets.py` → 8 `ok` lines | MET in substance (2026-11-25: the same urllib request with these headers fetched all eight; sizes in `plan-73-data/README.md`). The script itself is first run in Phase 1 |
| The six footprint ids are free | `for id in QFN-20-3X3-SL QFN-24-4X4-SL-N QFN-24-4X4-SL QFN-28-5X5-SL QFN-28-5X5-SL-C QFN-64-9X9-SL; do grep -c "\"id\": *\"$id\"" schematical/schematical/Parts/footprints.json; done` → six 0s | MET (2026-11-25) |
| No other plan names these ids | `grep -l -E 'QFN-(20-3X3\|24-4X4\|28-5X5\|64-9X9)-SL' planning/plan-*.md \| grep -v plan-73 \| wc -l` → 0 | MET (2026-11-25: 0) |
| No datasheet id taken | `for i in cp2102-9 cp2102n cp2102c cp2104 cp2105 cp2108 cp2110 ch340; do grep -c "\"id\": \"$i\"" schematical/schematical/Parts/datasheets.json; done` → eight 0s | MET (2026-11-25) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- The eight PDFs are in `datasheets/controller-uart/` as `<id>.pdf`. This mirrors
  the existing bridges: `ls datasheets/controller-uart | grep -c -E '^(xr21|cy7c6|mcp2200)'`
  → 10 at planning.
- `schematical/schematical/Parts/datasheets.json` has `{"id", "url"}` entries for
  them, with the URLs of the data README.
- `footprints.json` has the six footprints of § 3.
- `footprint_check` passes with 6 new lines (F0 + 6), each checked by hand.

### Non-goals

- No part, device or golden other than `footprints.txt`.
- No Markdown datasheet conversion (`"md"`).
- No new footprint for the CH340T.

## 2. Current State

- `footprints.json` has 3993 footprints:
  `python3 -c "import json;print(len(json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"`
  → 3993.
- `datasheets.json` has 10962 entries (`grep -c '"id"' …/datasheets.json` → 10962).
- The 4×4 and 5×5 QFNs already in the library come from other makers' land
  patterns, and none matches the Silicon Labs patterns:
  - `QFN-24-4X4`: MaxLinear, pads 0.65 × 0.30, span 3.75, centre 2.70.
  - `QFN-24-MCHP-ML`: pads 0.48 × 0.23, span 3.72, centre 2.6.
  - `QFN-28-5X5-XR`: pads 0.85 × 0.30, span 4.55, centre 3.1.
  - `QFN-64-MC-64M1`: pads 0.7 × 0.3, span 8.9, centre 5.4.
  - The Silicon Labs patterns are in § 3. All were read from
    `python3 -c` over `footprints.json`, 2026-11-25.
- `SSOP-20` is "20-pin SSOP (DB), 5.3 x 7.2 mm body, 0.65 mm pitch" (TI DB0020A,
  span 7, pads 1.85 × 0.45).

## 3. Design: the six footprints

The rule for Silicon Labs' ranged land tables (the CP2102/9, CP2104, CP2105 and
CP2110 tables give Min and Max) is to **use the Max column**. The single-value
tables of the newer CP2102N/CP2102C/CP2108 sheets print exactly those values. For
example, the CP2102N QFN28 table's X1 0.30 and Y1 0.95 are the CP2102/9 table's
X1 and Y1 Max. So one rule covers both table styles.

The names follow the land table: pad width is the land length (Y1), pad height is
the land width (X1), and the span is C1 (the pad-centre distance). The centre pad
is X2 square, unnumbered: its GND is the centre pad and no pin names it.

| id | Parts | Source (`datasheets/controller-uart/`) | style | pins | body | pitch | spanX = spanY | pad (w × h) | centre |
|---|---|---|---|---|---|---|---|---|---|
| `QFN-24-4X4-SL-N` | CP2102N QFN24 | `cp2102n.pdf` p.39 Table 7.2 (C1 3.90, X1 0.30, Y1 0.85, X2 2.55); body p.37 Table 7.1 (D 4.00 BSC) | quad [6,6,6,6] | 24 | 4.0 × 4.0 | 0.5 | 3.90 | 0.85 × 0.30 | 2.55 |
| `QFN-24-4X4-SL` | CP2104, CP2105, CP2110 QFN24 | `cp2104.pdf` p.11 Table 9 (C1 3.90–4.00, X1 0.20–0.30, Y1 0.65–0.75, X2 2.70–2.80); the same table in `cp2105.pdf` p.11 and `cp2110.pdf` p.13; body p.10 Table 8 (D 4.00 BSC) | quad [6,6,6,6] | 24 | 4.0 × 4.0 | 0.5 | 4.00 | 0.75 × 0.30 | 2.80 |
| `QFN-28-5X5-SL` | CP2102, CP2110 QFN28 | `cp2102-9.pdf` p.14 Table 11 (C1 4.80, X1 0.20–0.30, Y1 0.85–0.95, X2 3.20–3.30); the same in `cp2110.pdf` p.15; body p.13 Table 10 (D 5.00 BSC) | quad [7,7,7,7] | 28 | 5.0 × 5.0 | 0.5 | 4.80 | 0.95 × 0.30 | 3.30 |
| `QFN-28-5X5-SL-C` | CP2102C QFN28 | `cp2102c.pdf` p.24 Table 6.2 (C1 4.80, X1 0.30, Y1 0.95, X2 3.35); body p.22 Table 6.1 (D 5.00 BSC) | quad [7,7,7,7] | 28 | 5.0 × 5.0 | 0.5 | 4.80 | 0.95 × 0.30 | 3.35 |
| `QFN-64-9X9-SL` | CP2108 | `cp2108.pdf` p.17 (C1 8.90, X1 0.30, Y1 0.85, X2 4.25); body p.15 Table 5.1 (D 9.00 BSC) | quad [16,16,16,16] | 64 | 9.0 × 9.0 | 0.5 | 8.90 | 0.85 × 0.30 | 4.25 |
| `QFN-20-3X3-SL` | CP2102N QFN20 | `cp2102n.pdf` p.43 Table 8.2 and Figure 8.2; body p.41 Table 8.1 | explicit (below) | 20 | 3.0 × 3.0 | — | — | — | — |

**`QFN-20-3X3-SL` is `explicit`** because four of its pins are corner pads:

- The package drawing (`cp2102n.pdf` p.41, Figure 8.1 bottom view and Detail
  "A") numbers the corner pad 1. Pin 2 is beside it on the same side and pin 20
  on the next side.
- Table 8.1 gives the corner pad size `c` 0.25–0.35 at `f` 2.50 BSC.
- The land pattern (p.43) has 4 pads per side at e = 0.50 plus four square
  corner lands X2 × Y2 (0.25–0.35) at C3 = C4 = 2.50.

So, in top view, counter-clockwise from the top-left corner:

- 1 is the top-left corner, then 2–5 down the left side;
- 6 is the bottom-left corner, then 7–10 along the bottom, left to right;
- 11 is the bottom-right corner, then 12–15 up the right side;
- 16 is the top-right corner, then 17–20 along the top, right to left.

Coordinates in mm (origin = body centre, y down). Side pads are 0.90 long (Y1) ×
0.30 wide (X1) at ±1.55 (C1 3.10 / 2). Corners are 0.35 × 0.35 (Max) at ±1.25
(C3 2.50 / 2). The centre is a `"pin": 0` pad, 1.80 × 1.80 (X3, Y3).

| pins | x | y | width × height |
|---|---|---|---|
| 1 | −1.25 | −1.25 | 0.35 × 0.35 |
| 2, 3, 4, 5 | −1.55 | −0.75, −0.25, 0.25, 0.75 | 0.90 × 0.30 |
| 6 | −1.25 | 1.25 | 0.35 × 0.35 |
| 7, 8, 9, 10 | −0.75, −0.25, 0.25, 0.75 | 1.55 | 0.30 × 0.90 |
| 11 | 1.25 | 1.25 | 0.35 × 0.35 |
| 12, 13, 14, 15 | 1.55 | 0.75, 0.25, −0.25, −0.75 | 0.90 × 0.30 |
| 16 | 1.25 | −1.25 | 0.35 × 0.35 |
| 17, 18, 19, 20 | 0.75, 0.25, −0.25, −0.75 | −1.55 | 0.30 × 0.90 |
| 0 (centre) | 0 | 0 | 1.80 × 1.80 |

Clearances, worked from the table:

- Pin 2's land spans y −0.90…−0.60, and corner pin 1's spans y −1.425…−1.075, so
  the gap is 0.175.
- The centre's edge (0.90) to the side lands' inner ends (1.55 − 0.45 = 1.10)
  is 0.20.
- No two lands overlap.

This numbering matches `cp2102n.pdf` Table 5.3 (p.31): pin 1 GPIO.1/RS485,
3 GND, 6 VDD at the bottom-left corner, 11 SUSPENDb at the bottom-right corner,
16 RTS at the top-right corner. plan-73-C's pinout uses it.

**CH340T on `SSOP-20`.** WCH publishes no package drawing for the CH340T
(`ch340.pdf`, 17 pages). Its § 3 table, p.2, gives SSOP-20, width 5.3 mm, pin
spacing 0.65 mm. That is the JEDEC MO-150 20-lead SSOP that `SSOP-20`
(5.3 × 7.2 mm body, 0.65 mm pitch) already draws. Adding a second footprint with
the same dimensions would be a duplicate, so the CH340T uses `SSOP-20`.

Rejected alternatives:

- **The Min column, or the midpoint.** The newer single-value tables equal the Max
  column, so Max keeps the two generations of Silicon Labs tables consistent.
- **One footprint for CP2102C and CP2102/CP2110 QFN28.** Their centre lands differ
  (3.35 vs 3.30). The rule is one footprint per drawing that differs.
- **`quad` with `slots` for the QFN20.** A `quad` side holds evenly pitched pads
  of one size. The corner pins are a different size, off the side rows, so only
  `explicit` draws them.

## Phases

> **NOTE — keep the checkboxes current as you go** (as in plan-73-A).

### Phase 1 — datasheets

These are new files and new `datasheets.json` entries, which no part names yet.

- [ ] `python3 -I planning/plan-73-data/fetch-datasheets.py` (from the repo root) →
  eight `ok` lines; the PDFs are in `datasheets/controller-uart/`.
- [ ] `schematical/schematical/Parts/datasheets.json`: append the eight entries
  `{"id": "<id>", "url": "<URL>"}` from the data README's table, in its order.
- [ ] `node scripts/check-datasheets.js cp2102-9 cp2102n cp2102c cp2104 cp2105 cp2108 cp2110 ch340`
  → exit 0, no failure lines (each URL answers a PDF). This was probed at planning,
  2026-11-25, with the script's own request (Node `fetch` with its User-Agent):
  - `…/CP2104.pdf` returned 200 `application/pdf`;
  - `https://www.wch-ic.com/download/file?id=79` returned 200 with no
    content-type. Its body starts `%PDF-`, which is what the script checks.

Acceptance: the eight PDFs open and the entries load.
  Check: `for f in cp2102-9 cp2102n cp2102c cp2104 cp2105 cp2108 cp2110 ch340; do pdfinfo datasheets/controller-uart/$f.pdf | grep Pages; done` → the page counts 26, 48, 31, 22, 24, 35, 26, 17 (est. 5 s); `cmake --build /tmp/logic-p73 --target listing_check && /tmp/logic-p73/listing_check` → passes (datasheets.json still parses; est. 1 min).
Commit: —

### Phase 2 — the five `quad` footprints

Five new ids, unused by any part.

- [ ] `schematical/schematical/Parts/footprints.json`: add `QFN-24-4X4-SL-N`,
  `QFN-24-4X4-SL`, `QFN-28-5X5-SL`, `QFN-28-5X5-SL-C` and `QFN-64-9X9-SL` with the
  values of § 3's table.
  - Each `description` names the pins, body, pitch and centre land, as the
    neighbours do.
  - Each `note` names the land-pattern table, file and page, and says "Max
    column" for the ranged tables.
  - `"centre": {"width": X2, "height": X2, "shape": "rect"}`.
- [ ] `/tmp/logic-p73/footprint_check --print > backend/tests/golden/footprints.txt`.
  Then `git diff backend/tests/golden/footprints.txt`: 5 added lines. For each,
  hand-check pad 1's centre against the table:
  - x = −spanX/2, y = −(n/4 − 1)/2 × 0.5. QFN24: (−1.95, −1.25) for `-SL-N` and
    (−2.0, −1.25) for `-SL`; QFN28: (−2.4, −1.5); QFN64: (−4.45, −3.75).
  - The `-` centre pad is X2 square.

Acceptance: `footprint_check` passes against the regenerated golden, and the hand-check of each line holds.
  Check: `cmake --build /tmp/logic-p73 --target footprint_check && ctest --test-dir /tmp/logic-p73 -R footprint` → passed (est. 1 min).
Commit: —

### Phase 3 — the `explicit` QFN20

- [ ] `footprints.json`: add `QFN-20-3X3-SL`, `style: "explicit"`, `pins: 20`,
  `body: {"width": 3.0, "height": 3.0}`, with the 21 pads of § 3's table.
  - Every pad is `"shape": "rect"`.
  - The `note` cites `cp2102n.pdf` p.41 Figure 8.1 (corner pin 1) and p.43
    Table 8.2, "X2/Y2 Max 0.35".
- [ ] Regenerate `footprints.txt` as in Phase 2. One more line: pad 1 at
  `-1.25,-1.25,0.35x0.35`, pad 2 at `-1.55,-0.75,0.9x0.3`, pad 20 at
  `-0.75,-1.55,0.3x0.9`, the `-` pad `0,0,1.8x1.8`.

Acceptance: the line matches § 3's table pad for pad.
  Check: `ctest --test-dir /tmp/logic-p73 -R footprint` → passed; `wc -l backend/tests/golden/footprints.txt` → F0 + 6 (3999 at planning's F0) (est. 1 min).
Commit: —

## Validation Plan

- Tests: `footprint_check` over the six new lines; `listing_check` parses the new
  `datasheets.json`.
- Runtime proof: plan-73-C places parts on these footprints. `footprint_check`'s
  "every footprint used by some part" then covers them.
- Doc sync: none. The footprint and datasheet formats are unchanged, and
  `docs/parts-format.md` § Datasheets needs no new host note: both hosts answer a
  script.
- Final gate: plan-73-C § Validation Plan.

## Decisions

See plan-73-A § Decisions. Author's calls for B, none of them user-visible: Max
column, one footprint per differing drawing, `explicit` for the QFN20, `SSOP-20`
for the CH340T (§ 3).

## Corrections

## Summary

The only geometry risk is the QFN20's corner pins. § 3 settles their numbering
from the package drawing and the land pattern, and Phase 3's line check compares
every pad against the table.
