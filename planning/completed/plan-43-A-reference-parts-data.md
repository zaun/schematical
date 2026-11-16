# plan-43-A: Reference-board parts — census, datasheet filing, skips, the family list

Last updated: 2026-11-15
Overall Effort: x-large (1d–3d) — plan-43 is A → B → C → D → E
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

The 209 reference boards under `routing-example/` (git-ignored) use 900
part numbers the library lacks. Each was looked up on DigiKey on 2026-11-15:
300 distinct parts have stock and a datasheet. plan-43 adds the 47 of them
that fit a kind the library already has (diode, transistor, switch,
connector, buffer, oscillator, rtc, converter), with their footprints
and datasheet entries. It adds no kind, no model and no app code. The other
253 are recorded by family for the plans that follow (§ 5).

The single behavioral outcome of plan-43: loading the part files gives the
baseline 32854 parts (P0, § Prerequisites) plus the 47 parts of
`planning/plan-43-data/census/fit.tsv`; each is in the drawer leaf its device
names, has a footprint and a maker-site datasheet; `ctest` passes the same
36 programs; the app builds.

| Letter | Content | Effort |
|---|---|---|
| **A** | census re-check, PDF filing, maker copies of two datasheets, skipped parts, the family list, todo | medium |
| B | 14 diodes and 13 transistors, their footprints, goldens | large |
| C | 3 switches, 2 USB-C receptacles, 2 buffers, 7 oscillators, their footprints, goldens | large |
| D | 2 real-time clocks and 4 converters (declarative devices), their tests, goldens | large |
| E | docs, landing tier | small |

Outcome of A: every PDF the lookup downloaded is either in its kind's
datasheet directory (the 47), under `datasheets/reference-boards/<family>/`
(the 248 waiting for a later plan), or deleted (the 5 skipped parts);
`targets.tsv`'s `pdf` column names a file that exists for all 47 rows;
`datasheets/skipped-parts.md` lists the 5; `planning/todo.md` points at the
family list.

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets).
- `datasheets/skipped-parts.md` (format; line 480 holds `BAT54C-G3-08`).
- `planning/plan-43-data/` — `README.md`, `census/classify.py`,
  `census/lookup.py`, `census/refile.py`, `census/fit.tsv`, `census/skip.tsv`,
  `census/rename.tsv`, `candidates.tsv`, `facts.tsv`, `targets.tsv`, `raw/`
  (all written at planning time, 2026-11-15).
- `routing-example/datasheets-log.tsv` (the lookup log; not in the repository).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p43 && cmake --build /tmp/logic-p43 -j8 && ctest --test-dir /tmp/logic-p43 -j8` → `100% tests passed` | MET (2026-11-15 at 7410acd9, build directory `/tmp/logic-p43w`: `100% tests passed, 0 tests failed out of 36`; T0 = 36) |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt` | MET (2026-11-15 at 7410acd9): **P0 = 32854**, **R0 = 32854**, **F0 = 1624** |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p43-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-15 at 7410acd9: `** BUILD SUCCEEDED **`) |
| None of the 47 is in the library | `python3 -I -c "import json,glob,csv; n={p['name'] for f in glob.glob('schematical/schematical/Parts/*.json') for d in json.load(open(f)).get('devices',[]) for p in d['parts']}; print(sum(r['mpn'] in n for r in csv.DictReader(open('planning/plan-43-data/census/fit.tsv'),delimiter='\t')))"` → 0 | MET (2026-11-15 at 7410acd9: 0) |
| plan-42 has not started | `ls datasheets/regulator schematical/schematical/Parts/regulator.json 2>&1 \| grep -c 'No such file'` → 2 | MET (2026-11-15, re-run in the worktree: 2). Owner, 2026-11-15: plan-43 runs first; plan-42 starts after plan-43-E is archived |
| The downloaded PDFs are in the working tree | `python3 -I planning/plan-43-data/census/refile.py --dry-run` → `5 deleted`, `4 kept in place`, `38 to a kind directory`, `217 to reference-boards`, `240 datasheets.tsv lines carried` | MET (2026-11-15, re-run in the worktree after the PDFs were moved into it, § Corrections: those five lines) |
| The lookup log is present | `test -f routing-example/datasheets-log.tsv && echo ok` → `ok` | MET (2026-11-15: the log was copied into the worktree, § Corrections) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `cd planning/plan-43-data && python3 -I census/classify.py
  ../../routing-example/datasheets-log.tsv` prints `300 parts` and leaves
  `candidates.tsv` unchanged (`git diff --stat` shows nothing for it).
