# plan-37-F: Parts — onsemi and Nexperia parts of the `dff` register devices

Last updated: 2026-11-12
Effort: large (3h–1d)
Depends on: plan-37-E

F adds the onsemi and Nexperia `add` parts of the six register devices
`dff.json` already has — 74_174, 74_175, 74_273, 74_374, 74_574, 40175B —
to those devices, with the 22 timing entries their new families need,
merges `DHVQFN-20` and `DQFN-20-ONSEMI` from `footprints-B.json`, adds the
`datasheets.json` entries and regenerates the goldens.

Outcome of F: `grep -c '|dff|' backend/tests/golden/parts.txt` = 706 + F's
`add` rows (198 in-scope rows at planning, before A's verdicts); no
existing `dff` line changes (`git diff backend/tests/golden` shows only
added lines); MC74VHC574DTR2G and 74HC273D,653 have `parts.txt` and
`traces.txt` lines (`trace_check` drives every part).

References:

- plan-37-A § Prerequisites; D § 3 (specs flow, parts, package names).
- `docs/parts-format.md` § `dff` (`timing`: family → `{rise, fall,
  clearWins}` plus `vcc`, `inputMax`; `outputEnable`; `units` 1–8).
- `schematical/schematical/Parts/dff.json` (the six devices: pinouts,
  units, timing entries and their notes).
- `backend/src/models/dff.c` (unchanged).

## Prerequisites

plan-37-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-37-E complete | `ls planning/completed/plan-37-E-*.md` → exists | MET (2026-11-13: `plan-37-E-register-parts-wide.md`) |
| LVX, VHC, VHCT, LCX are known families | `grep -o '"LCX"\|"LVX"\|"VHC"\|"VHCT"' backend/src/library/kind_util.c \| wc -l` → 4 | MET (2026-11-13: 4) |
| The six devices are unchanged since planning | `python3 -c` over `dff.json`: timing keys 74_174 {HC, HCT}, 74_175 {HC, HCT}, 74_273 {AC, ACT, AHC, AHCT, HC, HCT}, 74_374 {AHCT, HC, HCT, LS, S}, 74_574 {AC, HC, HCT}, 40175B {C} | MET (2026-11-13, F start: the same keys). A family another plan added first makes its § 2 entry moot: keep the existing entry |

## 1. Goal

- Each of the six devices gains its F `add` parts (by name order) and the
  timing entries of § 2.
- `ctest` passes; `git diff <F base> -- backend/tests/golden | grep -c '^-[^-]'` → 0;
  `footprint_check` passes with `DHVQFN-20` and `DQFN-20-ONSEMI` in the golden.
- `node scripts/check-datasheets.js <every new id>` → every id answers PDF.

### Non-goals (explicit constraints)

- No TI part (owner's answer: onsemi and Nexperia only; plan-37-A § Decisions).
- No change to an existing pinout, unit list or timing entry of these
  devices, and no `dff.c` change.
- No 1-bit flip-flop ('74, '112 …) parts from these makers (plan-37-A § 3).

## 2. Current State

F's rows (`kind` `dff`, `scope` `in`): **198** (onsemi 112, Nexperia 86;
18 automotive): 74_273 58, 74_574 55, 74_374 51, 74_175 17, 74_174 11,
40175B 6 (`awk -F'\t' '$6=="in" && $5=="dff"{print $3}' candidates.tsv | sort | uniq -c`).

Timing entries to add (families of F's rows not in the device's `timing`):

| Device | New families |
|---|---|
| 74_174 | AC |
| 74_175 | VHC |
| 74_273 | LVC, LVX, VHC |
| 74_374 | AC, ACT, AHC, LCX, LVC, LVX, VHC, VHCT |
| 74_574 | ACT, AHC, AHCT, ALVC, LCX, LVC, LVX, VHC, VHCT |
| 40175B | — (C exists) |

22 entries in all. Families the device already has (HC, HCT, AC, ACT, AHC,
AHCT, LS, C) keep their TI-derived entry; F's parts of those families use it
(D § Decisions: shared timing per family) and set their own part `vcc` when
their datasheet's Recommended Operating range differs.

Footprints (`cut -f20` of F's rows): SOIC-20W 74, TSSOP-20 64, SOIC-16 18,
TSSOP-16 14, DHVQFN-20 9, SOP-20 9, SSOP-20 7, SSOP-16 1, SOP-16 1,
DQFN-20-ONSEMI 1.

## 3. Design

- **Pinouts**: the device's existing `p16` / `p20`. Each maker's pin table
  is compared pin by pin with it in Phase 1 (Nexperia `74HC_HCT273.pdf`
  Table 2 names MR 1, Q0–Q7 2/5/6/9/12/15/16/19, D0–D7 3/4/7/8/13/14/17/18,
  GND 10, CP 11, VCC 20: TI's numbers under other names); the DHVQFN / DQFN parts use `p20` through their footprint's
  `slots` (B). The aliases go in the device `note`.
- **Timing entries**: the dff form `{rise, fall, clearWins, vcc,
  inputMax?, note}`; `rise`/`fall` = CLK → Q tPLH / tPHL typical at the
  family's nominal VCC (5 V: AC, ACT, AHC, AHCT, VHC, VHCT; 3.3 V: LVC,
  LVX, LCX, ALVC); `clearWins` copies the device's existing entries;
  `inputMax` 5.5 for the 5 V-tolerant LVC, LVX, LCX inputs when the
  datasheet states it. The `note` cites the datasheet id and page.
- **Parts**: D § 3.3 (`manufacturer` `onsemi` / `Nexperia`; flags).
  Datasheets go to `datasheets/ff-d/` (A).
- **Generator**: `gen-parts.py --dff <dff.json>` mode: prints the parts
  and timing entries to append per device, and `--check` validates them
  against the existing device (pin count, family keys).

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Pin check and timing

- [x] For each (maker, device) of F, check its datasheet's pin table
      against the device's pinout (numbers and functions); record the
      aliases in `specs/dff-additions.json` (per device: new timing entries,
      note text). `census/pincheck.py` compares by pin function: 23 Nexperia
      tables match pin for pin (`ok`); the '175 / 40175B tables print
      ranges and lose the overbars, and onsemi draws its pinouts, so those
      31 sheets were read from their pinout drawings (contact sheets of the
      first pinout page): all are the standard 74x174 / 175 / 273 / 374 /
      574 / 4175 numbering. No mismatch.
- [x] The 22 timing entries — 21 (§ Corrections: the 74_174 AC entry is
      moot), written by `census/specs-f.py`.

Check: `python3 -I planning/plan-37-data/census/gen-parts.py --dff schematical/schematical/Parts/dff.json planning/plan-37-data/specs planning/plan-37-data/candidates.tsv --check`
→ `<n> rows ok`, n = F's `add` rows (est. 1 min).
Observed: `161 rows ok`.

Commit: (this commit)

### Phase 2 — Part file, footprints, datasheets

- [x] Append parts and timing entries to the six devices in `dff.json`
      (`gen-parts.py --dff --write` inserts lines only: 198 added, the 10
      changed lines each gain only a trailing comma; one `note` per device).
- [x] Merge every `footprints-B.json` id an F `add` row names: `DHVQFN-20`,
      `DQFN-20-ONSEMI` (`2 footprints appended`).
- [x] `datasheets.json` entries for the new PDFs: `51 datasheets added`;
      `check-datasheets.js` on the 51 → `51 of 51 URLs serve a PDF`.
- [x] Regenerate the three goldens; review (added lines only; the two
      footprint lines hand-checked against their drawings): +161, +161, +2,
      0 removed. DHVQFN-20 and DQFN-20-ONSEMI put pin 1 at the top end's
      left slot and pins 2–9 down the left side, as B drew them from
      SOT764-1 Fig. 1 and MLP20B Figure 5.

Check: `cmake --build /tmp/logic-p37 -j8 && ctest --test-dir /tmp/logic-p37 -j8`
→ `100% tests passed`; `git diff -- backend/tests/golden | grep -c '^-[^-]'` → 0;
`/tmp/logic-p37/footprint_check` → passes (no `unused footprint`) (est. 6 min).
Observed: `100% tests passed, 0 tests failed out of 33`; `0`;
`footprint_check` → `1390 footprints compared, 28209 parts loaded`;
`|dff|` → 867 (706 + 161); MC74VHC574DTR2G and 74HC273D,653 have
`parts.txt` and `traces.txt` lines.

Commit: (this commit)

## Decisions

- **Existing devices, existing pinouts.** These are the same 74-series
  pinouts from every maker; a mismatch found in Phase 1 is a transcription
  error to fix, not a reason for a new device.
- **onsemi and Nexperia only** (the owner's answer, plan-37-A § Decisions).
- **40175B gets parts, no timing**: its rows (Nexperia HEF40175B,
  onsemi MC14175B / NLV14175B) are family `C`, which the device has.

## Corrections

- **2026-11-13 — F's counts after A.** 162 `add` rows (onsemi 84,
  Nexperia 78) from A, 161 after the next item: 74_273 47, 74_574 45,
  74_374 38, 74_175 16, 74_174 9, 40175B 6.
- **2026-11-13 — MC74AC174D is out, and 74_174 gets no AC entry.** Its
  only sheet, `mc74ac174-d.pdf` (MC74AC174/D Rev. 7, June 2006), is
  stamped "THIS DEVICE IS OBSOLETE, PLEASE CONTACT YOUR ONSEMI
  REPRESENTATIVE FOR INFORMATION" across p.1 (an image, so A's text check
  could not see it), though DigiKey lists it Active with 13311 in stock.
  The owner's rule (obsolete parts are left out, plan-37-A § 3 rule 5)
  makes it `out`; the PDF is deleted and listed in
  `datasheets/obsolete-not-included.md`, its `datasheets.tsv` line
  dropped. The plan's 22 timing entries become 21. No other onsemi first
  page carries the stamp (all 31 were viewed for the pin check).
- **2026-11-13 — timing where the sheet gives no typical** (onsemi 74ACT574,
  LCX): the max at the nominal supply, as D. LCX for 74_574 takes
  MC74LCX574's 1.65–5.5 V range (that part runs at 5 V); for 74_374,
  74LCX374's 2.0–3.6 V.

## Summary

F gives the six `dff` register devices their onsemi and Nexperia parts and
22 timing entries, without touching an existing line.
