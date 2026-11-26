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

- [ ] `scripts/tests/` modules, fixtures and the fake `kicad-cli` (§ 3).
- [ ] `docs/testing.md`: the Python test command and `SCRIPTS_COVERAGE_DIR`.
- [ ] `COV --scripts --write-ratchet`: the Python rows gone.

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

(none yet)

## Summary

Small and self-contained: four test modules and one line of
`part-editor-schema.py`; no app or backend code touched.