- Every `pdf` of `targets.tsv` exists: `python3 -I -c "import csv,os;
  print(sum(not os.path.exists(r['pdf']) for r in
  csv.DictReader(open('planning/plan-43-data/targets.tsv'),delimiter='\t')))"`
  → 0.
- No PDF of the lookup is left in a directory named after a DigiKey
  category: `ls datasheets | wc -l` → 48 (the 47 entries git tracks today,
  `git ls-files datasheets | awk -F/ '{print $2}' | sort -u | wc -l` → 47,
  plus `reference-boards`).
- `datasheets/skipped-parts.md` has the section `## Reference boards
  (plan-43)` with the 5 rows of `census/skip.tsv`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No part of the 203 `later` rows or the 45 `plan-42` rows is added by any
  letter of plan-43, and no plan-42 file is edited.
- No new DigiKey lookups for the 600 part numbers the lookup did not find
  or found without stock.
- The 20 datasheets the lookup could not download are not chased.

## 2. Current State

`routing-example/datasheets-log.tsv` has 902 lines (900 part numbers and two
test lookups). Each line is one DigiKey search driven in Safari: the
most-stocked Active product whose part number starts with the board's value
(or the other way round), its stock, and where the datasheet was saved. The
PDFs sit untracked under `datasheets/`, in directories the lookup script
named after DigiKey's leaf category (`regulator-linear`,
`analog-switches-multiplexers-demultiplex`, …) or after an existing
directory (`diode`, `transistor`, `mcu`, `osc`, `switch`, `rtc`, `adc`).
Three tracked files gained lines: `datasheets/diode/datasheets.tsv`,
`datasheets/mcu/datasheets.tsv`, `datasheets/transistor/datasheets.tsv`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Lookups by result | not found 454, downloaded 263, no stock 120, already have 44, download failed 20, no datasheet 1 | `python3 -I -c "import csv,collections; print(collections.Counter(r['result'] for r in csv.DictReader(open('routing-example/datasheets-log.tsv'),delimiter='\t')))"` |
| Distinct parts with stock and a datasheet | 300 | `census/classify.py` → `300 parts` |
| Of those: plan-43 / skipped / plan-42 / later | 47 / 5 / 45 / 203 | same run |
| plan-43 by kind | diode 14, transistor 13, oscillator 7, converter 4, switch 3, buffer 2, connector 2, rtc 2 | same run |
| The 47 on DigiKey, 2026-11-15 | 47 Active, 47 with stock above 0 (lowest: 2N7002PW, 557) | `census/lookup.py` → `47 rows; 0 not found; 0 with no stock`; `facts.tsv` |
| Untracked PDFs under `datasheets/` | 264 | `git ls-files --others --exclude-standard datasheets \| grep -ci '\.pdf$'` |
| Their fate | 5 deleted, 4 kept in place, 38 to a kind directory, 217 to `reference-boards` | `census/refile.py --dry-run` |
| Datasheet ids the 47 need / already in `datasheets.json` | 46 / 5 (`bzx384`, `bat54-g_bat54a-g_bat54c-g_bat54s-g`, `onsemi-bc807-16lt1-d`, `onsemi-bc817-16lt1-d`, `sn74lvc1g126`) | `targets.tsv` columns `datasheet_id`, `datasheet_id_existing` |

### Verified properties

