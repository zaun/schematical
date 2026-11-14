# plan-34-D: Resistor footprints — axial through-hole resistors and arrays

Last updated: 2026-11-11
Effort: large (3h–1d)
Depends on: plan-34-C

D draws the land patterns of the through-hole (axial) resistors and the
resistor arrays the `add` rows use, by plan-34-C § 4's rules, into
`planning/plan-34-data/footprints-D.json`, and fills the `package`,
`pinout` and `footprint` columns of their `specs.tsv` rows. For arrays it
also reads each datasheet's circuit diagram into the pinout's resistor
pairs, which F writes as the device's `"resistors"` (plan-34-E § 4).

References:

- plan-34-C § 4 (the shared footprint rules, ids, reuse, `fp-load.sh`,
  `fp-check.py`).
- `planning/completed/plan-29-B-diode-footprints-vishay-single.md` § 4.1 R7
  (axial leads), plan-03-B § 3 R5 (through-hole pads from lead diameter).
- `docs/parts-format.md` § Footprints (`dual`, `header`, `explicit`).
- `planning/plan-34-data/footprints-needed.txt`, `specs.tsv`.

## Prerequisites

plan-34-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-34-C's footprint rules and tools | `ls planning/plan-34-data/census/fp-load.sh planning/plan-34-data/census/fp-check.py` | MET (2026-11-13; D ran beside C on the same pick, plan-34-C § Corrections 1) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every through-hole and array line of `footprints-needed.txt` (34 + 18 at
  planning time) has a footprint (`fp-check.py through-hole` and
  `fp-check.py array` → `0 without a footprint`).
- Every array `specs.tsv` row's `pinout` holds its pin pairs
  (`1-8,2-7,3-6,4-5`) read from the datasheet's circuit diagram, and its
  pinout name (plan-34-C § 4.2).
- `fp-load.sh footprints-C.json footprints-D.json` → only `extra
  footprint` / `unused footprint` failures, for exactly the new ids.

### Non-goals (explicit constraints)

