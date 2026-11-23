# plan-47-C: Relays and buzzers — verdicts; the final pick, skipped parts, obsolete PDFs

Last updated: 2026-11-16
Effort: large (3h–1d)
Depends on: plan-47-B

C gives the relay and buzzer rows of the download set their verdicts
(ordering table, datasheet status, pinout, facts), as B did for the other
three types, and then closes the data work for all five: the final pick
(`add`), the skipped parts, the obsolete PDFs, the pruned PDFs and the final
`footprints-needed.txt`. The behavioral outcome: `candidates.tsv` has one
`add` row per part the library will get, every other download-set row is
`skip` or `out` with a reason, `specs.tsv` has every column filled, and
every PDF under the four datasheet families is used by an `add` row or is a
fuse-clip datasheet.

References:

- plan-47-A § 3 (the inclusion rule and quotas), § 4 (placement), § 6 (the
  clips); plan-47-B § 3 (the scheme, status and fact method, the tools).
- plan-47-J § 3.1 and § 3.4 (the `relay` and `buzzer` kinds' keys: the pin
  names and facts C must deliver).
- `AGENTS.md` (§ Datasheets); `datasheets/skipped-parts.md`,
  `datasheets/obsolete-not-included.md` (formats).
- `planning/completed/plan-40-B-transistor-verdicts.md`,
  `planning/plan-40-data/census/` (`verdicts.py`, `prune.py`).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-B complete | `ls planning/completed/plan-47-B-*.md` → exists | NOT MET (2026-11-23: B's data is done for the current set; its last task and A's Phase 2 / 3 wait for DigiKey's quota) |
| The inductor, fuse and PTC rows have verdicts | `python3 -I planning/plan-47-data/census/scheme-check.py inductor fuse ptc` → `… 0 unchecked` | MET on the current set (2026-11-23: `scheme-check.py inductor fuse ptc` → `2213 rows: 2098 ok, 0 unchecked`) |
| The PDFs are placed | `python3 -I planning/plan-47-data/census/place.py --verify` → `0 missing, 0 unused` | MET on the current set (2026-11-23: `0 missing, 0 unused`) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- `python3 -I planning/plan-47-data/census/scheme-check.py` (all five types)
  prints `<n> rows: <k> ok, 0 unchecked` with `<n>` = the rows of the
  download set after A (3522 at planning time); every row not `ok` is `skip`
  or `out` with a `reason`.
- `specs.tsv` has every column filled on every row (1563 at planning time:
  inductor 418, fuse 237, PTC 122, relay 216, buzzer 570).
- `candidates.tsv` has `state` `add` on the final pick: per (type, maker,
  class) the first `quota` passing rows in rank order (candidate rows first,
  then reserve rows), quotas of plan-47-A § 3 rule 6; a shortfall is a number
  in § Corrections. At planning time the pick is 2953 (600 + 600 + 600 + 600
  + 553).
- `footprints-needed.txt` is regenerated from the `add` rows
  (`python3 -I planning/plan-47-data/census/footprint-lines.py planning/plan-47-data/candidates.tsv schematical/schematical/Parts/footprints.json --add`);
  D, E, F, G and H draw from it.
- `datasheets/skipped-parts.md` has a `## Inductors, fuses, relays and
  buzzers (plan-47)` section for the skips; every PDF whose rows are all
  obsolete is deleted and listed in `datasheets/obsolete-not-included.md`;
  every remaining PDF is used by at least one `add` row or is one of the
  five clip PDFs.

### Non-goals (explicit constraints)

- No change to `Parts/*.json`, `footprints.json`, `backend/` or the app.
- No DigiKey call and no download.
- No refill from outside the download set (plan-47-A § Decisions).

## 2. Current State

After B, the inductor, fuse and PTC rows have verdicts; the relay and buzzer
rows have `pdf`, `hit` and `dead` only. No row is `add`.

### Measured populations

| What | Count | Command |
|---|---|---|
| Download-set rows of C's verdicts | 1309: relay 738 (TE 245, Franjobaim 242, Panasonic 251), buzzer 571 (Same Sky 216, PUI 202, Mallory 153) | `awk -F'\t' 'NR>1 && ($23=="candidate" \|\| $23=="reserve"){print $3, $2}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` |
| `specs.tsv` rows of C | 786: relay 216 (TE 119, Panasonic 76, Franjobaim 21), buzzer 570 (Same Sky 215, PUI 202, Mallory 153: one PDF per part) | the distinct (type, company, series, url, package) of the download set |
| Distinct PDFs | relay 171, buzzer 569 | the distinct `url` per type |
| Relay classes in the pick | power 353, signal 154, ssr 44, auto 44, reed 5 | `awk -F'\t' '$23=="candidate" && $3=="relay"{print $4}' … \| sort \| uniq -c` |
| Relay contact forms and coil types in the pick | plan-47-J § 2 (16 forms; non-latching 481, dual coil 31, single coil 17; 44 solid-state) | there |
| Relay mounting in the pick | through hole 442, surface mount 158 | `awk -F'\t' '$23=="candidate" && $3=="relay"{print $13}' … \| sort \| uniq -c` |
| Solid-state packages (download set) | 6-SMD 7, 4-SOP 8, 8-SMD 6, 8-SOP 4, 4-SMD 4, 4-VSSOP 4, 6-SOP 3, 4-TSON 5, 8-DIP 3, 6-DIP 3, 4-SIP 3, 16-SOP 2, 4-SSOP 1, `-` (a 6-SIP) 1 | `awk -F'\t' 'NR>1 && $3=="relay" && $10!="" && ($23=="candidate" \|\| $23=="reserve"){print $11}' … \| sort \| uniq -c` |
| Buzzer drivers and technology (download set) | indicator and transducer per plan-47-A § 2; piezo 251, magnetic 180, electromechanical/magnetic 116, piezo with feedback 18, none given 6 | `awk -F'\t' 'NR>1 && $3=="buzzer" && ($23=="candidate" \|\| $23=="reserve"){print $9}' … \| sort \| uniq -c` |
| Buzzer terminations (download set) | PC pins 366, solder pads 205 | same, `$15` |
| Military-grade relays in the pick | 19 | `awk -F'\t' '$23=="candidate" && $14=="Military"' … \| wc -l` |

## 3. Design

### 3.1 Schemes, status, facts

The method and tools are B's (§ 3.1, § 3.2, § 3.4 there). One-PDF-per-part
makers (Same Sky, PUI, Mallory) use `verbatim`. A relay's value group is its
coil voltage where the part number carries it (`-DC12V`, `-012`, `12VDC`;
group name `coil`), checked against the row's `value`.

| Type | Fact (part key) | DigiKey parameter | Check against the datasheet (`specs.tsv` column) |
|---|---|---|---|
| relay | `coilVolts` | Coil Voltage (`12VDC` → 12) | in the series' coil table (`coils`) |
| relay | `coilOhms` | from Coil Voltage and Coil Current (V / I, rounded to 3 digits), or the coil table's value | within 15 % of the table's (`coil_ohms`: `<volts>=<ohms>` pairs); left out when the table has none |
| relay | `contactAmps` | Contact Rating (Current); solid-state: Load Current | the series' rating (`amps`) |
| relay | `contactVolts` | Switching Voltage, the DC figure (`250VAC, 30VDC` → 30); solid-state: Voltage - Load, the upper bound | the series' (`vdc`); left out when the datasheet gives an AC figure only |
| relay | `operateMs`, `releaseMs` | Operate Time, Release Time | the series' (`op`, `rel`); left out when DigiKey gives `-` |
| relay | `onOhms` (solid-state) | On-State Resistance (Max) | the part's line (`ron`) |
| buzzer | `hz` | Frequency (a range `150Hz ~ 4kHz` takes the datasheet's rated or resonant frequency) | the datasheet's rated frequency (`hz`) |
| buzzer | `volts`, `voltsMin`, `voltsMax` | Voltage - Rated, Voltage Range | the datasheet's (`volts`, `vrange`) |
| buzzer | `ma` | Current - Supply | not above the datasheet's maximum (`ma_max`) |
| buzzer | `db` | Sound Pressure Level (SPL), the dB figure | the datasheet's minimum or typical (`db`) |

### 3.2 Relay pinouts

`pinout` uses the pin names plan-47-J's device keys name; the designators
are the datasheet's terminal numbers in its PCB-pattern drawing (bottom or
top view as drawn; the footprint letter mirrors a bottom view, plan-47-G).

- coil: `A1`, `A2`; `A1` is the `+` terminal of a polarized coil
  (`polarized` column `1`: a relay with a built-in diode, a polarized
  single-side-stable relay whose datasheet marks coil polarity);
- single-coil latching: `A1` (+ for set), `A2`; `latching` column `1`;
- dual-coil latching: `S1`, `S2` (set), `R1`, `R2` (reset); a three-terminal
  coil: `S`, `C`, `R` (`coils` = `S-C,R-C`);
- one pole: `COM`, `NO`, `NC`; several: `COM1`, `NO1`, `NC1`, `COM2`, …;
  a Form A pole has no `NC`, a Form B pole no `NO`; a Form X (double-make)
  contact is `COM`–`NO` over its two terminals;
- two pins on one contact (a power relay's doubled terminals) carry the same
  name;
- solid-state: input `A`, `K`; one channel's output `D1`, `D2` (`common` =
  `D1`, `no` = `D2`, or `nc` for a Form B part); a six-pin PhotoMOS-style
  part in its AC/DC connection uses its two drain pins as `D1`, `D2` and its
  source pin is `NC`; several channels: `A1`, `K1`, `D1A`, `D1B`, `A2`, …,
  with `coil` numbers in the `contacts` column;
- every other pin (no-connect, case, test, mounting) is `NC`.

`specs.tsv` columns for a relay: `pinout`, `coils` (`A1-A2`; `S1-S2,R1-R2`),
`polarized`, `latching`, `contacts` (`COM/NO/NC` per pole, `;` separated,
`@<n>` for a channel: `D1A/D1B/@1;D2A/D2B/@2`), `rtype`
(`electromechanical`, `reed`, `solid-state`). `pinout-check.py relay`
checks: every coil and contact name of those columns is in the pinout once
(or more, for doubled terminals), every other pin is `NC`, the pole count
equals the row's DigiKey contact form, the pin count equals the datasheet's
terminal count (`pads`).

`label`: `THT` or `SMD` for an electromechanical or reed relay (the tactile
switches' labels); for a solid-state relay the package family `DIP`, `SOP`,
`SSOP`, `VSSOP`, `SON`, `SIP`, or `SMD` (a gull-wing DIP, DigiKey's
`n-SMD`).

A row whose datasheet shows a function the kind cannot describe (a
time-delay or safety relay with force-guided contacts wired as a unit, an
SSR with a zero-cross triac output and a three-pin output) is `out`, reason
`not a plain relay`, with the datasheet's words in § Corrections.

### 3.3 Buzzer pinouts

`positive` `+`, `negative` `-`; `pinout` `1=+,2=-` in the datasheet's pin
order (pin 1 is the datasheet's, else the pad marked `+`). A third terminal
is `NC`: a piezo's feedback electrode (`3=F` with role `nc`, name `F`), a
surface-mount buzzer's dummy pads. A transducer with no polarity mark in its
datasheet keeps `1=+,2=-` (its two terminals are interchangeable). `driver`
and `technology` columns: `indicator` / `transducer` from the row's class;
`piezo` for DigiKey's Piezo values, `magnetic` for Magnetic and
Electromechanical/Magnetic, the datasheet's for the 6 rows with none.
`label`: `THT` or `SMD`. A buzzer whose datasheet gives it more than one
tone input or a control pin (a pin-selectable siren, 19 listed rows with
Operating Mode "Pin Selectable") is `out`, reason `more than two terminals`.

### 3.4 The final pick, skips, obsolete PDFs, pruning

`census/verdicts.py` (adapted from plan-40's) writes the final states: per
(type, maker, class) the rows in rank order; a row with all verdicts `ok`
is `add` until the class quota of plan-47-A § 3 rule 6 is full (the quotas
`select.py --report` printed for the share, so a redistributed quota stays
redistributed); the rest of the passing rows stay `reserve`.

`datasheets/skipped-parts.md` gets one section, one table row per (maker,
series, reason): the parts, the device ("Bourns SRR1260 shielded power
inductor"), where they would have gone (`inductor` / Discrete → Inductors),
and why (`not in the datasheet's ordering table`, `no datasheet on the
maker's site`, `fact not in datasheet`). `out` rows (obsolete, more than two
terminals, not a plain relay) are not skips and are listed in § Corrections
by count.

