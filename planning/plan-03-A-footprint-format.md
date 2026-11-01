# plan-03-A: Footprint format, loader and LED footprints

Last updated: 2026-10-31
Overall Effort: x-large (1d–3d) — plan-03 is A → B → C → D
Effort: large (3h–1d)
Depends on: plan-02-C (all of plan-02: pin designators, file version 2, BGA ball pinouts)

plan-03 turns today's canvas into a **Schematic** tab and adds a **PCB** tab
where every placed part appears as its footprint (pads, body outline, pin-1
mark, reference) and can be moved and rotated. Footprints only: no copper
wires, no nets, no ratsnest, no board outline, no design-rule checks.

The footprint of each part comes from the part library. This sub-plan adds
the data format: a new part file `Parts/footprints.json` that defines named
footprints parametrically (package style + pad pitch, row spacing, pad size,
drill, body size, pin-to-pad map), a `"footprint"` key on every part that
names one, a C loader that checks and expands each footprint into a flat pad
list, and the API the app reads. It lands end to end on the 55 LED parts
(11 generic footprints whose dimensions this plan fixes); plan-03-B and C
fill in the other 3,760 parts, plan-03-D builds the PCB tab.

Outcome of A: `load_footprints()` loads `footprints.json`; the 55 LED parts
carry a footprint; `footprint_check` prints every footprint's expanded pads
and matches `backend/tests/golden/footprints.txt`; every other part loads
unchanged; the app still builds and loads the library.

References:

- `docs/parts-format.md` (part file format), `docs/backend.md` (layout, tests,
  goldens), `AGENTS.md` (datasheet and part-file rules).
- `backend/include/parts.h`, `backend/src/library/library.c`
  (`PART_KEYS`, `load_part`, `load_file`), `backend/src/library/kind_util.c`
  (`kind_check_keys`, `kind_string`, `kind_uint`, errors).
- `backend/tests/support/read_file.c:load_parts_dir`,
  `backend/tests/support/listing.c`, `backend/tests/listing_check.c` (`--print`).
- `schematical/schematical/Backend/PartCatalog.swift` (`BackendCatalog.load`,
  `CatalogPart.init(_:)`).

## Prerequisites

| Must be true | Command | Status |
|---|---|---|
| plan-01 complete | `ls planning/plan-01-* 2>/dev/null; ls planning/completed/plan-01-G-app-cutover.md` → no active plan-01 file; the completed file exists | MET (2026-11-01) |
| plan-02 complete (A, B, C archived) | `ls planning/plan-02-* 2>/dev/null` → no output; `ls planning/completed/plan-02-C-*` → exists | MET (2026-11-01: no active plan-02 file; plan-02-C-ball-data.md archived) |
| Pins have designators | `grep -n 'const char \*designator' backend/include/parts.h` → one match in `PartPin` | MET (2026-11-01: parts.h:39) |
| Backend tests green | `cmake -S backend -B /tmp/logic-plan03 && cmake --build /tmp/logic-plan03 -j8 && ctest --test-dir /tmp/logic-plan03` → `100% tests passed, 0 tests failed out of 7` | MET (2026-11-01, 7/7, 18.7 s) |
| Worktree clean | `git status --porcelain` → empty | MET (2026-11-01: only the four untracked plan-03 docs, committed first) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the status of *all*
> prerequisites.

## 1. Goal

- `footprint_check` (new CTest) loads `footprints.json` plus every part file
  and its output equals `backend/tests/golden/footprints.txt`: one line per
  footprint with its expanded pads, 11 lines in A.
- Every LED part's listing line ends in `|footprint=<id>`;
  `listing_check` passes after regenerating `parts.txt`, and the diff against
  the old golden touches exactly the 55 LED lines.
- `library_check` covers every new rejection rule (§4.4).
- The app builds and its drawer still lists 3,815 parts.

### Non-goals (explicit constraints)

