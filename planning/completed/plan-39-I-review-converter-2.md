# plan-39-I: Active-low review — Converters, devices 380–764 (ADS8555 … TLV571)

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-H

I reviews 429 pinouts of 385 devices (converter) against their datasheets and flags every active-low pin, recording each review in the ledger. Its outcome: `node scripts/pin-polarity.js check --complete` is clean for every pinout listed below, and the part files carry `"activeLow"` on exactly the pins the datasheets mark active low.

References:

- plan-39-D § 3 (sources, ledger, rules R1–R8) and § 4 (the review rules; apply them verbatim, there is nothing to decide beyond them).
- plan-39-A § 3 (the `activeLow` key), `docs/parts-format.md` § Pinouts and pins.
- `scripts/pin-polarity.js --help`; `planning/plan-39-data/ledger.tsv`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-H complete | `ls planning/completed/plan-39-H-*.md \| wc -l` → 1 | MET (2026-11-14: 1, at landing; the reviews ran alongside H, see Corrections 1) |
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
| I1 | converter | 380–505: ADS8555 … DAC60508 (126) | 143 | 2407 | 270 | 0 |
| I2 | converter | 506–627: DAC60508C … DAC8728 (122) | 143 | 2061 | 78 | 0 |
| I3 | converter | 628–764: DAC8734 … TLV571 (137) | 143 | 1940 | 60 | 0 |
| **total** | | 385 devices | **429** | **6408** | **408** | **0** |

Kind notes (what to expect; the review rules decide, not these):

- **converter** — ADCs, DACs and related: chip select, convert-start, read/write, reset and busy/data-ready pins; `AINxN`-style names are negative inputs, not active low (rule 2).

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

### Phase 1 — I1: converter 380–505 (143 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review converter devices 380–505 (ADS8555 … DAC60508): 143 pinouts, § 3 procedure; edits `schematical/schematical/Parts/converter.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind converter --devices 380-505 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 143 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: converter 380-505 `ok: 143 pinouts, 143 ledger rows checked`; golden-check (phase staged) `ok: 158 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: 3d728cb0

### Phase 2 — I2: converter 506–627 (143 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review converter devices 506–627 (DAC60508C … DAC8728): 143 pinouts, § 3 procedure; edits `schematical/schematical/Parts/converter.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind converter --devices 506-627 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 143 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: converter 506-627 `ok: 143 pinouts, 143 ledger rows checked`; golden-check (phase staged) `ok: 295 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: c9888f45

### Phase 3 — I3: converter 628–764 (143 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review converter devices 628–764 (DAC8734 … TLV571): 143 pinouts, § 3 procedure; edits `schematical/schematical/Parts/converter.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind converter --devices 628-764 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 143 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: converter 628-764 `ok: 143 pinouts, 143 ledger rows checked`; golden-check (phase staged) `ok: 453 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: 7bac7938

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

1. **I was reviewed after H's reviews and landed after H**, staged phase by
   phase as in plan-39-H Corrections 1. `converter.json` goes in whole with
   I1, so every I flag lands there. The Prerequisites row was measured at
   landing. Three reviewers stopped early. They were resumed with the
   pinouts their ranges still lacked, and `check … --complete` was run on
   each range.
2. **Catalog names that cannot carry a per-alternative flag.** Some
   catalog names join two signals without a `/`, so the tool sees one
   alternative:
   - THS12082 `CONV_CLK (CONVST)`: unflagged, since only CONVST is barred.
   - TLC2574/2578/3544/3548/3574/3578 `EOC(INT)`: unflagged, since only INT
     is barred.
   - AFE7070/7071 `ALARM_SDO`: `true`, since the pin's default function
     ALARM is active low (rule 5).
   - THS1207/THS12082 `WR (R/W)`: `true`, since both halves split at `/` are
     barred.

   Each row's note says which. A rename to a `/`-separated name would let
   the first two carry a list. That is out of plan-39's scope (no pin is
   renamed), so it is left for a later catalog fix.
3. **Reviewers' calls:** rising-edge LDAC/RST pins drawn without a bar are
   not flagged (rule 3). Complementary analog outputs drawn with a bar
   (DAC900–908, DAC2900–2932, DAC0800/0802 IOUT̄) are not flagged (rule 2).
   Complementary digital signals drawn with a bar (DDC114/118 DOUT̄, DIN̄,
   DCLK̄) are flagged, as inverted outputs. WR on ADS8555–8558 is flagged
   from "enabled when CS and WR are low" (rule 1).

## Summary

I's risk is reading: a missed overbar or a misread package. The tool's rules catch inconsistency, not a wrong reading; the page citations in the ledger let a later reader re-check any row. Nothing outside the part files, the ledger and the golden changes.