A PDF whose every download-set row is `out` for `obsolete` is deleted,
removed from its family's `datasheets.tsv` and added to
`datasheets/obsolete-not-included.md` (PDF name, family `inductor` / `fuse`
/ `relay` / `buzzer`, the drawer group, the datasheet's words and URL).
`place.py --prune` then deletes every PDF no `add` row cites, except the
five clip PDFs, and rewrites the four `datasheets.tsv`.

Risk: relay pinouts. A terminal-numbering mistake puts a contact on the coil
pads; `pinout-check.py`'s pole and pin counts and a hand check of one part
per (maker, series) against the datasheet's schematic bound it, and
plan-47-G draws each footprint from the same drawing.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; `- [~]` for partial; mark moot tasks with
> `~~text~~ — moot: <evidence>`; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — The relays

- [x] (2026-11-23: 294 relay specs rows, TE / Franjobaim / Panasonic by three agents (Panasonic split over four), `pinout-check.py` extended with the relay rules: `1625 specs rows: 0 errors`) Write `scheme`, `page`, `label`, `pinout`, `coils`, `polarized`,
      `latching`, `contacts`, `rtype`, `pads` and the relay fact columns on
      the 216 relay rows of `specs.tsv` (§ 3.1, § 3.2); extend
      `pinout-check.py` with the relay rules.
- [x] (2026-11-23: `scheme-check.py relay buzzer --facts --apply` → `1309 rows: 1056 ok, 0 unchecked`; `--sample | wc -l` → 120; hand checks: every Panasonic and Franjobaim series' pinout was read from its drawing by the agents, one part per series by eye; `dead` is 0 on every row) Run `scheme-check.py relay`, `--facts`, `pinout-check.py relay`; set
      the `skip` / `out` rows; hand-check one part per (maker, series)
      against its datasheet's schematic and the `--sample` (20 per maker);
      read every `dead` = 1 relay row.

Acceptance: every relay row of the download set has its verdicts.
  Check: `python3 -I planning/plan-47-data/census/scheme-check.py relay` → `<n> rows: <k> ok, 0 unchecked`; `python3 -I planning/plan-47-data/census/pinout-check.py relay` → `<m> specs rows: 0 errors` (est. 3 min).
Commit: 041f158f

### Phase 2 — The buzzers

- [x] (2026-11-23: 570 buzzer specs rows, one agent per maker; `0 errors`) The 570 buzzer rows of `specs.tsv` (§ 3.1, § 3.3; `verbatim` schemes),
      the three checks, the sample, the `dead` rows.

Acceptance: every buzzer row of the download set has its verdicts.
  Check: `python3 -I planning/plan-47-data/census/scheme-check.py buzzer` → `<n> rows: <k> ok, 0 unchecked`; `…/pinout-check.py buzzer` → `0 errors` (est. 3 min).
Commit: 041f158f

### Phase 3 — The final pick and the skip list

- [ ] After A's last refill (A Phase 2, Phase 3's re-run) and B's re-run task: run `specs-skeleton.py`, fill the specs rows the refill added (`census/frag-check.py` checks a fragment), re-run `scheme-check.py inductor fuse ptc relay buzzer --facts --apply` and `pinout-check.py` over all five types.
- [ ] `census/verdicts.py` (§ 3.4): `add` rows for all five types; record
      the pick per (type, maker, class) and each shortfall in § Corrections.
- [ ] `datasheets/skipped-parts.md`: the plan-47 section (§ 3.4).

Acceptance: the pick is final and every non-`add` download-set row has a reason.
  Check: `python3 -I planning/plan-47-data/census/scheme-check.py` → `<n> rows: <k> ok, 0 unchecked`; `awk -F'\t' '$23=="add"{print $3}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c` → the recorded pick; `awk -F'\t' 'NR>1 && ($23=="skip" \|\| $23=="out") && $24==""' planning/plan-47-data/candidates.tsv \| wc -l` → 0; `grep -c 'plan-47' datasheets/skipped-parts.md` → at least 1 (est. 2 min).
Commit: —

### Phase 4 — Obsolete PDFs, pruning, the final footprint lines

- [ ] Delete and record the all-obsolete PDFs (§ 3.4); `place.py --prune`;
      `place.py --verify` → `0 missing, 0 unused`.
- [ ] Regenerate `footprints-needed.txt` with `--add`; record the line
      counts per letter here and in D, E, F, G and H § 2 (each re-measures at
      its start).
- [ ] Commit the PDFs of the four families with their `datasheets.tsv`
      (the first commit that carries them).

Acceptance: the PDFs in the tree are exactly the ones the pick uses, plus the clips'.
  Check: `python3 -I planning/plan-47-data/census/place.py --verify` → `0 missing, 0 unused`; `awk -F'\t' '!/^#/{n[$1]++} END{for(k in n) print k, n[k]}' planning/plan-47-data/footprints-needed.txt` → the recorded counts; `grep -c 'plan-47' datasheets/obsolete-not-included.md` → the PDFs deleted for status (est. 3 min).
Commit: —

## Validation Plan

- Tests: none (data only); `scheme-check.py`, `pinout-check.py`,
  `place.py --verify` are the checks, with the hand checks of Phase 1 and 2.
- Coverage check: `0 unchecked` over all five types; `0 missing, 0 unused`.
- Runtime proof: n/a.
- Doc sync: `datasheets/skipped-parts.md`, `datasheets/obsolete-not-included.md`.
- Final gate: plan-47-O.

## Decisions

- **Pin names**: `A1`/`A2`, `S`/`R` pairs, `COM`/`NO`/`NC`, `A`/`K`/`D`, `+`/`-`
  (§ 3.2, § 3.3): the names relay datasheets print (`A1`, `A2`, `COM`, `NO`,
  `NC`), fixed here so 600 parts share a few pinouts per device.
- **Designators are the datasheet's terminal numbers**; a relay whose
  datasheet numbers no terminals gets 1…n in its PCB-pattern drawing's
  order, left to right, top to bottom, recorded in the device `note`.
  plan-47-G § 3.2 renumbers a sparse numbering (1, 2, 5, 6, 9, 10; A1, 11,
  14) to 1…n, because a footprint's pads are numbered 1…`pins`, and rewrites
  these pinouts with it.
- **A six-pin PhotoMOS-style part is wired as its AC/DC connection** (two
  drains), the connection its 1 Form A rating is given for.
- **A relay or buzzer the kind cannot describe is `out`**, not a new model
  (§ 3.2, § 3.3).
- **Military relays get `"flags": ["military"]`** and AEC-Q200 parts
  `"flags": ["automotive"]` (plan-47-K and L write them).
- **The skip list is by (maker, series, reason)**, not by part: up to
  hundreds of skipped rows would bury the file.

## Corrections

- **2026-11-23, the verdict data was entered before C's prerequisites held**
  (the quota days of A), for the current download set; Phase 3 (the final pick
  and the skip list) and Phase 4 are not started: they need A complete.
  Provisional pick (`census/verdicts.py`, a dry run that writes nothing):
  inductor 600, fuse 600, PTC 599 (Bourns through-hole 89 of 90), relay 504,
  buzzer 513; shortfalls (recorded again at Phase 3): relays TE signal 42, Franjobaim
  power 13, signal 12, auto 8, Panasonic SSR 7, TE auto 9, power 2, reed 2, SSR 1;
  buzzers 40 short of 553 by the skips.
- **Tools.** `census/verdicts.py` writes `state` / `reason` (`add`, `reserve`, `skip`,
  `out`) for the download-set rows with `--apply` (it stops when a row has no
  verdict; the quota of a group is its `candidate` count); `specs47.py`'s `in_set`
  also counts `add` and verdict-carrying `skip` / `out` rows, so a later check sees
  them. A relay specs key is `<package> / <contact form> / <coil type>` (a series
  shares one datasheet over forms and coil types with different pinouts; 294 rows,
  not 216). `pinout-check.py` counts poles from DigiKey's form (`(1 Form A, 1 Form B)`
  = 2, `x 2` multiplies) and reads `relay-poles.tsv` for a wrong DigiKey form
  (Panasonic AHER1281: the datasheet says 2 Form A).
