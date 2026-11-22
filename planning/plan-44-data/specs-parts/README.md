# plan-44-B: filling `specs.tsv` — the reader's brief

Everything below is for one *reader*: you fill the spec rows of one fragment
file `planning/plan-44-data/specs-parts/<name>.tsv` (a slice of
`specs.tsv`). Six fragments exist: `abracon-1`, `abracon-2`, `ecs-1`,
`ecs-2`, `ecs-3`, `suntsu`. All paths are relative to the worktree root
`/Users/justinzaun/Development/logic/.claude/worktrees/P-44` (use absolute paths
in tool calls).

## What this is for

The library gets one part per DigiKey-listed crystal / ceramic resonator the
datasheet really lists. For every group of download-set rows that share a
(maker, series, PDF, package) — one *spec row* — you read the placed datasheet
PDF and write: the **ordering-code scheme** (a regex over the part number),
the **pinout**, and the **facts** the datasheet gives (frequency range, load
capacitances, tolerances, stabilities, ESR). A script then checks every part
number against your regex and your facts: a row that is not in the datasheet's
ordering table, or has a fact the datasheet does not give, is *skipped* (not
added to the library). So:

- a regex that is **too loose** adds parts the datasheet does not list;
- one that is **too tight** drops real parts;
- **do not tune a regex to make DigiKey's part numbers match.** If a part
  number does not fit what the datasheet's table allows, that part is
  correctly skipped: leave the regex true to the table and say so in your
  report.

## Your files

- Fragment (edit this, in place): `planning/plan-44-data/specs-parts/<name>.tsv`
  — 17 tab-separated columns, header exactly
  `maker series pdf package rows example scheme label pinout terminals capacitors page hz_range loads tols stabs esr_max`.
  Columns 1–6 are the key and the skeleton: **never change them**. You fill
  columns 7–17 on every row. Edit with the Edit/Write tools; keep tabs as tabs
  (no spaces) and one line per spec row; no quotes around cells.
- The datasheets: the `pdf` column is a path under `datasheets/crystal/<maker>/`.
  Read them with `pdftotext -layout <pdf> -` (via Bash) — and with the Read
  tool's PDF support (`pages`) when a table or drawing is an image and the text
  extraction is empty.
- The rows each spec row must cover: `python3 -I planning/plan-44-data/census/spec-rows.py planning/plan-44-data/specs-parts/<name>.tsv`
  prints, per spec row, every download-set part number with DigiKey's facts
  (`hz`, `load`, `tol`, `stab`, `esr` in ohms, supplier package, `hit`: 1 means
  the part number is printed verbatim in the PDF, class `khz` / `mhz` /
  `resonator`). DigiKey's facts are the *carrier*; the datasheet is the
  *authority*.
- The full candidate table is `planning/plan-44-data/candidates.tsv` (rarely
  needed).
- The column rules are in the docstring of `planning/plan-44-data/census/specs.py`
  — read it first (also restated below).

## The columns (7–17)

