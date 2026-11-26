# plan-81-E: OLED footprints and parts: `gen-oled.py`, `Parts/oled.json`, `charlcd.json`, datasheets, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-81-D

E draws every footprint `planning/plan-81-data/footprints-needed.txt` lists, generates the `oled`
devices and parts and the new `charlcd` devices from `specs.tsv` and `verdicts.tsv`, registers the
datasheets, and regenerates the goldens. After E the library holds one part per `add` row.

References:

- `docs/parts-format.md` § Footprints (`header`, `explicit`, `dual`; `"connectorFootprint"`,
  line 257), § Datasheets, § `oled` (B–D), § `charlcd`.
- The character-LCD precedent: footprint `LCM-NHD-0216BZ` (module outline, the header row and four
  mounting holes; its `note` names file, page and every number) and connector footprint
  `HDR-1X16-P2.54` (`explicit`, the header alone); `planning/completed/plan-11-*`.
- `planning/plan-74-D-ethernet-footprints-ic.md` (drawing method, scratch-tree `footprint_check`),
  `planning/plan-74-F-ethernet-parts.md` (generator, `--check`, goldens).
- `docs/backend.md` § tests (line 895: regenerate the goldens with `listing_check --print` and
  `trace_check --print`, review the diff).

## Prerequisites

plan-81-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-81-D complete | `ls planning/completed/plan-81-D-*.md` → exists | NOT MET (2026-11-25) |
| A's specs pass | `python3 -I planning/plan-81-data/check-specs.py --phase 4` → `ok` | NOT MET (A writes it) |
| Goldens at E's start | `wc -l < backend/tests/golden/parts.txt`; `… footprints.txt` | 40832 / 3993 at planning; E records **P2**, **F2** |
| Footprint ids are new | each id of `footprints-needed.txt`: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` → 0 | a rule per id (Phase 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/oled.json` has one part per `add` row with target `oled-sim` or `oled-facts`;
  `Parts/charlcd.json` has the baseline 72 devices plus one device per `charlcd` specs row and one
  part per `charlcd` `add` row.
- `listing_check` counts: `grep -c '|oled|' backend/tests/golden/parts.txt` = the number of `oled-*`
  `add` rows; `parts.txt` grows by exactly the `add` rows (P2 + adds); no existing line changes
  (`git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0).
- `footprints.txt` grows by exactly the ids of `footprints-needed.txt`.
- The Goal parts of A exist: `NHD-2.23-12832UCB3` (`oled`, `"model": "ssd1305"`), an SSD1322 module
  (`oled`, facts), `NHD-0420DZW-AY5` (`charlcd`, `oled-yellow`).

### Non-goals (explicit constraints)

- No backend or app code change.
- No footprint style or key added: `header`, `explicit` and `dual` cover headers, mounting holes
  and FPC tails.

## 2. Current State

`footprints.json` has F0 = 3993 entries, among them 38 `LCM-` module footprints and one header
connector footprint, `HDR-1X16-P2.54` (`python3 -I -c "import json;f=[x['id'] for x in
json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']];print(sum(i.startswith('LCM') for i in f), [i for i in f if i.startswith('HDR')])"`
→ `38 ['HDR-1X16-P2.54']`). No OLED footprint exists. `Parts/oled.json` exists with no devices (B).

## 3. Design

### 3.1 Footprints

By the module's connection, read from its drawing (`specs.tsv` `drawing_page`):

| Connection | `footprint` | `connectorFootprint` |
|---|---|---|
| pin header or header holes (most Newhaven, Display Visions, Orient `…-ANO` boards) | `OLED-<MAKER>-<SERIES>`: the module's header holes plus its mounting holes, `explicit`, body = the module outline (the `LCM-` precedent) | `HDR-<R>X<N>-P<pitch>` (the header alone; `explicit`); reused when the id exists (`HDR-1X16-P2.54`) |
| FPC / FFC tail meant to be soldered (bonded) | `OLED-FPC-<N>-P<pitch>-<MAKER>-<SERIES>`: the tail's solder pads as the drawing's recommended land pattern, `explicit`, body = the tail end | none |
| FPC tail meant for a ZIF connector the drawing names | the named connector's footprint, drawn from its own drawing (`FPC-<maker>-<series>-<N>`) | none |

Each entry's `note` names the PDF, page and every number used; a drawing that gives no pad size
uses the `LCM-` rule (pin pad d1.8, pin 1 square, on the drawing's hole; mounting-hole pad
d5.0 on its hole). The footprints are written to `planning/plan-81-data/footprints-oled.json` and
merged into `footprints.json` with the parts that use them (`footprint_check` fails on a footprint
no part names: `backend/tests/footprint_check.c`, `unused footprint`).

