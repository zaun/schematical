# plan-39-A: Active-low pins — the `activeLow` pin key in the part format and backend

Last updated: 2026-11-13
Overall Effort: huge (> 3d) — plan-39 is A → B → C → D → E → F → G → H → I → J → K → L
Effort: medium (1h–2h)
Depends on: nothing (§ Prerequisites)

plan-39 marks the active-low pins of the part library and draws them with an
overline. It has three halves:

1. **The flag** (A, B). A pin in a pinout may carry `"activeLow"`; the C
   loader keeps it in `PartPin`, the app's `ChipPin` carries it, the Part
   Editor edits it, agent control reports it.
2. **The drawing** (C). A flagged pin's name is drawn with an overline in
   every block symbol, on screen, in the raster export and in the SVG export.
3. **The data** (D–L). Every pinout of the 1,739 devices in the 19 kinds
   that can have an active-low pin is reviewed against a datasheet and
   flagged, with a ledger that proves each pinout was reviewed.

A is the format and the loader. Its outcome: a part file whose pin says
`"activeLow": true` or `"activeLow": ["CS"]` loads, a malformed one is
rejected with a message that names the problem, `listing_check` prints the
flag, and `docs/parts-format.md` describes it. No shipped part has the flag
yet, so every golden file is byte-identical after A.

References:

- `docs/parts-format.md` § Pinouts and pins (the pin key table, line 104).
- `backend/src/library/library.c` (`PIN_KEYS`, the pin loader around
  `pin->side`), `backend/include/parts.h` (`PartPin`).
- `backend/tests/library_check.c` (the loader's rules; the `side` cases near
  line 1828 are the precedent), `backend/tests/support/listing.c` (the pin
  column of `golden/parts.txt`).
- `docs/backend.md` § Tests (build and golden-regeneration commands).
- `AGENTS.md` § Git (itemized commits, never touch a branch).

## Prerequisites

These are a precondition on the whole feature, not a dependency to negotiate.
Every other letter points here.

| Must be true | Command | Status |
|---|---|---|
| No plan-39 file but A–L is in `planning/` | `ls -d planning/plan-39-* \| grep -v -E 'plan-39-([A-L]-\|data)' \| wc -l` → 0 | MET (2026-11-14: 0) |
| `node`, `pdftotext`, `pdftoppm`, `cmake`, `xcodebuild` exist | `which node pdftotext pdftoppm cmake xcodebuild \| wc -l` → 5 | MET (2026-11-14: 5) |
| The part files hold no `activeLow` key yet | `grep -c '"activeLow"' schematical/schematical/Parts/*.json \| grep -v ':0$' \| wc -l` → 0 | MET (2026-11-14: 0) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify.
>
> **If you stop, report the current status of *all* prerequisites.**

## 1. Goal

- A pin object in a pinout takes `"activeLow"`: `true` (the whole name is
  active low) or a list of strings (those slash-separated alternatives of
  the name are active low; § 3). The loader stores it in `PartPin` as a bit
  mask of alternatives, and rejects every malformed use with a message.
- `listing_check` prints the flag, so the goldens record it.
- `docs/parts-format.md` documents the key.

### Non-goals (explicit constraints)

- No shipped part gets the flag in A: `git diff --stat backend/tests/golden`
  is empty after A.
- No pin is renamed. Names, kind keys, the 279 existing `#` names, and every
  saved schematic (wires name pins by designator, `docs/file-format.md`) are
  untouched.
- The simulator does not read the flag. It is display data; polarity that
  matters to a model stays in the kind keys (`chipEnable`, `"active": 0`, …).
- No change to the `.schematical` file format.

## 2. Current State

- `PIN_KEYS` (`backend/src/library/library.c:245`) is `number ball name role
  group side`; an unknown pin key is rejected by the loader's key check.
- The pin loader (`library.c:352–400`) reads `name`, `role`, `group`, `side`
  into a `PartPin` (`backend/include/parts.h:46–49`: `designator name role
  group side`). `side` is the precedent: an optional key, validated against
  the role (`"side" is for input, output and inout pins`).
- `backend/tests/support/listing.c:273–275` prints each pin as
  `designator:name:role:group[:in|:out]`; `golden/parts.txt` has one line per
  part (`wc -l backend/tests/golden/parts.txt` → 29633).
