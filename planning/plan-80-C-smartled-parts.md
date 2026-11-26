# plan-80-C: Addressable LED footprints and parts, the AVR chain test, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-80-B

C draws a footprint for every package of `planning/plan-80-data/footprints-needed.txt`, generates the
devices and parts of every `add` row into `Parts/smartled.json`, adds the datasheets to
`datasheets.json`, regenerates the goldens, and adds the test that proves a real chain works: an
ATmega328P at 16 MHz running `ws2812.hex` lights three shipped `IN-PI55TATPRPGPB` packages red, green
and blue.

References:

- `docs/parts-format.md` § Footprints (`dual`, `quad`, `explicit`, § Custom footprints), § Parts,
  § Datasheets, § `smartled` (B).
- `docs/backend.md` § tests (goldens, lines 896–935: `listing_check --print`, `trace_check --print`,
  `footprint_check --print`, `erc_pins_check --print`).
- `planning/plan-80-data/specs.tsv`, `verdicts.tsv`, `footprints-needed.txt` (A).
- `planning/plan-58-data/census/fp-merge.py` (the merge helper C copies to
  `planning/plan-80-data/fp-merge.py` and uses as is).
- `backend/tests/support/avr_rig.h` (`rig_new`, `rig_two_pin`, `rig_net`, `rig_start`, `rig_to`),
  `backend/tests/avr_board_test.c` `uno_case` (an ATmega328P with `GENERIC-CRYSTAL-16M` on PB6/PB7),
  `backend/tests/avr/src/build.sh` (firmware from `.asm` with the avr8js assembler, no `meta.json` →
  `backend/tests/avr/<name>.hex`), `backend/CMakeLists.txt:257` (the `foreach(AVR_TEST …)` that
  passes `AVR_DIR`).

## Prerequisites

plan-80-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-80-B complete | `ls planning/completed/plan-80-B-*.md` → exists, or every B box ticked | NOT MET (2026-11-25: B not started) |
| The kind loads an empty file | `grep -c '"kind": *"smartled"' schematical/schematical/Parts/smartled.json` → 1 and `ctest -R smartled_test` passes | NOT MET (2026-11-25: B adds them) |
| No `LED-ADDR-` footprint yet | `python3 -I -c "import json;print(sum(f['id'].startswith('LED-ADDR-') for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` → 0 | MET (2026-11-25: 0) |
| Goldens at C's start | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `footprints.txt` | 40832 / 40832 / 3993 at planning. C records its own P2, R2, F2 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `footprints.json` gains one `LED-ADDR-` footprint per line of `footprints-needed.txt`, each from its
  drawing's recommended land pattern; `footprints.txt` grows by exactly that count (N_fp).
- `smartled.json` holds one device per (maker, datasheet, package) row of `specs.tsv` and one part per
  `add` row; `parts.txt` and `traces.txt` grow by exactly the `add` count (N_add).
- `smartled_test` gains `avr_chain_case`, which passes.

### Non-goals (explicit constraints)

- No backend or app code change beyond `smartled_test.c`, the new firmware, and `CMakeLists.txt`'s
  `AVR_TEST` list.
- No change to an existing footprint or part.

## 2. Current State

`footprints.json` has 3993 footprints, 11 of them `LED-` and none for an addressable package
(plan-80-A § 2). `smartled.json` (B) has `"devices": []`. The AVR test programs get `AVR_DIR`
through `backend/CMakeLists.txt:257`; `smartled_test` is not in that list.

## 3. Design

### 3.1 Footprints

One footprint per line of `footprints-needed.txt`, written in `planning/plan-80-data/footprints-smartled.json`
(one object per line), merged with `fp-merge.py`. Four-pad PLCC-style packages (5050, 3535, 2020,
1515) are `explicit` footprints (pads at the land pattern's coordinates, the datasheet's pin 1
first); six-pad 5050 packages likewise; side-view and reverse-mount packages are `explicit` with the
drawing's slot or opening as a `npth` hole when the drawing shows one. Body = the outline's L × W.
Pad numbers follow the datasheet's pin table, so `LED-ADDR-5050-4-INOLUX` pad 3 is DIN.

### 3.2 Devices and parts (`planning/plan-80-data/gen-smartled.py`)

One device per `specs.tsv` row: id `<maker>-<series>` (e.g. `inolux-in-pi55`, `dialight-587-2056`),
function = the series, description `Addressable <RGB|RGBW|single-colour> LED, <body>, <n> pins`,
drawer `["Displays & Indicators", "Addressable LED"]`, pinout `p<pads>` from the `pins` column,
`din`/`dout` the pins whose roles are `input`/`output`, `order`, `leds` (omitted when 1), `timing`
from the ns columns, `vcc`. One part per `add` row of that (maker, datasheet, package): name,
manufacturer (`Inolux`, `Dialight`, `Würth Elektronik`, `Lite-On`), `package` (the body, e.g.
`5050`), `pinout`, `footprint`, `datasheet` (a new `datasheets.json` id per PDF:
`<maker>-<pdf stem lower-case>`). The generator replaces `"devices"` wholesale; the file is the
generator's output.

