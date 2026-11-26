# plan-68-A: Size and coverage rules, and the checks that enforce them

Last updated: 2026-11-25
Overall Effort: huge (>3d)
Effort: large (3h–1d)
Depends on: nothing (the Prerequisites below)

plan-68 adds two standing rules to `AGENTS.md` and then brings the whole tree
into line with them:

1. **No source file is over 1000 lines of code.** A line of code is a line
   that has something other than whitespace and comments on it.
2. **All code is covered by tests, 100% of its lines.** A change that adds
   lines adds the tests that run them, in the same commit.

The trigger is `SchematicDocument` (`schematical/schematical/Canvas/SchematicModel.swift`,
7,676 lines, 5,606 of them code): one class owns the schematic and the board,
and both multi-sheet and multi-unit work cut through it. It is one of 36 files
over the limit. Every over-limit file is broken into logical units that can be
tested on their own, and every line of first-party code (app, backend, scripts)
ends up run by a test.

**This letter delivers the rules and the checks**, and nothing is split or
tested yet: the two rules in `AGENTS.md`, `docs/testing.md`, a size checker
that runs inside `ctest`, a coverage tool that measures all three tiers, and
two ratchet files that hold today's violations so the checks pass now and can
only get stricter. Later letters remove the ratchet rows; the last letter
deletes the ratchet mechanism itself.

| Letter | Delivers | Effort |
|---|---|---|
| A | the rules, `docs/testing.md`, `scripts/source-size.js` + `source_size_check`, `scripts/coverage.js`, the two ratchet files, the basicsim test | large |
| B | app: the value types out of `SchematicModel.swift` | medium |
| C | app: `SchematicDocument` covered to 100% before it is taken apart | medium |
| D | app: the schematic sheet model | large |
| E | app: the board model | large |
| F | app: the document core; `SchematicModel.swift` gone | large |
| G | backend: the serial test files split into test programs | large |
| H | backend: the other nine over-limit C test files split | large |
| I | backend: the five over-limit serial model files split | large |
| J | backend: the other eleven over-limit backend files split | large |
| K | backend: every allocation can be made to fail; every out-of-memory path tested | large |
| L | backend coverage: library, engine, circuit, ERC, voltage | large |
| M | backend coverage: converters and the RTC | large |
| N | backend coverage: the ns16550 core, ext, LPT and Exar | large |
| O | backend coverage: the bridges and serial targets | large |
| P | backend coverage: the other UARTs and the serial plumbing | large |
| Q | backend coverage: the logic models | large |
| R | backend coverage: memories, passives, power, timers and displays | large |
| S | backend coverage: CPUs, PIOs, AVR, MCU, PLD | large |
| T | scripts: `datasheet-edit.js` split; the JavaScript at 100% | large |
| U | scripts: the Python at 100% | medium |
| V | app: the six other over-limit app files split, and `DXFFixtures.swift` | large |
| W | app coverage: Backend, Routing, Simulation, Memory, Boards, DRC, BOM, DocumentInfo, Help | large |
| X | app coverage: Registers, Cpu, Terminal, Automation | large |
| Y | app coverage: ElectricalRulesCheck, Document | large |
| Z | app coverage: Document (exports), Datasheet, BoardPreview | large |
| AA | app coverage: the app root, Preferences | large |
| AB | app coverage: Preferences, Drawer | large |
| AC | app coverage: Drawer, Editor | large |
| AD | app coverage: Editor | large |
| AE | app coverage: Editor (the Part Editor model) | large |
| AF | app coverage: Editor (the wizard) | large |
| AG | app coverage: Editor (wizard pages), Canvas | large |
| AH | app coverage: Canvas (sidebars and the canvas views) | large |
| AI | app coverage: Canvas (`PCBSidebarView` and its split pieces) | large |
| AJ | app coverage: Canvas (symbols and the rest) | large |
| AK | landing: the ratchets deleted, both checks strict, the final gates | medium |

**The dependency graph**: two tracks after A, which share no files and
may run side by side, then one chain.

- App track: A → B → C → D → E → F.
- Backend track: A → G → H → I → J → K → L → M → N → O → P → Q → R → S.
- Then T (needs S: every backend coverage row gone) and F → … T → U → V →
  W → X → Y → Z → AA → … → AJ → AK.

