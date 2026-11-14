# plan-39-D: Review tool, review rules, and the 279 `#` pins

Last updated: 2026-11-13
Effort: large (3h–1d)
Depends on: plan-39-A

D builds what the review (E–K) runs on, and does the part of the data that
needs no reading: the 279 pins whose names already end in `#` get
`"activeLow": true`. Its outcome:

- `scripts/pin-polarity.js`, a Node tool that lists the pinouts to review,
  finds and fetches each one's datasheet, edits a pin's flag in the part
  file in place (no reformatting), records a review in a ledger, and
  checks the ledger and the part files against eight rules.
- The review rules (§ 4): the one definition of "active low" every review
  applies.
- The 279 `#` pins flagged, the goldens regenerated for the 370 parts that
  carry them, and the check green for those pins.

References:

- plan-39-A § 3 (the key), § Prerequisites.
- `scripts/check-datasheets.js` (the precedent: one Node file, `--help`, no
  dependencies), `scripts/digikey.js`.
- `docs/parts-format.md` § Datasheets (the table of parts whose maker no
  longer hosts a datasheet, line 372; the tool reads it), `datasheets.json`
  (`id` → `url`), `datasheets/<group>/<id>.pdf` (3,220 local PDFs:
  `ls datasheets/*/*.pdf | wc -l`).
- `planning/plan-39-data/` (`census.py`: the populations; `slices.py`: the
  device slices E–K review, with `--devices` indices; `no-local-pdf.tsv`,
  `url-probe.sh`, `url-probe.txt`: the 165 URL-only datasheets and their
  reachability).
- Memory notes the tool's fetch step follows: silabs.com refuses automated
  fetches, onsemi PDFs need bare `curl`, Rochester/MaxLinear/Infineon have
  URL workarounds (`~/.claude/projects/-Users-justinzaun-Development-logic/memory/datasheet-download-blocks.md`).

## Prerequisites

plan-39-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-39-A complete | `grep -c 'active_low' backend/include/parts.h` → 1, and A's phases are ticked | MET (2026-11-14: 1; A Phases 1–2 ticked) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command before you continue. If you stop, report all
> prerequisites.

## 1. Goal

- `node scripts/pin-polarity.js check` exits 0 on a tree whose reviewed
  pinouts match the ledger, and says which rule failed otherwise.
- `grep -c '"activeLow":true' schematical/schematical/Parts/{sram,eeprom,cpu,converter}.json` — 147 + 81 + 43 + 8 = 279 flagged pins, every pin whose name ends in `#`.
- The tool has a test file that passes (`node --test scripts/pin-polarity.test.js`).

### Non-goals (explicit constraints)

- The tool does not decide polarity; a reviewer does. It never writes a flag
  except through `set` and `seed-hash`.
- The tool never reformats a part file: an edit changes only the one pin's
  line.
- No pin name changes, no role changes.
- D reviews no datasheet: the 279 seeded flags get their ledger rows in E–K
  (each device's review confirms or corrects its seeded flags).

## 2. Current State

- Every pin object of the 19 in-scope kinds is on one line
  (`python3 -I planning/plan-39-data/census.py` → no `MULTI-PIN LINES` line
  for them): `{"number":20,"name":"CE#","role":"input"},`. The tool edits by
  text on that line.
- `datasheets.json` has `{"format","version","datasheets":[{"id","url"}…]}`
  (6,789 ids). 3,220 PDFs are checked in under `datasheets/<group>/`.
- Of the 2,129 in-scope pinouts: 1,652 have a local PDF among their parts'
  datasheets, 454 have only URLs (165 ids, `planning/plan-39-data/no-local-pdf.tsv`),
  20 name no datasheet (their parts are in the "no longer hosts" table,
  each with a local PDF), 3 have no part (a sibling pinout of the same
  device is the source): `python3 -I planning/plan-39-data/census.py` →
  `{'a local PDF': 1652, 'no datasheet …': 20, 'no part': 3, 'URL only': 454}`.
- All 165 URL-only datasheets answered a ranged GET with a browser
  User-Agent: 24 × 200, 141 × 206, 0 failures
  (`cut -d' ' -f1 planning/plan-39-data/url-probe.txt | sort | uniq -c`).

### Measured populations

| What | Count | Command |
|---|---|---|
| In-scope kinds / devices / pinouts / signal pins | 19 / 1,739 / 2,129 / 41,484 | `python3 -I planning/plan-39-data/census.py` → `IN-SCOPE TOTAL` |
| Out-of-scope kinds (no signal pin name can be active low) | 10: 7seg, capacitor, charlcd, connector, diode, gate, header, led, supply, switch | same → `out of scope kinds and their signal-pin names` (every name is a letter/number label, a polarity-free connector or passive pin) |
| Pins to seed from `#` names | 279 | same → `pins with a trailing #` |
| Parts whose golden line changes when they are seeded | 370 (eeprom 174, sram 172, cpu 17, converter 7) | `python3 -I planning/plan-39-data/census.py` → `parts whose golden line changes` |
| URL-only datasheet ids (first-listed per pinout) | 165 | same → `distinct first-listed URL-only ids: 165`; `awk 'END{print NR}' planning/plan-39-data/no-local-pdf.tsv` → 165 |
| Pinouts with no datasheet at all / no part | 20 / 3 | same → `no datasheet … 20`, `no part 3` |
| sram/eeprom control pins (R6) not ending in `#` | 0, so R6 holds once the `#` pins are seeded | same → `sram/eeprom pins named by chipEnable/… whose name does not end in #` → `0 []` |

## 3. Design Overview

**Review unit: the pinout** (decided by the owner: one datasheet per
pinout). A device's pinouts can differ in names (72 of 342 multi-pinout
in-scope devices: census last block), so flags are per pinout, never copied
across by device. A pin is matched to the datasheet by its position
(number/ball) in the datasheet's pin table for that package, not by name text.

