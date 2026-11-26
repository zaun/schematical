# plan-71-B: Line transceivers — pinouts and facts per datasheet, CAN and LIN; the spec tools

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-71-A

B reads each CAN and LIN `add` row's datasheet once per (PDF, package). It
fills that row's `specs.tsv` pinout, its signal map (G § 3.1: drivers,
receivers, enables, analog pins, supply domains) and its timing and supply
facts, and checks them with `spec-check.py`. B also writes the spec tools
that C (RS-485) and D (RS-232) reuse.

References:

- plan-71-A (§ 2 the pieces, § 4 the verdict rules, `specs.tsv`).
- plan-71-G § 3.1 (the device format the `facts` column feeds) and § 3.3
  (the logic each key drives).
- `docs/parts-format.md` § Pinouts and pins, § Parts (the `"package"` label
  rule), § Two supply domains (`domains`, `vccB`).
- `planning/plan-48-data/census/pinout-check.py`, `specs-fill.py`,
  `slice-specs.py` (the scripts B adapts).

## Prerequisites

plan-71-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-71-A complete | `ls planning/completed/plan-71-A-*.md` → exists, or every A box ticked | NOT MET (2026-11-25: A not started) |
| The spec skeleton exists | `head -1 planning/plan-71-data/specs.tsv` → the header of A Phase 4 | NOT MET (written by A) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every `specs.tsv` row with protocol `can` or `lin` has `scheme`, `pinout`,
  `label` and `facts` filled (or `pinout` `-` when the PDF shows no pinout
  for that package, which makes its rows `skip: no pinout`).
- `python3 -I planning/plan-71-data/census/spec-check.py can lin` prints
  `<n> rows, 0 errors`.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No footprint is drawn here (E, F), and no part file is written (H).
- Data rate and grade are DigiKey's parameters of each row (plan-40's rule
  for ratings). B reads the pinout, the signal map, the supply ranges and
  the two delays from the PDF.

## 2. Current State

`specs.tsv` (A Phase 4) has one row per (protocol, company, pdf, package)
with `rows` and `example`. The column count is A's output. There is one row
per (PDF, package) of the CAN and LIN `add` rows (at most the 494 CAN and
LIN candidates of A § 2: 403 + 91).

## 3. What each row records

### 3.1 `pinout`

Every package pin, in plan-48-B's form `number:name:role[:group]`, with the
datasheet's names (`TXD`, `RXD`, `CANH`, `CANL`, `STB`, `S`, `VIO`, `LIN`,
`VBAT`, `SLP_N`, `EN`, `INH`, `WAKE`, `SPLIT`). Roles (G § 3.2):

| Pin | Role |
|---|---|
| TXD, STB, S, EN, SLP_N, WAKE, NSTB, SILENT | `input` |
| RXD, nFAULT, ERR_N | `output` |
| CANH, CANL, LIN | `inout` |
| VCC, VIO, VBAT, VSUP | `power` |
| GND, an exposed pad wired to ground (`EP` with its number when the drawing numbers it) | `ground` |
| INH, SPLIT, VREF, RS | `output` or `input` as the datasheet says, and listed under `analog` |
| NC, DNC | `nc` |

### 3.2 `facts` (one JSON object, G § 3.1's keys)

- `protocol` (`can` / `lin`).
- `drivers`: CAN `[{"input": "TXD", "outputs": ["CANH", "CANL"], "enable": […]}]`;
  LIN `[{"input": "TXD", "outputs": ["LIN"], "enable": […]}]`.
- `receivers`: CAN `[{"inputs": ["CANH", "CANL"], "output": "RXD", "enable": […], "disabled": "high"}]`;
  LIN `[{"inputs": ["LIN"], "output": "RXD", "enable": […], "outputStyle": "open-drain", "disabled": "z"}]`.
