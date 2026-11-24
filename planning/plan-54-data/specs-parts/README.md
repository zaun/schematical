# plan-54-B / C / D: filling `specs.tsv` — the reader's brief

Everything below is for one *reader*: you fill the lines of one fragment file
`planning/plan-54-data/specs-parts/<name>.tsv` (a slice of `specs.tsv`; `python3 -I planning/plan-54-data/census/assign.py --list`
names every fragment). All paths are relative to the worktree root
`/Users/justinzaun/Development/logic/.claude/worktrees/P-54` (use absolute paths in tool calls).

## What this is for

The library gets one `connector` part for every DigiKey-listed card slot (memory / SIM card socket, DIMM / SO-DIMM
module socket, card-edge socket) that its maker's datasheet really lists. For each group of rows that share a
(maker, series, PDF) — a *spec line* — you read the placed datasheet PDF and write: the **ordering-code scheme** (a
regex over the part number), the device's **label**, its **card** text, the **pinout** (pin names), the **positions**
(counts the datasheet lists), the **pitch** and a **status**. A script then checks every part number against your
regex and your facts: a part that is not in the datasheet's ordering table, or whose DigiKey position count or pitch the
datasheet does not give, is *skipped* (not added to the library). So:

- a regex that is **too loose** adds parts the datasheet does not list;
- one that is **too tight** drops real parts;
- **do not tune a regex to make DigiKey's part numbers match.** If a part number does not fit what the datasheet's
  ordering key allows, that part is correctly skipped: leave the regex true to the key and say so in your report.

## Your file

Edit your fragment in place: 13 tab-separated columns, header exactly
`family maker series pdf rows example scheme label card pinout positions pitch status`. Columns 1–6 are the key and the
skeleton: **never change them**. You fill columns 7–13 on every line. Keep tabs as tabs (no spaces), one line per
spec line, no quotes around cells. **A PDF that holds more than one scheme** (two card types, two pinouts, two
mountings with different keys, a series whose pin count changes the pinout) needs **several lines with the same first
four columns**: duplicate the line and give each its own `scheme`, `pinout`, `positions`, … so that every part number
matches exactly one line (a part number that two lines of one key both match is an error).

The datasheets: the `pdf` column is a path under `datasheets/connector-*/<maker>/`. Read them with
`pdftotext -layout <pdf> -` (via Bash). A PDF that gives no text (a drawing or a scan: `pdftotext` prints almost
nothing) is read with the Read tool's PDF support (`pages` = "1-3", at most 20 pages a call), or with
`pdftoppm -r 200 -png -f 1 -l 2 <pdf> <prefix>` then `tesseract <prefix>-1.png - ` for the text. A multi-page catalog
covers many series: find your series' pages first.

`python3 -I planning/plan-54-data/census/spec-rows.py planning/plan-54-data/specs-parts/<name>.tsv` prints, per spec line,
every candidate part number with DigiKey's facts (`pos` DigiKey's Number of Positions, `pitch` mm, `rows`, mounting, card
type, `hit`: 1 means the part number is printed verbatim in the PDF, and the DigiKey `variant`). DigiKey's facts
are the *carrier*; the datasheet is the *authority*.

## The columns (7–13)