**Source order** for a pinout (`sources` prints it):

1. a datasheet id of one of the pinout's parts, ordered: ids not ending in
   `-q1`/`-ep` first, then local before URL, then ASCII id order;
2. for a pinout whose parts name no datasheet: the local PDF the "no longer
   hosts" table assigns to its parts (`docs/parts-format.md` § Datasheets);
3. for a pinout with no part: a sibling pinout of the same device that already
   has a ledger row (`sibling:<pinout>`).

The ledger's `source` column holds the id, `local:datasheets/<group>/<file>.pdf`,
or `sibling:<pinout>`.

**Ledger** `planning/plan-39-data/ledger.tsv`, tab separated, header
`kind device pinout source pages flagged note`, rows sorted by kind, device
(ASCII), pinout (file order). `pages` is non-empty text (`p.4`, `pp.3-5`);
`flagged` is the pinout's flags as compact JSON, `{}` for none:
`{"CE#":true,"A0/CS":["CS"]}`.

**Rules `check` enforces** (`--kind K` and `--devices A-B` limit the rows
examined; `--complete` makes R7 apply inside that scope, and R7 always
applies without a scope):

| Rule | Violation |
|---|---|
| R1 | a ledger row names no in-scope pinout, repeats one, has an empty `pages`, or its `source` does not resolve (id not in `datasheets.json`, `local:` path missing, `sibling:` pinout without a row) |
| R2 | a ledger row's `flagged` differs from the flags in the part file |
| R3 | a flag is on a non-signal pin, lists a string that is not an alternative, lists every alternative, or is `false`/`[]` (the loader's rules, re-checked on the files) |
| R4 | a name alternative ending in `#` is not flagged (all shipped pins, not just those under review) |
| R5 | within one device, two pins with the same name and role are flagged differently (in pinouts that have rows) |
| R6 | for `sram` and `eeprom` devices, a pin named by the device's `chipEnable`, `outputEnable`, `writeEnable` or `byteEnable` is not flagged (those keys are documented active low: `docs/parts-format.md` lines 1136–1139 and 1205) |
| R7 | an in-scope pinout has no ledger row (runs without `--kind`/`--devices`, or in a scope when `--complete` is given) |
| R8 | a flag exists in an out-of-scope kind's file |

**`seed-hash`** sets `true` on every in-scope pin whose single alternative
ends in `#` (all 279; the census shows none has `/`).

**`golden-check [--since REV]`** proves a golden regeneration only added
flags: it reads `git diff -U0 [REV] -- backend/tests/golden/parts.txt` (REV
defaults to HEAD, the working-tree change), strips `:low` and `:low=…`
from every changed line, and requires the removed lines to equal the added
lines one for one; it also requires `traces.txt` and `footprints.txt`
unchanged. Exit 1 otherwise.

**Where risk is.** `set`'s text edit (a corrupted JSON line would break
loading, which `check` and the backend tests catch) and the source finder
(a wrong datasheet read). The tests build a fixture tree and cover both.

**Gate class.** Behaviour-adding tool, plus a data change whose only effect
is `:low` in golden lines (`golden-check`).

**Rejected alternatives.** *`JSON.parse` + `JSON.stringify` to edit* —
reformats every pin line of the file; the diff would be unreadable.
*A Python tool* — the repo's scripts are Node. *Fetching all 165 PDFs in D*
— the reviews fetch what they read; `fetch` caches under `/tmp`.