So T waits for both F and S. Within a track, each letter depends on the one
before it. The document work (B–F) comes first after the
rules, ahead of the lower-risk backend splits: it is the reason for the plan
(multi-sheet and multi-unit both wait on it) and it needs nothing from the
backend letters; its risk is held by letter C, which covers the whole
document before D–F move any of it. The app coverage letters are sized at
1,372–1,825 uncovered lines each from `planning/plan-68-data/app-missed.tsv`.

References:

- `AGENTS.md` (the whole file: the new section goes after § Git).
- `docs/backend.md` § Tests (line 865: the `ctest` command and the test list).
- `backend/CMakeLists.txt` (`logic_test`, `enable_testing`, the `find_program(LOGIC_NODE node)` precedent at the `cpu_core_test` block).
- `basicsim/CMakeLists.txt`, `basicsim/main.c`.
- `scripts/pin-polarity.test.js` (the `node --test` + `spawnSync` test style the new script tests copy).
- `planning/plan-68-data/` — the measurements this plan was written from (below).

Test command forms used by every letter:

- `CT`: `cmake -S backend -B /tmp/p68-build && cmake --build /tmp/p68-build -j8 && ctest --test-dir /tmp/p68-build -j8`. Measured 2026-11-25 at `e2f6811a` (instrumented): `100% tests passed, 0 tests failed out of 77`, 33.4 s of tests.
- `XB`: `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p68-dd CODE_SIGNING_ALLOWED=NO test`; a Check writes `XB -only-testing:schematicalTests/<Class>`. Debug, scoped to the classes named. The host is the real app: run it under a time limit (`perl -e 'alarm 1800; exec @ARGV' …`) and never point a test document at a file the test did not write.
- `NT`: `node --test scripts/*.test.js`. Measured 2026-11-25 at `e2f6811a`: `tests 56 … pass 56 … fail 0`, 7.9 s.
- `SZ`: `node scripts/source-size.js --check` (built in this letter).
- `COV`: `node scripts/coverage.js --check [--backend|--app|--scripts]` (built in this letter).

## Prerequisites

These are a precondition on the whole of plan-68, not a dependency to
negotiate. Letters B–Z point here.

| Must be true | Command | Status |
|---|---|---|
| plan-48 (timer / op-amp) is merged to `main`. Its worktree edits `PartCatalog.swift`, `SchematicCanvasView.swift`, `library_check.c` and `backend/CMakeLists.txt`, which this plan splits; the owner ruled it lands first. | `git log --oneline -1 --grep='plan-48-I: landing'` names a commit on `main`, and `ls planning/plan-48-*.md` → no matches (archived) | MET (re-run 2026-11-25 at plan start, `63e2f7d2`: `4d8148bb plan-48-I: landing gates met…`; `ls planning/plan-48-*.md` → no matches) |
| The backend tests pass on the starting tree | `CT` → `100% tests passed` | MET (re-run at plan start, `63e2f7d2`: `100% tests passed, 0 tests failed out of 77`) |
| The plan number is free | `ls planning planning/completed \| grep -c '^plan-68'` → only this plan's files | MET (re-run at plan start: 38 = the 37 plan files + `plan-68-data`) |
| Xcode's coverage tools are present | `xcrun --find llvm-cov` and `xcrun --find llvm-profdata` and `xcrun --find xccov` each print a path | MET (re-run at plan start: all three print a path) |
| node has the built-in test coverage | `node --help \| grep -c experimental-test-coverage` → `1` | MET (re-run at plan start: `1`) |

Everything below is written against the world where these hold.

Every count in this plan was measured at `e2f6811a`, after plan-48 merged
(plan-48's merge changed none of the over-limit list: the census before and
after names the same 36 files). Splits are described by section titles and
symbol names, not line numbers, so later commits that move lines do not move
the design; a line number that has moved is not a correction.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue, and again
> before you decide to stop. Never act on a status you did not just verify — a
> prerequisite recorded NOT MET may have landed since, and one recorded MET may
> have regressed.
>
> **If you stop, report the current status of *all* prerequisites** — not only
> the one that blocked you.

## 1. Goal

- `AGENTS.md` carries the two rules, and `docs/testing.md` says how to measure
  both.
- `SZ` exits 0 on the tree and is part of `CT` (the `source_size_check` test).
  It fails when any in-scope file not in the ratchet is over 1000 code lines,
  when a ratchet file grew past its recorded count, or when a ratchet row is
  stale (its file is now at or under 1000, or gone).
