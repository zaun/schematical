# plan-20-A: Buffers, drivers and bus transceivers — datasheets and part data

Last updated: 2026-11-07
Overall Effort: huge (> 3d) — plan-20 is A → B → C → D → E → F → G → H → I
Effort: large (3h–1d)
Depends on: nothing (§ Prerequisites)

plan-20 adds every Texas Instruments buffer, line driver and bus transceiver
DigiKey has in stock and the datasheet covers: plain buffers ('07, '17, '34,
4050 and the 1G/2G/3G parts), 3-state and open-drain buffers ('125, '126,
'240, '241, '244, '540, '541, '365–'368, Widebus 16/32-bit), bidirectional
transceivers ('245, '640, '645, '623, 16245 …), registered and latched
transceivers ('646, '652, '543, '2952, 16952 …) and dual-supply
level-translating transceivers (1T45, 2T45, 4T245, 8T245, 16T245, 4245,
164245, TXU/TXV fixed-direction translators, CD4504, CD4010). Plain buffers
join the `gate` kind as a new `buf` op; everything else is a new kind
`buffer`. Dual-supply parts get a second supply domain in the part format and
the voltage check.

The single behavioral outcome of plan-20: loading the part files gives the
baseline count + N20 parts (`add` rows of `planning/plan-20-data/parts.tsv`),
every new device runs its datasheet behavior in `gate` tests or
`buffer_test`, the voltage check judges each pin of a dual-supply part
against its own side's supply, and `ctest` passes T0 + 1 programs.

| Letter | Content | Effort |
|---|---|---|
| **A** | datasheets; `plan-20-data/parts.tsv`, `specs.md`; skipped parts | large |
| B | families (`FCT`, `ALVT`, `ALB`, `AXC`, `LXC`); the 45 new footprints | large |
| C | `gate` op `buf`; the plain-buffer parts into `gate.json` | medium |
| D | `buffer` kind; models `tristate`, `transceiver`; `buffer_test` | large |
| E | model `registered` (latched and registered transceivers) | large |
| F | dual supply: format, `Part` field, voltage check, listing; model `translator` | large |
| G | parts: 3-state buffers and transceivers into `buffer.json` | large |
| H | parts: registered transceivers and translators | medium |
| I | app: `buffer` kind, dual-supply rows in the sidebar; plan-20 landing tier | medium |

Outcome of A: `datasheets/buffer/`, `datasheets/transceiver/`,
`datasheets/translator/` hold every TI PDF the `add` rows cite;
`parts.tsv` has one row per candidate (3857) with model, device, family,
footprint and `add`/`skip`; `specs.md` has one section per device.

References:

- `AGENTS.md`; `docs/parts-format.md` (§ `gate`, § Values, § Supply voltage,
  § Footprints, § Sequential kinds, § Model `bus-mux`); `docs/backend.md`
  (voltages, adding a kind, tests).
- `planning/plan-20-data/candidates.tsv` (census), `census/` (scripts),
  `footprints-needed.txt` (packages with no footprint).
- `planning/completed/plan-10-A-groundwork.md` §3.4–§3.5,
  `plan-07-B-voltage-check.md` (the voltage check), `plan-10-J-app.md`;
  inverter precedent fd9a9fa (`not` op), 7022ae9, 0655d09.

## Prerequisites

This table is the gate for all of plan-20; B–I point here and add their own rows.

| Must be true | Command | Status |
|---|---|---|
| Backend tests green | `cmake -S backend -B /tmp/logic-p20 && cmake --build /tmp/logic-p20 -j8 && ctest --test-dir /tmp/logic-p20` → `100% tests passed` | MET; **T0 = 21** (re-run 2026-11-07 at 965a3d7: `100% tests passed, 0 tests failed out of 21`) |
| Golden sizes recorded | `wc -l < backend/tests/golden/parts.txt`; `footprints.txt` | MET: **P0 = 8022**, **F0 = 237** (re-measured 2026-11-07 at 965a3d7) |
| The app builds | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p20-dd CODE_SIGNING_ALLOWED=NO build` → `** BUILD SUCCEEDED **` | MET (2026-11-07 at 965a3d7: `** BUILD SUCCEEDED **`) |
| The `not` op exists (inverters landed) | `grep -c '"not"' backend/src/models/gate.c` → ≥ 1 | MET (re-run 2026-11-07: `1`) |
| No `buffer` kind yet | `ls schematical/schematical/Parts/buffer.json backend/src/models/buffer*.c 2>/dev/null` → no output | MET (re-run 2026-11-07: no output) |
| DigiKey script works | `node scripts/digikey.js --in-stock --manufacturer 296 --limit 1 SN74HC245N` → a part line | MET (re-run 2026-11-07: `SN74HC245NS  Obsolete  934  74HC …`, 277 calls left) |

plan-20 waits for no other plan (owner: 19–22 in any order, possibly in parallel).

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue and before you stop; if you stop,
> report all rows.

## 1. Goal

- `tail -n +2 planning/plan-20-data/parts.tsv | wc -l` → 4157 (corrected from 3857, § Corrections); each row has a
  `model` (`buf`, `tristate`, `transceiver`, `registered`, `translator`) or
  `skip` with a reason.
- `ls datasheets/{buffer,transceiver,translator}/*.pdf | wc -l` = distinct
  `pdf` values of `add` rows.
- No code change in A.

### Non-goals

- No existing part changes; no part already in `Parts/*.json` is re-added
  (inverters `74_04`, `4049UB`, … are in `gate.json`).
- Out of scope (owner, 2026-11-07), `skip` with reason "out of scope": BTL,
  GTL, GTLP, SSTL/SSTV/SSTU/SSQE, FB backplane parts; scan-test devices
  (8xxx, 18xxx, 182xxx); bus-termination and diode arrays; IEEE-1284
  translators; voltage clamps (TVC); CBT/CB3T FET bus switches;
  auto-direction translators (TXB, TXS, LSF, PCA930x, TCA, TXG); peripheral
  and line drivers (SN754xx, SN65, SN95, SN725, CYBUS).

## 2. Current State

### Measured populations

| What | Count | Command |
|---|---|---|
| Candidate parts | 3857 | `tail -n +2 planning/plan-20-data/candidates.tsv \| wc -l` |
| … from category 704 | 3668 | census `scope.py` (704 rows minus BTL/GTL/SSTL/scan/1284/termination/TVC/CBT families) |
| … from category 732 (translators) | 189 | census (1T45 42, 2T45 27, 4245 15, 164245 14 + 164245G 6, 3245 9, 4T774 9, 4504 9, TXU 24, TXV 2, 8T540 5, 8T541 5, 4010 5, 4T245 4, 324245 4, CALVC/CAVC/CLV8T 9) |
| Functions | 418 | `tail -n +2 … \| cut -f1 \| sort -u \| wc -l` |
| … DigiKey obsolete | 855 | `awk -F'\t' '$6=="Obsolete"' … \| wc -l` |
| … addable by status (Active or Last Time Buy) | 2925 (932 excluded: Obsolete, Discontinued at DigiKey, Not For New Designs) | `awk -F'\t' 'NR>1 && ($6=="Active"||$6=="Last Time Buy")' planning/plan-20-data/candidates.tsv \| wc -l` |
| Model by DigiKey type/output/description (A confirms from each PDF) | tristate 1838, transceiver 815, plain (`buf`) 541, registered 311, translator 292, unclassified 60 | classifier in `census/` (type "Buffer"+3-state → tristate; "Transceiver" → transceiver; register/latch/flip-flop words → registered; translation or dual supply → translator) |
| Peripheral/line drivers among them (out of scope) | 47 | `grep -cE $'^[^\t]*\t[^\t]*\t(SN75\|SN95\|SN725\|CYBUS\|SN65)' …` |
| Largest functions | 244: 275, 245: 256, 240: 196, 125: 168, 541: 154, 126: 118, 540: 99, 16244: 97, 16245: 91, 241: 75, 646: 62, 652: 55, 162244: 47, 2245: 46, 367: 44, 07: 43, 1T45: 42 | `cut -f1 … \| sort \| uniq -c \| sort -rn` |
| Families | LVC 412, ABT 294, FCT 289, HC 215, LV 175, ACT 159, LVTH 154, ALS 145, AHC 141, AHCT 132, AC 117, LS 109, HCT 108, AUP 108, LVCH 97, LVT 89, BCT 85, ALVCH 75, AUC 71, AVC 62, AS 53, CD4 45, ALVC 42, F 41, LVCZ 39, HCS 37, AVCH 30, ALVTH 29, LVCR 27, ABTH 21, LVCC 16, AXC 16, LXC 14, S 12, AXCH 10, ALB 9, others < 9; unparsed (military/TXU/…) 311 | `cut -f2 … \| sort \| uniq -c` |
| No DigiKey package (ceramic/military) | 369 | `planning/plan-20-data/footprints-needed.txt` |
| DigiKey packages with no existing footprint | 45 | `grep -vc 'blank\|==' planning/plan-20-data/footprints-needed.txt` |

Known census gap: DigiKey returns at most 300 parts per search; the
74LVC/Active bucket of category 704 has 340, and 36 keyword re-searches
recovered 317 (2026-11-07). A Phase 1's per-base-name `--in-stock` check is
the authoritative stock check (AGENTS.md); rows it finds that are not in
`candidates.tsv` are added to `parts.tsv` and counted in Corrections.

### Verified properties

| Claim | How verified |
|---|---|
| No candidate is already in the catalog | name set ∩ every `name` in `Parts/*.json` → 0 (2026-11-07, after the inverter merge 4ae312f) |
| A part has one supply range today | `backend/include/parts.h:132` `const PartVoltage *vcc`; `voltage.c:254-268` raises one `power` level over every `PIN_POWER` pin of the part |
| Bidirectional pins exist and a model can read a pin it is not driving | `PIN_INOUT` (`parts.h:32`); latch model `bus-mux` (74_16260) drives and reads its A/B ports (`docs/parts-format.md` § Model `bus-mux`) |

## 3. Design Overview (all of plan-20)

### 3.1 Where each part goes

| Kind / model | What | Sub-plan |
|---|---|---|
| `gate`, op `buf` | buffers with no enable: '07, '17, '34, 1G07/1G17/1G34, 2G/3G versions, 6T07/6T17, 1T34/1T17/1T50, 4050, 4041 (true + complement), CD74HC4049 (`not`), '756/'757/'760 and the 1034 drivers | C |
| `buffer` `tristate` | unidirectional buffers with output enables, inverting or not, push-pull or open-drain, incl. Widebus 16/32-bit, 11xxx center-pin, '365–'368, CD4502/CD4503 | D, G |
| `buffer` `transceiver` | bidirectional DIR + OE (or OEAB/OEBA) transceivers, inverting or not, incl. '243 / '623 dual-OE | D, G |
| `buffer` `registered` | '646/'648/'652/'654/'651/'653 (registers + real-time/stored select), '543/'544/16543 (latched, CE/LE/OE per direction), '2952/'2953/16952 (registered with clock enables), 29854-style parity transceivers as specs.md describes | E, H |
| `buffer` `translator` | dual-supply: DIR-controlled (1T45, 2T45, 4T245, 8T245, 16T245, 4245, 164245, 3245, 324245), fixed-direction (TXU, TXV, 8T540/8T541, 4T774 per specs.md, CD4504, CD4010) | F, H |

### 3.2 Why `buf` in `gate` and one `buffer` kind

- A buffer with no enable is a one-input gate; the `not` op (fd9a9fa)
  already allows one input, open-drain output and family timing. `buf` is
  the same with no inversion.
- Everything with an enable, a direction or storage shares one machine:
  ports of N bits, per-direction output enables, optional inversion,
  optional storage, optional second supply domain. One kind, four models,
  one `SimComponentType` (plan-10 precedent).

Rejected alternatives:

- **3-state buffers in `gate`** with an enable input: the gate kind has no
  3-state drive and no shared enable across gates; adding both to the kind
  every existing gate uses widens the blast radius for no gain.
- **Transceivers as two `tristate` units back to back**: a part is one
  component; the turnaround rule (a port never reads its own drive) needs
  both directions in one model.
- **Translators as single-supply parts**: the voltage check would judge
  B-side pins against the A supply, which is exactly the wrong answer for a
  level shifter.

### 3.3 Dual supply (F)

- Format: a device key `"domains"`: `[{"power": "VCCA", "pins": [...]},
  {"power": "VCCB", "pins": [...]}]` — every non-supply pin belongs to
  exactly one domain (control pins to the side the datasheet references,
  usually VCCA). Timing entries and parts carry `vcc`/`inputMax` (domain A,
  as today) and `vccB`/`inputMaxB` (domain B).
- `Part` gains `const PartSupplyDomain *domains; size_t domain_count;`
  (`PartSupplyDomain`: power pin index, `PartVoltage`, pin index list);
  NULL/0 for every existing part. `vcc` stays the A range so single-supply
  code paths are unchanged.
- Voltage check: for a part with domains, each domain is powered at the
  level on its own power pin, and each pin is checked against its domain's
  range and input maximum; outputs drive their domain's level.
- Listing: a dual-supply part's line appends `|vccb=<min>-<max>,in=<max>`;
  no existing line changes.

### 3.4 Conventions

plan-10-A §3.4 plus plan-19-A §3.3 (inclusion rule; military numbers to
their SN54 device; FCT/automotive-named parts to their device). Families:
bus-hold, damping-resistor and power-up-3-state variants fold into the base
(ABTH, ABTR → ABT; ALVCH, ALVCHR, ALVCHS → ALVC; LVCH, LVCR, LVCHR, LVCZ,
LVCC → LVC; LVTH, LVTZ → LVT; ALVTH, ALVTHR → ALVT; AVCH, AVCA, AVCAH,
AVCB, AVCBH → AVC; AUCH → AUC; AXCH → AXC; LXCH → LXC; CD4 → C). Drawer:
`["Logic"]`, `function` `"Buffer"` (buf, tristate), `"Transceiver"`
(transceiver, registered), `"Level shifter"` (translator).

### 3.5 Signal rules

plan-10-A §3.5; bus hold is not modeled (a floating input reads X). A
transceiver port drives only while its direction is enabled and reads
otherwise; it never reads its own drive. An X on DIR or an OE makes both
ports' outputs X where they could drive.

### 3.6 Risk

The voltage-check change (F) touches every circuit's voltage results; its
gate is that every existing `voltage_test` case and golden line is unchanged
and the new dual-supply cases pass. The `tristate` and `transceiver` models
carry about 2650 of the parts; D tests them before any data lands (G).

## 4. Detailed Design (this sub-plan)

- Datasheets: one PDF per distinct TI datasheet,
  `https://www.ti.com/lit/ds/symlink/<lowercase base>.pdf` (`curl -sfL -o`);
  fallback the DigiKey `datasheet_url` when it is a PDF (`file` → `PDF
  document`); a Rochester viewer page (HTML) is not a datasheet. Folders:
  `buffer/` (buf and tristate), `transceiver/` (transceiver, registered),
  `translator/`.
- `planning/plan-20-data/parts.tsv`: plan-19-A §4.2 columns plus `model`.
  `specs.md`: per device the PDF pages, pinouts, model keys of C–F filled in,
  domains for translators, timing per family (main path A → Y / A → B; clock
  → B for registered; VCCA/VCCB reference condition for translators, one
  pair per family at the datasheet's 3.3 V ↔ 5 V or nearest row), VCC and
  input max per domain. `README.md`.
- Work order (it is the largest data set of any plan): by model, in the
  order buf, tristate, transceiver, registered, translator, so C, D/G can
  start on finished sections.

## Phases

> **Status discipline:** tick with command and output; `Commit:` when landed;
> Corrections for differences; current branch; only this plan's files.

### Phase 1 — Stock and datasheets

- [x] Stock re-check, scoped to the two listings the census cut short (Corrections 1): series `*` Active of category 704 (610 parts, 300 listed) by 17 keyword searches, and 74LVC Active (340 parts, 317 known) by 11 more; `--in-stock --status Active --json`, about 150 calls. Found 300 rows not in `candidates.tsv` (290 + 10 LVC861/863), added to `parts.tsv` (`wc -l` 4157 + header); the rest of the census is the same day's in-stock listing. Rows not Active or Last Time Buy: `skip` "status …" (932).
- [x] Download the PDFs: TI symlink by the part's generic number (1109 names tried, 889 PDFs), then TI literature numbers for DigiKey's Rochester links (42 more). Check: `ls datasheets/{buffer,transceiver,translator}/*.pdf | wc -l` → `818`; `awk -F'\t' 'NR>1 && $11=="add"{print $10}' planning/plan-20-data/parts.tsv | sort -u | wc -l` → `818`.

Acceptance: both checks (hours: the call quota sets the pace).
Commit: —

### Phase 2 — Part data

- [x] `parts.tsv` (4157 rows: 2360 `add`, 1797 `skip`), `specs/` (187 device specs) + `specs.md`, `reports/G01-G16.md`, `README.md`. Checks: `awk -F'\t' 'NR>1 && !(($11=="add" && $13!="") || ($11=="skip" && $12!=""))' parts.tsv | wc -l` → `0`; `awk -F'\t' 'NR>1 && $11=="add" && $7==""' parts.tsv | wc -l` → `0` (21 of the footprint ids are new, plan-20-B); `add` names matching `TXB|TXS|LSF|PCA|TCA|TXG|CBT|CB3|GTL|SSTL|SN75|SN65|SN95|SN725|CYBUS|FB` → none.
- [x] `datasheets/skipped-parts.md` § TI buffers, bus transceivers and level shifters (110 reason groups); `not-included.md`: the 72 read PDFs left with no part to add (not copied into `datasheets/`). ~~`obsolete-not-included.md`~~ — moot: only PDFs of Active/LTB rows were fetched, so no PDF has only obsolete parts (`awk` over `parts.tsv`: every copied PDF has ≥ 1 `add` row).

Acceptance: the checks above (minutes).
Commit: —

## Validation Plan

Landing tier at the end of I. A has no code; its gate is Phase 2's checks.

## Decisions

- **Only Active and Last Time Buy parts are added** (owner, 2026-11-07): a part whose DigiKey status is Obsolete, Discontinued at DigiKey or Not For New Designs is `skip` with reason "status <status>", even when it is in stock and in the datasheet's ordering table. This overrides, for this plan, AGENTS.md's "add obsolete parts with the `obsolete` flag"; no part of this plan carries the `obsolete` flag. A PDF left with no addable part is deleted and recorded in `datasheets/obsolete-not-included.md` (all its parts obsolete) or `datasheets/not-included.md` (other statuses).
- **Scope** (owner, 2026-11-07): single-supply buffers and transceivers,
  plus registered/latched transceivers, plus dual-supply level shifters
  (the widest option). Backplane, scan-test, termination and FET-switch parts
  stay out (they were outside every option offered).
- **Auto-direction translators (TXB/TXS/LSF/…) are out**: their behavior is
  a pass transistor plus one-shot edge accelerators, which the logic engine
  cannot represent (author).
- **Plans 19–22 in any order** (owner, 2026-11-07).
- **Bus hold not modeled; variants fold into the base family** (plan-10
  precedent).

## Corrections

1. **Stock re-check scoped** (Phase 1). The per-base-name re-check (845 bases measured by `cut -f4 | sort -u`, not ~400) did not fit the day's quota (277 calls left at start, shared with plans 19, 21, 22, 24). The census itself was the same day's `--in-stock` listing, so it is the stock check; what it lacked was the two listings DigiKey cut at 300 (`/tmp/p19/cat704.err`): **series `*` Active, 610 parts** (the plan knew only the 74LVC one) and 74LVC Active, 340. Keyword searches recovered 597 of 610 and 327 of 340 (the 13 + 13 left are SN55/SN65/SN70/CDC line and clock drivers the keyword lookup could not reach, out of scope, and 13 LVC rows no keyword found). 300 new rows → `parts.tsv` has 4157 rows, not 3857.
2. **Datasheet names**: DigiKey's base number is not TI's datasheet name; the PDF is the TI generic part number derived from the part name (`sn74abt16244a`, `sn74lvc1g125`, `cy74fct244t`, `-q1`/`-ep` for those parts), with DigiKey's own TI link next. 320 rows have no TI PDF at all (DigiKey links a Rochester Electronics viewer page — HTML, not a datasheet, § 4): `skip` "no TI datasheet".
3. **Parts not in their datasheet's ordering table are skipped** (332), as in the TI inverter work (`datasheets/skipped-parts.md` § TI inverters): DigiKey lists packing and finish variants (E4/G4/LE, NS without R, "A" revisions such as SN74HC125A, LV…AT parts) that no current packaging table lists, so their package is not documented. The readers had placed most of them by package code; the generator drops them.
4. **Status follows DigiKey**, per § Decisions, where a PDF's table disagrees (it lists some Active DigiKey parts as NRND, LTB or Obsolete). The readers dropped five such parts; they are added back (SN74AHC541DGVR, SN74AHCT541NSR, SN74AHCT16541DGGR, SN74LS367AD; CY74FCT16543ETPVC has no pinout for its package and stays out).
5. **Specs are JSON, one file per device** (`specs/<id>.json`), with `specs.md` generated from them: 187 devices and 2360 parts are too many for a hand-written markdown table, and the generators of C, G and H read the JSON.
6. **Census model counts were rough**: by the datasheets, 224 gate (`buf`/`not`), 1331 tristate, 418 transceiver, 155 registered, 232 translator parts. Corrections to § 3.1: '756, '757 and '760 have enables (tristate, open drain), not gates; AUP1T34 is a dual-supply translator (`1T34_DUAL`); LV8T245/8T540/8T541/8T7541/8T9541 are single-supply (transceiver/tristate); 4010B is a translator (VDD/VCC). Parity transceivers ('657, '833, '853, 29854, 16657, 16833), CD4502B (inhibit), '644 (one direction inverted), '16470, '16501, '16973 and the AXC8T245 (two DIR pins) do not fit the models: `skip` with that reason.
7. **Existing devices**: SNJ5416J joins the existing `74_16` in `gate.json`; SN74LV6T06PWREP and SN74LVC1G06DBVRQ1/DCKRQ1 are already in the catalog (`skip`).

## Summary

A turns the largest census of plans 19–22 into datasheets and a reviewed
part table sorted by model.