- `Parts/footprints.json` and the goldens are not changed (F appends).
- No radial or vertical mounting is drawn for an axial part (one footprint
  per part: its datasheet's horizontal lead spacing).

## 2. Current State

Through-hole lines at planning time (34): YAGEO `YAGEO-MFR_DATASHEET.pdf`
bodies 1.9 × 3.4, 2.4 × 6.3, 3.3 × 9.0 mm (237 rows);
`YAGEO-FMP_DATASHEET` 2.4 × 6.3, 3.9 × 9.0, 4.5 × 11.5, 5.0 × 15.5 (57);
`YAGEO-MF0_DATASHEET` 2.4 × 6.3 (29); `YAGEO-RSF_DATASHEET.pdf` 3.3 × 9.0,
4.5 × 11.5, 5.0 × 15.5 (27); `YAGEO-FKN`, `KNP`, `PNP`, `PNP-V`, `HHV`,
`SQP_NSP`, `yageo-zor_datasheet.pdf` (ZOR), `NKN Series Spec.pdf` (a DigiKey
copy: B decides it); Vishay Dale `cmfind.pdf`, `cmfmil.pdf`, `erl.pdf`,
`rsns.pdf`; Viking `MFR_Series.pdf`. Array lines (18): YAGEO
`PYU-YC_TC_GROUP_51_ROHS_L` (0404 ×2, 0804 ×4, 1206 ×4 isolated and ×8
bussed 10-pin, 2512 ×8 bussed 10-pin), `PYU-AF122_124_162_164_51_ROHS_L`
(0804), `yageo_PYu-R_Mount_10_19050818_343.pdf`; Vishay Dale `msp.pdf`
(8-SIP), `msm.pdf` (6-SIP, 10-SIP), `somc.pdf` (16-pin SOIC, 5.59 mm,
isolated and bussed); Viking `CN_Series.pdf`, `ASN_Series.pdf`,
`AS42_AS43_Series_Draft_5-20-19.pdf` (DigiKey copy) (`grep -v '^chip'
planning/plan-34-data/footprints-needed.txt`).

Existing ids that may match: `RES-AXIAL-10.16` (generic 1/4 W, plan-26-A),
`SOIC-16` / `SOIC-16W` (3.9 mm / 7.5 mm bodies; Dale SOMC is 5.59 mm, so
neither fits unless its drawing agrees within 0.05 mm).

## 3. Design

As C, one PDF at a time. Axial: R7 with the datasheet's lead spacing when
it prints one per body (YAGEO MFR/FMP/MF0/RSF tables "lead spacing" or
"pitch"), else the R7 arithmetic. Chip arrays: R1 (YAGEO and Viking print
land patterns) as `dual` with `perSide` = pins / 2; SIP: the `header`
style (one row, 2.54 mm); SOIC: `dual` gull-wing per the drawing (R1 or
R2).

**Risk:** an array pin pair read wrong makes the simulation link the wrong
pins (every part of that pinout). Each pinout's pairs are checked against
the datasheet's circuit drawing twice (the `note` cites the page).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Axial resistors

- [x] Every through-hole line: one `RES-AX-…` object per distinct (span,
      pad, drill) (plan-34-C § 4.3), or a reuse of `RES-AXIAL-10.16` when
      it matches; the spec rows filled (`package` `Axial`, pinout
      `two-pin`). 29 lines → 13 new `RES-AX-…` footprints, ids with `X`
      (plan-34-C § Corrections 3), drill and pads per plan-03-B's R5
      (§ Corrections 4 there). Spans come from the YAGEO / Viking forming
      tables ("M TYPE" / "MA": 6.0, 10.0, 12.5, 15.0, 20.0 mm), else the R7
      arithmetic (FMP200 12.7, Vishay CMF55 10.16, CMF60 15.24, RN60 15.24,
      SQP 27.94 / 53.34). `RES-AXIAL-10.16` is not reused: its pads differ.

Acceptance: every through-hole line has a footprint.
  Check: `python3 -I planning/plan-34-data/census/fp-check.py through-hole` → `<n> lines, 0 without a footprint` (34 at planning time), and `fp-load.sh footprints-C.json footprints-D.json` → only the expected failures (est. 3 min).
  Result (2026-11-13): `29 lines, 0 without a footprint`; `44 new footprints: load ok`.
Commit: dbc0c8b

### Phase 2 — Arrays

- [x] Every array line: footprint object (or reuse), package name (DigiKey's
      common name: `0804`, `1206`, `2512`, `SIP`, `SOIC`), pinout name and
      pin pairs from the circuit diagram, in `specs.tsv`. 13 lines → 10
      new footprints (`RES-ARRAY-0404-4/0804-8/1206-8/1206-10/2512-10-YAGEO`,
      `RES-ARRAY-0804-8-VIKING`, `SIP-6/8/10-2.54-VISHAY` as `header`,
      `SOIC-16-5.59-VISHAY`). YAGEO's 8-pin arrays keep the datasheet's
      numbering (1–4 along one side, 5–8 along the other, left to right:
      `iso-8:1-5,2-6,3-7,4-8`, the footprint's `slots` placing the pads).
      Bussed arrays with two common pins carry `;common=a,b`: YC158 and
      YC358T `common=5,10`, YC358L `common=1,6`. These are separate
      `footprint-map.tsv` rows by series, because both YC358s are one
      land-pattern line. Second check of the pairs against the circuit
      diagrams (YC/TC sheet p. 5 Fig. 11, rendered at 300 dpi): YC122 1-4,
      2-3; YC104/124/164 1-5…4-8; YC358 L bus at 1 and 6, T and YC158 at 5
      and 10: all agree.

Acceptance: every array line has a footprint and pin pairs.
  Check: `python3 -I planning/plan-34-data/census/fp-check.py array` → `<n> lines, 0 without a footprint` (18 at planning time), `awk -F'\t' 'NR>1 && $7 ~ /^(iso|bus)/ && $7 !~ /:/' planning/plan-34-data/specs.tsv | wc -l` → 0 (every array pinout name carries its `:<pairs>`, Phase 2's format `iso-8:1-8,2-7,3-6,4-5`), and `fp-load.sh` as Phase 1 (est. 3 min).
  Result (2026-11-13): `13 lines, 0 without a footprint`; 0; `44 new footprints: load ok`.
Commit: dbc0c8b

## Validation Plan

- Tests: `footprint_check` on the copy (`fp-load.sh`).
- Coverage check: both `fp-check.py` runs print `0 without a footprint`.
- Runtime proof: E's `resistor_test` and F's goldens load parts on these.
- Doc sync: none.
- Final gate (once, after Phase 2): `cmake --build /tmp/logic-p34 -j8 &&
  ctest --test-dir /tmp/logic-p34` → `100% tests passed` (est. 1 min).

## Decisions

- **The datasheet's lead spacing first** — author: YAGEO's leaded sheets
  print the spacing per body; the R7 arithmetic is for sheets that do not.
- **SIP as `header`, chip arrays as `dual`** — author: the existing styles
  draw them; no new style.
- **The pinout cell carries the pairs** (`iso-8:1-8,2-7,3-6,4-5`) —
  author: F's generator reads one column; no second file.
- Owner decisions: plan-34-A § Decisions.

## Corrections

1. **D ran beside C, on the same provisional pick (2026-11-13).** See
   plan-34-C § Corrections 1–4: the pick before the stock check ends, the
   footprint map, ids with `X`, and plan-03-B's R5. Lines at that pick:
   through-hole 29, arrays 13 (planning time: 34, 18). The 20 arrays whose
   only datasheet is a DigiKey copy are skipped by B's verdicts.

## Summary

D's risk is the array pin pairs (simulation links) more than the pads;
both are cited to the datasheet page. Left untouched: `Parts/`, `backend/`,
the app.
