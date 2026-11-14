# plan-39-G: Active-low review — SRAM, EEPROM and serial controllers

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-F

G reviews 197 pinouts of 123 devices (eeprom, sram, serial) against their datasheets and flags every active-low pin, recording each review in the ledger. Its outcome: `node scripts/pin-polarity.js check --complete` is clean for every pinout listed below, and the part files carry `"activeLow"` on exactly the pins the datasheets mark active low.

References:

- plan-39-D § 3 (sources, ledger, rules R1–R8) and § 4 (the review rules; apply them verbatim, there is nothing to decide beyond them).
- plan-39-A § 3 (the `activeLow` key), `docs/parts-format.md` § Pinouts and pins.
- `scripts/pin-polarity.js --help`; `planning/plan-39-data/ledger.tsv`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-F complete | `ls planning/completed/plan-39-F-*.md \| wc -l` → 1 | MET (2026-11-14: 1, at landing; the reviews ran alongside F, see Corrections 1) |
| The tool's tests pass | `node --test scripts/pin-polarity.test.js` → all pass | MET (2026-11-14: pass 13, fail 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue. If you stop, report all prerequisites.

## 1. Goal

- Every pinout in the table of § 2 has a ledger row and its pins are flagged per plan-39-D § 4; `node scripts/pin-polarity.js check --complete --kind K --devices A-B` exits 0 for each row of the table.
- `backend/tests/golden/parts.txt` differs from HEAD only by `:low` / `:low=…` on the pins flagged here (`node scripts/pin-polarity.js golden-check` → exit 0).

### Non-goals (explicit constraints)

- No pin is renamed and no role changes, except a `#` pin the datasheet shows active high (plan-39-D § 4 rule 10), which is renamed in its own commit and logged under Corrections.
- No other column of any part changes; no pinout is added or removed.
- Flags follow the datasheet's marking, not the name's look (plan-39-D § 4 rule 2).

## 2. Current State

The slice, measured with `python3 -I planning/plan-39-data/slices.py` (sorted device ids, 1-based indices as `--devices` takes them; `signal` = input/output/inout pins; `multiplexed` = signal pins whose name has a `/`; `seeded` = pins whose name ends in `#`, already flagged by plan-39-D):

| Phase | Kind | Devices (index: first … last) | Pinouts | Signal pins | Multiplexed | Seeded |
|---|---|---|---|---|---|---|
| G1 | eeprom | 1–12: AT28BV256 … AT28LV010 (12) | 27 | 704 | 216 | 81 |
| G1 | sram | 1–21: AS6C1008 … AS7C4098A (21) | 37 | 1274 | 112 | 147 |
| G2 | serial | 1–49: 82510 … TL16C550D (49) | 67 | 2072 | 115 | 0 |
| G3 | serial | 50–90: TL16C552 … XR21V1414 (41) | 66 | 2764 | 172 | 0 |
| **total** | | 123 devices | **197** | **6814** | **615** | **228** |

Kind notes (what to expect; the review rules decide, not these):

- **eeprom** — parallel EEPROMs: `CE#`, `OE#`, `WE#` are seeded; rule R6 requires every pin named by `chipEnable`, `outputEnable` or `writeEnable` to be flagged.
- **sram** — SRAMs: `CE#`, `OE#`, `WE#`, `UB#`, `LB#` are seeded; rule R6 requires every pin named by `chipEnable`, `outputEnable`, `writeEnable` or `byteEnable` to be flagged; `CE2`-style active-high enables stay unflagged.
- **serial** — UARTs and serial controllers: chip selects, read/write strobes, reset, and the modem lines (`CTS`, `DSR`, `RTS`, `DTR`, `RI`, `DCD` are active low on most parts); names join up to 3 alternatives.

The 2,129 in-scope pinouts of the whole plan are 312 (E) + 341 (F) + 197 (G) + 429 (H) + 429 (I) + 211 (J) + 210 (K); `python3 -I planning/plan-39-data/census.py` → `IN-SCOPE TOTAL … pinouts 2129`.

## 3. Procedure (every phase)

For each pinout the phase's `list` prints (`node scripts/pin-polarity.js list --kind K --devices A-B --todo`):