- Enables from the mode table of the datasheet:
  - a CAN standby pin (`STB` high): driver and receiver off; RXD high
    (`"disabled": "high"`).
  - a silent pin (`S` high): driver off only.
  - TCAN-style `EN` + `nSTB`: normal mode only when `EN` 1 and `nSTB` 1.
    Both are controls of both enables, and listen-only (EN 0, nSTB 1) keeps
    the receiver on: `{"pin": "EN", …}` only on the driver.
  - LIN `SLP_N` low (sleep): driver off, RXD released (`"disabled": "z"`).
- `analog`: every pin of the § 3.1 analog row.
- `domains` when the part has a VIO or VCC1 / VCC2 (isolated) pin: the
  logic side first `{"power": "VIO", "pins": ["TXD", "RXD", "STB"]}`, then
  the bus side `{"power": "VCC", "pins": ["CANH", "CANL", …]}`. Ground pins
  belong to no domain (`docs/parts-format.md` § Two supply domains).
- `driverDelay`, `receiverDelay`: the datasheet's maximum propagation delay
  TXD → bus and bus → RXD in ns, at the 5 V (CAN) or 12 V VBAT (LIN) row.
  When the PDF gives only a loop delay (TXD → RXD), driver and receiver get
  half each, and the row's `note` says so.
- `vcc` (and `vccB` with domains): the datasheet's operating-conditions supply range;
  `inputMax`: the logic inputs' absolute maximum when the PDF gives one
  below the supply's.

### 3.3 `scheme` and `label`

`scheme` names the shared pinout a row reuses (`can-8-tja1051`,
`can-8-stb`, `can-8-vio`, `lin-8-tja1021`, …) so the parts file writes one
pinout per scheme. `label` is the package label of `docs/parts-format.md`
§ Parts (`"SOIC-8"`, `"VSON-8"`).

## 4. The tools (C and D reuse them)

- `census/slice-specs.py <protocol…>` prints the rows to fill, grouped by
  PDF.
- `census/specs-fill.py <file>` merges a filled slice back.
- `census/spec-check.py <protocol…>` checks, per row:
  - pin numbers are 1…N with no gaps (or the ball grid);
  - every name in `facts` is a pin of the pinout;
  - driver inputs are `input`, driver outputs `output` or `inout`, receiver
    inputs `input` or `inout`, receiver outputs `output`, enable pins
    `input`;
  - at least one `power` and one `ground` pin;
  - every `inout` pin is a bus pin;
  - with `domains`, every non-ground, non-NC pin is in exactly one domain;
  - the delays are above 0 and at most 100000 ns;
  - `vcc` lies within 0.5–60 V.

The rules are the loader's (G § 3.2), so a row that passes loads.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The tools

- [ ] `census/slice-specs.py`, `specs-fill.py`, `spec-check.py` per § 4.

Acceptance: `spec-check.py` catches each rule.
  Check: `python3 -I planning/plan-71-data/census/spec-check.py --self-test` → one `caught` line per § 4 rule (10), `0 missed` (est. < 1 min).
  Result:
Commit:

### Phase 2 — CAN rows

- [ ] Fill every `can` row per § 3, by PDF, maker by maker (TI, NXP, Microchip).

Acceptance: the CAN rows check clean.
  Check: `python3 -I planning/plan-71-data/census/spec-check.py can` → `<n> rows, 0 errors`; `awk -F'\t' '$1=="can" && $10==""' planning/plan-71-data/specs.tsv | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

### Phase 3 — LIN rows

- [ ] Fill every `lin` row per § 3 (TI, NXP, onsemi). A LIN part with an
      integrated regulator lists the regulator's VOUT under `analog` (its
      output is not driven in play mode, G § 3.5).

Acceptance: the LIN rows check clean.
  Check: `python3 -I planning/plan-71-data/census/spec-check.py lin` → `<n> rows, 0 errors`; `awk -F'\t' '$1=="lin" && $10==""' planning/plan-71-data/specs.tsv | wc -l` → 0 (est. < 1 min).
  Result:
Commit:

## Corrections

(none yet)