- Pin names join functions with `/` (1,806 of the library's pins:
  `python3 -I planning/plan-39-data/census.py` → alternatives per name 1:
  64648, 2: 1672, 3: 133, 4: 1), and 279 pins end in `#` (§ Measured).

### Measured populations

| What | Count | Command |
|---|---|---|
| Parts, devices, pinouts, all kinds | 29,633 parts, 2,576 devices, 3,169 pinouts | `python3 -I planning/plan-39-data/census.py` → `TOTAL` |
| Pins whose name ends in `#` | 279 (sram 147, eeprom 81, cpu 43, converter 8) | same → `pins with a trailing #` |
| Names with `#` not at the end, or `#` with `/` | 0 | same → `names with # not at the end` |
| Most alternatives in one name | 4 (`AIN7/GPIO3/DRDY/CLK`) | same → `alternatives per name` |
| Non-signal pins (power, ground, nc) with `#` in the name | 0 | same → `non-signal pins whose name has #` |

### Verified properties

| Claim | How verified |
|---|---|
| Every pin object of the in-scope kinds is on its own line, so D's tool can edit a pin by text without reformatting a file | `census.py` → `MULTI-PIN LINES` prints only charlcd, connector, header (all out of scope), none for the 19 in-scope kinds |
| Part files with unknown pin keys are rejected, so the key must be added to `PIN_KEYS` and to the Swift `PartDraft.pinKeys` | read `library.c:245`, `PartDraft.swift:152` (B adds the second) |
| The pin loader sees the pin's `name` and `role` before it reads `side`, so the new key validates against both | read `library.c:358–396` |

## 3. Design Overview

**The key.** `"activeLow"` on a pin object:

- `true`: every alternative of the name is active low.
- a non-empty list of strings: exactly those alternatives are active low.
  *Alternatives* are the name split at `/`: `"A0/CS"` has `A0` and `CS`; a
  name with no `/` has one. Each string must equal an alternative as written
  (a trailing `#` included). A string matches every equal alternative. The
  list may not contain every alternative (write `true`) and may not repeat a
  string.
- absent: no alternative is active low. `false` and `[]` are errors, so a
  file has one spelling of "not active low".
- Only `input`, `output` and `inout` pins take it (a power, ground, nc or
  absent pin is never active low).
- A name with more than 32 alternatives takes `true` only (the mask is 32
  bits); the library's maximum is 4.

**The struct.** `PartPin` gains `uint32_t active_low`: bit *i* set means
alternative *i* (0-based, left to right) is active low. `true` sets one bit
per alternative. `0` is the default.

**The listing.** After `side`, `listing.c` appends `:low` when every
alternative is flagged and `:low=A;B` (the flagged alternatives, `;`
separated, name order) when some are. Pins without the flag print as today,
so only flagged parts' golden lines change, and only from D onward.

**Where risk is.** The loader's error cases (a wrong message, a missed
rejection). The data is added in D–K, not here.

**Gate class.** A is behaviour-adding with no data change; the gate is the
goldens being byte-identical (`git diff --stat backend/tests/golden` empty)
plus `library_check`'s new cases.

**Rejected alternatives.**

- *A `"polarity": "low"` string.* Same information, but cannot say which
  alternative of `A0/CS`; the owner chose per-alternative flags.
- *Stripping `#` from names and using the flag alone.* Renames 279 pins that
  kind keys, tests and agent scripts name; the owner chose to keep names.
- *Deriving the flag from kind keys (`chipEnable`, …).* Covers a fraction of
  the pins and no datasheet is read; the owner asked for a datasheet review.

## 4. Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work. `- [~]` for partial (say what remains). A moot task is
> `- [x] ~~text~~ — moot: <evidence>`. Fill `Commit:` the moment a phase
> lands. An unticked box means NOT DONE.

### Phase 1 — loader, struct, listing

- [x] `backend/include/parts.h`: add `uint32_t active_low;` to `PartPin`
      with the comment "bit i: alternative i of `name` (split at `/`) is
      active low; 0 = none". — done: parts.h:49