### 3.3 The AVR chain test

- `backend/tests/avr/src/ws2812.asm`: ATmega328P at 16 MHz (62.5 ns per cycle), PB0 output, sends
  three GRB frames — `00 FF 00` (red), `FF 00 00` (green), `00 00 FF` (blue) — then holds PB0 low
  250 µs, and repeats. Bits are cycle-counted: a 0 is 5 cycles high (312.5 ns) and 15 low
  (937.5 ns), a 1 is 10 high (625 ns) and 10 low (625 ns); the code between bytes keeps the low
  under `max(t0l, t1l) + tolerance` = 1050 ns of the Inolux IN-PI55 device (§ 3.2 of B). Built by
  `sh backend/tests/avr/src/build.sh` into `backend/tests/avr/ws2812.hex`.
- `smartled_test.c` `avr_chain_case`: `rig_new(&r, "ATMEGA328P-PU", "ws2812")`, the 16 MHz crystal on
  PB6/PB7 as `uno_case`, three `IN-PI55TATPRPGPB` with VDD (pad 4) on `r.n_vcc`, GND (pad 1) on `r.n_gnd`, PB0 → DIN of LED 1,
  DOUT 1 → DIN 2, DOUT 2 → DIN 3; run 2 ms; `circuit_part_colors` → LED 1 R 255 G 0 B 0, LED 2 G 255,
  LED 3 B 255, every channel known, `circuit_error` NULL.
- `backend/CMakeLists.txt`: `smartled_test` added to the `foreach(AVR_TEST …)` list (grep first) so it
  gets `AVR_DIR`; `#include "avr_rig.h"` in `smartled_test.c`.

The rig adds parts with `rig_two_pin`; a smartled has four pins, so the case builds its packages with
the circuit API directly (`circuit_add_part`, `circuit_add_net`, `circuit_connect(c, part, "<pad>",
net)` by pad number), as `avr_rig.h`'s `rig_two_pin` does for two.

### 3.4 Rejected alternatives

- **One device per part**: parts of one datasheet and package differ only in name (bin codes, reel
  sizes); one device per (datasheet, package) is the library's rule (`docs/parts-format.md` § Devices).
- **Reusing `LED-0603` etc. for the single-colour 0603 smart LEDs**: those are two-pad; the Inolux
  IN-PIS63 is four pads.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> fill each `Commit:` line with the hash when the phase lands.

### Phase 1: footprints

- [ ] Copy `planning/plan-58-data/census/fp-merge.py` to `planning/plan-80-data/fp-merge.py`.
- [ ] Write `planning/plan-80-data/footprints-smartled.json`: one footprint per line of
      `footprints-needed.txt` (§ 3.1), each id checked absent first (plan-80-A § Prerequisites).
- [ ] Merge into `footprints.json`; regenerate `backend/tests/golden/footprints.txt`; hand-check each
      new line's pads against its drawing (pad count, pitch, pin 1).

Acceptance: `python3 -I planning/plan-80-data/fp-merge.py --expect <footprint_check output>
<footprint_check --print output> backend/tests/golden/footprints.txt
planning/plan-80-data/footprints-smartled.json` → exit 0 before the regeneration; after it,
`wc -l < backend/tests/golden/footprints.txt` → F2 + N_fp. (≈ 3 min.)

Commit:

### Phase 2: devices, parts, datasheets

- [ ] Write `planning/plan-80-data/gen-smartled.py` (§ 3.2); run it into `Parts/smartled.json`.
- [ ] Add one `datasheets.json` entry per PDF of the `add` rows.
- [ ] Regenerate `parts.txt`, `traces.txt`, `erc_pins.txt` (`docs/backend.md` lines 904–919); review
      the diff: only `smartled` lines added.

Acceptance: `ctest --test-dir /tmp/logic-p80 -j8` → `100% tests passed`; `git diff --numstat
backend/tests/golden/parts.txt` → N_add added, 0 deleted; the same for `traces.txt`;
`grep -c '|smartled|' backend/tests/golden/parts.txt` → N_add. (≈ 4 min.)

Commit:

### Phase 3: the AVR chain test

- [ ] `backend/tests/avr/src/ws2812.asm`; `sh backend/tests/avr/src/build.sh` → `ws2812.hex`.
- [ ] `avr_chain_case` in `smartled_test.c`; `smartled_test` in the `AVR_TEST` list (§ 3.3).

Acceptance: `ctest --test-dir /tmp/logic-p80 -R smartled_test --output-on-failure` → passes, with
the line `ok ATMEGA328P-PU at 16 MHz lights three IN-PI55TATPRPGPB red, green, blue`. (≈ 3 min.)

Commit:

## Validation Plan

- `node scripts/source-size.js --check` and `node scripts/coverage.js --check --backend` → exit 0.
- Phase 2's acceptance re-run on the final tree. (≈ 6 min together.)

## Corrections