- No part's behavior changes: `traces.txt` is byte-identical (`git diff
  --exit-code backend/tests/golden/traces.txt`).
- No listing line changes except the appended `|footprint=` field.
- Part file `version` stays `1`. The new key is optional in A and B; plan-03-C
  makes it required.
- No app UI change in A (the PCB tab is plan-03-D).
- `.schematical` files are untouched in A.

## 2. Current State

- **Part files are strict.** `library.c:162-168`: `FILE_KEYS` =
  format/version/kind/devices, `PART_KEYS` = name/package/pinout/flags; any
  other key is rejected unless the kind lists it (`ops->part_keys`). So the
  new key must be added to `PART_KEYS` and a footprint table cannot live in a
  kind file without changing `FILE_KEYS`.
- **Every bundled JSON is loaded as a part file.**
  `PartCatalog.swift:BackendCatalog.load` loads every `*.json` resource in
  file-name order through `load_parts`; `read_file.c:load_parts_dir` does the
  same for the tests. A `footprints.json` would be rejected (`"format" must be
  "schematical-parts"`) unless both loaders treat it separately.
- **JSON in the app folder is bundled automatically.** The app folder is an
  Xcode synchronized group; `Parts/*.json` land flat in
  `Contents/Resources/` (verified in plan-01-G §2). No `project.pbxproj` edit
  is needed for `footprints.json`.
- **Part lines are one per line.** Every part is a single line
  `{"name":"…","package":"…","pinout":"…",…}` (measured below), so adding
  `"footprint":"…"` after `"pinout"` is a line-local edit.
- **Listing goldens.** `listing.c` builds one line per part; `listing_check
  --print` regenerates `parts.txt` (`listing_check.c:67`).
- **Packages are display strings, not geometry.** `Part.package` is free text
  (`"DIP"`, `"2520 VSON"`); the same string covers different physical
  packages (below), so a footprint cannot be derived from it.

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts in the library | 3,817 | `python3 -I` sum of `len(device["parts"])` over `Parts/*.json` → 3817 |
| Parts per kind | gate 2552, dff 672, jkff 186, sram 172, oscillator 93, led 55, 7seg 49, switch 36, supply 2 | same script, `Counter` by kind |
| Part lines matching `^ *{"name":"…","package":"…","pinout":"…"` | 3,817 of 3,817 | `cat Parts/*.json \| grep -c '^ *{"name":"[^"]*","package":"[^"]*","pinout":"[^"]*"'` → 3817; `grep -c '"pinout":'` → 3817 |
| Distinct (package string, pin count) pairs | 88 | `Counter((package, len(pinout)))` → 88 |
| Pairs that are more than one physical package | 4 (SON-6: DRY 36 / DSF 33; DSBGA-6: YZP 7 / YFP 4; DSBGA-8: YZP 13 / YFP 5; X2SON-5: DPW 10 / DTX 4) | designator substring count over part names, no part left unmatched |
| Footprints plan-03 defines | 126: A 11 (LED), B 47 (logic), C 68 (SRAM 20, oscillator 6, switch 15, 7-seg 27) | listed in each sub-plan |
| LED parts by package | 0402, 0603, 0805, 1204, 1205, 1206, 1209, 1411, 1414, DIP (4 pins), Radial: 5 each | `led.json` parts, 5 colors × 11 |

### Verified properties

- LED parts have no datasheet in the repo (`ls datasheets` → no LED
  directory; `led.json` devices carry no `note`). Their footprints therefore
  use the fixed dimensions in §4.5, chosen by this plan.
- LED `two-pin` pinout: pin 1 = K, pin 2 = A; `dip4`: 1 = K, 2 = A, 3–4 = NC
  (`led.json` device `led-red`).
- Loading order: `footprints.json` sorts after `7seg.json` and `dff.json`
  (`ls Parts`), so file-name order cannot be relied on; both loaders load it
  explicitly first (§4.3).

## 3. Design Overview

Four pieces, each small:

1. **`footprints.json`** — a new file format (`"schematical-footprints"`)
   holding named, parametric footprints in millimetres. Three parametric
   styles cover nearly every package in the library: `dual` (two rows: DIP, SOIC, SOP, SSOP,
   TSSOP, SOJ, TSOP, SOT-23, SC-70, SON, chip LEDs, oscillators, switches,
   7-seg), `quad` (four sides: LCCC, QFN), `grid` (BGA). A `slots` map places
   pins on non-sequential positions (SOT-23-5, LCCC, odd 7-seg layouts), and
   an optional `centre` pad covers exposed pads and X2SON-5's middle pin. A
   fourth style, `explicit`, lists pads literally for the few land patterns
   with mixed pad sizes.
2. **The loader** — `load_footprints()` parses and checks the file, then
   *expands* each footprint into a flat list of pads (pin, x, y, width,
   height, shape, drill) stored in the library arena. The app never computes
   geometry; it draws rectangles and ellipses.
3. **The part key** — `"footprint": "<id>"` on a part; `load_part` resolves it
   to a `const PartFootprint *` on `Part` and checks that every pin's
   designator (plan-02) has a pad and every numbered pad a pin.
4. **Goldens** — `footprints.txt` (one line per footprint, expanded pads) and
   the `|footprint=` suffix on listing lines.

Correctness risk concentrates in the expansion arithmetic (pad positions per
style and slot mapping). It is pinned by the `footprints.txt` golden, whose
first 11 lines are hand-checked against §4.5 before being committed.

**Byte-identity is not this plan's gate** for `parts.txt` (55 lines change on
purpose) but **is** for `traces.txt` (behavior must not change).

Rejected alternatives:

- *Geometry inline on each part / a table in each kind file* — owner chose a
  shared file (Decisions). Inline repeats a 14-pad SOIC 1,000+ times.
- *Derive footprint from `package` + pin count* — four (package, pin count) pairs cover
  two physical packages each (§2), and 7-seg / switch "DIP" packages have
  non-IC row spacings.
- *Expand pads in Swift* — the app has no test target; C has CTest and
  goldens. Expansion in C is tested once and drawn trivially.
- *Only explicit pad lists* — 126 footprints, up to 90 pads each;
  parametric styles keep entry short and checkable against a datasheet's
  pitch/span numbers. `explicit` exists only for the odd land pattern.
- *Load `footprints.json` through `load_parts` dispatching on `"format"`* —
  part files earlier in name order would see unknown footprints; a separate
  call made first is simpler.

## 4. Detailed Design

### 4.1 Coordinates and units

Millimetres, JSON numbers. A footprint's origin is the centre of its body;
x grows right, y grows down (the app's convention). Pin 1 is at the top of
the left side. Pads are axis-aligned.

### 4.2 `footprints.json`

```json
{
  "format": "schematical-footprints",
  "version": 1,
  "footprints": [
    {
      "id": "SOIC-14",
      "description": "14-pin SOIC, 3.9 mm body, 1.27 mm pitch",
      "note": "TI D0014A example board layout (sn74hc00.pdf p.24)",
      "style": "dual",
      "pins": 14,
      "body": {"width": 3.9, "height": 8.65},
      "pitch": 1.27,
      "span": 5.4,
      "pad": {"width": 1.55, "height": 0.6, "shape": "rect"}
    }
  ]
}
```

Common keys (every style):

| Key | Value |
|---|---|
| `"id"` | unique footprint name, `[A-Z0-9.-]+` |
| `"description"` | one line for people |
| `"note"` | optional provenance; ignored |
| `"style"` | `"dual"`, `"quad"`, `"grid"` or `"explicit"` |
| `"pins"` | number of designated pads. Parametric `dual`/`quad`/`explicit` pads are designated `"1"`…`"<pins>"`; `grid` pads by ball name. Every part that uses the footprint must have exactly these designators (plan-02 `PartPin.designator`). |
| `"body"` | `{"width", "height"}` > 0, optional `"round": true` (drawn as an ellipse) |
| `"pad"` | `{"width", "height", "shape"}`; width/height > 0; shape `"rect"` or `"round"` (ellipse; a circle when width = height). Not used by `explicit`. |
| `"drill"` | optional, default 0 (surface mount); > 0 = through-hole diameter, must be smaller than both pad sides |
| `"pin1Shape"` | optional; pin 1's pad shape (THT parts use `"rect"` to mark pin 1) |
| `"centre"` | optional, `dual` and `quad` only: one pad at the origin, `{"width", "height", "shape"}` — an exposed/thermal pad, or the middle terminal of an X2SON-5 |
| `"slots"` | optional; `slots[i]` = the slot (1-based) pin i+1 sits on; `0` = the pin is the `centre` pad. Default: pin n on slot n. Length = `pins`; non-zero values distinct and ≤ the slot count; at most one `0`, and only with `centre`. Slots not named get no pad; a `centre` no pin names is unnumbered. |

`dual` keys: `"pitch"` (> 0), `"span"` (centre-to-centre distance between the
two pad columns, > 0), `"perSide"` (optional, default `pins / 2`; required when
`pins` is odd). Slot count = 2 × perSide. Slots 1…perSide run down the left
column, slots perSide+1…2·perSide run up the right column (the usual
counter-clockwise numbering):

```
slot s ≤ P:   x = −span/2,  y = (s − 1 − (P−1)/2) · pitch
slot s > P:   x = +span/2,  y = ((2P − s) − (P−1)/2) · pitch
```

`quad` keys: `"sides"` = `[left, bottom, right, top]` slot counts (each ≥ 0),
`"pitch"`, `"spanX"` (centre distance of the left and right pad columns),
`"spanY"` (of the top and bottom rows). Slots run counter-clockwise: left side
top→bottom, bottom left→right, right bottom→top, top right→left; each side's
pads are centred on that side. Left/right pads are `pad.width × pad.height`;
top/bottom pads are rotated (`pad.height × pad.width`).

`grid` keys: `"rows"` (1–20), `"columns"` (1–30), `"pitch"`, `"balls"`
(array of the populated ball names, length `pins`, any order). A pad's
designator is its ball name, which is what plan-02-C's ball pinouts call the
pin, so a grid footprint needs no pin mapping. Rows are letters
A, B, C, … skipping I, O, Q, S, X, Z; columns are 1…columns. Row A is the top,
column 1 the left (top view, as on a land-pattern drawing). Ball names are
distinct and inside the grid. `"slots"` is not allowed on `grid`. Ball pads are
`pad` (normally `"round"`).

```
column c, row index r (0 = A):  x = (c − 1 − (columns−1)/2) · pitch,
                                 y = (r − (rows−1)/2) · pitch
```

`explicit` keys: `"pads"` = array of `{"pin", "x", "y", "width", "height",
"shape"}` plus optional `"drill"`, coordinates as in §4.1. Every pin 1…`pins`
appears exactly once; `"pin": 0` marks an unnumbered pad (any number of them).
`"slots"`, `"centre"`, `"pin1Shape"` and the top-level `"pad"`/`"drill"` are
not allowed. This is the escape hatch for land patterns the three parametric
styles cannot express — mixed pad sizes such as the SiT1534 2012 package
(two large XTAL pads with VDD/GND between them).

No other key is allowed anywhere; a duplicate key is an error (same rule as
part files).

### 4.3 C API (`backend/include/parts.h`)

```c
typedef enum { PAD_RECT, PAD_ROUND } PadShape;

typedef struct {
    const char *designator;  /* "14" or "A1", matching PartPin.designator; NULL = unnumbered centre pad */
    double x, y;             /* centre, mm, footprint origin = body centre */
    double width, height;    /* mm */
    double drill;            /* mm, 0 = surface mount */
    PadShape shape;
} FootprintPad;

