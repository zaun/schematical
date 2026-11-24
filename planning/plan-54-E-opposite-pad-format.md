# plan-54-E: Footprint format — opposite-side pads and edge-mounted footprints (backend)

Last updated: 2026-11-22
Effort: medium (1h–2h)
Depends on: plan-54-D

A straddle-mount card-edge socket (DigiKey "Board Edge, Straddle Mount": 99 of the 1270
candidates) has contacts on both faces of the board, so its land pattern has surface-mount
pads on the top layer and on the bottom layer, and its housing hangs over the board edge
on purpose. The footprint format has neither (`docs/parts-format.md` § `connector` says
parts that need pads on the bottom layer are not in the library). The owner decided
(2026-11-22) to add support as part of plan-54. E is the backend half: an `explicit` pad
may be an **opposite-side** pad (`"opposite": true`: a surface-mount pad on the other face
of the board from the footprint's own side) and a footprint may be **edge-mounted**
(`"edgeMount": true`: it straddles the board edge). E changes the loader, the golden
printer, the tests and `docs/parts-format.md`; it adds no footprint and no part. F and G
make the app use the two keys. The behavioral outcome: a footprint with an opposite-side
pad or `edgeMount` loads and exposes `opposite` and `edge_mount`; every malformed one is
rejected with a message that names the key; `footprint_check` prints them; the 1895
existing footprints and every golden line are unchanged.

References:

- `docs/parts-format.md` § Footprints (§ Common keys, § `explicit`, the USB paragraph in
  § `connector`); `docs/backend.md` § Tests (the footprint golden).
- `backend/include/parts.h` (`FootprintPad` at line 248, `PartFootprint` at line 262),
  `backend/src/library/footprints.c` (`COMMON_KEYS` line 33, `PAD_KEYS` line 47,
  `explicit_hole` line 685), `backend/tests/footprint_check.c` (`footprint_line`),
  `backend/tests/library_check.c` (the explicit slotted and non-plated pad tests at
  line 2780: the precedent for a pad attribute with its own negative tests).

## Prerequisites

plan-54-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-54-D complete | `ls planning/completed/plan-54-D-*.md` → exists | NOT MET (2026-11-22: not started) |
| The format has no opposite pad or edge mount | `grep -c 'opposite\|edgeMount' backend/src/library/footprints.c backend/include/parts.h` → `0` and `0` | MET (2026-11-22 at 7b449718: 0, 0) |
| `FootprintPad` ends at `corners` | `grep -c 'unsigned corners;        /\* PAD_CORNER_\* bits the chamfer cuts \*/' backend/include/parts.h` → 1 | MET (2026-11-22: 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- An `explicit` footprint pad may carry `"opposite": true`: allowed only on a
  surface-mount pad (no `drill`, no `slotLength`, no `"plated": false`); `"opposite":
  false` and any other value are rejected (`"opposite" may only be true`).
- A footprint of any style may carry `"edgeMount": true` (only true, like `plated`).
- `FootprintPad` has `int opposite` and `PartFootprint` has `int edge_mount` (0 unless set).
- `footprint_check` prints an opposite pad with `,opp` and an edge-mounted footprint with a
  last `|edge` field; the 1895 existing lines are unchanged
  (`git diff --stat backend/tests/golden` → empty).
- `docs/parts-format.md` describes both keys and no longer says that pads on the bottom
  layer are outside the format.

### Non-goals (explicit constraints)

- No footprint and no part uses the keys yet (the footprints are H to K's, the parts M and N's).
- No Swift change (F and G); the app still builds because it ignores the new fields.
- No inner-layer pads and no per-pad layer choice other than "opposite": a pad is on the
  footprint's own face or on the other face.
- No change to `plated`, `slotLength`, the styles `dual`, `quad`, `grid`, `header`.

## 2. Current State

`footprints.c` expands each footprint into a flat `FootprintPad` list. An `explicit` pad
takes `PAD_KEYS` (`pin`, `ball`, `x`, `y`, `width`, `height`, `shape`, `drill`,
`slotLength`, `plated`, `radius`, `chamfer`, `corners`); `explicit_hole` validates the
hole keys and sets `npth`; the other styles' pads take `SIZE_KEYS` only, so an unknown key
on them is already rejected. `COMMON_KEYS` is `id, description, note, style, pins, body`;
`kind_check_keys` rejects any other footprint key. `footprint_check.c` prints each pad as
`designator:x,y,wxh,shape[,d…][,s…][,npth][,r…][,c…]` and each footprint as
`id|pins=N|body=WxH[ round]|pads=…`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Footprints in the library | 1895 | `wc -l < backend/tests/golden/footprints.txt` |
| Straddle-mount candidates (they need both keys) | **99** (Samtec 28, TE Connectivity 32, Amphenol ICC 39) | `awk -F'\t' '$20=="candidate" && $11=="Board Edge, Straddle Mount"{n[$2]++} END{for(k in n) print k, n[k]}' planning/plan-54-data/candidates.tsv` |
| Backend test programs | 41 | `grep -c '^logic_test' backend/CMakeLists.txt` |
| Existing negative pad tests of the same shape | `expect_fp_fail("explicit: plated true", …)` and its neighbours | `grep -n 'expect_fp_fail("explicit' backend/tests/library_check.c` |

### Verified properties

- `"plated"` is the precedent: optional, only `false`, rejected on other values with a
  message that names the key (`footprints.c:703`), and printed by `footprint_check` as a
  token only when set (`npth`), so adding a token printed only when set leaves every
  existing golden line byte for byte.
- A pad on another style (`dual`, `quad`, `grid`, `header`) cannot carry the key: those
  styles' pads read `SIZE_KEYS`, so `"opposite"` there is an unknown key.

## 3. Design

| Key | Rule | Message |
|---|---|---|
| pad `opposite` | optional, only `true`; `explicit` pads only | `"opposite" may only be true` |
| pad `opposite` with `drill`, `slotLength` or `"plated": false` | rejected | `an opposite pad is surface mount` |
| footprint `edgeMount` | optional, only `true` | `"edgeMount" may only be true` |

`COMMON_KEYS` gains `"edgeMount"`; `PAD_KEYS` gains `"opposite"`. `FootprintPad` gains
`int opposite; /* explicit only: 1 = a surface-mount pad on the other face of the board */`
and `PartFootprint` gains `int edge_mount; /* 1 = straddles the board edge */`. The loader
copies them after the pad's size and hole are read.

Meaning (the docs say it): a footprint is drawn as seen from its own side; an opposite pad
is copper on the other face; when the part is placed on the bottom side every pad's layer
swaps, so the opposite pads are then on Top. `edgeMount` marks a footprint whose housing
hangs over the board edge: the app checks only its pads against the board outline; the
footprint's `note` says where the board edge is relative to the pads.

`footprint_check.c` prints `,opp` after the pad's hole tokens (after `npth`) and `|edge`
as the last field of the line (only when set).

**Correctness risk** is small: the keys are new and no shipped footprint uses them. The
one existing behavior they touch is the `explicit` pad's key check, covered by the existing
negative tests. **Byte-identity** is the gate for the existing footprints: the golden does
not change; the expected diff is empty.

Rejected alternatives: a `"layer": "top" | "bottom"` pad key (rejected: a footprint is drawn
for its own side and flips with the part, so the absolute layer would be wrong after a flip);
a body outline flag on the straddle part's housing (rejected: the placement check needs it on
the footprint).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The loader and its tests

- [x] `backend/include/parts.h`: `FootprintPad` gains `int opposite;` after `corners`,
      `PartFootprint` gains `int edge_mount;`, each with its comment (§ 3).
- [x] `backend/src/library/footprints.c`: `PAD_KEYS` and `COMMON_KEYS` gain the keys; the
      explicit pad reader sets `pad->opposite` with the rules of § 3 (a helper beside
      `explicit_hole`); the footprint reader sets `edge_mount`.
- [x] `backend/tests/library_check.c` (`check_opposite_pads`): beside the explicit hole tests, `ok   explicit:
      opposite-side pads and edge mount load` (a footprint with one top pad, one `opposite`
      pad and `"edgeMount": true` loads; `opposite` 1 on the second pad and 0 on the first,
      `edge_mount` 1) and `expect_fp_fail` for: `"opposite": false`, `"opposite": 1`,
      `opposite` with `drill`, with `slotLength`, with `"plated": false`, `"edgeMount":
      false`, `"edgeMount": "yes"`, `opposite` on a `dual` footprint's `pad`.

Acceptance: the loader accepts the keys and rejects the malformed ones.
  Check: `cmake -S backend -B /tmp/logic-p54 && cmake --build /tmp/logic-p54 -j8 --target library_check && /tmp/logic-p54/library_check` → every line `ok`, exit 0, including `ok   explicit: opposite-side pads and edge mount load` (est. 8 min: a first build).
  Result: exit 0, 481 `ok` lines, 0 `FAIL`, the `ok   explicit: opposite-side pads and edge mount load` line present.
Commit: see Phase 2 (one commit for both phases)

### Phase 2 — The golden printer and the docs

- [x] `backend/tests/footprint_check.c`: the grammar comment and `footprint_line` print `,opp`
      and `|edge` (§ 3); `docs/backend.md` § Tests' footprint line grammar gains both.
- [x] `docs/parts-format.md`: § Common keys gains `"edgeMount"`; § `explicit` gains `"opposite"`
      with the example `{"pin": 3, "x": 1.5, "y": 2, "width": 0.4, "height": 1.2, "shape":
      "rect", "opposite": true}`; the USB paragraph in § `connector` now reads: "Parts that need
      rotated slots or L-shaped / keyhole shell slots are not in the library
      (`datasheets/skipped-parts.md`); a pad on the other face of the board is an `opposite`
      pad (§ `explicit`), and the USB parts skipped for pads on the bottom layer stay skipped."
- [x] `footprint_check` still passes against the unchanged golden (`ctest -R 'library_check|footprint_check|listing_check'` → `100% tests passed, 0 tests failed out of 3`).

Acceptance: the printer prints the keys and the existing golden is unchanged.
  Check: `cmake --build /tmp/logic-p54 -j8 && ctest --test-dir /tmp/logic-p54 -R 'library_check|footprint_check|listing_check'` → `100% tests passed`; `git diff --stat backend/tests/golden` → empty; `grep -c 'opposite\|edgeMount' docs/parts-format.md` → at least 4 (est. 10 min).
  Result: `100% tests passed, 0 tests failed out of 3`; the golden diff is empty; `grep -c` → 8.
Commit: COMMIT_E

## Validation Plan

- Tests: `library_check` (the new cases, negative included), `footprint_check` (the golden
  unchanged; the printer's new tokens are exercised by F's and G's fixtures and by the
  parts of H to N).
- Coverage check: `footprints.c` is in the `logic` library every test links
  (`add_library(logic STATIC` in `backend/CMakeLists.txt`); a fixture with the keys goes
  through `footprint_read_entries` in `library_check`.
- Runtime proof: the `library_check` line of Phase 1.
- Doc sync: `docs/parts-format.md` § Footprints and § `connector`, `docs/backend.md` § Tests.
- Final gate: none; the landing tier is P's.

## Decisions

- Owner (2026-11-22): **add support for straddle-mount parts as part of plan-54** (it settles
  that the 99 rows are footprints, not skips).
- Author: **Opposite-side pads are a pad key relative to the footprint's own side** (§ 3 rejected
  alternative), **surface mount only**, **explicit only**.
- Author: **`edgeMount` is a boolean on the footprint** and the board edge is described in the
  `note`, not modelled: the app only relaxes the placement check (F).

## Corrections

- **E was worked while B's readers ran, before D.** E's `Depends on: plan-54-D` and its "plan-54-D complete" prerequisite row
  order the letters; nothing in E reads B, C or D's output (it is a loader, a printer and docs, with no part or footprint using
  the keys). The prerequisite is a serialization choice, so E's code was written while the data readers of B ran
  (2026-11-23). Every other row of E's Prerequisites was measured at its start: `grep -c 'opposite\|edgeMount'
  backend/src/library/footprints.c backend/include/parts.h` → `0` and `0`; the `corners` comment line → 1.

## Summary

E is two optional keys, their loader, printer, tests and docs. Nothing ships with them yet; the
goldens must not move.
