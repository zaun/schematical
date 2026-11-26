# Testing

How the code is tested, and how the two rules in `AGENTS.md` § Source size
and coverage are measured.

## Test commands

| Tier | Command |
|---|---|
| Backend (C) | `cmake -S backend -B /tmp/logic-build && cmake --build /tmp/logic-build -j8 && ctest --test-dir /tmp/logic-build -j8` (programs: `docs/backend.md` § Tests) |
| App (Swift) | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/logic-dd CODE_SIGNING_ALLOWED=NO test [-only-testing:schematicalTests/<Class> …]` |
| Scripts (JavaScript) | `node --test scripts/*.test.js` |
| Scripts (Python) | `python3 -m unittest discover -s scripts/tests` |

The app's test host is the real app. Run it under a time limit
(`perl -e 'alarm 1800; exec @ARGV' xcodebuild …`), never give a test document
a file the test did not write, and close every window a test opens. The full
Debug app suite takes about 27 minutes; run the classes a change touches.

## Lines of code (the size rule)

`node scripts/source-size.js` lists every in-scope file over 1000 code lines;
`--all` lists every in-scope file; `--check` fails on a violation. `ctest`
runs `--check` as `source_size_check`.

A line counts when, after removing comments, it has a character that is not
whitespace. Text inside a string literal is code, even when it looks like a
comment.

- Swift: `//` to end of line; `/* … */`, nested; strings `"…"`, `"""…"""`, and
  raw strings `#"…"#` / `#"""…"""#` with any number of `#`.
- C and headers: `//`, `/* … */`, `"…"`, `'…'`.
- JavaScript: as C, plus template literals `` `…` ``.
- Python and shell: a line whose first non-blank character is `#` is a
  comment. Python docstrings count as code.

In scope: files tracked by git with extension `.swift`, `.c`, `.h`, `.js`,
`.py` or `.sh`, except `datasheets/`, `planning/`, `examples/`,
`routing-example/`, and vendored code (`backend/src/third_party/`,
`backend/tools/avr8js/`, `*.min.js`, and the three jsmooch generator files
`backend/tools/jsmooch-gen/{generation,wdc65816_opcodes,wdc65816_core_generator}.js`).
Test files are in scope. HTML and CSS are not.

## Coverage (the coverage rule)

`node scripts/coverage.js [--backend] [--app] [--scripts]` (none = all three)
measures line coverage and prints `path:line` for every uncovered line, then
`<uncovered>\t<lines>\t<path>` per file and a total per tier.

| Tier | Files measured | How |
|---|---|---|
| backend | `backend/src/**/*.{c,h}` except `third_party/`; `basicsim/main.c` | an instrumented build of `basicsim/` (which builds the backend and its tests), `ctest`, `llvm-cov export -format=lcov` |
| app | `schematical/schematical/**/*.swift` | `xcodebuild test -enableCodeCoverage YES CODE_SIGNING_ALLOWED=NO`, `xccov`. Unsigned on purpose: the app is sandboxed, and a signed test host cannot write its coverage profile. |
| scripts | `scripts/*.js` (not `*.test.js`), the datasheet renderer's `datasheet*.js`, `backend/tools/avr8js-oracle/oracle.js`, `backend/tools/jsmooch-gen/run.js`; `scripts/*.py` | `node --test --experimental-test-coverage`; `python3 -m trace` over the Python tests |

Test code is not measured. Out of scope: `scripts/harness/`, vendored code,
shell scripts (size-checked only).

The Python tests live in `scripts/tests/` (`python3 -m unittest discover -s
scripts/tests`; standard library only). They load a hyphen-named script with
`support.load` and run one as a program with `support.run`; they never run
KiCad or the network (`erc-compare.py` gets the fake `scripts/tests/fakes/kicad-cli`
through its `--kicad-cli` argument, the KiCad comparisons read the recorded
library in `scripts/tests/fixtures/`). For coverage, `coverage.js` runs the suite
under `python3 -m trace --count --no-report --file=<counts>` and sets
`SCRIPTS_COVERAGE_FILE=<counts>`. With `--no-report`, `trace` does not save the
counts, so `scripts/tests/support.py` saves the suite's own counts into that
file when it exits, and `support.run` starts a program-style case through
`scripts/tests/traced.py`, which adds the child's counts to the same file and
keeps its exit code (`python3 -m trace <script>` exits 0 whatever the script
does). `trace` also swallows the exit status of the `unittest` run, so a
failing Python test does not fail the coverage command: run the plain
`unittest` command for the result.

Options:

- `--only <path-prefix>`: print, check and write only files under the prefix.
- `--tests <Class,…>` (app): run only those test classes. A scoped run can
  only under-count, so a file it shows fully covered is fully covered.
- `--check`: fail when a file has more uncovered lines than allowed.
- `--write-ratchet`: rewrite the ratchet rows from this run.

## Ratchets (temporary, plan-68)

While plan-68 brings the tree into line, the two checks read files of the
violations that still exist:

- `scripts/source-size-ratchet.tsv`: `<code lines>\t<path>` per over-limit file.
- `scripts/coverage-ratchet.tsv`: `<uncovered lines>\t<path>` per file with
  uncovered lines.

A check fails when a file without a row violates the rule, when a listed file
got worse than its row, or when a row is stale (the file got better or is
gone): lower or remove the row in the same commit. Rows only ever go down.
plan-68's last letter deletes both files and the code that reads them.
