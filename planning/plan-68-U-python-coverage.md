# plan-68-U: Scripts — the Python at 100%

Last updated: 2026-11-25
Effort: medium (1h–2h)
Depends on: plan-68-T

The five Python files in `scripts/` have no tests. This letter adds
`unittest` tests under `scripts/tests/` that run every line of them, measured
by the standard library's `trace` module through `COV --scripts`
(plan-68-A § 3.3). Nothing outside `scripts/` changes.

Prerequisites: plan-68-A's table, and plan-68-T complete (`grep -c '\.js'
scripts/coverage-ratchet.tsv` → 0).

References:

- `scripts/erc-compare.py` (136 code lines), `kicad-compare.py` (703),
  `kicad-pin-types.py` (106), `part-editor-schema.py` (380),
  `part-editor-schema-extra.py` (40, data read by `part-editor-schema.py`).
  Counts: `node planning/plan-68-data/loc.js scripts/*.py`.
- `docs/export.md` and `planning/plan-63-data/` (what `erc-compare.py` and
  `kicad-compare.py` compare, and their inputs).

## 1. Goal

- `python3 -m unittest discover -s scripts/tests` passes.
- `COV --scripts` reports no uncovered line in the five files; their ratchet
  rows are gone.

### Non-goals

- No script's command line or output changes (the one-line `compile` change
  in § 3 keeps execution identical). No pip package.
- Tests never run KiCad: where a script shells out (`erc-compare.py` 8 and
  `kicad-compare.py` 3 `subprocess`/`kicad-cli` references, `grep -cE
  'subprocess|kicad-cli|KiCad.app'`), the test puts a fake `kicad-cli` first
  on `PATH` (a small Python script in `scripts/tests/fakes/` that prints a
  recorded output from `scripts/tests/fixtures/`), or the script's path to the
  tool is passed by the argument it already has.

## 2. Current State

No Python tests exist (`grep -l unittest scripts/*.py` → none). The files are
hyphen-named, so tests load them with `importlib.util.spec_from_file_location`
(`kicad-pin-types.py:30` already loads `kicad-compare.py` this way) and also run each as a program (`subprocess.run([sys.executable, path, …])`)
for the `__main__` block. `trace` follows child Python processes only when
they are started under it, so `COV --scripts` runs the program-style cases
as `python3 -m trace --count --coverdir=<dir> <script>` through an
environment variable the tests read (`SCRIPTS_COVERAGE_DIR`), and merges the
`.cover` counts (`coverage.js` does the merge; plan-68-A Phase 3 tested its
`.cover` parser).

## 3. Design

One test module per script in `scripts/tests/`: `test_erc_compare.py`,
`test_kicad_compare.py`, `test_kicad_pin_types.py`,
`test_part_editor_schema.py` (which also covers `part-editor-schema-extra.py`,
a data module it imports). Each:

- unit cases for the script's functions on small fixtures in
  `scripts/tests/fixtures/<script>/` (a KiCad `.kicad_sym` with two symbols, a
  netlist of two parts, a one-kind part file …), with expected values read off
  the fixture by hand and written in the test;
- one program-style case per command-line mode, checking the exit code and
  the printed lines;
- error cases for each argument check and file-read failure the script has.

`part-editor-schema.py` generates Swift (`KindSchema`, written to
`<repo>/schematical/schematical/Editor/<Name>Draft.swift` by `generate(repo,
kind, extra)`); its test calls `generate` with `repo` = a temporary directory
holding symlinks to the real `schematical/schematical/Parts` and
`backend/src` and an empty `schematical/schematical/Editor/`, then compares
each written file with the committed one — so the test also proves the
committed files are current, and never writes into the checkout.

`part-editor-schema.py:445` reads its data file with
`exec(open(_extra).read())`, which compiles the text under the file name
`<string>`, so `trace` cannot attribute those lines to
`part-editor-schema-extra.py`. It becomes `exec(compile(open(_extra).read(),
_extra, "exec"))`: the same execution, with the real file name on the code
object. That one line is the only change to a script in this letter.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — the four test modules

- [x] `scripts/tests/` modules, fixtures and the fake `kicad-cli` (§ 3).
  `test_erc_compare.py`, `test_kicad_compare.py`, `test_kicad_pin_types.py`,
  `test_part_editor_schema.py`, plus `support.py` and `traced.py` (Corrections
  2), `fakes/kicad-cli`, `fixtures/{erc-compare,kicad,kicad-pin-types}/`.
  Observed: `python3 -m unittest discover -s scripts/tests` → `Ran 74 tests …
  OK (expected failures=1)`; `LOGIC_COV_WORK=/tmp/logic-cov-U node
  scripts/coverage.js --scripts` → no `.py` path:line, `scripts: 6744 lines, 0
  uncovered, 100.00%`; `--only scripts/kicad` → 734 lines, `--only
  scripts/part-editor` → 390, `--only scripts/erc` → 110, each 0 uncovered.
- [x] `docs/testing.md`: the Python test command and `SCRIPTS_COVERAGE_DIR`.
  The command was already in the table; § Coverage now says how the suite is
  traced, with the variable's real name `SCRIPTS_COVERAGE_FILE` (Corrections 1).
- [x] `COV --scripts --write-ratchet`: the Python rows gone (the parent
  removes them in this letter's commit). Observed: `python3 -m unittest
  discover -s scripts/tests` → `Ran 74 tests`, `OK` (no expected failure,
  after the generator fix below); `COV --scripts` → `scripts: 6750 lines, 0
  uncovered, 100.00%`.

Acceptance: every Python line in scope is run and checked.
  Check: `python3 -m unittest discover -s scripts/tests` → `OK`; `COV
  --scripts --only .py` → no uncovered line (est. 1 min).
Commit: —

## Validation Plan

- Final gate (once): `python3 -m unittest discover -s scripts/tests`; `COV
  --scripts --check` → exit 0 with no Python row; `SZ` (est. 1 min).

## Decisions

- (author) Standard library only (`unittest`, `trace`), a fake `kicad-cli` on
  `PATH` (§ 3).

## Corrections

1. (2026-11-26) The variable is `SCRIPTS_COVERAGE_FILE`, not
   `SCRIPTS_COVERAGE_DIR`, and it names one `trace` counts file, not a
   `--coverdir`: `scripts/coverage.js` runs `python3 -m trace --count
   --file=<counts> --no-report --module unittest discover -s scripts/tests`
   with `SCRIPTS_COVERAGE_FILE=<counts>`, then `trace --report --missing`.
2. (2026-11-26) That command saves no counts. Python 3.14's `trace.main()`
   calls `write_results` (the only place the counts file is written) only
   without `--no-report`; checked: `python3 -m trace --count --file=cnt
   --no-report hello.py` leaves no `cnt`. The first run of `COV --scripts`
   with only `test_erc_compare.py` showed every line of `erc-compare.py`
   outside module level uncovered (81 of 110). `coverage.js` is not this
   letter's file, so the tests do it: `scripts/tests/support.py`, when
   `SCRIPTS_COVERAGE_FILE` is set, takes the running `Trace` object from
   `sys.gettrace().__self__` (its class is `__main__.Trace`, not
   `trace.Trace`) and merges its counts into the file at exit. A
   program-style case runs through `scripts/tests/traced.py`, which counts the
   child into the same file. `python3 -m trace <script>` was not usable for the
   child: it catches `SystemExit`, so every exit code would read 0. Do not
   also drop `--no-report` from `coverage.js` while `support.py` saves: both
   would add the suite's counts (lines stay covered, counts double), and
   without `--coverdir` `trace` would write `.cover` files beside the sources.
3. (2026-11-26) Also seen in `coverage.js`'s Python step: `trace` swallows
   the `unittest` run's `SystemExit`, so a failing Python test still exits 0
   and `COV --scripts` does not notice it. Documented in `docs/testing.md`;
   the fix (check the suite separately, or read the summary) is
   `coverage.js`'s, not this letter's.
4. (2026-11-26) Not every committed Draft equals the generator's output, so
   § 3's "compares each written file with the committed one" holds for 36 of
   the 50 kinds in `KIND_FILES`, not all:
   - 12 Drafts were edited by hand after generation (their header says "Edit
     by hand freely"): counter, dekatron, dff, gate, nixie, pld, powerswitch,
     serial, 7seg, shift, socket, switch (`diff` of generated vs committed: 2
     to 84 changed lines each; `GateDraft.swift` adds `enum SchemaValues`).
     `test_hand_edited_drafts_still_differ_from_the_generator` asserts they
     still differ, so the list cannot go stale silently.
   - converter and rtc cannot be generated today: `generate` raises
     `ValueError: cannot add under ['commands']`. `part-editor-schema-extra.py`
     (`CV_EXTRA`) places `wordBits` under `protocol.commands`, which the
     shipped converter parts now infer as `listOf(valueMap)`, not a list of
     objects (the committed `ConverterDraft.swift` has
     `.listOf(.object([... wordBits]))`, edited by hand). Not fixed here: any
     fix changes the generated schema, which is the owner's call. The test is
     `@unittest.expectedFailure`, so it turns into an "unexpected success"
     when the generator is fixed.
5. (2026-11-26) `part-editor-schema.py` lost a second piece besides the
   `compile` line: `class Ctx` (5 lines), never constructed (`grep -n Ctx
   scripts/*.py scripts/tests/*.py` → only its definition; `git log
   -S'Ctx('` → no commit that called it). An unreachable line is deleted
   (AGENTS.md § Source size and coverage). Output unchanged: the 36 Drafts
   still equal the committed files.
6. (2026-11-26) `kicad-compare.py` and `kicad-pin-types.py` read the parts
   from the module constant `PARTS_DIR`; the in-process cases patch it to
   `scripts/tests/fixtures/kicad*/Parts`, and the one program-style case of
   each reads the real library (with the recorded KiCad library: a fraction
   of a second, no KiCad needed).
- (parent) **The converter/rtc generator failure was the plan-68-J split.**
  `kind_files` listed only `KIND_FILES`' own sources, so the keys in the
  pieces split out of them (`converter_load_proto.c` holds the command keys)
  were not "known", and `is_object` read 19 unknown command keys as a map.
  `kind_files` now adds each file's pieces (the files `<stem>_….c` that
  include `<stem>_int.h`, as PartKindKeysTests does); the header still names
  the kind's own files. Then converter and rtc generate, and counter and
  shift equal their committed Drafts too (they were in HAND_EDITED only
  because of the missing keys): HAND_EDITED is 10 kinds, the expected
  failure is gone, and `test_split_pieces_are_read_but_not_listed` checks
  the rule. `ConverterDraft.swift` and `RtcDraft.swift` are regenerated: new
  `.suggested` values from parts added since, and rtc's `timing` is no
  longer required (the loader reads it as optional; DS3231M and PCF8563 ship
  without it, so the form marked required a key two shipped parts omit).
  Checked in a snapshot of HEAD plus the two Drafts: `XB -only-testing`
  PartKindRoundTripTests, WizardKindStepsTests, PartKindKeysTests → 55
  tests, 0 failures.
- **(parent) One commit.** The letter landed in one commit.

## Summary

Small and self-contained: four test modules and one line of
`part-editor-schema.py`; no app or backend code touched.