- **Each of the 47 fits its kind without a loader change, and the app shows it correctly with no code change.** Read against
  `docs/parts-format.md`: the 14 diodes are single rectifier, Schottky,
  switching and Zener diodes and one common-cathode pair (§ `diode` types);
  the 13 transistors are single BJTs and MOSFETs, one dual NPN and one N + P
  pair (§ `transistor` elements); the switches are two SPST-NO tactile
  switches and one 8-position SPST DIP switch (§ `switch`); the connectors
  are USB-C receptacles (`"usb": "C"`, `backend/src/models/connector.c:26`
  accepts A, B, C); the buffers join the existing devices `1G125` and
  `1G126`, which already have the `LVC` family; the oscillators have one
  output and at most an enable pin (§ `oscillator`); the RTCs and converters
  are I2C and SPI register devices like the library's `BQ32000`, `ADS1110`
  (data-then-configuration read with no register pointer, as the MCP3428)
  and `DAC7311` (write-only SPI frame, as the AD5621).
- **The parts left for later do not fit.** TVS and ESD parts have no diode
  type; micro-USB, JST and coaxial connectors have no `connector` function;
  auto-direction translators (TXB, TXS, PCA9306) have neither a DIR pin nor
  a fixed direction (§ `buffer`, model `translator`); `eeprom` is parallel
  only; crystals have no output; SPDT slide switches are not contacts that
  start open; the microcontrollers need a `core` the loader rejects
  (`unknown mcu core`), and the one Cortex-M0+ part, ATSAMD21G18A-AUT,
  would be shown as "Dual Cortex-M0+" (`Backend/PartFacts.swift`
  `mcuCoreName`: the label was written for the RP2040), so it waits for
  the `mcu` family's plan; ULN2003 has a pin (`COM`) in no element.
- **Five fitting parts have no maker-site datasheet** (`census/skip.tsv`):
  DigiKey links only its own copy, and `docs/parts-format.md` § Datasheets
  requires the manufacturer's site. Probed 2026-11-15 with `curl -sL -o
  /dev/null -w '%{http_code}'`: `mccsemi.com/pdf/Products/BAT54ADW.pdf`,
  `…/BAT54TW.pdf`, `…/BAT54TW(SOT-363).pdf` → 404.
- **Maker copies exist for the others DigiKey hosts itself** (same probe,
  each `200` and `%PDF-`): `https://aosmd.com/sites/default/files/res/datasheets/AO3400A.pdf`,
  `https://gct.co/files/specs/swt0105-spec.pdf`,
  `https://download.epsondevice.com/td/pdf/brief/SG-210STF_en.pdf`,
  `https://www.vishay.com/docs/86474/bzx384.pdf` (already `bzx384`),
  `https://www.ti.com/lit/ds/symlink/csd17311q5.pdf`.
- **ATSAMD21G18A-AUT's DigiKey "datasheet" is an errata sheet.** It is
  filed with the `mcu` family; that plan fetches the family data sheet
  (`https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D21-DA1-Family-Data-Sheet-DS40001882.pdf`,
  `200`, `%PDF-`, 11870237 bytes on 2026-11-15).
- **BAT54C-G3-08 is in Vishay's -G document** (`datasheets/diode/vishay/bat54-g_bat54a-g_bat54c-g_bat54s-g.pdf`
  p.1 ordering table, `pdftotext -layout -l 1 … | grep BAT54C-G3-08`).
  plan-29 skipped it because DigiKey links the non-G document.
- **One lookup took a sibling.** The board value `ASE-18.432MHZ-L-C-T`
  matched `ASE-1.8432MHZ-L-C-T`; `census/rename.tsv` puts the board's part
  back, and `facts.tsv` shows it Active with 3053 in stock.

## 3. Design Overview

A is bookkeeping: three scripts already written and run at planning time,
one of them (`refile.py`) with its writes still to do. Nothing here can
break the library. The correctness risk of plan-43 is in B–D (pinouts and
land patterns read from drawings).

Rejected alternatives:

- **One generic unsimulated kind for everything.** Owner, 2026-11-15: each
  family gets its own kind and plan.
- **Adding the five parts without a `"datasheet"`.** The rule allows that
  only for makers that no longer host the document.