- `COV` measures line coverage of the app, the backend and the scripts, prints
  every uncovered line, and with `--check` exits 0 only when no file has more
  uncovered lines than its row in the coverage ratchet (a file with no row may
  have none).
- `basicsim` runs again and is a `ctest` test.

### Non-goals (explicit constraints)

- No source file is split, and no behavior of the app, the backend or a script
  changes, except `basicsim` (which fails today, § 5) and the new test entries.
- `CT`'s time grows only by `source_size_check` (one node run over `git
  ls-files`; the prototype counts all 936 candidate files in 0.42 s).
  Coverage is never run by `CT` or `XB`; it is its own command (owner's ruling).
- No new dependency: the size checker and the coverage tool use node, Xcode's
  `llvm-cov`/`llvm-profdata`/`xccov` and Python's standard library only.

## 2. Current State

There is no size rule and no coverage measurement anywhere in the repo
(`grep -rni coverage AGENTS.md docs/*.md backend/CMakeLists.txt
schematical/schematical.xcodeproj/xcshareddata/xcschemes/schematical.xcscheme`
→ no output). The scheme's `TestAction` has no `codeCoverageEnabled`.
`node --test` already runs four script test files (`NT`).

### Measured populations

All files are in `planning/plan-68-data/`. The counter is
`planning/plan-68-data/loc.js` (the prototype of `scripts/source-size.js`;
§ 3.1 is its algorithm).

| What | Count | Command |
|---|---|---|
| In-scope source files (§ 3.2) | 895 | `wc -l < planning/plan-68-data/census-in-scope.tsv` |
| Code lines in them | 240,621 | `awk -F'\t' '{s+=$1} END{print s}' planning/plan-68-data/census-in-scope.tsv` |
| In-scope files over 1000 code lines | 36 (68,508 code lines): 24 production, 12 test | `cat planning/plan-68-data/over-limit.tsv`; `grep -cE 'Tests/\|/tests/' …` → 12 |
| Vendored/generated files over 1000 (exempt) | 6: `wdc65816_generated_opcodes.c` 47,654, `m6502.h` 2,478, `cJSON.c` 2,467, `z80.h` 2,463, `wdc65816_core_generator.js` 2,449, `w65c02.h` 2,407 | `awk -F'\t' '$1>1000' planning/plan-68-data/census.tsv \| grep -E 'third_party\|jsmooch-gen'` |
| `SchematicModel.swift` | 7,676 lines, 5,606 code | `wc -l`; `node planning/plan-68-data/loc.js schematical/schematical/Canvas/SchematicModel.swift` |
| Backend own code (`backend/src`, not `third_party`): line coverage under `CT` | 88.30%: 60,333 lines, 7,057 uncovered, in 115 of 132 files | `planning/plan-68-data/backend-coverage.txt` (the `llvm-cov report` of the instrumented `CT`); `backend-missed.tsv` per file; `backend-missed-by-letter.tsv` assigns each file to its coverage letter |
| Script JavaScript under `NT` | `datasheet-edit.js` 35.04%, `gen-sockets.js` 98.00%, `gen-terminals.js` 100%, `pin-polarity.js` 91.72%; `datasheet.js`, `digikey.js`, `check-datasheets.js`, `gen-headers.js`, `jsmooch-gen/run.js`, `avr8js-oracle/oracle.js` have no tests (0%) | `node --test --experimental-test-coverage scripts/*.test.js` |
| Script Python | 5 files in `scripts/`, 0 tests | `ls scripts/*.py`; `grep -l unittest scripts/*.py` → none |
| App Swift line coverage (`schematical/schematical/**/*.swift`) under the full Debug suite | 75.99% per line: 49,627 executable lines, 11,916 uncovered, in 246 files (`SchematicModel.swift`: 221 of 5,420) — see Corrections: the first figure written here (69.45%, 22,860) was `xccov --report`'s per-function line total | `bash planning/plan-68-data/scov.sh`, then `python3 planning/plan-68-data/appcov.py`; `app-coverage.txt`, `app-missed.tsv` |
| Full Debug suite | 2,078 tests in 1,634 s wall; 10 failures on the measured tree, listed in `app-coverage.txt` (the standing set every later gate compares against) | same run |
| Backend allocation calls (`malloc`/`calloc`/`realloc`/`strdup`) in `backend/src`, not `third_party` | 194 | `grep -rnoE '\b(malloc\|calloc\|realloc\|strdup)\(' backend/src --include='*.c' \| grep -v third_party \| wc -l` |

### Verified properties

- **node's test coverage includes child processes.** `pin-polarity.test.js`
  runs the tool only through `childProcess.spawnSync('node', [TOOL, …])`
  (`scripts/pin-polarity.test.js:163`), and the coverage report gives
  `pin-polarity.js` 91.72% with a precise uncovered-line list. So a script that
  is only ever run as a program (oracle.js, run.js, coverage.js itself) is
  covered by a test that spawns it.
- **An instrumented `CT` works as-is.** Configuring with
  `-DCMAKE_C_FLAGS="-fprofile-instr-generate -fcoverage-mapping"
  -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"` and running `ctest`
  with `LLVM_PROFILE_FILE=<dir>/%p.profraw` built in 6 s and tested in 33 s,
  77/77 passing; `xcrun llvm-profdata merge -sparse` + `xcrun llvm-cov report
  -object <each test program>` gave the per-file table
  (`planning/plan-68-data/bcov.sh` is the exact script).