typedef struct {
    const char *id, *description;
    unsigned pins;
    double body_width, body_height;
    int body_round;
    const FootprintPad *pads; size_t pad_count;   /* "1"…"N" order (grid: row-major), unnumbered centre last */
} PartFootprint;

/* Part gains: */
    const PartFootprint *footprint;   /* NULL = none (supplies) */

int load_footprints(PartLibrary *library, const char *json, const char *source, char **error);
size_t part_library_footprint_count(const PartLibrary *library);
const PartFootprint *part_library_footprint_at(const PartLibrary *library, size_t index);
const PartFootprint *part_library_find_footprint(const PartLibrary *library, const char *id);
```

- `load_footprints` follows `load_parts`' contract: 0 on success; -1 and a
  malloc'd message naming the place (`footprints.json: footprints[4] (SOT23-5):
  slots: slot 7 is beyond 6 slots`); nothing from a failed file is added.
  It may be called more than once; ids are unique across calls.
- `load_part` reads `"footprint"` (added to `PART_KEYS`), looks it up, and
  rejects `unknown footprint "X"`, `footprint "X" has 14 pads, pinout
  "p16" has 16 pins` and `pin "A1" has no pad in footprint "X"` (a
  designator match, so the pin↔pad link is by designator, never by array
  position). A footprint must be loaded before the part file that names it.
- Pads are emitted in designator order "1"…"N" (grid: row-major), then an
  unnumbered centre pad. Every pin gets a pad; the app skips pins whose role
  is `absent` (numbered pinouts only, plan-02-A §4.1).
- Implementation: a new `backend/src/library/footprints.c` (parser, checks,
  expansion) using the `KindContext` helpers (`kind_check_keys`,
  `kind_string`, `kind_uint`, and a new `kind_number` for doubles in
  `kind_util.c`/`kind.h`); the library keeps footprints in a `NameSet` plus an
  array for `_at`. Add the file to `backend/CMakeLists.txt` `add_library`.
  (The app compiles `backend/src/` through its synchronized folder, so Xcode
  picks it up with no project edit.)

### 4.4 Loader rules (each one a `library_check` case)

File: format must be `"schematical-footprints"`; version 1; only `format`,
`version`, `footprints`. Footprint: unknown key; missing required key; unknown
style; `pins` 0; non-positive size, pitch or span; drill ≥ a pad side; odd
`pins` on `dual` without `perSide`; `perSide·2 < pins` without slots;
`quad` slot total < pins without slots; `slots` wrong length, duplicate,
beyond the slot count, a `0` without `centre`, or two `0`s; `centre` on `grid`; `explicit`: a pin missing or repeated, a parametric key present; `grid` with `slots`; ball outside the grid, duplicate
ball, wrong `balls` length, a name that is not a ball name (plan-02-A's
`is_ball_name` rule); duplicate id. Part: unknown footprint; pad count
mismatch; a pin designator with no pad; `"footprint"` not a string.

### 4.5 LED footprints (fixed by this plan)

LEDs have no datasheet in the repo, so these dimensions are decided here.
Chip LED codes are inch codes: body = (first two digits × 0.254 mm) ×
(last two × 0.254 mm). Pads: width = 0.3·L + 0.25, height = W + 0.25, outer
pad edge 0.2 mm beyond the body end, all rounded to 0.05 mm (computed with
`python3 -I` on 2026-10-31). Pin 1 (K) is the left pad.

| id | body W × H | pad W × H | span | notes |
|---|---|---|---|---|
| `LED-0402` | 1.0 × 0.5 | 0.55 × 0.75 | 0.9 | |
| `LED-0603` | 1.5 × 0.75 | 0.7 × 1.0 | 1.2 | |
| `LED-0805` | 2.05 × 1.25 | 0.85 × 1.5 | 1.6 | |
| `LED-1204` | 3.05 × 1.0 | 1.15 × 1.25 | 2.3 | |
| `LED-1205` | 3.05 × 1.25 | 1.15 × 1.5 | 2.3 | |
| `LED-1206` | 3.05 × 1.5 | 1.15 × 1.75 | 2.3 | |
| `LED-1209` | 3.05 × 2.3 | 1.15 × 2.55 | 2.3 | |
| `LED-1411` | 3.55 × 2.8 | 1.3 × 3.05 | 2.7 | |
| `LED-1414` | 3.55 × 3.55 | 1.3 × 3.8 | 2.7 | |
| `LED-RADIAL-5MM` | 5.8 × 5.8 round | 1.8 × 1.8 round, drill 0.9, pin1Shape rect | 2.54 | T-1¾, one pad per side (`pitch` 2.54, `perSide` 1) |
| `LED-DIP4` | 7.62 × 7.62 | 1.6 × 1.6 round, drill 0.8, pin1Shape rect | 5.08 | 4 leads on a 5.08 mm square (`perSide` 2, `pitch` 5.08) |

All chip LEDs: `dual`, `pins` 2, `perSide` 1, `pitch` 1 (one pad per side,
pitch unused but required > 0), `pad.shape` rect.

### 4.6 Golden format

`footprints.txt`, one line per footprint in library order:

```
SOIC-14|pins=14|body=3.900x8.650|pads=1:-2.700,-3.810,1.550x0.600,rect;2:…
```

Numbers `%.3f`; round body adds ` round`; a pad with drill adds `,d0.900`;
pads print as `<designator>:` (`A1:` on a grid); an unnumbered centre pad is `-:`. `footprint_check --print` writes the current listing
(same convention as `listing_check`).

Listing lines (`listing.c`) append `|footprint=<id>` when `part->footprint`
is set.

## Compatibility / Format Impact

- Part files: one new optional part key `"footprint"`; `version` stays 1.
- New file `Parts/footprints.json`, format `"schematical-footprints"` v1.
- `parts.h`: new types, one new `Part` field (appended), four functions.
- Both loaders (`BackendCatalog.load`, `load_parts_dir`) load
  `footprints.json` first with `load_footprints` and skip it in the part loop;
  a missing `footprints.json` is an error.
- `.schematical` files: unchanged.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial with what remains; mark moot tasks
> `- [x] ~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands;
> add any task you discover. **An unticked box means NOT DONE.**