- **Leaving the 217 waiting PDFs in DigiKey-named directories.** They would
  read as kind directories; one `reference-boards/` tree keeps them apart
  until their plan files them.

## 4. Filing

`census/refile.py` moves every untracked PDF of `candidates.tsv`:

| `where` | Destination |
|---|---|
| `plan-43` | the kind's directory: `datasheets/diode/<maker>/`, `datasheets/transistor/<maker>/`, `datasheets/switch/cts/`, `datasheets/switch/tactile/<cit\|gct>/`; flat in `datasheets/connector-usb/`, `datasheets/buffer/`, `datasheets/osc/`, `datasheets/rtc/`, `datasheets/adc/`, `datasheets/dac/` |
| `plan-42`, `later` | `datasheets/reference-boards/<family>/<maker directory>/`, with one `datasheets/reference-boards/datasheets.tsv` (path, url, bytes, sha256) |
| `skipped` | deleted (`skipped-parts.md`: "Their PDFs are not in the tree") |

Short maker names are the script's `MAKER` table (`diodes`, `nexperia`,
`onsemi`, `vishay`, `vishay-siliconix`, `infineon` exist; `diotec`,
`panjit`, `slkor`, `aos`, `goodwork`, `mcc`, `ti`, `cts` are new).

Then three files are replaced by the maker's copy or dropped (`targets.tsv` column
`pdf` is the final path):

| Part | Remove | Fetch | To |
|---|---|---|---|
| AO3400A | `datasheets/transistor/aos/AO3400A_ds.pdf` | `https://aosmd.com/sites/default/files/res/datasheets/AO3400A.pdf` | `datasheets/transistor/aos/AO3400A.pdf` |
| SG-210STF 27.0000ML5 | `datasheets/osc/SG-210STF_X1G0041710031_en.pdf` | `https://download.epsondevice.com/td/pdf/brief/SG-210STF_en.pdf` | `datasheets/osc/SG-210STF_en.pdf` |
| BZX384C5V1-E3-08 | `datasheets/diode/vishay/PdfFile_148489.pdf` | nothing: `datasheets/diode/vishay/bzx384.pdf` is tracked | — |
| BAT54C-G3-08 | nothing: the lookup found the tracked non-G document already in place | nothing: the -G document `datasheets/diode/vishay/bat54-g_bat54a-g_bat54c-g_bat54s-g.pdf` is tracked | — |

`swt0105-spec.pdf` and `csd17311q5.pdf` are byte-for-byte the maker's files
already (GCT's and TI's own documents); only their URL differs, and that is
in `targets.tsv`.

Every `datasheets.tsv` line the script carries keeps the URL the file was
fetched from; for the replaced files, fix the line by hand (path, the maker
URL, `wc -c`, `shasum -a 256`).

## 5. The families that follow

`candidates.tsv` column `family`, rows with `where` = `later` (203) and
`plan-42` (45). Each family is one later plan; none has a number yet (plans
are numbered when written). "Needs" is what the library lacks today.

