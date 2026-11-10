# plan-29-B: Footprints and specs — Vishay single diodes (and the shared footprint rules)

Last updated: 2026-11-09
Effort: large (3h–1d)
Depends on: plan-29-A

B reads the package drawing of every Vishay single-diode package the `add`
and `pending` rows use, and writes (1) a ready footprint object per land
pattern into `planning/plan-29-data/footprints-B.json` and (2) the
`pinout`, `pins`, `package` and `footprint` columns of `specs.tsv` for those
rows. It also settles the `pending` Vishay single rows (ordering-code
schemes). § 4 holds the footprint rules C–G reuse.

References:

- `docs/parts-format.md` § Footprints (styles, R-rule pointer, mid-values,
  no overlapping pads, `"footprint"` part key).
- `planning/completed/plan-03-B-logic-footprints.md` § 3 "Pad rules" (R1–R6).
- `planning/plan-27-B-tactile-footprints-1.md` § 4.4 (ready objects and
  `fp-load.sh`: the precedent B copies).
- `planning/plan-29-data/footprints-needed.txt`, `specs.tsv`,
  `candidates.tsv` (plan-29-A).

## Prerequisites

plan-29-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-29-A complete | `ls planning/completed/plan-29-A-*.md` → exists | MET (2026-11-09: archived at this commit) |
| `specs.tsv` skeleton exists | `test -s planning/plan-29-data/specs.tsv && echo ok` → `ok` | MET (2026-11-09: `ok`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Vishay single diodes (`maker` = Vishay General Semiconductor - Diodes
  Division, `category` = `single`): no row is `pending`; each is `add`,
  `skip` or `out` with a reason.
- Every `add` row of that scope has a `specs.tsv` row whose `footprint`
  names an object in `footprints-B.json` (or an existing footprint in
  `Parts/footprints.json`), with a pad for every pin number in `pins`.
- `census/fp-load.sh planning/plan-29-data/footprints-B.json` reports only
  `extra footprint` / `unused footprint` for exactly B's new ids.

Planning-time scope (2026-11-08): 3410 `candidate` + 1 `check` Vishay single
rows (`awk -F'\t' '$1=="single" && $2 ~ /^Vishay/ && ($24=="candidate"||$24=="check")' planning/plan-29-data/candidates.tsv | wc -l` → 3411)
in **58** (maker, supplier package, mounting) lines
(`grep -c ' Vishay / ' planning/plan-29-data/footprints-needed.txt` → 58;
DigiKey's facet for all Vishay in-stock singles shows 63 packages, the 5
others are chassis / stud packages that are `out`). A's re-run (§ Corrections
of A) replaces these numbers.

### Non-goals (explicit constraints)

- `Parts/footprints.json`, `Parts/diode.json`, the goldens and the backend
  are not touched: the objects stay in `footprints-B.json` until plan-29-I
  lands the parts that use them (`footprint_check` fails on an unused
  footprint, `docs/parts-format.md` § The `"footprint"` part key).
- No part is added.

## 2. Current State

- `footprints.json` has no diode land pattern: 772 footprints
  (`wc -l < backend/tests/golden/footprints.txt` → 772 at f68e13f; 766 at 0f87917; 508 at 8cce726), none named for a
  diode package (`grep -c -i 'SOD\|SMA\|DO-\|MELF\|TO-2' backend/tests/golden/footprints.txt` → 0).
  `RES-AXIAL-10.16` is a resistor pattern (plan-26-A, generic), not from a
  diode drawing, so it is not reused.