### Phase 1 — Footprint loader and API (no data yet)

Adds the format and the loader; no part file uses it, so nothing else changes.

- [ ] `backend/include/parts.h`: `PadShape`, `FootprintPad`, `PartFootprint`,
      `Part.footprint` (last field before `internal`), the four functions (§4.3).
- [ ] `backend/src/library/kind.h`, `kind_util.c`: `kind_number(ctx, json, key,
      double *out)` (rejects non-numbers; message names the key).
- [ ] Move plan-02-A's `static int is_ball_name(const char *)` from
      `library.c` to `kind_util.c` (declared in `kind.h`) so `footprints.c`
      checks grid ball names with the same rule.
- [ ] `backend/src/library/footprints.c`: parse, check (§4.4), expand (§4.2
      formulas) into the arena; register in the library's footprint set/array.
      Add to `backend/CMakeLists.txt`.
- [ ] `backend/src/library/library.c`: `"footprint"` in `PART_KEYS`; resolve in
      `load_part`; pin-count check; the library struct gains the footprint
      set/array, freed in `part_library_destroy`.
- [ ] `backend/tests/library_check.c`: one accepted footprint per style with its
      expanded pads asserted (a `dual` 14-pin, a `dual` with `slots` [1,2,3,4,6],
      a `quad` 20-pin with slots and an unnumbered `centre`, a `dual` 5-pin with slots [1,2,0,3,4] and a `centre`, an `explicit` 4-pin with mixed pad sizes, a `grid` 2×3), plus one case per
      rule in §4.4.