| Column | What |
|---|---|
| `scheme` | Python regex that `re.fullmatch`es **every part number the datasheet's ordering / part-numbering table allows for this series and package**. Options are groups listing the codes the table lists. A frequency spelled as number + unit in the part number (`32.768KHZ`, `8.000MHZ`, `4M`) is a group named `hz`: `(?P<hz>\d+(?:\.\d+)?(?:KHZ|MHZ))` (restrict it to the table's frequency range when the table gives one); the script parses it and demands it equal the row's `hz`. A part number that spells the frequency as a code (e.g. `AWSCR-800CV`) uses a group `hzc` and appends a map after a `;` — `<regex>;800=8000000,455=455000` (so the regex itself must not contain `;`). `-` means the PDF has no ordering table at all (then only rows printed verbatim, `hit` 1, count) — use it only when you looked and there is none. The regex is **case-sensitive** and DigiKey part numbers are upper case (`KHZ`, `MHZ`). Part numbers may carry a packing suffix (`-T`, `-TR`, `-T3`, `-T5`, `-RL`…), a leading series or a `-xxx` variant code: look at the datasheet's table *and* at the real part numbers to see how its positions are spelled. |
| `label` | one of `SMD`, `HC-49/US`, `HC-49/U`, `HC-46/X`, `SOJ`, `Cylinder`. From DigiKey's Package / Case (the `package` column): every `2-SMD…`, `3-SMD…`, `4-SMD…` and `1512 (3731 Metric)` → `SMD`; `HC-49/US`, `HC-49/US - 3 Lead`, `HC-49/US - 4 Lead` → `HC-49/US`; `HC-49/U` → `HC-49/U`; `HC-46/X` → `HC-46/X`; `4-SOJ…` → `SOJ`; `Cylindrical Can, Radial` → `Cylinder`. A package of `-` (rare): the datasheet's package family from this list. |
| `pinout` | the **whole pin list in pad order**, `<pad>=<name>` joined by commas: names are `X1`, `X2` (the crystal's two terminals), `NC` (a pad that is not connected / only case ground — the datasheet may say "GND or NC"; write `NC`), and `GND` (only the common middle pad of a **three-pad** ceramic resonator or three-lead part). Exactly one `X1` and one `X2`. Examples: two-pad `1=X1,2=X2`; four-pad SMD crystal `1=X1,2=NC,3=X2,4=NC` (use the datasheet's pad table / drawing: in most four-pad crystals pads 1 and 3 are the crystal, 2 and 4 are NC or case ground, but check; if pads 1 and 2 are the crystal, `1=X1,2=X2,3=NC,4=NC`); three-pad resonator with built-in capacitors `1=X1,2=GND,3=X2`; a four-lead HC-49/US or a SOJ-4: as the four-pad crystal. The pin count must equal a pad count DigiKey's package names (`2-SMD`→2, `4-SMD`→4, `3-SMD`→3, `HC-49/US - 3 Lead`→3, `4-SMD, Flat Lead, 3 Leads`→4 or 3, `4-SOJ`→4; a bare `HC-49/U`, `HC-49/US`, `HC-46/X`, `Cylindrical Can` → 2) unless the datasheet clearly shows otherwise (then say so in your report). |
| `terminals` | always `X1,X2` |
| `capacitors` | `1` only when the datasheet says the part has **built-in load capacitors** (a three-pad ceramic resonator, e.g. AWSCR-…, ECS-CTP…), else `0` |
| `page` | the datasheet page(s) of the ordering table and the electrical table, e.g. `3` or `2,4` |
| `hz_range` | the frequencies the datasheet gives for this series (in the part's package), `min-max` in **Hz**; several ranges comma-separated (`3500000-4999999,5000000-70000000`); a fixed frequency `32768-32768`. `*` only if the datasheet gives none (then not checked). |
| `loads` | load capacitances in **pF** the datasheet offers (numbers, `a-b` ranges, comma-separated); `-` stands for **series resonance** (DigiKey shows a blank load for those) and is part of the list when the datasheet offers a series-resonant option; `*` = the datasheet gives none (not checked). A resonator with built-in capacitors: use what the datasheet states (often `30` or a range); if it states none, `*`. |
| `tols` | frequency tolerances in **ppm** the datasheet offers (numbers / ranges, e.g. `10,20,30,50` or `10-100`); `*` = none given |
| `stabs` | frequency stabilities over temperature in **ppm** the datasheet offers; `*` = none given. (A resonator's one tolerance figure such as ±0.5 % = 5000 ppm goes in `tols`; if DigiKey's `tol` is in ppm convert the datasheet's percent to ppm: 0.5 % = 5000, 0.3 % = 3000, 0.1 % = 1000.) |
| `esr_max` | the **largest** ESR (resistance, for a resonator: impedance) in **ohms** the datasheet gives for the series, over every frequency / option. A datasheet in kΩ is multiplied by 1000 (`95 kΩ` → `95000`). `*` = none given. |

Facts are checked against DigiKey's numbers: a datasheet that gives *typical*
instead of *max* ESR, or a table with several tolerance grades, still gives
the set / ceiling to use. When DigiKey's number would plainly be outside what
you wrote *only because the datasheet table is silent for that option* (e.g.
the datasheet lists a load capacitance up to 20 pF and DigiKey says 32 pF for a
frequency the table does not cover) the row is correctly skipped; do not widen
the lists to include it.

## Procedure

1. Run `spec-rows.py` on your fragment once and keep the output.
2. For each spec row (group the rows by PDF — one PDF often serves several
   spec rows of the same series with different packages; a PDF can also cover
   several series): read the PDF's ordering table / part-numbering guide,
   electrical table and pin / pad table; fill columns 7–17.
3. Run the checks (below) and iterate. For every row reported `skip`, decide:
   regex too tight → fix it; the table really does not allow it → leave it.
4. Report (see the end).

## Checks (run them; they are your feedback loop)

```
python3 -I planning/plan-44-data/census/scheme-check.py --only-specs --list planning/plan-44-data/candidates.tsv planning/plan-44-data/specs-parts/<name>.tsv
python3 -I planning/plan-44-data/census/pinout-check.py planning/plan-44-data/specs-parts/<name>.tsv
python3 -I planning/plan-44-data/census/scheme-check.py --only-specs --sample planning/plan-44-data/candidates.tsv planning/plan-44-data/specs-parts/<name>.tsv
```

The first prints, per row not `ok`, `skip | mpn | series | reason | detail`
(`not in the datasheet's ordering table` or `fact not in datasheet` with the
fact), and ends with `<n> rows: <k> ok, 0 unchecked` — `0 unchecked` means
every spec row has all the cells filled. The second must end `<n> rows, 0 bad`.
The third prints 20 random `ok` rows with their regex: open the PDF for a
handful and check that the table allows them.

## Constraints

- Edit **only** your fragment file (and scratch files in
  `/private/tmp/claude-501/-Users-justinzaun-Development-logic/ede5c4a1-c37d-46ad-aa3b-165359fd7ae6/scratchpad/`).
  Do not edit `specs.tsv`, `candidates.tsv`, any script, or anything else in the tree.
- No git commands, no network, no DigiKey calls. Do not delete or move PDFs.
- The shell here refuses heredocs, loops and some compound commands: put any
  script you need in a file with the Write tool (under the scratchpad above) and
  run it with `python3 -I <file>`; use plain separate commands.
- Do not spend effort on parts DigiKey lists that the datasheet does not: skip
  is the right outcome and costs nothing.

## Report (your final message)

Keep it short and exact:

1. The final output of the three checks (counts).
2. Every spec row where you are **not** sure (a pinout you could not confirm
   from text or drawing — say which pads and what you assumed; an ordering table
   you could not read; a PDF that does not describe this part at all), one line
   each: `series | package | what`.
3. The rows that stay `skip` after your regexes (count per series and the
   reason you believe the table excludes them) — in two lines, not a list of
   every part number unless there are fewer than 15.
4. Anything in the datasheets that says obsolete, discontinued, NRND or end of
   life for a series you read.