- [x] `backend/src/library/library.c`: add `"activeLow"` to `PIN_KEYS`; after
      the `side` block of the pin loader, read it when present: `true` →
      mask of all alternatives; an array → the bits of the listed
      alternatives; set `pin->active_low = 0` when absent. Errors (each via
      `kind_error`, text exactly):
      `"activeLow" is for input, output and inout pins`;
      `"activeLow" must be true or a list of the name's alternatives`
      (false, a string, a number, an object, an array holding a non-string);
      `"activeLow" is empty`; `"activeLow" has no alternative "X" in "NAME"`;
      `"activeLow" lists "X" twice`;
      `"activeLow" lists every alternative of "NAME"; write true`;
      `"activeLow" is for names of at most 32 alternatives`. — done: `load_active_low` in library.c; all 7 messages seen in library_check output
- [x] `backend/tests/support/listing.c`: print `:low` / `:low=A;B` after the
      `side` field (§ 3). — done: `print_active_low` in listing.c
- [x] `backend/tests/library_check.c`: one accepted and one rejected case per
      error above, plus: `true` on a one-alternative name → mask 1; `true`
      on `A0/CS` → mask 3; `["CS"]` on `A0/CS` → mask 2; `["CE#"]` on `CE#`
      → rejected (lists every alternative); a pin without the key → mask 0;
      a power pin with it → rejected. — done: `check_pin_active_low_cases`: 7 accepted + 12 rejected, all `ok`
- [x] `backend/tests/part_file_test.c`: one single-part-file case with a
      flagged pin loads and `part->pins[i].active_low` matches. — done: case (j), `ok (j) activeLow loads from a single part file`

Acceptance: the loader accepts and rejects exactly as § 3 says, and the
goldens do not move.
  Check: `cmake -S backend -B /tmp/logic-p39 && cmake --build /tmp/logic-p39 -j8 && ctest --test-dir /tmp/logic-p39 -R '^(library_check|part_file_test|listing_check|trace_check)$' && git diff --stat backend/tests/golden` → 4 of 4 passed, diff stat empty (est. 6 min: first build of the backend; no smaller check builds the loader).
  Observed 2026-11-14: `100% tests passed, 0 tests failed out of 4`; `git diff --stat backend/tests/golden` empty.
Commit: —

### Phase 2 — format documentation

- [ ] `docs/parts-format.md` § Pinouts and pins: add the `"activeLow"` row to
      the pin key table (after the `side` row, line 131) and a paragraph under it giving § 3's
      rules (true, list, alternatives, roles, `#` strip is drawing-only),
      with a worked example: `{"number": 20, "name": "CE#", "role": "input",
      "activeLow": true}` and `{"number": 5, "name": "A0/CS", "role":
      "input", "activeLow": ["CS"]}`.
- [ ] `docs/backend.md` § Tests: the `listing_check` pin column now has
      `:low` / `:low=A;B`; one sentence.

Acceptance: both docs state the key, its two forms, and the print format.
  Check: `grep -c 'activeLow' docs/parts-format.md docs/backend.md` → at
  least 4 and 1 (est. 1 min; a doc edit needs only a grep).
Commit: —

## Validation Plan

- Tests: `library_check` (every accept/reject case), `part_file_test`
  (single-file round trip), `listing_check` (the print path is exercised by
  D–K once flags exist; A has no flagged shipped part).
- Coverage check: `library_check` and `part_file_test` are in
  `backend/CMakeLists.txt` (`grep -c 'library_check\|part_file_test' backend/CMakeLists.txt` → ≥ 2).
- Runtime proof: none beyond the tests; no app behaviour changes in A.
- Doc sync: `docs/parts-format.md`, `docs/backend.md` (Phase 2).
- Final gate: plan-39-L runs the landing tier once. A's own gate is the
  Phase 1 check.

## Decisions

- Keep the 279 `#` names, add the flag, and draw the overline with the
  trailing `#` stripped — decided by the owner, 2026-11-13. No pin is
  renamed. (§ 1, § 3)
- The flag is per alternative: `true` or a list of the name's slash
  alternatives — decided by the owner, 2026-11-13. (§ 3)
- Each pinout is reviewed against one datasheet — decided by the owner,
  2026-11-13. (plan-39-D § 3)
- Author's calls: `false` and `[]` are errors (one spelling per meaning); the
  mask is 32 bits; only signal-role pins take the flag; the simulator ignores
  the flag.

## Corrections

(empty until execution)

## Summary

A is a small loader change with a wide test surface. The risk is a wrong
rejection message or a missed rejection; the data and the drawing come later.
