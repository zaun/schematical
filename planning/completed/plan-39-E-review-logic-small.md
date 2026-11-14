# plan-39-E: Active-low review — Counters, decoders, latches, registers, shifters, flip-flops, mux, arith, CPU, PIO, RTC

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-D

E reviews 312 pinouts of 258 devices (counter, decoder, latch, register, shift, arith, dff, jkff, mux, cpu, pio, rtc) against their datasheets and flags every active-low pin, recording each review in the ledger. Its outcome: `node scripts/pin-polarity.js check --complete` is clean for every pinout listed below, and the part files carry `"activeLow"` on exactly the pins the datasheets mark active low.

References:

- plan-39-D § 3 (sources, ledger, rules R1–R8) and § 4 (the review rules; apply them verbatim, there is nothing to decide beyond them).
- plan-39-A § 3 (the `activeLow` key), `docs/parts-format.md` § Pinouts and pins.
- `scripts/pin-polarity.js --help`; `planning/plan-39-data/ledger.tsv`.

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-D complete | `ls planning/completed/plan-39-D-*.md \| wc -l` → 1, and `grep -oE '"activeLow": ?true' schematical/schematical/Parts/{sram,eeprom,cpu,converter}.json \| wc -l` → 279 | MET (2026-11-14: 1 and 279; the grep's spacing corrected per plan-39-D Corrections 1) |
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
| E1 | counter | 1–57: 40102B … 74_97 (57) | 67 | 968 | 28 | 0 |
| E1 | decoder | 1–32: 40147B … 74_49 (32) | 37 | 486 | 1 | 0 |
| E2 | latch | 1–28: 4042B … TPIC6B259 (28) | 32 | 662 | 0 | 0 |
| E2 | register | 1–26: 40174B … 74_874 (26) | 28 | 706 | 0 | 0 |
| E2 | shift | 1–36: 40105B … TPIC6C596 (36) | 46 | 684 | 45 | 0 |
| E3 | arith | 1–16: 4063B … 74_885 (16) | 16 | 257 | 3 | 0 |
| E3 | dff | 1–22: 1G175 … TPIC6B273 (22) | 31 | 284 | 0 | 0 |
| E3 | jkff | 1–6: 4027B … 74_76 (6) | 8 | 108 | 0 | 0 |
| E3 | mux | 1–20: 40257B … 74_670 (20) | 25 | 322 | 8 | 0 |
| E3 | cpu | 1–4: MCS6502 … Z84C00 (4) | 8 | 298 | 1 | 43 |
| E3 | pio | 1–4: W65C21N … W65C22S (4) | 7 | 266 | 0 | 0 |
| E3 | rtc | 1–7: BQ32000 … DP8573 (7) | 7 | 123 | 1 | 0 |
| **total** | | 258 devices | **312** | **5164** | **87** | **43** |

Kind notes (what to expect; the review rules decide, not these):

- **counter** — counters; the clear/load/enable/count-direction pins are the usual active-low ones, and some counters mark the clock pin (rule 3).
- **decoder** — decoders and demultiplexers: outputs and enables are often active low; select inputs are not.
- **latch** — latches: output enable and latch enable can be either polarity; the datasheet decides.
- **register** — registers: output enable and clock enable can be either polarity.
- **shift** — shift registers: 45 signal pins have two alternatives in their names; use the per-alternative form where only one alternative is marked.
- **arith** — arithmetic parts: carry-in/out and compare outputs; an active-low output is marked in the function table.
- **dff** — flip-flops: preset, clear and output enable are the usual ones.
- **jkff** — JK flip-flops: preset, clear and (often) the clock are marked.
- **mux** — multiplexers: enables and strobes.
- **cpu** — processors: 43 pins end in `#` (seeded by plan-39-D); `docs/parts-format.md` lists the signals the kind documents as active low (lines 1250–1253 for the 6502/65C02 family, 1341–1342 for the Z80).
- **pio** — peripheral interface adapters: chip selects, read/write and reset.
- **rtc** — real-time clocks: chip enables, interrupt and reset outputs.

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

### Phase 1 — E1: counter 1–57 + decoder 1–32 (104 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review counter devices 1–57 (40102B … 74_97): 67 pinouts, § 3 procedure; edits `schematical/schematical/Parts/counter.json`. — done: 67 rows (2 reviewers + 74_161 p16 by hand)
- [x] Review decoder devices 1–32 (40147B … 74_49): 37 pinouts, § 3 procedure; edits `schematical/schematical/Parts/decoder.json`. — done: 37 rows
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: regenerated over all of E, then this phase's lines staged (Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind counter --devices 1-57 --complete && node scripts/pin-polarity.js check --kind decoder --devices 1-32 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 104 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: counter `ok: 67 pinouts, 67 ledger rows checked`; decoder `ok: 37 pinouts, 37 ledger rows checked`; golden-check (phase staged) `ok: 853 parts.txt line(s) changed, only by :low`; ctest on the full E tree `100% tests passed, 0 tests failed out of 3`.
Commit: 500cb44a

### Phase 2 — E2: latch 1–28 + register 1–26 + shift 1–36 (106 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review latch devices 1–28 (4042B … TPIC6B259): 32 pinouts, § 3 procedure; edits `schematical/schematical/Parts/latch.json`. — done: 32 rows
- [x] Review register devices 1–26 (40174B … 74_874): 28 pinouts, § 3 procedure; edits `schematical/schematical/Parts/register.json`. — done: 28 rows
- [x] Review shift devices 1–36 (40105B … TPIC6C596): 46 pinouts, § 3 procedure; edits `schematical/schematical/Parts/shift.json`. — done: 46 rows (2 reviewers)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: staged from the full E regeneration (Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind latch --devices 1-28 --complete && node scripts/pin-polarity.js check --kind register --devices 1-26 --complete && node scripts/pin-polarity.js check --kind shift --devices 1-36 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 106 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: latch `ok: 32 pinouts`, register `ok: 28 pinouts`, shift `ok: 46 pinouts` (each with as many ledger rows); golden-check (phase staged) `ok: 1446 parts.txt line(s) changed, only by :low`; ctest on the full E tree `100% tests passed, 0 tests failed out of 3`.
Commit: b6091a52

### Phase 3 — E3: arith 1–16 + dff 1–22 + jkff 1–6 + mux 1–20 + cpu 1–4 + pio 1–4 + rtc 1–7 (102 pinouts)

One commit per phase: the edited part file(s), `planning/plan-39-data/ledger.tsv` and `backend/tests/golden/parts.txt`.

- [x] Review arith devices 1–16 (4063B … 74_885): 16 pinouts, § 3 procedure; edits `schematical/schematical/Parts/arith.json`. — done: recorded; check ok (see Observed)
- [x] Review dff devices 1–22 (1G175 … TPIC6B273): 31 pinouts, § 3 procedure; edits `schematical/schematical/Parts/dff.json`. — done: recorded; check ok (see Observed)
- [x] Review jkff devices 1–6 (4027B … 74_76): 8 pinouts, § 3 procedure; edits `schematical/schematical/Parts/jkff.json`. — done: recorded; check ok (see Observed)
- [x] Review mux devices 1–20 (40257B … 74_670): 25 pinouts, § 3 procedure; edits `schematical/schematical/Parts/mux.json`. — done: recorded; check ok (see Observed)
- [x] Review cpu devices 1–4 (MCS6502 … Z84C00): 8 pinouts, § 3 procedure; edits `schematical/schematical/Parts/cpu.json`. — done: recorded; check ok (see Observed)
- [x] Review pio devices 1–4 (W65C21N … W65C22S): 7 pinouts, § 3 procedure; edits `schematical/schematical/Parts/pio.json`. — done: recorded; check ok (see Observed)
- [x] Review rtc devices 1–7 (BQ32000 … DP8573): 7 pinouts, § 3 procedure; edits `schematical/schematical/Parts/rtc.json`. — done: recorded; check ok (see Observed)
- [x] Rebuild and regenerate the golden: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && /tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done: the full E regeneration (Corrections 2)

Acceptance: every pinout of the phase has a ledger row, the ledger matches the files, and the golden moved only by flags.
  Check: `node scripts/pin-polarity.js check --kind arith --devices 1-16 --complete && node scripts/pin-polarity.js check --kind dff --devices 1-22 --complete && node scripts/pin-polarity.js check --kind jkff --devices 1-6 --complete && node scripts/pin-polarity.js check --kind mux --devices 1-20 --complete && node scripts/pin-polarity.js check --kind cpu --devices 1-4 --complete && node scripts/pin-polarity.js check --kind pio --devices 1-4 --complete && node scripts/pin-polarity.js check --kind rtc --devices 1-7 --complete && node scripts/pin-polarity.js golden-check && ctest --test-dir /tmp/logic-p39 -R '^(library_check|listing_check|trace_check)$'` → every `check` exits 0, `golden-check` exits 0, 3 of 3 tests passed (est. 90 min of reading for 102 pinouts, then 6 min for the checks: the loader and `listing_check` load every part file the phase edited, which no smaller check does).
  Observed 2026-11-14: arith 16, dff 31, jkff 8, mux 25, cpu 8, pio 7, rtc 7 pinouts — each `ok: N pinouts, N ledger rows checked`; golden-check `ok: 1424 parts.txt line(s) changed, only by :low`; ctest on the full E tree `100% tests passed, 0 tests failed out of 3`.
Commit: 0f983267

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

1. **A datasheet that contradicts itself.** Some sheets draw a pin with an
   overbar while their own function text says it acts high (CD74HC4514's
   LE; CD74AC373/573 and CD74HC533/563 LE; CD74HC75's enables; the
   SN74ALVCH32373 pin table's 1LE–4LE). Rule 1 assumes a consistent sheet.
   Applied uniformly: the function text (truth table, "outputs follow when
   high") decides and the overbar is treated as a drawing error. Those pins
   are unflagged, and each row's note says so. A pin whose datasheet *name*
   carries the bar (Q̄, Ē on the 74_354, `QBAR`/`KBAR`/`YBAR` catalog names)
   is flagged: the bar names the signal, so there is no contradiction.
   74_4514 and 74_4515 LE were first flagged by one reviewer and then
   unflagged to match this.
2. **The three phases were reviewed together and committed one by one.**
   They edit different part files. The ledger and `parts.txt` are shared, so
   each phase's commit holds the ledger rows and golden lines of its own
   kinds only (staged by filtering the full state). The phase's
   `check … --complete` and `golden-check` ran on that staged state.
   `library_check`/`listing_check`/`trace_check` ran once on the full tree,
   because a partial golden cannot match part files that already carry the
   other phases' flags.
3. **R5 across datasheets.** A device's pinouts are read from different
   datasheets (74_193 p16 from cd74hc192, fk20 from sn74ls193; 74_390,
   74_393, 74_4020 and 74_173 likewise), and the sheets do not always mark
   the same pin. R5 requires one flag per pin name in a device. Where one of
   the device's listed datasheets marks the pin, it is flagged in every
   pinout, with a note naming that sheet. 74_192 p16 CO/BO were flagged
   from sn74ls193, which covers the '192.
4. **Inverted outputs.** Q̄ outputs drawn with an overbar are flagged (rule
   1). This covers names like `QBAR`, `1QBAR` and `QHBAR`, and also `nQ` on
   inverting registers whose sheet overbars them (74_534/564/576/577,
   74_533/563/580). All reviewers applied it the same way.
5. **WDC's `B` suffix** (RESB, IRQB, CS2B) is not defined as a negation
   marker in the W65C02S/W65C816S/W65C21/W65C22 sheets. Pins are flagged
   where the sheet's text says low-active or held low. W65C22 RESB (its sheet
   says neither) and RWB (one name, low = write) are not flagged.

## Summary

E's risk is reading: a missed overbar or a misread package. The tool's rules catch inconsistency, not a wrong reading; the page citations in the ledger let a later reader re-check any row. Nothing outside the part files, the ledger and the golden changes.
