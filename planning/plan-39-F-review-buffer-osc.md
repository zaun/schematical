# plan-39-F: Active-low review — Buffers and oscillators

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-E

F reviews 341 pinouts of 291 devices (buffer, oscillator) against their datasheets and flags every active-low pin, recording each review in the ledger. Its outcome: `node scripts/pin-polarity.js check --complete` is clean for every pinout listed below, and the part files carry `"activeLow"` on exactly the pins the datasheets mark active low.

References:

- plan-39-D § 3 (sources, ledger, rules R1–R8) and § 4 (the review rules; apply them verbatim, there is nothing to decide beyond them).
- plan-39-A § 3 (the `activeLow` key), `docs/parts-format.md` § Pinouts and pins.
- `scripts/pin-polarity.js --help`; `planning/plan-39-data/ledger.tsv`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-E complete | `ls planning/completed/plan-39-E-*.md \| wc -l` → 1 | MET (2026-11-14: 1, at landing; the reviews ran alongside E, see Corrections 1) |
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
| F1 | buffer | 1–70: 1G125 … 74_244 (70) | 90 | 1804 | 1 | 0 |
| F2 | buffer | 71–152: 74_245 … TXV0106 (82) | 90 | 1797 | 0 | 0 |
| F3 | oscillator | 1–77: ash5kw-0 … ecs-3951m-bn-2 (77) | 81 | 155 | 2 | 0 |
| F4 | oscillator | 78–139: ecs-3951m-bn-3 … sit1534-0 (62) | 80 | 204 | 38 | 0 |
| **total** | | 291 devices | **341** | **3960** | **41** | **0** |

Kind notes (what to expect; the review rules decide, not these):

- **buffer** — buffers and transceivers: output enables and direction pins; DIR/OE polarity is whatever the datasheet marks.
- **oscillator** — oscillators: the enable/standby pin (`OE`, `O/E`, `E/D`, `Stand-by`, `Tri-State`) is active high on some parts and active low on others; only a datasheet mark flags it (rule 1).

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

### Phase 1 — F1: buffer 1–70 (90 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review buffer devices 1–70 (1G125 … 74_244): 90 pinouts, § 3 procedure; edits `schematical/schematical/Parts/buffer.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind buffer --devices 1-70 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 90 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: buffer 1-70 `ok: 90 pinouts, 90 ledger rows checked`; golden-check (phase staged) `ok: 1041 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: f8013565

### Phase 2 — F2: buffer 71–152 (90 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review buffer devices 71–152 (74_245 … TXV0106): 90 pinouts, § 3 procedure; edits `schematical/schematical/Parts/buffer.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind buffer --devices 71-152 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 90 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: buffer 71-152 `ok: 90 pinouts, 90 ledger rows checked`; golden-check (phase staged) `ok: 845 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: —

### Phase 3 — F3: oscillator 1–77 (81 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [ ] Review oscillator devices 1–77 (ash5kw-0 … ecs-3951m-bn-2): 81 pinouts, § 3 procedure; edits `schematical/schematical/Parts/oscillator.json`.
- [ ] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`.

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind oscillator --devices 1-77 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 81 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
Commit: —

### Phase 4 — F4: oscillator 78–139 (80 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [ ] Review oscillator devices 78–139 (ecs-3951m-bn-3 … sit1534-0): 80 pinouts, § 3 procedure; edits `schematical/schematical/Parts/oscillator.json`.
- [ ] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`.

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind oscillator --devices 78-139 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 80 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
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

1. **F was reviewed alongside G and H and landed after E.** The reviews
   touch only their own kinds' part files and ledger rows. The gate exists
   to land letters in order, so each phase is committed in order once E is
   archived, staged as in plan-39-E Corrections 2. The Prerequisites row
   was measured at landing.
2. **Reviewers' calls, applying plan-39-E's rulings:**
   - 74_241_CD 2OE: the shared table bars it, but the '241 text says active
     high. The text decides, so it is unflagged.
   - Inverted outputs drawn with a bar are flagged (74_240, 74_366,
     74_368_CD, 74_540_CD, 74_540_FCT, 74_2240_FCT). They are not flagged
     where the sheet draws plain Y (74_368, 74_368_LS, 1G240).
   - TXU0101–TXU0304 OE: the sheets say pulling OE to GND turns the
     outputs off, so it is active high and unflagged.
   - Oscillators: only ECS-1000/2200 INH̄ ("TRI-STATE"), LMK6x `ST`
     ("Standby (Active Low)", the ST alternative only) and NJU6366 CONT̄ are
     flagged. Every other enable/standby pin is active high or open-enabled.
   - NJU6366 pin 3 XT̄ is barred, but the catalog names pins 3 and 5 both
     `XT`, so R5 cannot hold with one of them flagged. It stays unflagged
     and the row's note says so; flagging it needs a rename first.
3. **The tool's writes are now atomic** (temp file, then rename). A
   reviewer's read once caught a part file mid-write and failed to parse.
   The retry worked and no file was damaged.

## Summary

F's risk is reading: a missed overbar or a misread package. The tool's rules catch inconsistency, not a wrong reading; the page citations in the ledger let a later reader re-check any row. Nothing outside the part files, the ledger and the golden changes.
