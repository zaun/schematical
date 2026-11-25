# Reader guide — filling `specs.tsv` rows from a datasheet (plan-55-C § 3, used for C, D and E)

You read power-switch datasheets and describe each package's pinout and switches in the form the `powerswitch`
kind loads. Everything you need is here; the full kind is `docs/parts-format.md` § `powerswitch`, the rules are
`planning/completed/plan-55-A-powerswitch-kind.md` § 3.1 and `planning/plan-55-C-powerswitch-verdicts-ideal-load.md` § 3.
Work in the worktree `/Users/justinzaun/Development/logic/.claude/worktrees/P-55`. **Touch no file but your own packet's
`out.tsv` (and `overrides.tsv`); run no `git` command.**

## Your packet

`/tmp/p55/packets/pNN/assign.tsv` (tab separated, header first). One line per **key**: a (company, PDF, DigiKey
`Package / Case`, `Supplier Device Package`) the candidate parts use. Columns: `company`, `family` (`ideal` =
integrated ideal diode, `load` = load switch, `limited` = current-limited or USB switch), `pdf` (the placed datasheet),
`text` (its `pdftotext -layout` text: read and grep this, not the PDF), `package`, `supplier_package` (DigiKey's
two package labels), `rows` (how many parts), `parts` (`mpn:hit`, space separated, `_` is a space in the part
number; `hit` 1 = the part number is printed in the PDF), `digikey` (JSON: DigiKey's own parameters of the
most-stocked part: Switch Type, Number of Outputs, Voltage - Load, Current - Output (Max), Rds On (Typ), Features,
Fault Protection — a cross-check, **the datasheet wins**).

You write `/tmp/p55/packets/pNN/out.tsv`: **one or more rows per assigned key**, the columns of
`planning/plan-55-data/census/common.py` `SPEC_COLS` in this order:

`company series pdf package supplier_package rows example ordering pinout type switches analog vin_min vin_max iout ilimit ron iq flags note skip`

Copy `company`, `series` (always `-`), `pdf`, `package`, `supplier_package` exactly from the assignment line (the `series` value
is `-`); leave `rows`, `example`, `flags` empty. Write the file with a small Python script and `csv.writer(delimiter='\t',
lineterminator='\n')` so JSON cells are quoted safely. Worked examples (read them first):
`/tmp/p55-worked.tsv` (LM66100, LM66200, TPS22918) and `planning/plan-55-data/specs.tsv` (rows of TI worked examples).

## What a row says

* `pinout`: JSON `[[number, "NAME", "role"], ...]`, **every lead of the package once**, in the numbers of the
  package drawing (`1`, `2`, … integers; a ball-grid package (DSBGA, WLCSP, BGA) uses ball names `"A1"`, `"B2"`, …
  as strings, every ball once). The number of leads must equal the number in DigiKey's label (`8-SOIC` = 8,
  `SOT-23-5` = 5, `SC-70-6` = 6, `4-DSBGA` = 4 balls). **An exposed pad or a tab with no lead number is NOT a pin** (leave it out; if
  the datasheet numbers it as a pin of the same function as another pin, e.g. a numbered tab pin that is a second GND or a second OUT, it is a pin).
  A numbered position with no lead is `["5", ..., "absent"]` as `[5, "-", "absent"]`.
  **Names** as the datasheet's pin table prints them (ASCII; `EN/UV` fine; no spaces: use `_`). Several pins with the same
  name (`IN` on 2 and 3, `OUT` on 6, 7, 8, a tab) are one node: give them the **same name** and the same role. `NC` /
  `DNC` / `N/C` pins: role `nc`.
  **Roles**: `power` = a switch **input** pin (IN, VIN); `ground` = GND (and PGND, AGND); `output` = a switch output pin (OUT, VOUT) and
  every output-type pin; `input` = an enable or other control input; `inout` = a bidirectional pin; `nc`; `absent`.
  A separate bias or logic supply (VBIAS, VCC, VDD) is `power`, and goes in `analog` (it is not a switch input).
* `type`: `load` (a high-side switch with no current limit, no fault flag required), `limited` (a current-limiting or USB power
  switch: current limit, usually with a fault flag), `ideal-diode` (an ideal diode / ORing part with an internal pass element).
  Take it from the datasheet, not only from `family`.
