# plan-51-D: IC sockets — footprints: DIP sockets

Last updated: 2026-11-21
Effort: large (3h–1d)
Depends on: plan-51-C

D writes the footprint generator and runs it for the **DIP** socket groups
(through hole and surface mount), including the through-hole footprints the generic
DIP sockets use. The objects go to a side file, not `footprints.json`: the loader
check `footprint_check` fails on a footprint no part uses, so plan-51-F merges
them with the parts (`gen-sockets.js --write`).

Outcome of D: `node scripts/gen-sockets.js --letter D --out planning/plan-51-data/footprints-D.json`
writes one `dual` object per distinct `(DIP row of series.tsv, pin count)` geometry;
every object loads in the real loader with no overlapping pads; the generic DIP
footprints (Mill-Max 110 through hole, 11 shapes) are among them.

References:

- `docs/parts-format.md` § Footprints (`dual`; § The `"footprint"` part key: ids,
  notes, R5, R7–R9); plan-51-C § 3.1 (the socket pad rules) and `series.tsv`.
- `scripts/gen-headers.js` (the precedent: a generator that writes parts and
  footprints, idempotent, replaces its own ids in `footprints.json`).
- `planning/plan-51-data/fits.tsv`.

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-51-C complete | `ls planning/completed/plan-51-C-*.md` → exists | NOT MET (2026-11-21: not started) |
| `series_check.py` passes | `python3 -I planning/plan-51-data/census/series_check.py` → `ok` | NOT MET (C's output) |
| The backend builds | `cmake -S backend -B /tmp/logic-p51 -DCMAKE_BUILD_TYPE=Release && cmake --build /tmp/logic-p51 -j8` | MET (2026-11-21) |

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> If you stop, report the status of *all* prerequisites.

## 1. Goal

- `footprints-D.json` loads, and the footprint check reports only `extra footprint`
  and `unused footprint` lines, for exactly its ids.
- `gen-sockets.test.js` passes.

### Non-goals (explicit constraints)

- No change to `footprints.json`, `Parts/*.json`, the goldens, the backend or the app (F merges).
- No existing footprint changes; chip footprints (`PDIP-14` …) are not reused as sockets.
- No PLCC (E).

## 2. Current State

- `dual` footprints: `pins`, `pitch`, `span`, `pad` `{width, height, shape}`, `drill`,
  `pin1Shape`, `body` (`docs/parts-format.md` § `dual`). A through-hole DIP uses round
  pads and `pin1Shape` `rect` (`PDIP-14` in `footprints.json`).
- A surface-mount DIP socket's pads are rectangles on `dual` (`SOIC-14` is the style
  precedent: `dual`, rect pads, no drill).
- `scripts/gen-headers.js` writes `Parts/header.json` and replaces every `PINHDR-`
  object of `footprints.json`; running it twice changes nothing (`docs/parts-format.md`
  § `header`).
- `footprint_check` (`backend/tests/footprint_check.c`) compares every footprint with
  `tests/golden/footprints.txt` and fails on a footprint no part names
  (`unused footprint`, `:172`); it reads `PARTS_DIR` set by `backend/CMakeLists.txt:136`
  relative to the source tree.

### Measured populations