- **`basicsim` fails today.** `cmake -S basicsim -B /tmp/p68-basicsim && cmake
  --build /tmp/p68-basicsim --target basicsim && /tmp/p68-basicsim/basicsim`
  prints `gate.json: devices[0] (74_00): parts[0] (5962-8403701VCA): unknown
  datasheet "sn74hc00"` / `Could not create simulation` and exits 1: it loads
  `gate.json` without loading `datasheets.json` first, which the loader has
  required since datasheet names became references.
- **The basicsim project already builds the whole backend and its tests**:
  `basicsim/CMakeLists.txt` does `add_subdirectory(../backend backend)`, so
  configuring `-S basicsim` builds every `logic_test` program too.
- **App coverage needs an unsigned build.** The app target has
  `ENABLE_APP_SANDBOX = YES`; with signing on, the sandboxed test host cannot
  write its `.profraw` into DerivedData, and the result bundle's coverage
  report is empty with `Failed to merge raw profiles … No profile data files
  were written` (seen on a full `xcodebuild test -enableCodeCoverage YES`
  run at `e2f6811a`: 2,068 tests ran, the report had no files). The same
  command with `CODE_SIGNING_ALLOWED=NO` (as `XB` already passes) wrote the
  profile: `-only-testing:schematicalTests/NetLabelTests` gave
  `schematical.app 367 files 11.08% (20509/185175)` from `xcrun xccov view
  --report --only-targets`.
- **Python's standard library can measure line coverage**: `python3 -m trace
  --count --missing --coverdir=<dir> -m unittest …` writes one `.cover` file
  per module with `>>>>>>` on each executable line that never ran. `python3
  -m coverage` is not installed (`No module named coverage`), and the plan
  adds no dependency.

## 3. Design Overview

Two independent tools and two ratchet files.

### 3.1 What a line of code is (the counter)

A line counts when, after removing comments, it has a character that is not
whitespace. Text inside a string literal is code, even when it looks like a
comment. Per language:

- Swift: `//` to end of line; `/* … */`, **nested**; string literals `"…"`,
  multi-line `"""…"""`, and raw strings `#"…"#` / `#"""…"""#` with any number
  of `#`; `\` escapes except in raw strings.
- C, headers: `//`, `/* … */` (not nested), `"…"` and `'…'` with escapes.
- JavaScript: as C, plus template literals `` `…` ``. (A regex literal
  containing `//` or `/*` would be misread; there are none in scope:
  `grep -nE '/[^/*]*(//|/\*)[^/]*/[gimsuy]*[,;)]' scripts/*.js` reviewed by the
  author → no regex literal hits.)
- Python, shell: a line whose first non-blank character is `#` is a comment.
  Docstrings are string statements and count as code.

That is `planning/plan-68-data/loc.js`, which produced every count here;
`scripts/source-size.js` is that algorithm with tests.

### 3.2 Which files are in scope

`git ls-files` with extension `.swift`, `.c`, `.h`, `.js`, `.py` or `.sh`,
minus:

- `datasheets/`, `planning/`, `examples/`, `routing-example/` (data and
  archives, not code the app or the tests run);
- vendored code: `backend/src/third_party/`, `backend/tools/avr8js/`,
  `**/*.min.js`, and the three vendored jsmooch generator files
  `backend/tools/jsmooch-gen/{generation.js,wdc65816_opcodes.js,wdc65816_core_generator.js}`
  (`run.js` beside them is ours and in scope).