| Family | Parts | Needs |
|---|---|---|
| regulator | 45 | the `regulator` kind of plan-42. plan-42 covers TI, ADI and Microchip; these 45 (Diodes Incorporated, Microchip, onsemi, TI, Torex, Toshiba, ST, GOODWORK, Allegro, EVVO) are parts for a follow-up once the kind exists |
| inductor | 26 | a kind for inductors and ferrite beads (`planning/todo.md`: "Add inductors, fuses / polyfuses, relays and buzzers") |
| connector | 22 | `connector` functions for micro-USB, JST GH / PH / SH, pin-header families, U.FL and SMA, RJ45 with magnetics, audio jack, pluggable terminal blocks, Tag-Connect, spring contacts |
| protection | 18 | TVS diodes, TVS / ESD arrays with supply and ground pins, polymer ESD suppressors |
| bus-switch | 17 | analog switches and multiplexers, bus switches, I2C multiplexers and buffers, auto-direction translators |
| serial-memory | 16 | I2C and Microwire EEPROMs, SPI NOR flash, one DDR3L SDRAM, one crypto element |
| power-management | 14 | load switches, power multiplexers, chargers, fuel gauges, supervisors, PoE controllers, a PMIC, an ideal diode |
| amplifier | 13 | op-amps, comparators, audio amplifiers, one audio codec |
| mcu | 14 | `mcu` cores for Cortex-M0, M4, M7, M33, MSP430, PIC baseline, and a core label for single Cortex-M0+ parts (ATSAMD21G18A); FPGAs; one RF microcontroller |
| transceiver | 12 | Ethernet and USB PHYs, CAN, LIN and RS-232 transceivers, a digital isolator |
| sensor | 10 | light, temperature, CO2, magnetic angle, thermocouple, energy metering, fan control, touch, a photodiode |
| crystal | 8 | a kind for quartz crystals |
| driver | 8 | LED drivers, motor drivers, I/O expanders, ULN2003 |
| electromechanical | 6 | slide switches, a rotary encoder, a trimmer, a speaker, a vibration motor |
| rf-passive | 5 | a balun, chip antennas, a SAW filter, an RF front end |
| display | 4 | a TFT module, a touch panel, a 7-segment LED not in `7seg`, a white LED |
| fuse | 4 | resettable fuses (same todo item as inductors) |
| power-module | 3 | DC-DC and AC-DC modules |
| optocoupler | 2 | an optocoupler and a photo relay |
| capacitor | 1 | a 2.2 pF 0201 part: a value and size the `capacitor` kind's parts do not cover |

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Census re-check and filing

- [x] Run `cd planning/plan-43-data && python3 -I census/classify.py
      ../../routing-example/datasheets-log.tsv`; `git diff --stat
      planning/plan-43-data/candidates.tsv` shows nothing. — `300 parts`,
      `plan-43 47`; `git status --short planning` empty.
- [x] Run `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID|SECRET)=' ~/.zshrc)"
      && rm planning/plan-43-data/raw/*.json && (cd planning/plan-43-data &&
      python3 -I census/lookup.py)` (47 API calls). A row that is now `NOT
      FOUND`, not Active or without stock leaves plan-43: move its line from
      `census/fit.tsv` to `census/skip.tsv` with the reason, re-run
      `classify.py`, and record it in § Corrections. — `47 rows; 0 not
      found; 0 with no stock` (899 calls left); `facts.tsv` unchanged.
- [x] Run `python3 -I planning/plan-43-data/census/refile.py` from the
      repository root. A second run changes nothing: `git status --short
      -uall datasheets | shasum` is the same before and after it. — `5
      deleted, 4 kept in place, 38 to a kind directory, 217 to
      reference-boards, 240 datasheets.tsv lines carried`; shasum
      `86550af2…` both times.
- [x] Replace the two files and delete the third as § 4's table says
      (`curl -sL -A 'Mozilla/5.0' -o <to> <url>`; `head -c 5 <to>` →
      `%PDF-`), and fix their `datasheets.tsv` lines. — AO3400A.pdf 317848
      bytes, SG-210STF_en.pdf 916955 bytes, both `%PDF-`; the AO3400A line
      rewritten, the PdfFile_148489 line removed (`datasheets/osc` has no
      `datasheets.tsv`).
- [x] (added) Drop `MBT3904DW1T3G`: moved from `census/fit.tsv` to
      `census/skip.tsv`, its `targets.tsv` row, PDF and `datasheets.tsv`
      line removed, `classify.py` and `lookup.py` re-run (§ Corrections). —
      `plan-43 46`, `skipped 6`; `46 rows; 0 not found; 0 with no stock`.

Acceptance: § 1's second and third goals.
  Check: the two commands of § 1 → `0` and `48` (est. 1 min). — `0`, `48`.
Commit: 6ce8fcfd

### Phase 2 — Skips, todo