Acceptance: the new loader accepts all four styles with exact pad positions
and rejects every §4.4 case with a message naming the place; existing part
files load unchanged.
  Check: `cmake --build /tmp/logic-plan03 -j8 && ctest --test-dir /tmp/logic-plan03 -R library_check --output-on-failure` → passes (est. 1 min).
  Check: `ctest --test-dir /tmp/logic-plan03 -R 'listing_check|trace_check'` → both pass, `3817 parts compared` (est. 1 min).
Commit: —

### Phase 2 — `footprints.json`, LED footprints, loaders, goldens

The first real data: 11 LED footprints and the 55 LED parts.

- [ ] `schematical/schematical/Parts/footprints.json`: the 11 footprints of §4.5.
- [ ] `led.json`: add `"footprint":"<id>"` after `"pinout"` on all 55 parts
      (package `0402` → `LED-0402`, …, `DIP` → `LED-DIP4`, `Radial` →
      `LED-RADIAL-5MM`).
- [ ] `backend/tests/support/read_file.c:load_parts_dir`: load
      `footprints.json` with `load_footprints` first; skip it in the part loop;
      fail if it is missing.
- [ ] `backend/tests/support/listing.c`: append `|footprint=<id>`.
- [ ] `backend/tests/footprint_check.c` + `logic_test(footprint_check)` in
      `CMakeLists.txt`: prints/compares `footprints.txt` (`--print` regenerates);
      also fails if any loaded footprint is used by no part.