1. `sources <device> <pinout>` → the datasheet; `fetch <id>` when it is a URL.
2. Read the pin table and pin descriptions for that package; render pages where overbars matter (plan-39-D § 4 rule 9).
3. `pins <device> <pinout>`, then `set <device> <pinout> <name> true|ALT[,ALT]|off` for every pin the datasheet marks active low (and `off` for a seeded `#` pin it does not — a `#` pin the datasheet shows active high is rule 10).
4. `record <device> <pinout> --source <source> --pages <pages>` (add `--note` for a rule-7 or rule-10 finding).
The reviews of different devices are independent; an implementer may fan devices out to subagents, each given plan-39-D § 4 verbatim and a slice of the `list` output, with the main thread running `check` and committing. A subagent edits only the part file and ledger rows of its own devices.

## 4. Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit as the work; `- [~]` partial; moot tasks stay with a reason; fill `Commit:` when a phase lands; an unticked box means NOT DONE.

### Phase 1 — G1: eeprom 1–12 + sram 1–21 (64 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review eeprom devices 1–12 (AT28BV256 … AT28LV010): 27 pinouts, § 3 procedure; edits `schematical/schematical/Parts/eeprom.json`. — done: rows recorded, check ok (Observed)
- [x] Review sram devices 1–21 (AS6C1008 … AS7C4098A): 37 pinouts, § 3 procedure; edits `schematical/schematical/Parts/sram.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind eeprom --devices 1-12 --complete && node scripts/pin-polarity.js check --kind sram --devices 1-21 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 64 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: eeprom 1-12 `ok: 27 pinouts, 27 ledger rows checked`; sram 1-21 `ok: 37 pinouts, 37 ledger rows checked`; golden-check (phase staged) `ok: 0 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: —

### Phase 2 — G2: serial 1–49 (67 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [ ] Review serial devices 1–49 (82510 … TL16C550D): 67 pinouts, § 3 procedure; edits `schematical/schematical/Parts/serial.json`.
- [ ] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`.

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind serial --devices 1-49 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 67 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
Commit: —

### Phase 3 — G3: serial 50–90 (66 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [ ] Review serial devices 50–90 (TL16C552 … XR21V1414): 66 pinouts, § 3 procedure; edits `schematical/schematical/Parts/serial.json`.
- [ ] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`.

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind serial --devices 50-90 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 66 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
Commit: —

## Validation Plan

- Tests: `check` (R1–R8) and `golden-check` per phase; the backend loader through `library_check`/`listing_check`.
- Coverage check: each phase's `check --complete` covers its whole slice (`list … --todo | wc -l` → 0 after the phase).
- Runtime proof: plan-39-L opens flagged parts in the app.
- Doc sync: none in this letter.
- Final gate: plan-39-L, once.

## Decisions

- One datasheet per pinout — owner, 2026-11-13.
- Per-alternative flags; names kept — owner, 2026-11-13.
- Author's calls: phases are about 100 pinouts each, split on device boundaries in sorted-id order; one commit per phase.

## Corrections

1. **G was reviewed alongside F and H and landed after F**, staged phase by
   phase as in plan-39-E Corrections 2. The Prerequisites row was measured
   at landing.
2. **Memory:** every pre-flagged `#` pin (CE#, OE#, WE#, LB#, UB#, B0#–B3#,
   CE1#) was confirmed by an overbar or a `#` in its sheet. There were no
   rule-10 findings and no new flags. CE2 is active high on every Alliance
   sheet.
3. **Serial, reviewers' calls:**
   - The Exar/TI Motorola-mode alternatives (IRQ#, RESET#) are not in the
     catalog names, so they are not flagged (rule 12).
   - `16/68` is flagged `["68"]`.
   - Combined names whose catalog form has no separate alternative for the
     barred half (8251A `CD` for C/D̄, CDP65C51 `RW`, CDP1854A `RD_WR`) are
     not flagged.
   - CDP1854A DA/THRE are barred in mode 1 only, so they are not flagged.
   - W65C51N RTSB (no low-active text) and RWB are not flagged, as in
     plan-39-E Corrections 5.
   - XR16C864 DACKA–D are flagged: the description says the DMA cycle
     starts when the input goes low.
   - Programmable pins with a stated active-low default (USB_STAT2,
     LOWPOWER on XR21B14xx) are flagged (rule 5). LOWPOWER on XR21V141x is
     strapped with no default, so it is not flagged.
   - TL16C2550 qfn32: the package drawing's bars on XTAL2/GND are a
     drawing error (its pin table has none), so they are not flagged.

## Summary

G's risk is reading: a missed overbar or a misread package. The tool's rules catch inconsistency, not a wrong reading; the page citations in the ledger let a later reader re-check any row. Nothing outside the part files, the ledger and the golden changes.