Test files are in scope for the size rule (owner). HTML and CSS are markup and
out.

### 3.3 Which lines coverage counts

The denominator is production code; test code is not measured.

| Tier | Files | Tool |
|---|---|---|
| backend | `backend/src/**/*.{c,h}` except `third_party/`, `basicsim/main.c` | instrumented build of `-S basicsim` (§ 2), `ctest`, `llvm-cov export -format=lcov` |
| app | `schematical/schematical/**/*.swift` | `xcodebuild test -enableCodeCoverage YES -resultBundlePath`, `xcrun xccov view --archive --json` per file |
| scripts | `scripts/*.js` except `*.test.js`, `schematical/schematical/Datasheet/Renderer/datasheet.js` and `datasheet-edit.js`, `backend/tools/avr8js-oracle/oracle.js`, `backend/tools/jsmooch-gen/run.js`; `scripts/*.py` | `node --test --experimental-test-coverage --test-reporter=lcov`; `python3 -m trace --count --missing` over `python3 -m unittest discover -s scripts/tests` |

Out (owner): `scripts/harness/**` (GUI driver scripts), vendored code, shell
scripts (no line tool; the size rule still applies to them), and the test-data
helpers under `backend/tests/**` (they are test code).

The app target also compiles `backend/src` (synchronized folder); those C
lines are counted in the backend tier only, so the app tier filters to
`.swift` under `schematical/schematical/`.

### 3.4 The ratchets

Each check reads a ratchet file of today's violations:

- `scripts/source-size-ratchet.tsv`: `<code lines>\t<path>` for each of the 36
  over-limit files.
- `scripts/coverage-ratchet.tsv`: `<uncovered lines>\t<path>` for each file
  with uncovered lines.

A check fails when a file **not** in its ratchet violates the rule, when a
listed file got **worse** than its row, or when a row is **stale** (the file
got better, or is gone): a stale row must be lowered or removed in the same
commit, so the files only ever get stricter. A letter that splits or tests a
file removes or lowers its rows; plan-68-AK deletes both files and the code that
reads them, and the checks become plain "no file over 1000" and "no uncovered
line".

### Rejected alternatives

- **Make `ctest`/`xcodebuild test` fail on coverage** — the owner chose a
  separate coverage command: every run would have to be an instrumented one.
- **A per-line coverage exclusion list** — the owner chose none: an unreachable
  line is deleted or restructured.
- **Starting red** (the size test failing until every split lands) — every
  letter's `CT` would be red for days; the ratchet keeps it green and still
  forbids any new violation.
- **cloc, tokei, coverage.py, c8** — new dependencies for what 200 lines of
  node and the standard library do.
- **Counting test code in the coverage denominator** — a test's failure
  branches (the `printf` before `return 1`) run only when the test fails.

### Where the risk is

Low: this letter changes no product code. The one fact the later letters lean
on is that `COV` reports exactly the uncovered lines (a tool that over-reports
sends implementers after lines that are covered; one that under-reports lets
the plan land short). Both tools are tested against fixtures whose uncovered
lines are known (§ Phase 3).

## 4. `scripts/source-size.js`

- `node scripts/source-size.js` prints every in-scope file over 1000, most
  first, as `<code>\t<path>`, and exits 0.
- `--all` prints every in-scope file.
- `--check` applies § 3.4 against `scripts/source-size-ratchet.tsv` and prints
  each failure as `over: <path> <n>`, `grew: <path> <n> > <row>`, or
  `stale: <path> <n> (row <row>)`; exit 1 if any.
- `--root <dir>` runs against another checkout (the tests use temporary git
  repos, as `pin-polarity.test.js` does).
- `module.exports = { countCode(text, ext), inScope(path) }` for the unit tests.

`backend/CMakeLists.txt` adds, inside the existing `if(LOGIC_NODE)`:
`add_test(NAME source_size_check COMMAND ${LOGIC_NODE}
${CMAKE_CURRENT_SOURCE_DIR}/../scripts/source-size.js --check --root
${CMAKE_CURRENT_SOURCE_DIR}/..)`.

## 5. `basicsim`

