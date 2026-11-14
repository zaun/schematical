# plan-39-H: Active-low review — Converters, devices 1–379 (ADC0802 … ADS8548)

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-G

H reviews 429 pinouts of 379 devices (converter) against their datasheets and flags every active-low pin, recording each review in the ledger. Its outcome: `node scripts/pin-polarity.js check --complete` is clean for every pinout listed below, and the part files carry `"activeLow"` on exactly the pins the datasheets mark active low.

References:

- plan-39-D § 3 (sources, ledger, rules R1–R8) and § 4 (the review rules; apply them verbatim, there is nothing to decide beyond them).
- plan-39-A § 3 (the `activeLow` key), `docs/parts-format.md` § Pinouts and pins.
- `scripts/pin-polarity.js --help`; `planning/plan-39-data/ledger.tsv`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-G complete | `ls planning/completed/plan-39-G-*.md \| wc -l` → 1 | MET (2026-11-14: 1, at landing; the reviews ran alongside G, see Corrections 1) |
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
| H1 | converter | 1–133: ADC0802 … ADS1211 (133) | 143 | 1779 | 128 | 6 |
| H2 | converter | 134–260: ADS1213 … ADS7825 (127) | 143 | 2442 | 246 | 2 |
| H3 | converter | 261–379: ADS7826 … ADS8548 (119) | 143 | 2294 | 128 | 0 |
| **total** | | 379 devices | **429** | **6515** | **502** | **8** |

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

### Phase 1 — H1: converter 1–133 (143 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review converter devices 1–133 (ADC0802 … ADS1211): 143 pinouts, § 3 procedure; edits `schematical/schematical/Parts/converter.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind converter --devices 1-133 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 143 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: converter 1-133 `ok: 143 pinouts, 143 ledger rows checked`; golden-check (phase staged) `ok: 162 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: b6fe7dc1

### Phase 2 — H2: converter 134–260 (143 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review converter devices 134–260 (ADS1213 … ADS7825): 143 pinouts, § 3 procedure; edits `schematical/schematical/Parts/converter.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind converter --devices 134-260 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 143 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: converter 134-260 `ok: 143 pinouts, 143 ledger rows checked`; golden-check (phase staged) `ok: 256 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: 2037b123

### Phase 3 — H3: converter 261–379 (143 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review converter devices 261–379 (ADS7826 … ADS8548): 143 pinouts, § 3 procedure; edits `schematical/schematical/Parts/converter.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind converter --devices 261-379 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 143 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: converter 261-379 `ok: 143 pinouts, 143 ledger rows checked`; golden-check (phase staged) `ok: 298 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: 85cfe5cf

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

1. **H was reviewed alongside F and G and landed after G.** Its three
   phases share `converter.json`, so each commit stages the ledger rows and
   golden lines of its own device range (plan-39-E Corrections 2), and the
   part file goes in whole with H1. Every H flag lands in H1's commit; H2
   and H3 commit their ledger rows and golden lines. The Prerequisites row
   was measured at landing.
2. **Reviewers' calls:**
   - Programmable ALERT/ALERT-RDY pins (ADC081C021, ADC101/121C021,
     ADS1014/1015/1114/1115(L), ADS7128/7138) are unmarked in the pin
     table, so they are not flagged, though the register reset default is
     active low. Rule 5 says the pin-table marking decides. ADS8201
     BUSY/INT is flagged: its pin table states the active-low default.
   - Barred complementary analog inputs (IN̄ on ADS802–831, ADS2806/2807,
     ADS5231–5237) are negative differential inputs and are not flagged
     (rule 2). The catalog also names both halves alike.
   - ADS5423's pre-flagged `AIN#` and `CLK#` stay flagged (rule 11: only an
     active-high reading, rule 10, removes a seeded flag).
   - ADS5204 C̄OUT (pin 25) shares the catalog name `COUT` with pin 26, so
     it cannot be flagged without a rename (R5). Its note says so.
   - ADS5500–ADS5542 SEN is flagged from the serial-interface text
     ("enabled when SEN is low"): rule 1's "active when low".
   - ADS7882 PWD/RST is `true`: the pin text says "Active low input" for
     the whole pin.
   - ADS8508/8509 BUSY is barred in the drawing, and its text says it is
     low during conversion, so it is flagged.

## Summary

H's risk is reading: a missed overbar or a misread package. The tool's rules catch inconsistency, not a wrong reading; the page citations in the ledger let a later reader re-check any row. Nothing outside the part files, the ledger and the golden changes.