## 4. The review rules

Every review in E–K applies these; there is nothing to decide beyond them.

1. **Active low means the datasheet says so.** A signal is active low when
   the datasheet draws its name with an overbar, writes it with the
   datasheet's own negation marker (`#`, `/`, `~`, `_N`, `_B`, a leading or
   trailing `N`/`B` that the datasheet defines as "active low"), or its pin
   description says active low, low-true, or asserted/active when low.
2. **Name patterns are not evidence.** `AIN0N` (negative input), `OUTN`
   (negative differential output), `A/B`, `R/W` and `SDA` are not active low
   because of how they look. In `R/W` with an overbar over `W` only, the
   flag is `["W"]`.
3. **Clocks.** A falling-edge clock is flagged only when the datasheet marks
   the name active low (an overbar on `CP`); edge sensitivity alone does not
   flag it.
4. **Open-drain and open-collector outputs** follow rule 1 (an `IRQ` marked
   active low is flagged; an unmarked open-drain output is not).
5. **Programmable polarity.** Use the datasheet's pin-table marking, which is
   the reset default.
6. **Multiplexed names.** Flag each alternative the datasheet marks active
   low: `true` when all are, a list otherwise.
7. **Roles are not edited.** Only input, output and inout pins take a flag.
   If the datasheet marks a power, ground or nc pin active low, say so in
   `note` and leave the flag off.
8. **Pin matching.** Match the catalog pin to the datasheet's pin table by
   number or ball for that package. A name that differs between catalog and
   datasheet is matched by position.
9. **Reading overbars.** `pdftotext -layout -f N -l M <pdf> -` drops overbars.
   Where the text does not say "active low", render the page:
   `pdftoppm -r 150 -f N -l N -png <pdf> /tmp/pin-polarity/pages/<id>` and look
   at the image. Overbars are the evidence; record the page in `pages`.
10. **A `#` pin the datasheet shows active high** is a catalog error: rename
    the pin in the part file (and every kind key, test and doc that names it),
    leave it unflagged, record it under Corrections of that sub-plan, and
    commit the rename separately.
11. **Where the flag disagrees with a `#` name seeded by D**, the reviewer's
    reading wins only through rule 10.
12. **The flag describes what the catalog name denotes.** A pin the catalog
    names for its port or general function (`PC6`) is not flagged because
    the datasheet's name for the same pin has a marked alternative the
    catalog name omits (`PC6/RESET`); `RESET` named pins are flagged.

## 5. Phases

> **NOTE — keep the checkboxes current as you go.** Tick in the same commit
> as the work; `- [~]` partial; moot tasks stay with a reason; fill `Commit:`
> when a phase lands; an unticked box means NOT DONE.

### Phase 1 — the tool

- [x] `scripts/pin-polarity.js`: `--help` text; global `--root <dir>`
      (default the repo); subcommands `list`, `sources`, `fetch`, `pins`,
      `set`, `record`, `check [--kind K] [--devices A-B] [--complete]`,
      `status`, `seed-hash`, `golden-check [--since REV]` as § 3 defines.
      `list [--kind K] [--devices A-B] [--todo]`: one line per
      pinout, tab separated `kind device pinout signalPins parts
      todo|done`, kinds in the in-scope list's order, devices in ASCII id
      order, `--devices` 1-based inclusive indices into that sorted device
      list. `fetch <id>`: download to `/tmp/pin-polarity/<id>/<id>.pdf` with
      `curl -L` and a browser User-Agent, retry once with bare `curl`, accept
      only a file that starts `%PDF`, print the path; an existing file is
      reused. `set <device> <pinout|all> <name> <true|ALT[,ALT…]|off>`:
      edits the matching pin lines by text, writes the canonical form
      (`true` when the list covers every alternative), refuses a non-signal
      pin. `record <device> <pinout> --source S --pages P [--note N]`:
      computes `flagged` from the file and upserts the row, keeping the file
      sorted. — done: all ten subcommands; also `set … @designator` selects a pin by number/ball (rule 8), and `set`/`record`/`seed-hash` hold a lock so parallel reviewers cannot race; `fetch slas107` → `/tmp/pin-polarity/slas107/slas107.pdf` (starts `%PDF`)
- [x] `scripts/pin-polarity.test.js` (`node:test`): builds a fixture root in
      a temp dir (two kinds: one with a `#` pin, one with a multiplexed
      name; one device with two pinouts; one pinout with no part; one
      `docs/parts-format.md` with a "no longer hosts" row) and asserts: `list`
      ordering and `--devices`; `sources` order (non-`-q1` first, local
      first) and the three source kinds; `set` changes exactly one line
      (byte diff of the file is that line) and writes `true` for a full
      list; `set` refuses a power pin; `record` writes the JSON `flagged`;
      each of R1–R8 fails on a crafted tree and passes on the fixed one;
      `golden-check` passes when only `:low` was added and fails when
      anything else moved, with and without `--since`. — done: 13 tests, `ℹ pass 13`, `ℹ fail 0`