`basicsim/main.c:load_gates` loads `datasheets.json` from `PARTS_DIR` with
`load_datasheets` before `gate.json`, as `tests/support/read_file.c`'s library
loader does. `basicsim/CMakeLists.txt` gains `enable_testing()` and
`add_test(NAME basicsim COMMAND basicsim)` with `PASS_REGULAR_EXPRESSION` set
to the program's last output line once fixed (recorded in the task when it
runs).

## 6. `scripts/coverage.js`

- `node scripts/coverage.js [--backend] [--app] [--scripts]` (none = all
  three) runs each tier (§ 3.3), prints `<path>:<line>` for every uncovered
  line, then a per-file summary `<uncovered>\t<lines>\t<path>` and per-tier
  totals.
- `--check` compares with `scripts/coverage-ratchet.tsv` (§ 3.4); exit 1 on
  any failure. `--write-ratchet` rewrites the file from the run (used by this
  letter once, and by a letter whose work lowered rows).
- `--only <path-prefix>` limits the printed lines, `--check` and
  `--write-ratchet` to files under that prefix: other rows are neither
  checked nor rewritten.
- `--tests <Class,…>` (app tier) runs only those test classes
  (`-only-testing:schematicalTests/<Class>` each). A scoped run can only
  under-count coverage, so a file it shows fully covered is fully covered by
  the whole suite; the coverage letters use `--tests` with `--only`, and only
  this letter and plan-68-AK run the full app tier.
- Work directories: `/tmp/logic-cov/{backend,app,scripts}`; the app tier's
  `xcodebuild test -enableCodeCoverage YES CODE_SIGNING_ALLOWED=NO
  -resultBundlePath …` (unsigned: § 2, the sandbox otherwise blocks the
  profile) runs under `perl -e 'alarm 3600; …'` and kills the test host
  afterwards (`pkill -f /tmp/logic-cov/app/dd/Build/Products/Debug/schematical.app`);
  per-file lines come from `xcrun xccov view --report --json` and per-line
  counts from `xcrun xccov view --archive --file <path>`.
- The app tier skips no test. `AutorouteModelTests/testTheSummaryOnAFU`,
  which earlier plans recorded as stalling a full Debug run (plan-51-G), is
  slow, not stuck: alone, unsigned, it passed in 200.6 s (`xcodebuild test …
  CODE_SIGNING_ALLOWED=NO -only-testing:schematicalTests/AutorouteModelTests/testTheSummaryOnAFU`
  → `passed (200.600 seconds)`, 232 s wall with the build), so the full tier
  is the 1,634 s census run plus that test.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes. `- [~]` is partly done, with what remains.
> A moot task is `- [x] ~~text~~ — moot: <evidence>`, never deleted. Fill each
> `Commit:` line when the phase lands. **An unticked box means NOT DONE.**

### Phase 1 — the rules in AGENTS.md and docs/testing.md

Text only; safe alone.