| What | Count | Command |
|---|---|---|
| DIP `(maker, series, mounting, spacing, pins)` variants at planning time | 188 | `awk -F'\t' 'NR>1{k=$1"\|"$3"\|"$4"\|"$5"\|"$6"\|"$7; if(!(k in s)){s[k]=1; c[$4]++}} END{for(f in c) print f, c[f]}' planning/plan-51-data/candidates.tsv` → `DIP 188`, `PLCC 15` (an upper bound on D's objects: equal geometries share one) |
| Chip footprints the DIP sockets hold | 19 of the 24 (the other 5 are PLCC) | `python3 -I planning/plan-51-data/census/chips.py` |

## 3. Design

### 3.1 The generator

`scripts/gen-sockets.js` (one script for D, E and F):

| Mode | Does |
|---|---|
| `--letter D --out F` | writes `{"footprints": […]}` for the DIP rows of `series.tsv` |
| `--letter E --out F` | the same for the PLCC rows (plan-51-E) |
| `--write` | plan-51-F: replaces every `SKT-` object of `footprints.json` by the objects the `socket` parts name, and writes `Parts/socket.json` |
| `--check` | exits 1 when `footprints.json` or `socket.json` differ from what `--write` would write |

Input: `planning/plan-51-data/series.tsv`. One object per `(row, N)` for each `N`
in the row's `pins`; `dual`: `pins` N, `pitch` 2.54, `span` the row spacing,
`pad` and `drill` from the row (`shape` `round` through hole; `rect` pad
`pad_w × pad_h` surface mount), `pin1Shape` `rect` through hole, `body`
`{width: body_w, height: body_len[N]}`, `note` naming the drawing, file and page and
the numbers read (`Mill-Max 088-145.pdf p.<n>: hole Ø…, body …; R1`).

Id: `SKT-DIP<N>-<300|600>-<TH|SM>-<maker code>-<series>` with maker codes `MM`
(Mill-Max), `PD` (Preci-Dip), `AS` (Assmann), e.g. `SKT-DIP14-300-TH-MM-110`.
A row whose object equals an earlier one in every key but `id`, `description` and
`note` reuses the earlier id: no two footprints have the same pads. `description`:
`14-pin DIP socket, Mill-Max 110, through hole, 7.62 mm row spacing`.

### 3.2 The check script

`planning/plan-51-data/census/fp-load.sh <side.json>`: copies `backend/` and
`schematical/schematical/Parts/` to `/tmp/p51-fp/` (keeping their relative layout,
which is how `PARTS_DIR` finds the parts), appends the side file's objects to the
copy's `footprints.json`, builds `footprint_check` there with cmake, runs it, and
prints `<n> new footprints: load ok` when its output has no loader error, no
`differs` or `missing`, and `extra footprint` and `unused footprint` lines for exactly
the `n` new ids. The loader rejects overlapping pads (`docs/parts-format.md`
§ Footprints: "No overlapping pads"), so the check covers it.

### Risk

Wrong numbers read in C. The loader's checks (pad overlap, drill smaller than pad)
catch the impossible ones; the golden (`footprints.txt`, F) records the rest for review.

## 4. Phases

### Phase 1 — the generator and its check

- [ ] `scripts/gen-sockets.js` per § 3.1 (`--letter D`, `--check` stubbed to the DIP rows until F).
- [ ] `scripts/gen-sockets.test.js` (`node --test`): a one-row `series.tsv` fixture for a 14-pin through-hole socket gives one `dual` object with `pins` 14, `span` 7.62, `pin1Shape` `rect`, the id `SKT-DIP14-300-TH-MM-110`; two rows with equal geometry give one object; a surface-mount row has no `drill` and `rect` pads.
- [ ] `planning/plan-51-data/census/fp-load.sh` per § 3.2.

Acceptance: the test passes; `fp-load.sh` accepts a known-good side file and rejects one with a footprint of overlapping pads.
  Check: `node --test scripts/gen-sockets.test.js` → `# pass` and no `# fail` (est. 5 s).
Commit: —

### Phase 2 — the DIP footprints

- [ ] `node scripts/gen-sockets.js --letter D --out planning/plan-51-data/footprints-D.json`.
- [ ] `--letter D` exits 1, writing nothing, when one of the 11 DIP shapes of `fits.tsv` has no `generic` through-hole row in `series.tsv` (the Mill-Max 110 rows, plan-51-C § 3.3); its test case is in Phase 1's test file.
- [ ] `docs/parts-format.md` § The `"footprint"` part key: a rule **R10 — IC sockets** after R9: through-hole sockets R1 or R5 (hole, pad = drill + 0.7 at least 1.5, `pin1Shape` `rect`), surface-mount R1, R2 or R3, body the housing outline, as plan-51-C § 3.1.

Acceptance: the side file loads; its ids are exactly the "new" ids.
  Check: `planning/plan-51-data/census/fp-load.sh planning/plan-51-data/footprints-D.json` → `<n> new footprints: load ok` (est. 5 min: it builds `footprint_check` in a copy).
Commit: —

## Validation Plan

- Tests: `gen-sockets.test.js`; `fp-load.sh` through the loader.
- Coverage check: `fp-load.sh` counts the new ids against the `extra footprint` lines, so a dropped object fails.
- Runtime proof: F merges the objects and runs the whole `ctest`.
- Doc sync: `docs/parts-format.md` § Footprints (R10).
- Final gate: plan-51-J.

## Decisions

See plan-51-A § Decisions. Author decisions:

- **One footprint per distinct geometry**, id by the first `(maker, series)` that has it.
- **Footprints wait in a side file for F**, because `footprint_check` fails on an unused footprint (plan-45-C precedent).
- **One generator script for footprints and parts**, as `gen-headers.js`.

## Corrections

## Summary

D's risk is the numbers C read; the loader's pad checks and the golden review bound it.
Untouched: `footprints.json`, parts, backend, app.