| Column | What |
|---|---|
| `scheme` | Python regex that `re.fullmatch`es **every part number the datasheet's ordering key / part-number table allows for this line**. Options are groups listing the codes the key lists (`SIM8055-6-[01]-14-0[01]-A`). A position-count field is the set of counts the key lists. A part number may carry a packing suffix (`-TR`, `-T`, `/TR`…): look at the key *and* at the real part numbers to see how each position is spelled. For a catalog that lists *whole part numbers in a table* (Amphenol, TE, Molex): an alternation of the listed numbers, or a regex over the dash numbers the table lists. `-` means the PDF has no ordering key or table at all (then only the parts printed verbatim in it, `hit` 1, count) — use it only after you looked. The regex is **case-sensitive**; DigiKey part numbers are upper case. |
| `label` | the device's short name: `<series> <card>` (`SIM8055 Nano SIM`, `MEC8 edge`, `PCIe x4`, `DDR4 SODIMM 260`). |
| `card` | the text the sidebar shows. **Card sockets**: the standard — `SD`, `microSD`, `SIM` (full size), `Micro SIM`, `Nano SIM`, `Mini SIM`, `Smart card`, `SIM / SAM`, `microSD / UFS` (DigiKey's Card Type with `™` dropped; the datasheet decides). **Module sockets**: `<generation> <style>` from the datasheet (`DDR4 SODIMM`, `DDR5 DIMM`, `DDR3 SODIMM`, `SIMM`, `MiniDIMM`, `SOCAMM`); the style alone when the datasheet names no memory generation. **Card-edge sockets**: `PCI Express`, `M.2`, `MicroTCA`, `MXM 3.0`, `Mini PCI Express`, `Dual edge` or `Single edge` for a plain edge connector (one or two rows of contacts, DigiKey's "Non Specified"); a datasheet's own name wins (`SATA`, `PCI`, `AGP` …). |
| `pinout` | **how the pins are named** — one of: `seq` (pins `1` … `N`, named `1` … `N`: the datasheet numbers the contacts 1 … N); `AB` (pins `A1` … `A<N/2>` then `B1` … `B<N/2>`, N even: the datasheet labels the two sides A and B); or an **explicit list** `1=NAME,2=NAME,…` giving the pad (the datasheet's pin number) and the name of **every electrically distinct pad, in the datasheet's numbering**. Names: for a **card socket** the signal names as the datasheet prints them (SD: `DAT2`, `CD/DAT3`, `CMD`, `VDD`, `CLK`, `VSS`, `DAT0`, `DAT1`; SIM: `VCC`, `RST`, `CLK`, `GND`, `VPP`, `IO`; and the datasheet's detect / write-protect switch contacts as it names them, `CD`, `WP`, `COM`, `DETECT`), `NC` for a contact the datasheet marks not connected; a **module socket** `seq` (the contacts as the drawing numbers them, front side then back side); a **card-edge socket** `seq` or `AB` unless the datasheet's pin table prints signal names (PCI Express, M.2: then the list with the printed names; do not invent names). **Shield**: when the datasheet connects the metal shell / shield tabs, add one last pin `SHIELD` (pad number: the next free one, or the datasheet's). Mounting pegs, board-lock holes, shell-less hold-down posts are **not** pins. Names are unique except `NC`, `GND`, `VSS`, `SHIELD` may repeat. Pin count must equal DigiKey's positions (`pos=`) for every part of the line, **plus one if you list `SHIELD`** and DigiKey's number leaves the shell out; DigiKey writes a card socket's positions as `10 (8 + 2)` = contacts + switch contacts (`pos` is the 10). |
| `positions` | DigiKey's position counts that the datasheet lists for this line: `a,b,c-d,e-f/s` (`c-d` every integer in the range, `e-f/s` every s-th: `20-100/2` is 20,22,…,100). `*` only if the datasheet gives no list (then not checked). |
| `pitch` | the contact pitch in mm from the datasheet: `0.6`, `1.0`, `0.8,1.0` for several; `-` for a **card socket** (no pitch is kept). For module and card-edge sockets always read it. Rounding is fine (`0.635` for 0.64). |
| `status` | `;`-separated words: `active` (the usual); `obsolete` (the datasheet marks the part obsolete, discontinued, NRND, "not recommended for new designs", last time buy or end of life — those rows are skipped; **say so in your report with the page**); `edge-mount` (the connector straddles the board edge — DigiKey's mounting type `Board Edge, Straddle Mount`, contacts on both faces of the board; also say which pad rows are on the bottom face if the drawing shows it); `footprint-skip:<feature>` when the datasheet's land pattern needs a **rotated (non-axis-aligned) slot** or a **keyhole / L-shaped shell slot** (not expressible: those rows are skipped; pads on the other face and an edge-straddling body are NOT reasons to skip); `pins n, digikey m` when the datasheet confirms a pin count that differs from DigiKey's; `note:<free text>` for anything else (everything after `note:` is ignored by the scripts). |

## Procedure

1. Run `spec-rows.py` on your fragment once and keep the output.
2. For each spec line (group by PDF — one PDF often serves several lines; a catalog PDF covers many series): read the
   ordering key / part-number table, the pin table or drawing (and, for the status, the cover or revision text);
   fill columns 7–13.
3. Run the checks (below) and iterate. For every `skip` the checker prints, decide: regex too tight → fix it; the
   table really does not allow it → leave it.
4. Report (see the end).

## Checks (run them; they are your feedback loop)

```
python3 -I planning/plan-54-data/census/scheme-check.py --only-specs --list planning/plan-54-data/candidates.tsv planning/plan-54-data/specs-parts/<name>.tsv
python3 -I planning/plan-54-data/census/pinout-check.py --list planning/plan-54-data/specs-parts/<name>.tsv
python3 -I planning/plan-54-data/census/scheme-check.py --only-specs --sample planning/plan-54-data/candidates.tsv planning/plan-54-data/specs-parts/<name>.tsv
```

The first prints, per row not `ok`, `skip | mpn | series | reason | detail` (`not in the datasheet's ordering table` or
`fact not in datasheet` with the fact), and ends with `<n> rows: <k> ok, 0 unchecked` — `0 unchecked` means every spec
line has all its cells filled and no part matches two lines. The second must end `<n> rows, 0 bad` (it lists every part
whose pin count differs from your pinout). The third prints 20 random `ok` rows per maker with their regex: open the PDF
for a handful and check that the key allows them.

## Constraints

- Edit **only** your fragment file (and scratch files in
  `/private/tmp/claude-501/-Users-justinzaun-Development-logic/8343ba34-242d-4698-8fa3-e4a4af725824/scratchpad/`).
  Do not edit `specs.tsv`, `candidates.tsv`, any script, or anything else in the tree.
- No git commands (do not even write that word in a command), no network, no DigiKey calls. Do not delete or move PDFs.
- The shell here refuses heredocs, loops and some compound commands: put any script you need in a file with the Write
  tool (under the scratchpad above) and run it with `python3 -I <file>`; use plain, separate commands.
- Do not spend effort on parts DigiKey lists that the datasheet does not: skip is the right outcome and costs nothing.
- Do not guess: a pinout you cannot confirm from text or drawing is reported (below), with what you assumed.

## Report (your final message)

Keep it short and exact:

1. The final output of the checks (counts).
2. Every spec line where you are **not** sure (a pinout you could not confirm from text or drawing — say which pads and
   what you assumed; an ordering key you could not read; a PDF that does not describe this part at all), one line each:
   `series | pdf | what`.
3. The lines that stay `skip` after your regexes (count per series and why you believe the key excludes them) — in two
   lines, not a list of every part number unless there are fewer than 15.
4. Everything the datasheets say about obsolete / discontinued / NRND / end of life / last time buy for a series you read
   (series, page, the words), and every line you marked `edge-mount` or `footprint-skip:`.