- **Datasheet notes.** The TE catalog PDFs are two-page extracts: 60 rows on the CII
  sheets (`5-1773450-5_sec1_*`) have no ordering code in the PDF (scheme `-`, skipped);
  31 TE rows point at application notes, socket sheets or 3D-model sheets (no pinout);
  several TE rows mix variants (12 A and 16 A doubled pins) and use the first variant's
  pinout; `DS11-1000` and `1617110-6` are `out` (not a plain relay). Franjobaim: 33
  rows disagree with the datasheet on release time or switching voltage (DigiKey's
  AC figure read as DC), 17 have no drawing for their form. Panasonic: JVN1A-xV-F is
  `JVN1aF-` in the datasheet (DigiKey drops the F: skipped), DSP1-L-… are cited to a
  PDF that does not cover them, the ST series says 30 V DC (DigiKey 250). Buzzers: 20
  `out` (pin-selectable or control pin: Mallory 14, Same Sky 3, PUI 3). No obsolete or
  NRND text was found in any relay or buzzer PDF.
- **Judgement calls to revisit:** TX / TX-TH / TX-S dual-coil latching relays are
  `polarized` 1 (their drawings mark + and -); Mallory MSW5GH and MSW5GLMCT technology
  `magnetic` is a guess by analogy; PUI, Same Sky and Mallory four-pad buzzers carry two NC
  dummy pads.

## Summary

C's risk is relay pinouts (terminal numbering, bottom-view drawings,
doubled terminals) across 216 (series, datasheet) rows; the pole and pin
counts and the per-series hand check bound it. After C the data is final:
`candidates.tsv`'s `add` rows, `specs.tsv` and `footprints-needed.txt` are
what D to L build from. Untouched: every part file, the footprints, the
backend, the app and DigiKey.
