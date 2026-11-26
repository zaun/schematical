# plan-76-M: Sensor parts: `gen-sensor.py`, `Parts/sensor.json`, datasheets, goldens

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-76-L

M turns the seven `specs-<family>.tsv` files and their pinout files into
`schematical/schematical/Parts/sensor.json`, adds the datasheet entries, and regenerates the
goldens. After M the library lists every `add` row of `candidates.tsv`.

References:

- plan-76-C § 3 (the specs columns), plan-76-F § 3.2 (per-part ranges), plan-76-L § 3.1–3.3 (keys,
  units, listing line), plan-76-B § 3.1 (PDF names).
- `docs/parts-format.md` § Devices, § Parts, § Datasheets, § The drawer's categories;
  `docs/backend.md` § goldens (line 895).
- Precedents: `planning/plan-74-F-ethernet-parts.md`; `planning/completed/plan-48-G-timer-opamp-parts.md`
  (one device per (datasheet, package, pinout), idempotent generator, `--check`).

## Prerequisites

plan-76-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-76-L complete | `ls planning/completed/plan-76-L-*.md` → exists, or every L box ticked | NOT MET (2026-11-25: L not started) |
| The part file has no devices | `python3 -I -c "import json;print(len(json.load(open('schematical/schematical/Parts/sensor.json'))['devices']))"` → 0 | NOT MET (L writes the empty file) |
| Every family's specs and footprints pass | `for f in temperature humidity light pressure motion magnetic current; do python3 -I planning/plan-76-data/census/specs-check76.py --family $f && python3 -I planning/plan-76-data/census/fp-merge76.py --check $f; done` → `0 errors` and `0 missing` for each | NOT MET (C–K) |
| Goldens at M's start | `wc -l < backend/tests/golden/parts.txt` / `traces.txt` / `erc_pins.txt` | M records P1, R1, E1 |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `Parts/sensor.json` has one part per `add` row of `candidates.tsv` (B's verdicts, H's skips
  applied), on devices built by § 3.1, each with a footprint from `footprints.json`.
- `datasheets.json` has an entry for each PDF a part cites.
- `parts.txt`, `traces.txt` and `erc_pins.txt` grow by exactly the new parts (no existing line
  changes); `ctest` passes.

### Non-goals (explicit constraints)

- No change to `sensor.c`, any other kind, or the app.
- No hand-edited device: `sensor.json` is the generator's output, and `gen-sensor.py --check`
  proves it.

## 2. Current State

`sensor.json` exists with `"devices": []` (L). Manufacturer names already used in the library
(`cat schematical/schematical/Parts/*.json | grep -o '"manufacturer": *"[^"]*"' | sort | uniq -c`,
2026-11-25): "Texas Instruments", "Analog Devices", "Microchip Technology", "STMicroelectronics",
"Infineon Technologies", "Diodes Incorporated", "TE Connectivity", "Vishay", "NXP Semiconductors".

## 3. Design

### 3.1 `census/gen-sensor.py`

- One device per (maker, datasheet, pinout file): id `sensor-<maker>-<datasheet>-<package>`
  (lower case, the datasheet's file stem); `function` = the series name the datasheet's title gives
  (`TMP117`, `SHT4x`, `ACS37002`); `description` = "<Family> sensor, <output names>" plus the
  datasheet title's product phrase; `drawer` = `["Sensors", "<Family>"]` with Family one of
  Temperature, Humidity, Pressure, Motion, Magnetic, Light, Current; `"simulated": false`.
- Device keys from the specs row: `family`, `measures`, `output`, `ranges`, `accuracy`, `address`,
  `vcc`, and `extra`'s `axes` / `pressureType` / `switchType` / `isolated`.
- One part per part number of the row: `name`, `manufacturer` (the names of § 2; new makers by
  their own spelling: "Sensirion", "Amphenol Telaire", "Amphenol All Sensors", "Amphenol
  NovaSensor", "Innovative Sensor Technology", "Honeywell", "Memsic", "Allegro MicroSystems",
  "ams OSRAM", "Broadcom", "LEM", "Melexis"), `datasheet` (the `datasheets.json` id), `package`,
  `pinout`, `footprint`; per-part `ranges` / `accuracy` / `address` where the row maps them
  (plan-76-F § 3.2); `"flags": ["obsolete"]` where `verdicts.tsv` says `obsolete=1`.
- Output sorted by family, maker, device id, part name; `--check` regenerates in memory and
  compares with the file.

### 3.2 Datasheets

Each cited PDF gets `{"id": <stem>, "url": <the URL B downloaded from>}` in `datasheets.json`,
added in id order; an id already present is reused (its URL must match, else the new one is
`<stem>-<maker>`).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — The generator, on temperature

- [ ] `planning/plan-76-data/census/gen-sensor.py` per § 3.1–3.2, with `--family` and `--check`.
- [ ] Generate the temperature family; regenerate `parts.txt`, `traces.txt`, `erc_pins.txt`
      (`docs/backend.md` § goldens); review the diff.

Acceptance: the temperature parts load, and the goldens only grew by them.
  Check: `python3 -I planning/plan-76-data/census/gen-sensor.py --check` → `ok`; `ctest --test-dir /tmp/logic-p76 -j8` → `100% tests passed`; `grep -c '|sensor|' backend/tests/golden/parts.txt` equals `awk -F'\t' 'NR>1 && $1=="temperature" && $16=="add"' planning/plan-76-data/candidates.tsv | wc -l`; `git diff backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 6 min).
  Result:
Commit:

### Phase 2 — The other six families

- [ ] Generate humidity, light, pressure, motion, magnetic, current; regenerate the goldens;
      review the diff family by family.

Acceptance: every `add` row is a part, and no existing golden line changed.
  Check: `grep -c '|sensor|' backend/tests/golden/parts.txt` equals `awk -F'\t' 'NR>1 && $16=="add"' planning/plan-76-data/candidates.tsv | wc -l` (at most 3,380); `ctest --test-dir /tmp/logic-p76 -j8` → `100% tests passed`; `git diff <Phase 1 commit> -- backend/tests/golden | grep -c '^-[^-]'` → 0 (est. 8 min).
  Result:
Commit:

## Validation Plan

- M's checks are its two acceptance commands.
- Final gate: plan-76-O.

## Decisions

- Author: **one device per (maker, datasheet, pinout)**, the plan-48 and plan-74 rule, so a
  datasheet's packages are separate devices and its part numbers share one.
- Author: **temperature first, alone**: the generator is proven on one family and its golden diff
  is read before the other six reuse it.
- Author: **new makers keep their own short names**, matching how the library writes the existing
  ones (no "Inc.", "Corporation" or DigiKey division suffixes).

## Corrections

## Summary

M generates every sensor part from the specs files, adds their datasheets, and grows the goldens
by exactly those parts.