### 3.2 Devices (`planning/plan-81-data/gen-oled.py`)

- `oled`: one device per (maker, datasheet, pinout) specs row with an `oled-*` target; id
  `oled-<maker>-<series>`; `function` the format (`128x32`, `256x64`, `20x4`); `description`
  "<size>" <colour> OLED module, <controller>"; drawer `["Displays & Indicators", "OLED",
  "Graphic" | "Character"]`; `note` the facts' sources. Simulated rows get `model` (C § 3.1 by
  controller), `bus`, `glass`, and `timing` when the bus is parallel-capable (D § 3.2); facts rows
  `"simulated": false`. Parts: one per `add` row, `manufacturer` "Newhaven Display" (the existing
  `charlcd` spelling), "Display Visions", "Orient Display", "DLC Display"; `package` "Module";
  `panel` on the part when one datasheet covers several colours.
- `charlcd`: one device per `charlcd` specs row, the existing keys (`columns`, `rows`,
  `controller` `"WS0010-compatible"` or the named one, `rom` `"A00"`, `panel` `oled-<colour>`, `bus`,
  `timing`, `exec`, `vcc`; no `contrast`, no `backlight`), drawer `["Displays & Indicators", "OLED",
  "Character"]`.
- `gen-oled.py --check` re-generates in memory and exits 0 only if the files match.

### 3.3 Datasheets

`Parts/datasheets.json` gains one entry per distinct module PDF of an `add` row (maker URL from
`datasheets.tsv`; the file under `datasheets/display-oled/<maker>/`). Controller PDFs are listed
too, named from each simulated device's `note`.

## 4. Phases

### Phase 1 — footprints

- [ ] Draw every id of `footprints-needed.txt` into `planning/plan-81-data/footprints-oled.json` (§ 3.1).
- [ ] Load-check on a scratch tree: copy the tree to `/tmp/p81-fp`, append the entries to its
      `footprints.json`, build, run `footprint_check`.

**Acceptance:** in `/tmp/p81-fp`, `footprint_check` prints only `unused footprint` lines, one per new
id (`… | grep -c 'unused footprint'` = `wc -l < planning/plan-81-data/footprints-needed.txt`), no load
error, no pad overlap. (≈ 3 min: a scratch build.)

Commit:

### Phase 2 — devices, parts, datasheets, goldens

- [ ] `planning/plan-81-data/gen-oled.py` (§ 3.2); write `Parts/oled.json` and append the `charlcd.json` devices.
- [ ] Merge `footprints-oled.json` into `Parts/footprints.json`.
- [ ] `Parts/datasheets.json` entries (§ 3.3).
- [ ] Regenerate the goldens (`docs/backend.md` § tests, lines 901–932): `listing_check --print >
      backend/tests/golden/parts.txt`, `trace_check --print > …/traces.txt`, `erc_pins_check
      --print > …/erc_pins.txt`, `footprint_check --print > …/footprints.txt` (from
      `/tmp/logic-p81`); review the diff.

**Acceptance:** `cmake --build /tmp/logic-p81 -j8 && ctest --test-dir /tmp/logic-p81 -j8` → `100%
tests passed`; `grep -c '|oled|' backend/tests/golden/parts.txt` = `awk -F'\t' '$5=="add" &&
$6 ~ /^oled-/' planning/plan-81-data/verdicts.tsv | wc -l`; `git diff backend/tests/golden | grep -c
'^-[^-]'` → 0; `python3 -I planning/plan-81-data/gen-oled.py --check` → exit 0. (≈ 6 min: a build and
the suite.)

Commit:

### Phase 3 — the Goal's example in the backend

- [ ] `backend/tests/oled_test.c`: a case that loads the shipped `Parts/` tree, places
      `NHD-2.23-12832UCB3` and an ATmega328P (the plan-62 AVR rig, `backend/tests/support/avr_rig.h`)
      running new firmware `backend/tests/avr/oled_i2c.hex` (source
      `backend/tests/avr/src/oled_i2c.asm`, built by `backend/tests/avr/src/build.sh`, the
      `blink` precedent; CLKPR = 0 first, as the plan-62 tests do) that bit-bangs the I2C init and a
      checkerboard on PC4/PC5; the case compares `circuit_oled_state` with the checkerboard.

**Acceptance:** `ctest --test-dir /tmp/logic-p81 -R oled_test --output-on-failure` → passed. (≈ 1 min.)

Commit:

## 5. Validation Plan

End of E: `ctest --test-dir /tmp/logic-p81 -j8` → `100% tests passed` out of T1 + 1;
`python3 -I planning/plan-81-data/gen-oled.py --check` → exit 0; `git diff --stat
backend/tests/golden` shows only additions.