- What the three makers' datasheets give (read 2026-11-08 with `pdftotext
  -layout`): Vishay `1n4001.pdf` (DO-41) has a "PACKAGE OUTLINE DIMENSIONS
  in inches (millimeters)" drawing and no pad layout (axial: R7); onsemi
  `mbr0520lt1-d.pdf` (SOD-123) ends with its "MECHANICAL CASE OUTLINE",
  CASE 425 ISSUE H (p.4; dimensions in the drawing image, "STYLE 1: PIN 1.
  CATHODE 2. ANODE" in text), so its page is read as an image (R1 when the
  drawing shows a recommended footprint, else R2);
  Taiwan Semi `SS12 SERIES_R2305.pdf` (SMA) has a "SUGGESTED PAD LAYOUT"
  (R1). Vishay SMD datasheets are read the same way; where a drawing has no
  pad layout, R2 / R4 apply (§ 4.1).
- Ordering tables are often patterns, not lists: Vishay prints
  "ORDERING INFORMATION (Example)" with one base part and its package codes
  (`1n4001.pdf`: `1N4004-E3/54`, `1N4004-E3/73`, package codes 54 and 73);
  Taiwan Semi prints `SS1x` with "x defines voltage from 20V(SS12) to
  150V(SS115)"; onsemi lists every orderable number (`MBR0520LT1G`,
  `MBR0520LT3G`). § 4.5 turns a pattern into a regex.

## 3. Design

B works datasheet by datasheet down `specs.tsv` (Vishay, `single`), and per
row: reads the ordering information (settles `pending` rows, § 4.5), the
pin configuration (writes `pinout`, `pins`), the package drawing (finds or
adds the footprint), and the package name (§ 4.3).

**Risk:** a land pattern copied wrong (span or pad size) lands on many parts
at once. Each new footprint is checked against its drawing by the
`footprint_check` load (malformed / overlapping pads) and by a second read of
the drawing's numbers when its `note` is written.

## 4. Footprint rules (shared by B–G)

### 4.1 Which rule

- **R1** — the datasheet (or the maker's package-outline document it points
  to) gives a recommended land pattern ("Mounting pad layout", "Soldering
  footprint", "Suggested pad layout", "Recommended land pattern"): use its
  pad sizes and centres.
- **R2 / R4** (plan-03-B) — outline only, gull-wing or flat leads (SOD-123F,
  SOT-23, SMA/SMB/SMC J-bend ends count as flat leads) / leadless terminals
  (DFN, SMP, MicroSMP, SlimSMA bottom terminals).
- **R5** (plan-03-B) — through-hole leads in rows with a stated spacing
  (TO-220, TO-247, bridge rectifiers with a lead pitch).
- **R7 (new) — axial leads** (DO-35, DO-41, DO-15, DO-201AD, SOD-57,
  SOD-64): `dual`, `pins` 2, `perSide` 1, `pitch` 1; `span` = the smallest
  multiple of 2.54 mm that is at least body length (max) + 3.0 mm (1.5 mm
  bend room at each end); `drill` and round `pad` per R5 from the lead
  diameter (max); `pin1Shape` `rect` (pin 1 is the cathode, § 4.2). The
  `note` gives the body length, the lead diameter and the arithmetic, e.g.
  DO-41 body 5.2 max → 8.2 → span 10.16.
- **MELF / MiniMELF** (cylindrical SMD): R1 (Vishay gives the pad layout).
- Mid-values and "no overlapping pads" as `docs/parts-format.md` says.

### 4.2 Pin numbers and pinouts

- **Two-terminal packages** (axial, SOD, SMA/SMB/SMC, SMF, SMP, MELF): pin 1
  = cathode (the band / the marked end), pin 2 = anode, matching
  `GENERIC-LED`'s `two-pin` pinout (1 K, 2 A). Pinout name `two-pin`
  (`[{"number":1,"name":"K","role":"inout"},{"number":2,"name":"A","role":"inout"}]`).
- **Packages with numbered leads** (SOT-23, SOT-323, SOD-523 has two, TO-220,
  TO-247, D2PAK, DPAK, TO-277A, FlatPAK, DFN): the datasheet's pin numbers
  and names, mapped to `K`, `A` (singles), `A1`, `A2`, `K1`, `K2`, … (arrays),
  `AC1`, `AC2`, `+`, `-` (bridges); a tab or exposed pad that is a terminal is
  the last pin number, with its terminal's name (two pins named `K`: the
  kind ties them, plan-29-H § 5.2). A lead the datasheet marks NC is `nc`.
- Pinout names: `two-pin`, `sot23-a1k3` (pin 1 A, 2 NC, 3 K), … — the
  shape in words, one name per distinct pin map, listed in `specs.tsv`'s
  `pinout` column with the map in `pins` (`1:K,2:A`).

### 4.3 Package names (the part's `"package"`)

The DigiKey supplier package's common name, no JEDEC code, no pin count
(`docs/parts-format.md` § Parts, `"package"`): `DO-214AC (SMA)` → `SMA`,
`DO-204AL (DO-41)` → `DO-41`, `DO-204AH (DO-35)` → `DO-35`, `DO-204AC
(DO-15)` → `DO-15`, `TO-263AB (D2PAK)` → `D2PAK`, `TO-252AA (DPAK)` →
`DPAK`, `TO-277A (SMPC)` → `SMPC`, `DO-219AB (SMF)` → `SMF`, `DO-220AA
(SMP)` → `SMP`, `SOD-80 MiniMELF` → `MiniMELF`, `TO-220AC` → `TO-220`,
`ITO-220AC` → `ITO-220`, `SOD-123W` → `SOD-123W`; a name with no common
name is used as DigiKey prints it (`SlimDPAK`, `DFN3820A`). Recorded per row
in `specs.tsv`.

### 4.4 Footprint ids, reuse, ready objects

- **Ids.** Two-terminal diode packages: `DIODE-<package>` (`DIODE-SMA`,
  `DIODE-DO-41`, `DIODE-MINIMELF`); multi-lead packages: the package's own
  name in the existing style (`SOT23-3`, `SC70-3`, `TO-220-2`,
  `TO-263-2-TAB`, `TO-252-2-TAB`, `TO-247-3`). A second land pattern for
  the same package (another maker's drawing that differs) appends the
  maker: `DIODE-SMA-TSC`.
- **Reuse.** A drawing whose land pattern equals an existing footprint
  (every pad's size and centre within 0.05 mm, same drill) reuses it; the
  first letter to draw a package owns its id. B owns the Vishay ids; C–G
  check `footprints-[B-F].json` and `Parts/footprints.json` first.
- **Shared ids.** `SOT23-3`, `SC70-3`, `SOT363-6`… may also be added by
  another open plan. Before adding one: `grep -c '"id":"SOT23-3"'
  schematical/schematical/Parts/footprints.json planning/plan-*-data/footprints-*.json`;
  a hit means reuse it (its pads are checked against this drawing as any
  reuse is) and the add step is marked moot with that grep's output.
- **Ready objects.** `footprints-B.json` is `{"footprints":[…]}` of objects
  in `footprints.json`'s format, each with a `note` naming the drawing, the
  PDF path under `datasheets/diode/` and the page.
- **`census/fp-load.sh`** (written in B Phase 1, the plan-27 script's
  copy): copies `backend/` and `Parts/` to `/tmp/p29-fp`, appends the
  objects of the `footprints-*.json` files named on the command line to the
  copy's `footprints.json`, builds `footprint_check` there and runs it.
  Expected: failures only of the form `extra footprint (not in the golden)`
  and `unused footprint`, for exactly the new ids; no load error.

### 4.5 Settling `pending` rows

A `pending` row has a PDF but its part number is not printed verbatim
(plan-29-A § 3 rule 4). The reader writes the datasheet's ordering-code key
as a regular expression in `specs.tsv`'s `mpn_regex`, built only from what
the datasheet prints: the base part numbers it covers and the suffixes or
package codes its ordering table shows. `1n4001.pdf` gives
`^1N400[1-7]-E3/(54|73)$` (bases 1N4001–1N4007 from its title table, codes
54 and 73 from the example table), so `1N4001-E3/53` (code 53, printed
nowhere in that PDF) is `skip`. `census/scheme-check.py` (A Phase 4) sets
`add` for each pending row of that spec row that fully matches, and `skip`
(reason `not in the datasheet's ordering table`) for the rest.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Tools