- [ ] Regenerate `backend/tests/golden/parts.txt` (`listing_check --print`) and
      write `footprints.txt` (`footprint_check --print`); hand-check the 11
      lines against §4.5 before committing.
- [ ] `schematical/schematical/Backend/PartCatalog.swift`: `BackendCatalog.load`
      loads `footprints.json` via `load_footprints` first and skips it in the
      loop; `CatalogPart` gains `footprint: Footprint?` (Swift copy of
      `PartFootprint`: id, description, body `CGSize`, `bodyRound`, pads with
      `designator: String?` (nil = unnumbered centre pad; the app pairs pads with `ChipPin.designator`), centre `CGPoint`, size `CGSize`, `drill`,
      `shape`), built once per footprint id during load and shared by the parts
      that use it.

Acceptance: LED parts carry their footprints in the C listing and in the app's
catalog; nothing else changed.
  Check: `ctest --test-dir /tmp/logic-plan03 --output-on-failure` → 8 tests pass (est. 1 min).
  Check: `git diff -U0 backend/tests/golden/parts.txt | grep -c '^+[^+]'` → 55, and every `+` line starts `LED-` (est. seconds).
  Check: `git diff --exit-code backend/tests/golden/traces.txt` → no diff (est. seconds).
  Check: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/schematical-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` (est. 3 min).
Commit: —

### Phase 3 — Docs

- [ ] `docs/parts-format.md`: a `## Footprints` section (§4.1–4.2, the styles
      with a small diagram of slot order, the `"footprint"` part key, the
      loading order) and `footprint` in the Parts key table.
