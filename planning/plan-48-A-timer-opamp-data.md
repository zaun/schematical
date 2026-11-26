# plan-48-A: 555 timers and common op-amps — part-by-part stock check, datasheet downloads, ordering-table verdicts

Last updated: 2026-11-16
Overall Effort: huge (> 3d) — plan-48 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d of work; plus one DigiKey quota day)
Depends on: nothing (§ Prerequisites)

plan-48 adds **555-type timers** and **common op-amps** to the parts library
and the app (`planning/todo.md:70`: "Add the 555 timer and common op-amps
(LM358, TL07x…)"), from the top three makers of each type at DigiKey by
in-stock count (§ 2):

| Type | Makers (rank order) | In scope |
|---|---|---|
| timer | **Texas Instruments**, **Analog Devices** (Maxim), **Renesas** | DigiKey's "555 Type, Timer/Oscillator" single and dual parts: bipolar and CMOS, surface mount and through hole, AEC-Q100 included |
| op-amp | **Texas Instruments**, **Analog Devices** (with Maxim), **STMicroelectronics** | DigiKey amplifier types Standard (General Purpose), J-FET and CMOS; one, two or four amplifiers; surface mount and through hole; AEC-Q100 included; the 200 most-stocked per maker |

In play mode a 555 runs from the resistors and capacitors wired to it: the
backend recognizes the standard astable, 50 % astable, diode astable and
one-shot circuits, computes the times from the parts' values and toggles OUT
with them; RESET works; with no capacitor on its timing pins it is a
set/reset latch on logic levels; any other wiring keeps OUT low and the app
says why. An op-amp is placed, wired, checked by the voltage check and laid
out on a board; it drives nothing in play mode. The two new kinds are
`timer` and `opamp`.

The single behavioral outcome of plan-48: loading the part files gives the
baseline P0 parts (§ Prerequisites) plus two generics (`GENERIC-555`,
`GENERIC-OPAMP`) plus one part per `add` row of
`planning/plan-48-data/candidates.tsv` (662 candidates at planning time:
timer 62, op-amp 600); the app lists them under **Analog → Timers** and
**Analog → Op-amps → Single / Dual / Quad**; in play mode a `GENERIC-555`
with 1 kΩ, 10 kΩ and 100 nF in the astable circuit blinks an LED on OUT at
687 Hz; the same circuit with the 10 kΩ resistor wired to the supply
instead of DISCH keeps OUT low and shows "Not timing" with the reason; a
placed `GENERIC-OPAMP` does not turn Run off; `ctest` passes T0 + 2
programs (`timer_test`, `opamp_test`).

| Letter | Content | Effort |
|---|---|---|
| **A** | part-by-part stock check, datasheet text scan, ordering-table and status verdicts, datasheet placement, `specs.tsv` skeleton | large + 1 quota day |
| B | specs: pinouts and facts per (datasheet, package); the final pick, skipped parts, obsolete PDFs, the final `footprints-needed.txt` | large |
| C | footprints: leaded packages (DIP, SOIC, SOP, SSOP, TSSOP, MSOP / VSSOP, SOT-23, TSOT, SC-70, TO-263) | large |
| D | footprints: leadless and ball packages (DFN, SON, QFN, LFCSP, LGA, WLP / WLCSP / BGA) | large |
| E | backend: the `timer` kind, the 555 model, the RC timing analysis, `circuit_set_part_value`, `circuit_timer_state`, tests, docs | large |
| F | backend: the `opamp` kind (passive, analog pins), tests, docs | medium |
| G | parts: the two generics, timers, op-amps; datasheets, footprints, goldens, the editor's kind lists | large |
| H | app: details, facts, `kindName`, drawer, placed values to the backend, the timer's play state in the sidebar and on the block, the harness | large |
| I | landing tier | small |

Outcome of A: `planning/plan-48-data/stock-check.tsv` checks every picked
row in stock, Active or Last Time Buy, part by part (all three op-amp pieces
hit DigiKey's 300-result cap, § 2); `verdicts.tsv` gives every picked row
`add` or `skip`/`out` with a reason from its datasheet's text (ordering
table, status words); `datasheets/timer/<maker>/` and
`datasheets/opamp/<maker>/` hold every maker-site PDF an `add` row cites,
with one `datasheets.tsv` per folder; `specs.tsv` has one row per (type,
company, PDF, package, circuits) of the `add` rows, its pinout and facts
columns empty (B fills them).

References:

- `AGENTS.md` (§ Datasheets); `docs/parts-format.md` (§ Parts, § Datasheets,
  § Footprints); `docs/backend.md` (§ Adding a kind).
- `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md` (formats).
- `planning/plan-48-data/` — `README.md`; `census/dk.js`, `count.js`,
  `list.js`, `list.sh`, `common.py`, `urlcheck.py`, `select.py`,
  `to-check.py`, `stock-check.sh`, `footprint-lines.py`; `raw/` (the
  planning listing and the two count runs), `url-check.tsv`,
  `candidates.tsv`, `footprints-needed.txt` (written at planning time,
  2026-11-16).
- `planning/completed/plan-40-A-transistor-data.md` and
  `planning/plan-40-data/census/` (`textscan.py`, `place.py`,
  `specs-skeleton.py`, `verdicts.py`: the scripts A adapts);
  `planning/completed/plan-47-A-ifrb-data.md` (the same census method).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p48 && cmake --build /tmp/logic-p48 -j8 && ctest --test-dir /tmp/logic-p48 -j8` → `100% tests passed` | MET (2026-11-24 at 0b5788f5, worktree P-48: `100% tests passed, 0 tests failed out of 65`; T0 = 65, `grep -c '^logic_test' backend/CMakeLists.txt` → 65; was 37 at planning) |
| Golden sizes recorded | `wc -l backend/tests/golden/parts.txt backend/tests/golden/traces.txt backend/tests/golden/footprints.txt` | at planning (59b90a8f): **P0 = 32897**, **R0 = 32897**, **F0 = 1646**; measured 2026-11-24 at 0b5788f5: **P0 = 39903**, **R0 = 39903**, **F0 = 3825**; E, F and G re-measure them at their start and use their own |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p48-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-24 at 0b5788f5: `** BUILD SUCCEEDED **` with a fresh `-derivedDataPath /tmp/p48-dd2`; a reused `/tmp/p48-dd` failed with `Framework … XCTest.framework did not contain an Info.plist`, a stale derived-data tree, not a code defect) |
| Neither kind exists | `grep -c 'PART_TIMER\|PART_OPAMP' backend/include/parts.h` → 0; `ls schematical/schematical/Parts/{timer,opamp}.json` → no such file; `ls datasheets/timer datasheets/opamp` → no such directory | MET (2026-11-16 and again 2026-11-24: 0; no file; no directory. `datasheets/instrumentation-op-amps-buffer-amps/` and `datasheets/audio-amplifiers/` exist, empty and untracked; plan-48 does not use them) |
| No other open plan adds these parts or kinds | `grep -l -i 'PART_TIMER\|PART_OPAMP\|Parts/timer\.json\|Parts/opamp\.json\|datasheets/timer\|datasheets/opamp' planning/plan-*.md \| grep -v plan-48 \| wc -l` → 0 | MET (2026-11-16 and 2026-11-24: 0). plan-42 (regulators, `PART_REGULATOR`), plan-44 (crystals, `PART_CRYSTAL`), plan-45 (terminal blocks, the existing `connector` kind) and plan-47 (`PART_INDUCTOR`, `PART_FUSE`, `PART_RELAY`, `PART_BUZZER`) are open: the lines several plans touch (`PartKind`, `KINDS[]`, `TRACE_LAST_KIND`, the listing arrays, the editor's kind lists, `SimComponentType`, `circuit.h`) are added only if absent and after whatever is last; each such step greps first (E Phase 1, F Phase 1, G Phase 1, H Phase 1) |
| Shared footprint ids are added only once | before adding an id: `grep -c '"id": *"<id>"' schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json` | rule, applied per footprint in C and D (§ 3.1 of C): plan-42 names `TSOT-5-ADI-UJ` and `LFCSP-6-ADI-CP-6-3` (`grep -o 'TSOT-5-ADI-UJ\|LFCSP-6-ADI-CP-6-3' planning/plan-42-*.md \| sort -u` → 2), which C and D reuse when present |
| DigiKey credentials and quota | `grep -cE '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc` → 2; `eval "$(grep -E '^export DIGIKEY_CLIENT_(ID\|SECRET)=' ~/.zshrc)" && node scripts/digikey.js --in-stock --pages 1 --limit 1 --category 689 --manufacturer 20` prints `left today` ≥ 700 before Phase 1's run (about 1000 a day, shared with other sessions; it resets at about 00:00 UTC) | NOT MET (2026-11-24 05:24 UTC: credentials 2 of 2 for key 1 and key 2 (`~/.zshrc` lines 130–135); key 1 (`--opt2` absent) printed `20 left today`, key 2 (`--opt2`) `646 left today` (other sessions share both); the Safari route answers; the next reset is 2026-11-26 00:00 UTC. Phase 1 needs 662+ calls and the row asks for ≥ 700 on the key it uses). **Owner, 2026-11-24 (chat): "just use the API calls you have… 666 total"**: the gate is waived for the calls on hand (key 1 20, key 2 646); both keys read `0 left today` at 05:50 UTC after Phase 1 (other sessions share them) |
| The maker sites answer Python's `urllib` | `python3 -I planning/plan-48-data/census/urlcheck.py planning/plan-48-data/raw /tmp/p48/pdf \| awk -F'\t' 'NR>1{n[$4]++} END{print n[1], n[0]}'` → `424 3` | MET (2026-11-16 and 2026-11-24: `424 3`; `/tmp/p48/pdf` 646 MB) |
| `pdftotext`, `pdfinfo` | `which pdftotext pdfinfo` → two paths | MET (2026-11-24: `/Users/justinzaun/local/brew/bin/pdftotext`, `…/pdfinfo`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. If you stop, report the current status of *all*
> prerequisites.

## 1. Goal

- `planning/plan-48-data/stock-check.tsv` has one row per picked row (662 at
  planning time, `awk -F'\t' 'NR>1 && $22=="candidate"' planning/plan-48-data/candidates.tsv | wc -l`
  → 662), each `found` 1 with stock above 0 and status Active or Last Time
  Buy; a row that fails is `out` (reason `stock check`) in `candidates.tsv`
  and the next `reserve` row of its maker moves up and is checked too.
- `planning/plan-48-data/verdicts.tsv` has one row per picked row: `add`, or
  `skip` / `out` with one of the reasons of § 4; a `skip` or `out` moves the
  next reserve row up, which gets its stock check and verdict in turn, until
  each maker has 200 `add` op-amp rows or no reserve row is left.
- `datasheets/timer/{ti,adi,renesas}/` and `datasheets/opamp/{ti,adi,st}/`
  hold each maker-site PDF at least one `add` row cites (331 distinct PDFs
  for the 662 candidates at planning time: timer 14, op-amp 317); each
  folder's `datasheets.tsv` maps each file to its URL, size and SHA-256.
- `specs.tsv` exists with one row per (type, company, PDF, package,
  circuits) of the `add` rows, its `pinout` and fact columns empty.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No row is final in this letter: B reads the pinouts, and a row whose
  package has no pinout in its PDF is skipped there.
- No datasheet hunting beyond the DigiKey link and the rewrites of
  `common.resolve` (§ 3).
- No new listing: the planning listing is complete for the pick (§ 2).
  `scripts/digikey.js` is not changed.
- Military and hi-rel parts (DigiKey Grade Military, MIL-STD-883, ceramic
  DIP, metal can, flatpack, LCC), comparators, instrumentation, current
  sense, zero-drift, audio and every other amplifier type are out of scope.
- The legacy labels DigiKey keeps for Rochester Electronics stock (Harris
  Corporation, National Semiconductor) are not makers of this plan (§ 2).

## 2. Current State

The library has no timer or op-amp kind and no such part
(`grep -c '^\(NE555\|NA555\|LM555\|LMC555\|TLC555\|ICM7555\|LM358\|TL07[1-4]\)' backend/tests/golden/parts.txt`
→ 0). The `oscillator` kind (`backend/src/models/oscillator.c`)
is a fixed-frequency clock source; resistors are weak links whose ohms have
no effect (`resistor.c` header comment) and capacitors are passive
(`capacitor.c`). `footprints.json` (1646 footprints) already has the common
IC outlines (`PDIP-8`, `PDIP-14`, `SOIC-8`, `SOIC-14`, `TSSOP-8`,
`TSSOP-14`, `VSSOP-8`, `VSSOP-10`, `SOT23-5`, `SOT23-6`, `SC70-5`, `SC70-6`).

### Measured populations

All DigiKey numbers 2026-11-16, in stock (`MinimumQuantityAvailable` 1),
statuses Active (0) and Last Time Buy (4). `C` =
`node planning/plan-48-data/census/count.js` (one call each; credentials as
in § Prerequisites); `T555` = `--param 183=243487,243489` (Type "555 Type,
Timer/Oscillator (Dual)" and "(Single)"); `OPA` = `--param
161=863824,362007,331587 --param 69=409393,411897` (Amplifier Type Standard
(General Purpose), J-FET, CMOS; Mounting Type Surface Mount, Through Hole).
Planning used 27 calls (148 left before the first, 121 after the last).
`scripts/digikey.js` has no parametric filter and no sort by stock, so the
counts and the listing call the same API through `count.js` and `list.js`;
two of the numbers were cross-checked with `scripts/digikey.js` itself (the
last two rows of the first block).

| What | Count | Command |
|---|---|---|
| Programmable Timers and Oscillators (689), any type | **444** | `C 689 --makers 8` → `all` |
| …555 type, single / dual | **63 / 23** = 86 | same → `183 Type:` lines; `C 689 T555 --makers 12 --facets 183,69,1977` → `all 86` (`raw/count-timer.txt`) |
| …555 type by mounting / grade | surface mount 64, through hole 22; Automotive 3 | same → `69 Mounting Type:`, `1977 Grade:` |
| Instrumentation, Op Amps, Buffer Amps (687), any type | **12439** | `C 687 --makers 8` → `all` |
| …in scope (`OPA`) | **7981**: General Purpose 5911, CMOS 1386, J-FET 684; 1 / 2 / 3 / 4 / 32 circuits 2886 / 3130 / 13 / 1935 / 1; surface mount 6837, through hole 1144; Automotive 937, Military 7 | `C 687 OPA --makers 12 --facets 161,69,1977,2094` (`raw/count-opamp.txt`) |
| Cross-check with `scripts/digikey.js`: STMicroelectronics in 687 | 798 (equal to `C 687`'s `maker: 497 STMicroelectronics 798`) | `node scripts/digikey.js --category 687 --manufacturer 497 --in-stock --status 0 --status 4 --pages 1 --list` → `50 of 798 parts` |
| Cross-check: Renesas in 689 | 11 parts, 3 of them 555 type (`ICM7555CBAZ`, `ICM7555CBAZ-T`, `ICM7555IPAZ`) | `node scripts/digikey.js --category 689 --manufacturer 20 --in-stock --status 0 --status 4 --list` → `11 of 11 parts` |
| **Makers ranked, 555 type** (`C 689 T555`) | **Texas Instruments 52**, **Analog Devices/Maxim 8**, Harris Corporation 4, then five makers with 3: Advanced Linear Devices, Diodes Incorporated, onsemi, **Renesas**, UMW; Microchip 2, TAEJIN 2, EVVO 1, National Semiconductor 1 | `raw/count-timer.txt` → `maker:` lines |
| …the third maker | **Renesas**: Harris Corporation and National Semiconductor are legacy labels (below), and of the five makers with 3 parts Renesas has the most stock: 132032 against onsemi 87933, Diodes Incorporated 50621, Advanced Linear Devices 247, UMW 0 (no stock figure) | `gzip -dc raw/timer-all.jsonl.gz \| python3 -I -c "import sys,json,collections;c=collections.Counter();[c.update({d['maker']:d.get('stock') or 0}) for d in map(json.loads,sys.stdin)];print(c.most_common())"` |
| **Makers ranked, op-amps** (`C 687 OPA`) | **Texas Instruments 3423**, **Analog Devices 1697** (ids 505: 1397, 175 Maxim: 300), **STMicroelectronics 657**; Microchip 551, onsemi 252, Harris Corporation 243, National Semiconductor 194, Rohm 182, 3PEAK 132, Renesas 119, UMW 114 | `raw/count-opamp.txt` → `maker:` lines |
| Legacy labels | every one of the 55 listed Harris Corporation and National Semiconductor rows (5 timers, 50 op-amps) links a `rocelec.widen.net` (Rochester Electronics) datasheet: no vendor datasheet, not counted as makers | `gzip -dc raw/opamp-legacy.jsonl.gz raw/timer-all.jsonl.gz \| grep -c 'Harris Corporation\|National Semiconductor'` → 55; `… \| grep 'Harris Corporation\|National Semiconductor' \| grep -vc rocelec.widen.net` → 0 |
| Planning listing (`census/list.sh raw`, 5 pieces) | **1033 rows**, 1032 distinct part numbers: `timer-all` 86 of 86 (83 distinct); `opamp-ti` 300 of 3422; `opamp-adi` 300 of 1697; `opamp-st` 300 of 657; `opamp-legacy` 50 of 437 (one page, `LIST_CAP=50`); **21 calls** | `cat raw/*.err`; `python3 -I census/select.py raw /tmp/p48-c.tsv --urls url-check.tsv --report` → `1032 parts listed` |
| **Pieces cut at the 300-result cap** | **3 of 4 in-scope pieces**: `opamp-ti` (300 of 3422, the 300th has stock 26250), `opamp-adi` (300 of 1697, 3640), `opamp-st` (300 of 657, 3465). `timer-all` is complete | `grep -h 'cut at 300' raw/*.err` → 3 lines |
| Complete for the pick | each cut piece is sorted by stock, so it holds its maker's 300 most-stocked parts; the 200th picked row has stock 37914 (TI), 5164 (ADI), 5175 (ST), above the 300th's; an unlisted part has less stock than every listed one | `awk -F'\t' 'NR>1 && $22=="candidate" && $24==200{print $2,$7}' candidates.tsv` |
| Outcome of `select.py` | **662 picked** (timer 62: TI 51, ADI 8, Renesas 3; op-amp 600: 200 each), **294 reserve** (TI 96, ADI 98, ST 100); out 76: legacy label 55, maker 16 (timers of other makers), no datasheet 5 | `python3 -I census/select.py raw candidates.tsv --urls url-check.tsv --pdf /tmp/p48/pdf --report` |
| …picked by class | timer: single 42 (TI 34, ADI 5, Renesas 3), dual 20 (TI 17, ADI 3). op-amp: general purpose 428 (1 / 2 / 4 circuits: 210 / 145 / 73), J-FET 88 (29 / 31 / 28), CMOS 84 (2 / 62 / 20) | `awk -F'\t' 'NR>1 && $22=="candidate"{n[$3" "$4" "$5]++} END{for(k in n)print k,n[k]}' candidates.tsv` |
| …picked by status, grade, mounting | Active 657, Last Time Buy 5 (`AD8531ARTZ-REEL7`, `AD8592ARMZ-REEL`, `AD8531AKSZ-REEL7`, `MAX4490AUK+T`, `AD8534ARUZ`); AEC-Q100 121 (timer 3, op-amp 118); surface mount 600, through hole 62 | `awk -F'\t' 'NR>1 && $22=="candidate"{print $6}' candidates.tsv \| sort \| uniq -c`; the same with `$11`, `$10` |
| …military or hi-rel rows listed | 0 (the scope rule of `common.classify` found none in the 1032) | `awk -F'\t' '$23=="military" \|\| $23=="hi-rel package"' candidates.tsv \| wc -l` → 0 |
| Datasheet URLs: `urlcheck.py` | **427** distinct maker-site URLs of the in-scope rows, **424** answer a PDF; 3 do not (HTTP 404): `ti.com/lit/gpn/opa234` (2 rows), `ti.com/lit/gpn/tl322` (1), `analog.com/…/ADA4622-1-4622-2-4622-4.pdf` (1); a fifth row (`MAX74825ARTZ-R7`) has no link at all | `awk -F'\t' 'NR>1{n[$4]++} END{print n[1], n[0]}' url-check.tsv` → `424 3`; `awk -F'\t' '$23=="no datasheet"{print $1}' candidates.tsv` |
| …picked rows whose URL is not DigiKey's link | 41 `guess` (TI 33: DigiKey links a Rochester copy; ST 8: DigiKey links its own copy) | `awk -F'\t' 'NR>1 && $22=="candidate" && $19=="guess"' candidates.tsv \| wc -l` |
| **The download set** (picked + reserve) | **956 rows**, **422 distinct URLs**, **418 distinct PDFs**; 894 rows from a cut piece | `select.py … --report` → `download set:` line |
| …the picked rows' PDFs | **331** distinct: timer 14, op-amp 317 (TI 128, ADI 105, ST 84) | `awk -F'\t' 'NR>1 && $22=="candidate"{print $21}' candidates.tsv \| sort -u \| wc -l` |
| **Part number printed in its PDF** (`hit`) | picked: **571 of 662**: op-amp TI 188 / 200, ADI 154 / 200, ST 177 / 200; timer TI 49 / 51, Renesas 3 / 3, ADI 0 / 8 (the Maxim table prints `ICM7555ISA+` with a space; § 4 rule 2). Reserve: 250 of 294 | `select.py … --pdf /tmp/p48/pdf --report` → `literal hits` lines |
| Package lines, download set | **96**: leaded (C) 61 lines (57 with a picked row; 619 picked parts), leadless and ball (D) 35 lines (26; 43). No existing footprint of the same family and pin count: 7 lines (C 3: `SC-70-8`, `TSOT-23-5`, `TO-263 (DDPAK-7)`; D 4: `14-WLCSP`, `6-UTLGA`, two 9-ball WLPs) | `awk -F'\t' '!/^#/{n[$1]++} END{for(k in n)print k,n[k]}' footprints-needed.txt`; `awk -F'\t' '!/^#/ && $12==""' footprints-needed.txt \| wc -l` → 7 |
| Size of the downloaded PDFs | 646 MB for the 424 URLs that answered (the tree's `datasheets/` is 6.5 GB) | `du -sh /tmp/p48/pdf` |

### Verified properties

- **Sorting by stock works with parametric filters.** `list.js` sends
  `SortOptions {Field: "QuantityAvailable", SortOrder: "Descending"}` with
  `ParameterFilterRequest`; `raw/opamp-ti.err` shows `piece opamp-ti: 300 of
  3422 (cut at 300: 26250)` and the parts arrive in falling stock.
- **ti.com, analog.com, st.com and renesas.com answer Python's `urllib`**
  with a Safari user agent (424 PDFs, § 2). `https://www.ti.com/lit/gpn/<name>`
  redirects to `https://www.ti.com/lit/ds/symlink/<name>.pdf` (`curl -sIL
  https://www.ti.com/lit/gpn/opa363` → `302`, `location:
  …/lit/ds/symlink/opa363.pdf`, `200 application/pdf`); `url-check.tsv`'s
  `final` column records the symlink URL, which is the form
  `docs/parts-format.md` § Datasheets requires.
- **Every orderable packing is its own part**, as for the logic families
  (`grep -o '"SN74HC00[A-Z0-9]*"' schematical/schematical/Parts/gate.json |
  sort -u | wc -l` → 15: `SN74HC00D`, `SN74HC00DR`, `SN74HC00DRG4`, …). The
  listing is not de-duplicated by packing.
- **Two listed timer rows have no stock figure** (`SA555DR`, UMW;
  `NE555DR-EV`, EVVO); both are out as other makers' parts.
- **TI rows under DigiKey's TI label can be Rochester stock**: 38 of the 300
  `opamp-ti` rows link `rocelec.widen.net`; 33 of them are picked through
  the guessed `ti.com/lit/gpn/<base>` URL. A row whose number TI does not
  print (the `-TI` suffixed `LMV931MF-TI`, `LM2904MX`) fails § 4 rule 2.

## 3. Method

`census/common.py` holds the rules; they are not re-decided here.

- **Scope** (`classify`): maker of the type's three; a stock figure; status
  Active or Last Time Buy; not Grade Military / MIL-STD-883; package not
  ceramic, metal can, flatpack or LCC; op-amps of the three amplifier types
  with 1, 2 or 4 circuits.
- **Datasheet URL** (`resolve`): TI — the `gotoUrl` of DigiKey's
  `suppproductinfo` link (`https://www.ti.com/lit/gpn/<name>`) or its direct
  ti.com PDF link; for a Rochester, DigiKey-hosted or missing link, the
  guess `https://www.ti.com/lit/gpn/<letters+digits of the part number>`
  (tried again with `-n`). ADI — the analog.com link; a
  `datasheets.maximintegrated.com` link becomes
  `https://www.analog.com/media/en/technical-documentation/data-sheets/<file>`.
  ST — the st.com link; for a DigiKey-hosted link, the guess
  `https://www.st.com/resource/en/datasheet/<base>.pdf`. Renesas — the
  renesas.com link. A row with no URL that answers a PDF is `no datasheet`.
- **Pick**: timers, every in-scope row; op-amps, the 200 most-stocked
  in-scope rows per maker; the rest are `reserve`, in stock order.

## 4. Verdict rules (`census/verdicts.py`, Phase 3)

Per picked row, in this order; the first that applies decides:

1. Stock check failed (Phase 1) → `out`, `stock check`.
2. **Ordering table.** `add` needs the part number in the PDF's text
   (`pdftotext -layout`, spaces removed, upper case): verbatim (`hit` 1), or
   by its maker's printed packing rule: ADI/Maxim — the number without `+`
   and without a trailing `T`, `-T`, `-REEL`, `-REEL7`, `-RL`, `-RL7`, `-R7`
   or `-R2` stands in the text and the text has `TAPEANDREEL` or
   `DENOTESALEAD`; ADI (Linear Technology) — `#TRPBF` / `#TRMPBF` read as
   `#PBF`; TI — a `/NOPB` number also matches without the slash. Otherwise
   `skip`, `not in the datasheet's ordering table`.
3. **Status words.** `OBSOLETE`, `DISCONTINUED`, `NRND`, `NOT RECOMMENDED
   FOR NEW DESIGNS`, `END OF LIFE` or `LIFEBUY` within 300 characters after
   the part number (a TI orderable-addendum row, an ADI or ST ordering row)
   → `out`, `datasheet status`. DigiKey's Last Time Buy status alone keeps
   a row in.
4. Otherwise `add`.

The five `no datasheet` rows of § 2 and every `skip` row go to
`datasheets/skipped-parts.md` (B writes it, from `verdicts.tsv`). A PDF
whose listed rows are all `out` by rule 3 is not placed and goes to
`datasheets/obsolete-not-included.md` (B).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Part-by-part stock check

- [x] `cd planning/plan-48-data && DK_OPT2=1 VERDICTS=verdicts.tsv census/stock-check.sh raw candidates.tsv
      stock-check.tsv candidate` (one `node scripts/digikey.js` call per
      picked row; Corrections 1: only the rows whose datasheet lists them).
      Result: 651 of 656 picked rows checked on 2026-11-24 (key 2 for 644, key 1 for 20); both
      keys ran dry. The last 11 rows (5 TI timers `TLC3555IDR`, `TLC555CPWR`, `TLC555CPSR`, `NE555PS`,
      `LM555CMM/NOPB` and the op-amps `AD823AARZ-RL`, `MAX4493AUK+T`, `TL031IP`, `TLV2322IDR`, `LMV341IDCKR`,
      `MC33078DGKR`) were looked up on 2026-11-26 after the reset
      (`/tmp/p48/sc.sh 2 1 add`, state `add` because `final.py --apply` had already run):
      `digikey: 1 of 1 parts … 989 left today`, 11 rows appended to `stock-check.tsv`, all `found 1`, Active, stock 696 to 35073.
      `to-check.py raw candidates.tsv stock-check.tsv add verdicts.tsv` → `656 add rows, 0 unchecked`.
- [x] `python3 -I census/select.py raw candidates.tsv --urls url-check.tsv
      --pdf /tmp/p48/pdf --stock stock-check.tsv --verdicts verdicts.tsv --report` (the refill loop, Phase 3):
      4 rows failed the lookup (`LTC6241HVIS8#TRPBF`, `TLV2450ID`, `TLV9102IDR`
      op-amps, replaced from reserve; `LMC555CMM/NOPB` timer, dropped); the final 11 lookups all passed,
      so the pick did not change: rerun of the loop (`/tmp/p48/final.sh` then `final.py --apply`) →
      656 `add` rows (opamp ti 200, adi 200, st 200; timer ti 48, adi 5, renesas 3), `specs.tsv` `503 rows; 503 kept from before`,
      and only the `stock_checked` column of those 11 rows changed in `candidates.tsv`.
- [x] Record in § Corrections: the rows that failed and the calls used (Corrections 4).

Acceptance: every picked row passed its own lookup (observed 2026-11-26: `awk … $22=="add" && $27==""` → 0).
  Check: `awk -F'\t' 'NR>1 && $22=="candidate" && $27==""' planning/plan-48-data/candidates.tsv | wc -l` → 0, and `awk -F'\t' 'NR>1 && $2!="1"' planning/plan-48-data/stock-check.tsv | cut -f1 | while read m; do awk -F'\t' -v m="$m" '$1==m{print $22}' planning/plan-48-data/candidates.tsv; done | sort -u` → `out` or nothing (est. 45 min of calls at about 23 a minute).
Commit: —

### Phase 2 — The PDFs and the text scan

- [x] `python3 -I census/urlcheck.py raw /tmp/p48/pdf > /tmp/p48/urlcheck.tsv` on a
      cold cache (`/tmp/p48/pdf` was empty; 646 MB downloaded): `awk -F'\t' 'NR>1{n[$4]++} END{print n[1], n[0]}'` → `424 3`;
      its columns 1–4 equal `url-check.tsv`'s (`diff` empty), so that file stays.
      The count of `ok` 1 stays 424; a URL
      that stopped answering makes its rows `no datasheet` on the next
      `select.py` run.
- [x] `census/textscan.py` (from `planning/plan-40-data/census/textscan.py`,
      with § 4's packing rules and status words): for every picked row
      `hit` (verbatim), `scheme` (matched by a packing rule) and `dead`;
      writes `textscan.tsv` (`mpn`, `hit`, `scheme`, `dead`, `context`: the
      80 characters around the match). Written (`planning/plan-48-data/census/textscan.py`;
      `python3 -I census/textscan.py candidates.tsv /tmp/p48/pdf textscan.tsv` → `656 rows scanned`);
      rule 3's window and rule 2's `dash` and `slash` forms are Corrections 2 and 3.
- [x] Read `context` for every `scheme` row and every `dead` row (the rule
      must have matched an ordering-table line, not prose); a wrong match is
      listed in `verdict-overrides.tsv` (`mpn`, `verdict`, `reason`, `why`).
      Read: the 25 `scheme` rows of the first scan are all ordering-table lines
      (`pbf` 13: `LT…#PBF`/`#TRPBF` lines; `tr` 12: `MAX….+` lines); the 4 `dead`
      rows were all false (an `OBSOLETE PACKAGE` heading of the next group, two
      revision-history lines) and are `add` overrides; 5 ADI timers are overrides (Corrections 3).

Acceptance: every picked row has a scan result.
  Check: `awk -F'\t' 'NR>1' planning/plan-48-data/textscan.tsv | wc -l` → the picked count (`awk -F'\t' 'NR>1 && $22=="candidate"' planning/plan-48-data/candidates.tsv | wc -l`) (est. 15 min: 418 `pdftotext` runs are cached in `/tmp/p48/pdf`).
  Result: 656 lines of data; picked 656. MET.
Commit: —

### Phase 3 — Verdicts, the refill, placement, the specs skeleton

- [x] `census/verdicts.py` (from plan-40's): `verdicts.tsv` (`mpn`,
      `verdict`, `reason`, `pdf`) by § 4, with `verdict-overrides.tsv`
      applied last. Written; `python3 -I census/verdicts.py candidates.tsv textscan.tsv verdicts.tsv` → `200 opamp adi add`, `200 opamp st add`, `200 opamp ti add`, `5 timer adi add`, `3 timer renesas add`, `48 timer ti add`.
- [x] `select.py … --stock stock-check.tsv --verdicts verdicts.tsv --report`
      moves reserve rows up; run Phase 1's check and Phase 2's scan for
      them and `verdicts.py` again, until every picked row is `add` (a maker
      whose reserve runs out stays below 200). `/tmp/p48/refill.sh` (select → textscan →
      verdicts) ran 4 passes until `non-add groups 0`: 48 rows are `skip` (not in the
      datasheet's ordering table), 0 `out` by status; no maker's reserve ran out (reserve left: TI 82, ADI 76, ST 90).
- [x] `census/place.py` (from plan-40's): copy each distinct PDF (by SHA-256)
      an `add` row cites from `/tmp/p48/pdf` to
      `datasheets/<timer|opamp>/<ti|adi|st|renesas>/<name>.pdf` (`<name>` =
      the last path segment of `final`, lower case; the Renesas document
      `icm7555-icm7556-datasheet` gets `.pdf` appended), write each
      folder's `datasheets.tsv` (`file`, `url`, `bytes`, `sha256`) and set
      `pdf` in `candidates.tsv` to the path. `place.py candidates.tsv /tmp/p48/pdf` → `293 PDFs placed for 656 rows`
      (timer: adi 1, renesas 1, ti 11; the rest op-amp; `du -sh` timer 18M, op-amp 389M).
- [x] `census/specs-skeleton.py` (from plan-40's): `specs.tsv`, one row per
      (type, company, pdf, package, supplier_package, circuits) of the `add`
      rows: `rows`, `example` (the most-stocked part), and empty `pinout`,
      `scheme`, `facts` columns. → `502 rows`.
- [x] Update `planning/plan-48-data/README.md` (the new files and columns).

Acceptance: every picked row is `add`, its PDF is in the tree, and each has a
spec row.
  Check: `python3 -I planning/plan-48-data/census/place.py --verify planning/plan-48-data/candidates.tsv` → `0 missing, 0 unused`; `awk -F'\t' 'NR>1 && $2=="add"' planning/plan-48-data/verdicts.tsv | wc -l` equals `awk -F'\t' 'NR>1 && $22=="candidate"' planning/plan-48-data/candidates.tsv | wc -l`; `awk -F'\t' 'NR>1{s+=$7} END{print s}' planning/plan-48-data/specs.tsv` (the `rows` column) equals the same count (est. 10 min).
  Result: `0 missing, 0 unused`; add 656, picked 656; `rows` sum 656. MET for the 656 rows; the 5 unchecked timers can only remove rows (then `select.py` → `textscan.py` → `verdicts.py` → `place.py` → `specs-skeleton.py` again).
Commit: —

## Validation Plan

- Tests: none (no code of the product changes); the checks are the three
  acceptance commands.
- Coverage check: `git status --short backend schematical` → empty after A.
- Runtime proof: `python3 -I planning/plan-48-data/census/select.py … --report`
  prints the final per-maker counts; they are recorded in § Corrections.
- Doc sync: `planning/plan-48-data/README.md`.
- Final gate: plan-48-I.

## Decisions

- Owner, 2026-11-16: **op-amps are DigiKey's Standard (General Purpose),
  J-FET and CMOS types, singles, duals and quads, the 200 most-stocked per
  maker** (at most 600).
- Owner, 2026-11-16: **a 555 takes its timing from the resistors and
  capacitors wired to it**; other wiring keeps OUT low and the app says why
  (plan-48-E, H).
- Owner, 2026-11-16: **an op-amp is facts only** in play mode (plan-48-F).
- Owner, 2026-11-16: **556 duals and CMOS 555s are in; automotive (-Q1)
  grades are in; military / hi-rel packages and comparators are out.**
- Owner, 2026-11-16: only the top three makers; inclusion needs stock above
  0, the datasheet's ordering table, and status Active or Last Time Buy.
- Author: **the makers are ranked per type, by companies** (plan-47's rule):
  Analog Devices and its Maxim id are one company.
- Author: **Harris Corporation and National Semiconductor are not makers**:
  the labels are Rochester Electronics stock with Rochester datasheets
  only (55 of 55 rows). They are out as `legacy label`, not listed in
  `skipped-parts.md` (no part of the three makers is behind them).
- Author: **the third timer maker is Renesas**: five makers tie at 3 parts;
  the tie is broken by summed stock (132032).
- Author: **timers have no per-maker cap** (62 rows in all).
- Author: **the stock check covers every picked row**, not only rows from a
  cut piece: 894 of the 956 download-set rows are from one anyway.
- Author: **`datasheets/timer/` and `datasheets/opamp/`** (named by kind,
  with one folder per maker, as plan-44 and plan-47 do); the empty
  DigiKey-named folders stay untouched.
- Author: **DigiKey's Last Time Buy keeps a row; a datasheet's NRND or
  obsolete mark removes it** (§ 4 rule 3).

## Corrections

1. **Verdicts before stock checks (measured 2026-11-24).** The calls on hand
   were 666 (key 1 20, key 2 646) against 662 picked rows plus the refill, and
   66 picked rows were `skip` by the text rules anyway. The stock check
   therefore runs on the rows whose verdict is `add` (`to-check.py … <verdicts.tsv>`,
   `stock-check.sh` with `VERDICTS=`), after the select → textscan → verdicts
   loop has settled the pick; the result is the same set of checked rows with
   no call on a row the datasheet does not list. Before this was found the
   first (full-order) run had spent about 110 calls on the most-stocked rows,
   some of them rows that are now `skip`.
2. **Rule 3 (status words) is read at the part, not within 300 characters,
   for TI.** A TI orderable-addendum row reads `<part>[.A]<STATUS>PRODUCTION`;
   the 300-character window matched the next row's `OBSOLETE` (47 `dead` rows
   of 662, nearly all false). `textscan.py` takes a TI status only right after the
   part (`<part>[.A]<STATUS>`); ADI and ST keep the 300 characters. 4 `dead`
   rows remained, all false (`verdict-overrides.tsv`).
3. **Rule 2 had two gaps (measured: 76 of 662 first-scan rows `skip`).**
   (a) ADI / Maxim `+T`: the datasheets print `MAX4040EUK-T` (same part,
   tape and reel); `textscan.py` scheme `dash` accepts a printed `<base>-T`
   for `<base>+T` (8 picked rows). (b) ST prints order codes as
   `TL061CD/CDT` (the later code repeats the tail of the first): scheme
   `slash` expands them (13 picked rows). The plan's claim that the Maxim
   timer table "prints `ICM7555ISA+` with a space" is **false**: it prints
   `ICM7555ISA` and a note that a `+`, `#` or `-` in the package code is RoHS
   status. The 5 `+` rows (`ICM7555ESA+`, `ICM7555ISA+`, `ICM7555IPA+`,
   `ICM7556ISD+`, `ICM7556IPD+`) are `add` by override (the ordering table
   lists the base number); the 3 `+T` rows (`ICM7555ESA+T`, `ICM7555ISA+T`,
   `ICM7556ISD+T`) stay `skip`: the document never says what `T` means.
   The 48 rows left as `skip` by the rules (TI `-TI` and `TLC27L…` rows, LT
   parts whose PDF prints a family name, ADI `MAX` rows with no `-T` form, …)
   go to `datasheets/skipped-parts.md` in B.
4. **Stock check outcome.** `stock-check.tsv` has 664 rows (2026-11-24); 4 of
   the picked rows failed the lookup: `LTC6241HVIS8#TRPBF`, `TLV2450ID`,
   `TLV9102IDR` (op-amps, the next reserve rows moved up and passed) and
   `LMC555CMM/NOPB` (timer, no reserve). Calls: planning 27, this run about
   675 (key 2 646 → 0, key 1 20 → 0); both keys were at 0 at 05:50 UTC, so
   5 rows are unchecked (Phase 1). `stock-check.sh` had a bug that stopped the
   run on the text `429 left today` (it grepped for `429`); fixed to
   `rate limited`.
5. **Final pick (2026-11-24, before the 5 timers are checked).** 656 rows:
   op-amp 600 (TI 200, ADI 200, ST 200), timer 56 (TI 48, ADI 5, Renesas 3);
   293 distinct PDFs placed, 502 spec rows. The plan's 662 → 656: 8 ADI timer
   rows → 5 add, 2 TI timers (`TLC555IP-P2`, `TLC3555QDDFRQ1`) are `skip`,
   `LMC555CMM/NOPB` is out by stock.
6. **Prerequisite values moved** (main advanced since planning): T0 = 65
   (was 37), P0 = R0 = 39903, F0 = 3825.
7. **The last 11 stock lookups passed (2026-11-26).** After the quota reset
   `census/stock-check.sh` (state `add`; the state was no longer `candidate`
   after `final.py --apply`) looked up the 5 TI timers and 6 op-amps with
   key 2 (1 call each, 989 left afterwards): all Active, stock 696 to 35073.
   The pick of Correction 5 stands (656 rows, 292 PDFs, 503 spec rows).

## Summary

The risk is in § 4 rule 2: 91 of the 662 picked rows do not print verbatim,
and the packing rules decide them, so every `scheme` match is read in
context. Nothing of the product changes in A.