- [x] `planning/plan-29-data/census/fp-load.sh` (§ 4.4) and
      `census/specs-check.py` (written 2026-11-09; `fp-load.sh` passes on
      an empty file: `0 new footprints: load ok`, exit 0; its helper
      `fp-merge.py` is plan-27's, fixed to leave `footprints.json` alone
      when there is nothing to append; `specs-check.py --maker Vishay
      --category single` runs on A's skeleton: `5990 problems in 2253 add
      rows` (every spec row still empty, every pending row listed)): every `add` row's spec row has `package`,
      `pinout`, `pins`, `footprint`; the footprint exists in
      `Parts/footprints.json` or a `footprints-*.json`; its pad designators
      equal the pin numbers in `pins`; one `pinout` name has one `pins` map.
      Filters: `--maker <prefix of the maker column>`, `--category <c>`,
      `--mounting smd|th` (smd = Surface Mount and Wettable Flank); prints
      `specs ok` or one line per problem.

Acceptance: both run on an empty `footprints-B.json`.
  Check: `echo '{"footprints":[]}' > /tmp/fp0.json && planning/plan-29-data/census/fp-load.sh /tmp/fp0.json` → `footprint_check` passes (est. 3 min).
Commit: —

### Phase 2 — Surface-mount packages

- [ ] For each Vishay single SMD line of `footprints-needed.txt` (Surface
      Mount and Wettable Flank), in descending part count: read the drawing,
      reuse or add the footprint (§ 4.1, § 4.4), fill the spec rows (§ 4.2,
      § 4.3), settle pending rows (§ 4.5).

Acceptance: every Vishay single SMD spec row is complete.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --maker Vishay --category single --mounting smd` → `specs ok` (est. 1 min).
Commit: —

### Phase 3 — Through-hole packages

- [ ] The same for the Through Hole lines (axial R7; TO-220 / ITO-220 /
      TO-247 R5 or R1).

Acceptance: every Vishay single spec row is complete and the objects load.
  Check: `python3 -I planning/plan-29-data/census/specs-check.py --maker Vishay --category single && planning/plan-29-data/census/fp-load.sh planning/plan-29-data/footprints-B.json` → `specs ok`, then only extra/unused failures for B's ids (est. 4 min).
Commit: —

## Validation Plan

- Tests: the `fp-load.sh` run of Phase 3; `specs-check.py`.
- Coverage check: `awk -F'\t' '$1=="single" && $2 ~ /^Vishay/ && $24=="pending"' planning/plan-29-data/candidates.tsv | wc -l` → 0.
- Runtime proof: not applicable (no part loads yet).
- Doc sync: none (R7 is written into `docs/parts-format.md` by plan-29-I,
  when the first R7 footprint ships).
- Final gate: the Phase 3 check (est. 4 min).

## Decisions

- **Pin 1 = cathode on two-terminal packages** — author: the LED pinout's
  convention, and the band end of every in-scope drawing.
- **R7 for axial leads** — author: axial drawings give no hole spacing;
  rounding to 2.54 mm keeps parts on the usual grid.
- **Ids `DIODE-<package>`; multi-lead packages by package name** — author;
  generic ids are shared (add only if absent, § 4.4).
- **Ready objects, merged with the parts** — author (`footprint_check`
  rejects unused footprints; plan-27-B precedent).

## Corrections

## Summary

B's risk is volume and copying: 58 package lines for 3411 rows; a single
wrong span lands on hundreds of parts, so each footprint's numbers are read
twice and loaded once. Left untouched: every shipped file.