- [ ] `docs/backend.md`: `load_footprints` in "Using it", `footprints.c` in
      Layout, `footprint_check` and `footprints.txt` in Tests.

Acceptance: the docs describe every key the loader accepts.
  Check: `grep -c 'perSide\|"slots"\|"balls"\|"centre"\|"explicit"\|pin1Shape\|"footprint"' docs/parts-format.md` → ≥ 7, and `grep -c 'load_footprints\|footprint_check' docs/backend.md` → ≥ 2 (est. seconds).
Commit: —

## Validation Plan

- Tests: `library_check` (every rule, every style's arithmetic),
  `footprint_check` (golden), `listing_check` (suffix), `trace_check`
  (unchanged).
- Coverage check: `footprint_check` fails on any unused footprint, and
  `listing_check` shows the suffix on all 55 LED lines, so the new key path is
  exercised by real data, not only by `library_check` fixtures.
- Runtime proof: in the Debug build, the drawer footer still reads
  "3,815 parts from backend" (launch with the `run` skill).
- Doc sync: `docs/parts-format.md`, `docs/backend.md`.
- Final gate (once, after Phase 3): `cmake -S backend -B /tmp/logic-final && cmake --build /tmp/logic-final -j8 && ctest --test-dir /tmp/logic-final` (est. 1 min) + the Phase 2 `xcodebuild` (est. 3 min) + the runtime proof (est. 5 min).

## Decisions

- Footprint geometry lives in one shared `Parts/footprints.json`; each part
  names its footprint with `"footprint"` — decided by the owner, 2026-10-31.
  Settles the data home (§4.2).
- Every part except VCC/GND gets a footprint in plan-03, from its datasheet's
  package drawing, and the key becomes required (plan-03-C) — owner,
  2026-10-31.
- Every schematic part is on the PCB automatically — owner, 2026-10-31
  (plan-03-D).
- PCB snap grid 0.635 mm (25 mil), move + rotate in 90° steps, top side only —
  owner, 2026-10-31 (plan-03-D).

## Corrections

## Summary

The risk is the pad arithmetic, pinned by a hand-checked golden and
per-style `library_check` cases. The simulator, the traces, the drawer and
the file format are untouched; only LED listing lines change.