Acceptance: the tool's own tests pass.
  Check: `node --test scripts/pin-polarity.test.js` → all pass (est. 1 min).
  Observed 2026-11-14: `ℹ tests 13`, `ℹ pass 13`, `ℹ fail 0`.
Commit: b089e5f

### Phase 2 — seed the `#` pins and regenerate the goldens

- [x] `node scripts/pin-polarity.js seed-hash` (writes 279 flags into
      `sram.json`, `eeprom.json`, `cpu.json`, `converter.json`). — done:
      `cpu: 43`, `eeprom: 81`, `sram: 147`, `converter: 8`, `seeded 279 pin(s)`.
- [x] Build the backend (`cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8`)
      and regenerate `backend/tests/golden/parts.txt`:
      `/tmp/logic-p39/listing_check --print > backend/tests/golden/parts.txt`. — done.
- [x] `docs/backend.md` § Tests: one sentence — `scripts/pin-polarity.js`
      is the review tool and `golden-check` the golden gate. — done (after the
      golden regeneration block).

Acceptance: 279 pins flagged, 370 golden lines changed by `:low` only,
nothing else moved, every test green.
  Check: `grep -oE '"activeLow": ?true' schematical/schematical/Parts/{sram,eeprom,cpu,converter}.json | wc -l` → 279 (corrected: Corrections 1); `git diff --numstat backend/tests/golden/parts.txt` → `370 370`; `node scripts/pin-polarity.js golden-check` → exit 0; `node scripts/pin-polarity.js check --kind sram` → R4 and R6 clean for sram (R7 is skipped when scoped); `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && ctest --test-dir /tmp/logic-p39` → 100 % passed (est. 8 min: the whole backend suite is the only check that loads every edited part file through the loader and the simulator).
  Observed 2026-11-14: grep → 279; numstat `370	370	backend/tests/golden/parts.txt`;
  golden-check `ok: 370 parts.txt line(s) changed, only by :low`; `check --kind
  sram` → `ok: 37 pinouts` (also eeprom 27, cpu 8, converter 858: ok);
  ctest `100% tests passed, 0 tests failed out of 34`.
Commit: —

## Validation Plan

- Tests: `scripts/pin-polarity.test.js`; `library_check` already covers the
  loader; the backend suite loads every flagged part file.
- Coverage check: the test's R1–R8 cases each fail first on a crafted tree
  (so a rule that never fires is caught).
- Runtime proof: Phase 2's `golden-check`.
- Doc sync: `docs/backend.md` (Phase 2).
- Final gate: plan-39-L, once.

## Decisions

- One datasheet per pinout — owner, 2026-11-13. A device's pinouts are
  reviewed separately because their names can differ.
- `#` names keep their names and get the flag — owner, 2026-11-13.
- Author's calls: Node tool with a text-edit `set`; the ledger is a TSV with
  JSON in one column; sources in the order of § 3; the review rules of § 4
  including that polarity is the datasheet's marking, not a name pattern.

## Corrections

1. **Not every in-scope pin is on its own line** (§ 2 and A's Verified
   properties said so). `converter.json` has 20,570 pins and `rtc.json` 152
   whose object spans several lines (`"role": "output"` on its own line):
   `grep -c '^ *"role": *"[a-z]*",\{0,1\}$' schematical/schematical/Parts/<kind>.json`.
   The census's `MULTI-PIN LINES` test (lines holding `"role"` vs `"role"`
   count) cannot see this. Nothing rested on it: `set` and `seed-hash` edit
   by parsed offsets inside the pin's own object and copy that object's
   separator style, so a multi-line pin gets `,\n      "activeLow": true`
   and nothing else in the file moves (converter.json diff: 8 pins, 16
   lines added, 8 changed). The Phase 2 grep was miscalibrated by the
   spacing: `"activeLow":true` found 271; `"activeLow": ?true` finds all 279.
2. **Additions to the tool** beyond § 3, both needed for E–K: `set` takes
   `@<designator>` in place of a name (rule 8 matches by position), and the
   writing commands take a lock so reviews of different devices can run in
   parallel against one tree (E § 3 allows fanning out).

## Summary

The tool carries the whole review's bookkeeping; its risk is a wrong source
or a corrupted edit, both tested. The only data D changes is the 279 `#`
flags, which golden-check proves are display data.