* `switches`: JSON list of **1 to 4** objects, one per channel in the datasheet's numbering (`IN1`/`OUT1` first), keys:
  `in` (the input pin **name**), `out` (output name), optional `enable` (pin name) with `enableActive` (`"high"` or `"low"`: the level
  that turns the switch ON; read the datasheet's truth table; *default high*) and `enableRef` (`"logic"` default, or `"input"` only when the datasheet
  says the pin is compared with the input voltage, as the LM66100's CE, and it is active low), optional `flag` (a pin name) with
  `flagWhen` (`"fault"` default, or `"disabled"`). A part with no enable pin has none: it is on whenever its input has power.
  Channels that share an input or an output share its name (the TPS2053B's IN1 serves OUT1 and OUT2; the LM66200's two inputs share VOUT and ON).
* `flag`: only an **open-drain, active-low fault / overcurrent / thermal / status** output of that channel (FLG, OC, FAULT, nFAULT, ST).
  `"fault"` if it flags overcurrent or thermal faults; `"disabled"` only when the datasheet says the pin is low while the switch is disabled/off
  (the LM66100's ST). A push-pull output, a power-good, a comparator status such as the LM66200's ST, a current-sense output (IMON, CS), a
  temperature output, a reverse-current flag that is not open-drain active-low: **not a flag**: put its name in `analog`.
* `analog`: JSON list of pin **names** the voltage check skips and the model never drives: slew / soft-start capacitor pins (CT, SS), quick output
  discharge (QOD), current-limit set pins (ILIM, SET, RLIM), bias supplies (VBIAS), mode / config straps, test pins, sense pins, status outputs
  that are not a flag, power-good, `FB`, a thermal-shutdown output… **Every pin of the pinout that is not ground / nc / absent must be named by a switch (in,
  out, enable, flag) or be in `analog`.** `[]` when none. Analog pins keep their electrical role (`input`, `output`, `inout`, `power`).
* `ordering`: a regular expression (Python `re`, matched case-insensitively with `fullmatch` against the **exact part number string** as written in the `parts`
  column, with `_` read as a space) for the orderable part numbers of THIS row. Leave it **empty** when one row covers the whole key and every part has `hit` 1.
  **Give one when** the key needs several rows (pin-out or enable variants told apart by part number), or when some part has `hit` 0 and the datasheet's
  *ordering table / part-number decoder* nevertheless covers it (a part number is covered when the table lists it or its base: `TPS2051BDBVR` is printed as `TPS2051BDBV`
  plus a suffix `R` for tape and reel; `MIC2025-1YM-TR` as `MIC2025-1YM`). A part with `hit` 0 that no `ordering` fully matches is dropped from the catalog
  (`not in the datasheet's ordering table`): do not use a loose regex to rescue it; match only part numbers the datasheet's table supports. If a key has
  several rows, **every part must be selected by exactly one row's `ordering`** (or be a part you deliberately leave unmatched with `hit` 0).
* Facts (numbers, no units): `vin_max` = the **recommended operating maximum input voltage** (V), the absolute maximum only when no recommended one is given;
  `vin_min` = the undervoltage lockout or the recommended minimum input (V); `iout` = the continuous output current **per switch** (A); `ilimit` = the typical
  current limit of a **`limited` part with one fixed value** (A; empty for an adjustable limit, and empty for `load` and `ideal-diode`); `ron` = the typical
  on-resistance at the highest recommended input (ohms); `iq` = the typical quiescent / supply current (A, e.g. `1.32e-06`). A fact the datasheet does not give is
  **empty**. If a part number sets a fixed value (a current-limit option per part number such as `MIC2026-1` 0.5 A / `-2` 1 A) split into rows by `ordering` or
  leave `ilimit` empty and say so in `note`. Ranges: `vin_*` and `iout`/`ilimit` above 0 and at most 100, `ron` at most 10, `iq` at most 1.
* `note`: free text: the datasheet page / table you read the pins from, doubts, anything a reviewer should check.
* `skip`: leave **empty** for a row you read. For a key the kind cannot express, give **one** row with `skip` set to exactly one of
  `control` (switched by anything but a plain enable pin: I²C, SPI, PWM/phase, an analog level, a multi-level setting; also a gate/half-bridge power stage or
  a device that is not a switch), `select logic` (two or more inputs with a select / priority / mode pin deciding which one powers the output: a power mux),
  `external pass element` (a gate pin drives a pass transistor outside the package: an ORing controller, a hot-swap or eFuse controller with an external FET),
  `more than 4 switches`, `negative supply` (a supply below ground), `no pin table` (no pin table and no figure the pinout can be read from). Leave
  `pinout`, `type`, `switches` empty on a skip row and put the reason in detail in `note`. **Ideal diodes with an internal FET that need no select pin are in**
  (two diodes into one output with at most a shared enable, as the LM66200, are `add`). eFuses, hot-swap and power-path controllers with an internal FET and a
  plain enable (and an adjustable current limit pin = `analog`) are in as `limited`; with a PMBus / I²C / SPI interface, a DAC or a gate pin to an external FET they are skips.
  A `limited` switch with an overcurrent **timer pin** or a **mode strap pin** is in (the pin is `analog`).

## How to read a datasheet quickly

`grep -n -i "pin functions\|pin configuration\|pin description\|pin assignment\|ordering\|orderable\|part number" <text>`; the pin tables are near the start; the package
code (DBV, DCK, DRL, D, DGN, DRC, YZP …) of your key is in `package`/`supplier_package` and in the ordering / "Package Options" table (it tells you which
package drawing's pin table applies, and which part-number suffix = which package). The same PDF can have several pin tables (one per package); a figure caption
"Top View" shows the pin numbers. DigiKey's labels: `SC-74A, SOT-753` = SOT-23-5; `SOT-23-6 Thin` = TSOT-23-6; `6-TSSOP, SC-88, SOT-363` = SC-70-6; `8-TSSOP, 8-MSOP … Exposed Pad`
= MSOP/HVSSOP-8 with a pad; `4-XFBGA, DSBGA` = a 4-ball DSBGA/WLCSP (balls A1, A2, B1, B2). For a DSBGA/WLCSP read the ball map and name balls as printed (A1, B1, …).

## Check your work

From the worktree: `P55_OVERRIDES=/tmp/p55/packets/pNN/overrides.tsv python3 -I planning/plan-55-data/census/pinout-check.py --specs /tmp/p55/packets/pNN/out.tsv --all`
→ it prints one line per problem and `<n> rows checked; 0 pinout errors`. It checks the lead count against DigiKey's label, pin numbers, roles, that
every name a switch uses is a pin of the right role, that every pin is used or analog, `enableRef`, and the fact ranges. If DigiKey's label gives a
wrong lead count (e.g. a label counts the pad), write `overrides.tsv` (tab separated, header `pdf	package	supplier_package	count	evidence`) and say why in `note`.

Report back (short): packet, rows written, rows skipped and why, rows you are unsure of (pdf, package, what), overrides written. Do not paste the TSV.