- [x] `AGENTS.md`: a new `## Source size and coverage` section after `## Git`:
  - **No source file over 1000 lines of code.** Code lines are non-blank lines
    that are not only comments. Scope and counting: `docs/testing.md`; check:
    `node scripts/source-size.js --check` (also run by `ctest` as
    `source_size_check`). When a file would go over, split it into logical
    units that can be tested on their own — never into arbitrary halves — and
    aim for no new file over 800.
  - **100% line coverage.** Every line of app, backend and script code is run
    by a test. A change that adds or changes code adds the tests that run it
    in the same commit. An unreachable line is deleted or restructured; there
    is no exclusion list. Check: `node scripts/coverage.js --check`, run in
    every plan's landing tier (it is not part of `ctest` or `xcodebuild
    test`).
- [x] `docs/testing.md` (new): the three test commands (`CT`, `XB`, `NT`, plus
  `python3 -m unittest discover -s scripts/tests`), § 3.1 counting, § 3.2
  scope, § 3.3 coverage tiers, how to read `COV` output, the ratchets (§ 3.4)
  and that they are temporary (plan-68).
- [x] `docs/backend.md` § Tests: one line for `source_size_check` and
  `basicsim`, pointing at `docs/testing.md`.

Acceptance: both rules are in `AGENTS.md` and the doc names every command.
  Check: `grep -c '1000 lines of code\|100% line coverage' AGENTS.md` → 2, and
  `grep -c 'source-size.js\|coverage.js' docs/testing.md` → at least 2 (est. 1 s).
  Observed: `2` and `2`.
Commit: 693e0c87

### Phase 2 — the size checker and its ctest

- [x] `scripts/source-size.js` (§ 4), from `planning/plan-68-data/loc.js`.
- [x] `scripts/source-size.test.js`: `countCode` on one fixture string per
  language and construct (nested Swift comment, `"""` with `//` inside, raw
  string with `#`, C `'/'` char, JS template literal with `/*`, Python `#`
  line, blank and comment-only lines); `inScope` on each § 3.2 exclusion and a
  kept `run.js`; `--check` in a temporary git repo for each of over / grew /
  stale / pass; `--all` output; argument errors.
- [x] Re-run the census on the post-plan-48 tree and write
  `scripts/source-size-ratchet.tsv` from `node scripts/source-size.js` (36
  rows expected; a different count is recorded under Corrections with the
  file names). Observed: 36 rows; `--all` equals `census-in-scope.tsv` exactly (`diff` → no output).
- [x] `backend/CMakeLists.txt`: `source_size_check` (§ 4).

Acceptance: the checker passes on the tree, fails on each violation, and its
own code is fully covered.
  Check: `SZ` → exit 0; `node --test --experimental-test-coverage
  scripts/source-size.test.js` → `source-size.js | 100.00` lines; `CT` → 78
  tests, all pass (est. 1 min).
  Observed: `SZ` exit 0; `source-size.js | 100.00` lines (9 tests pass);
  `100% tests passed, 0 tests failed out of 78`.
Commit: 9506ba24

### Phase 3 — the coverage tool

- [x] `scripts/coverage.js` (§ 6) with the backend, app and scripts tiers.
- [x] `scripts/coverage.test.js`: the lcov and `xccov` JSON and `trace`
  `.cover` parsers on fixture files in `scripts/fixtures/coverage/` whose
  uncovered lines are written by hand; the ratchet compare (worse / stale /
  pass / file without row); `--only`; `--write-ratchet`; a tier whose tool
  fails (non-zero exit) is an error, never a pass. The tiers' process runs are
  tested through an injected runner (the module takes a `run(cmd, args)`
  function; the tests pass a fake that returns fixture output), so the test
  does not build the app.
- [x] `scripts/tests/__init__.py` (empty) so `unittest discover -s
  scripts/tests` has a home for plan-68-U.
- [x] Run `node scripts/coverage.js --write-ratchet` once on the full tree;
  commit `scripts/coverage-ratchet.tsv`, and record each tier's total in
  `planning/plan-68-data/coverage-start.txt` (the starting line for every
  coverage letter).

Acceptance: the tool reports what the fixtures say, the ratchet holds today's
numbers, and the check passes on the tree.
  Check: `node --test --experimental-test-coverage scripts/coverage.test.js`
  → `coverage.js | 100.00` lines; `COV --scripts` → exit 0 (est. 15 s); the
  full `COV` once → exit 0 (est. the full Debug suite, as measured in
  `planning/plan-68-data/app-coverage.txt`; nothing smaller checks that all
  three tiers parse their real tool output).
  Observed: the full run (app tier 27 min) exited 0 and wrote 314 rows; totals in `planning/plan-68-data/coverage-start.txt` (backend 7,090 uncovered of 60,689; app 11,991 of 49,627; scripts 3,950 of 6,401). `COV --scripts --check` → exit 0. `coverage.test.js` → `coverage.js | 100.00`, 16 tests pass. The full `COV --check` on the same tree is the run that wrote the rows, so it passes by construction; each later letter checks its own files with `--only`.
Commit: b98bb436, 73ac1f3c

### Phase 4 — basicsim

- [x] `basicsim/main.c`: load `datasheets.json` first (§ 5).
- [x] `basicsim/CMakeLists.txt`: `enable_testing()` + the `basicsim` test.

Acceptance: basicsim runs to its end and is a test.
  Check: `cmake -S basicsim -B /tmp/p68-basicsim && cmake --build
  /tmp/p68-basicsim -j8 && ctest --test-dir /tmp/p68-basicsim -R basicsim` →
  `1/1 … Passed` (est. 30 s).
  Observed: `1/79 Test #1: basicsim … Passed 0.19 sec` (the full ctest of the basicsim build: `100% tests passed, 0 tests failed out of 79`). The fix also loads `footprints.json` (gate.json names footprints too). The test passes on the last line, `55 ns: AND = 0, NAND = 1`.
Commit: 044bfdf2

## Validation Plan

- Tests: `scripts/source-size.test.js`, `scripts/coverage.test.js`, the
  `basicsim` ctest; the existing suites unchanged.
- Coverage check: the two new scripts are in the scripts tier's denominator
  (`COV --scripts` lists them at 100%).
- Runtime proof: `SZ` on the real tree exits 0 and `node scripts/source-size.js`
  lists the 36 files.
- Doc sync: `AGENTS.md`, `docs/testing.md`, `docs/backend.md` § Tests.
- Final gate (once, after Phase 4): `CT` → all pass including
  `source_size_check`; `NT` → all pass; `COV --check` → exit 0.
  Observed: `CT` (on A's tree, before G) → 78/78 pass with
  `source_size_check`, then 79/79 on the basicsim build; `NT` → 81 tests
  pass; `COV`: the full run that wrote the ratchet, then `COV --scripts
  --check` → exit 0 (plan-68-A complete).

## Decisions

- The size rule covers first-party code and tests; vendored and generated
  code is exempt (§ 3.2) — owner, 2026-11-25.
- Blank lines do not count; comments do not count — owner, 2026-11-25.
- Coverage is 100% of **lines**, over the app, the backend and the scripts;
  `scripts/harness` and vendored code are out; no exclusion list — owner,
  2026-11-25.
- `AGENTS.md` gets both rules, not only the size rule — owner, 2026-11-25.
- The size rule is a `ctest` test; coverage is `scripts/coverage.js`, run in
  landing tiers, not by `ctest`/`xcodebuild test` — owner, 2026-11-25.
- `SchematicDocument` is split into real model types, not into extension
  files of the same class (letters B–F) — owner, 2026-11-25.
- plan-48 lands before plan-68 starts — owner, 2026-11-25.
- (author) Test code is not in the coverage denominator; shell scripts are
  size-checked only; HTML/CSS are out of both (§ 3.2, § 3.3).
- (author) Violations are held in ratchet files that only get stricter, and
  are deleted in plan-68-AK (§ 3.4).
- (author) New files from a split aim at 800 code lines or fewer, so the next
  feature does not immediately re-split them.

When the owner was asked, the over-limit count was given as 35 (22 production);
the census, re-checked while writing, is 36 (24 production, 12 test): the
question's list had dropped `serial_test_i.h` and `eeprom.c` (1,001). No ruling
depends on the count.

## Corrections

- **The letters were written as one chain; they are two tracks.** The app
  letters B–F touch only `schematical/` and the backend letters G–S only
  `backend/` (and their tests), so G needs only A. plan-68-G's `Depends on:`
  is now plan-68-A and plan-68-T's is S and F; found while the full app
  coverage run (whose test host is the real app, so no second app test can
  run beside it) held up letter C.

- **The app census used the wrong line count.** The plan first recorded app
  coverage from `xccov view --report --json`'s `executableLines` /
  `coveredLines` (74,819 / 22,860 uncovered). Those totals count lines per
  function, so a line inside a closure is counted again for each function
  range that spans it (`NetLabel.swift`: report 154 executable, 44
  uncovered; per-line archive 123 executable, 38 uncovered, checked with
  `xccov view --archive --json` on the same result bundle). The tool counts
  per line, from the archive, so the census is now that: 49,627 executable,
  11,916 uncovered (`planning/plan-68-data/app-missed.tsv`; the report-based
  file is kept as `app-missed-report.tsv`). The app coverage letters W–AJ
  keep their file groups (no re-split) and their tables now carry the
  per-line numbers: 603–1,234 uncovered lines each; plan-68-C's
  `SchematicModel.swift` figure is 221 of 5,420.
- **`--tests` and `--only` were added to the tool's design** (§ 6) while
  writing the app letters, so a coverage letter never needs the full app
  run; recorded here because § 6 was edited after the first draft.
- **basicsim also needs `footprints.json`.** § 5 said datasheets; gate.json
  names footprints too, so `load_gates` loads both before `gate.json`.
- **The backend tier needs `enable_testing()` in `basicsim/`** (Phase 4's
  task), or `ctest` on the basicsim build registers no tests: the first
  `COV --backend` run reported `no profile was written` because `ctest -N`
  listed 0 tests. Phase 4 landed before Phase 3 for that reason.

## Summary

Tooling and text only. The risk is a coverage tool that misreports, which is
why it is tested on fixtures with known answers before any letter relies on
it. Nothing in the app or the backend changes except `basicsim`, which is
broken today.