- [x] `datasheets/skipped-parts.md`: add `## Reference boards (plan-43)` at
      the end, with one table row per line of `census/skip.tsv` (part,
      maker, "would have gone into" = `diode` / Discrete → Diodes →
      Schottky for the three diodes, `transistor` / Discrete → Transistors
      → MOSFET → Array for FS8205A, → BJT → NPN for S8050; the reason
      column verbatim). Add the 20 downloads that failed as one sentence
      naming `routing-example/datasheets-log.tsv`. — done; the section
      also points at the plan-40 row that already lists `MBT3904DW1T3G`.
- [x] `datasheets/skipped-parts.md` line 480: remove `BAT54C-G3-08` from
      the part list (plan-43-B adds it).
- [x] `planning/todo.md`, section Datasheets: add `- [ ] Reference-board
      parts: 248 parts in 20 families wait for a kind (planning/plan-43-A
      § 5, planning/plan-43-data/candidates.tsv).`

Acceptance: `grep -c 'Reference boards (plan-43)' datasheets/skipped-parts.md`
→ 1; `sed -n 480p datasheets/skipped-parts.md | grep -c 'BAT54C-G3-08'` → 0.
  Check: those two commands (est. 1 min). — `1`, `0`.
Commit: 8b0aa4b2

## Validation Plan

- Tests: none; A changes no code and no part file. `ctest` is E's.
- Doc sync: `datasheets/skipped-parts.md`, `planning/todo.md`.
- Final gate: plan-43-E.

## Decisions

- Each family without a kind gets its own kind and plan; no generic
  unsimulated kind — decided by the owner, 2026-11-15. It limits plan-43 to
  the 47 parts that fit a kind today (§ 5).
- The reference boards' regulators are not added here; they wait for the
  `regulator` kind — decided by the owner, 2026-11-15 (§ 5, first row).
- plan-43 runs before plan-42 — decided by the owner, 2026-11-15. plan-42's
  gate "no other plan is open" is met again when plan-43-E is archived.
- A plan is written before any part is added — decided by the owner,
  2026-11-15.
- A part whose only datasheet is DigiKey's copy is skipped — author, from
  `docs/parts-format.md` § Datasheets.
- The waiting PDFs live under `datasheets/reference-boards/` — author.

## Corrections

- **The worktree had none of the planning-time working files** (2026-11-15).
  `/follow-plan` works in `.claude/worktrees/P-43`, forked from `main`; the
  264 untracked PDFs, 71 untracked `datasheets.tsv` files, the three modified
  `datasheets.tsv` files and the git-ignored lookup log were in the main
  checkout. Before the gate was run, the untracked files were moved into the
  worktree, the three modified files copied and restored in the main
  checkout (`git checkout -- <the three paths>`), and
  `routing-example/datasheets-log.tsv` copied. The plan files were committed
  on `main` first (7410acd9), since the worktree is forked from it.
- **The backend was built in `/tmp/logic-p43w`** (Release), not
  `/tmp/logic-p43`, to keep it apart from the planning-time build. Every
  check of B–E that names `/tmp/logic-p43` uses `/tmp/logic-p43w`.
- **46 parts, not 47: `MBT3904DW1T3G` is skipped.** Its datasheet's ordering
  table lists `MBT3904DW1T1G`, `SMBT3904DW1T1G` and `NSVMBT3904DW1T3G` only
  (`pdftotext -layout datasheets/transistor/onsemi/MBT3904DW1T1-D.PDF - |
  grep -A14 ORDERING`), and `datasheets/skipped-parts.md` § Transistors
  (plan-40) already lists it as "not in the datasheet's ordering table".
  Planning missed that row. Effects: `census/skip.tsv` has 6 rows;
  `classify.py` prints `plan-43 46`, `skipped 6`; `targets.tsv` and
  `facts.tsv` have 46 rows; B adds 12 transistors (26 parts, 32854 → 32880);
  C ends at 32894 and D at 32900; the untracked PDF count after filing is
  one lower. Every "47" above reads 46, every "5 skipped" reads 6.
  The same check (the part number is in its datasheet's ordering
  information) is applied to every other part as B, C and D read its
  datasheet.

## Summary

A moves files and records decisions. The risk it carries is a stale census:
Phase 1 re-checks stock for all 47 before anything is added.
