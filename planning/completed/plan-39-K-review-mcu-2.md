# plan-39-K: Active-low review — Microcontrollers, devices 146–303 (attiny3226 … rp2040)

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-J

K reviews 210 pinouts of 158 devices (mcu) against their datasheets and flags every active-low pin, recording each review in the ledger. Its outcome: `node scripts/pin-polarity.js check --complete` is clean for every pinout listed below, and the part files carry `"activeLow"` on exactly the pins the datasheets mark active low.

References:

- plan-39-D § 3 (sources, ledger, rules R1–R8) and § 4 (the review rules; apply them verbatim, there is nothing to decide beyond them).
- plan-39-A § 3 (the `activeLow` key), `docs/parts-format.md` § Pinouts and pins.
- `scripts/pin-polarity.js --help`; `planning/plan-39-data/ledger.tsv`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-J complete | `ls planning/completed/plan-39-J-*.md \| wc -l` → 1 | MET (2026-11-14: 1, at landing; the reviews ran alongside J, see Corrections 1) |
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
| K1 | mcu | 146–216: attiny3226 … atxmega64a3u (71) | 105 | 2764 | 0 | 0 |
| K2 | mcu | 217–303: atxmega64a4u … rp2040 (87) | 105 | 2923 | 0 | 0 |
| **total** | | 158 devices | **210** | **5687** | **0** | **0** |

Kind notes (what to expect; the review rules decide, not these):

- **mcu** — microcontrollers: no mcu signal pin name contains `/`, so every mcu flag is `true`. The catalog names most pins for their port (`PC6`); only a pin whose catalog name denotes the active-low function is flagged (plan-39-D § 4 rule 12). 103 pins are named `RESET` (`grep -c '"name":"RESET"' schematical/schematical/Parts/mcu.json` → 103).

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

### Phase 1 — K1: mcu 146–216 (105 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review mcu devices 146–216 (attiny3226 … atxmega64a3u): 105 pinouts, § 3 procedure; edits `schematical/schematical/Parts/mcu.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind mcu --devices 146-216 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 105 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: mcu 146-216 `ok: 105 pinouts, 105 ledger rows checked`; golden-check (phase staged) `ok: 89 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: e28ab196

### Phase 2 — K2: mcu 217–303 (105 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review mcu devices 217–303 (atxmega64a4u … rp2040): 105 pinouts, § 3 procedure; edits `schematical/schematical/Parts/mcu.json`. — done: rows recorded, check ok (Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over the full tree, this phase staged (plan-39-E Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind mcu --devices 217-303 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 105 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: mcu 217-303 `ok: 105 pinouts, 105 ledger rows checked`; golden-check (phase staged) `ok: 26 parts.txt line(s) changed, only by :low`; ctest on the full tree `100% tests passed, 0 tests failed out of 3`.
Commit: d8745cce

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

1. **K was reviewed alongside J and landed after J**, staged as in
   plan-39-J Corrections 1. Its flags reached `mcu.json` in J1's commit.
   The Prerequisites row was measured at landing.
2. **Rule 12, as in J:** ATtiny, AVR Dx/Ex/Sx/LA parts name their reset pin
   by its port, so it is not flagged. XMEGA RESET is flagged on all its
   pinouts. Some BGA ball tables print it without the bar that the same
   sheet's QFP figure has, so it is flagged under R5.
3. **A catalog error found, not fixed here:** `avr32sd20` p20 names pin 2
   `PF6` and pin 3 `PF7`. The AVR32SD20/28/32 sheet (DS40002629 p.18,
   I/O mux p.22) shows pin 2 as a dedicated RESET (barred) and pin 3 as
   UPDI, with no PF6/PF7. A reviewer flagged `PF6` by position (rule 8).
   That was reverted: the flag describes what the catalog name denotes
   (rule 12), and plan-39 renames no pin. The row's note records the error
   for a catalog fix.
4. **rp2040 QSPI_SS_N** is flagged: the RP2040 sheet defines the `_n`
   suffix as active low ("active low chip select … ss_n", p.502). RUN
   ("run when driven high") and the ESP32 modules' EN ("High: on") are
   active high and not flagged.

## Summary

K's risk is reading: a missed overbar or a misread package. The tool's rules catch inconsistency, not a wrong reading; the page citations in the ledger let a later reader re-check any row. Nothing outside the part files, the ledger and the golden changes.
