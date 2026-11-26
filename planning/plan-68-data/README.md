# plan-68 data

Measurements plan-68 was written from, at `e2f6811a` (2026-11-25).

| File | What | Made by |
|---|---|---|
| `loc.js` | the code-line counter (prototype of `scripts/source-size.js`; plan-68-A § 3.1) | hand-written |
| `sections.js` | code lines per section comment (`/* ---- … */`, `// MARK:`) of a file | hand-written |
| `census.tsv` | `<code lines>\t<path>` for every candidate file (`.swift .c .h .js .py .sh` outside `datasheets/ planning/ examples/ routing-example/`) | `git ls-files -z \| grep -zvE '^(datasheets\|planning\|examples\|routing-example)/' \| grep -zE '\.(swift\|c\|h\|js\|py\|sh)$' \| xargs -0 node loc.js` |
| `census-in-scope.tsv` | the same minus vendored code (plan-68-A § 3.2) | `grep -vE 'backend/src/third_party/\|backend/tools/jsmooch-gen/(generation\|wdc65816_opcodes\|wdc65816_core_generator)\.js\|backend/tools/avr8js/\|\.min\.js' census.tsv` |
| `over-limit.tsv` | in-scope files over 1000 code lines (36) | `awk -F'\t' '$1>1000' census-in-scope.tsv \| sort -rn` |
| `bcov.sh` | instrumented backend build + `ctest` + `llvm-cov report` | hand-written; run from any directory |
| `show.sh` | `llvm-cov show` over the instrumented test programs, for per-line counts | hand-written |
| `backend-coverage.txt` | the `llvm-cov report` of `bcov.sh` | `bash bcov.sh` |
| `backend-missed.tsv` | `<uncovered>\t<lines>\t<file>` for each `backend/src` file with uncovered lines | from `backend-coverage.txt` |
| `backend-missed-by-letter.tsv` | the same, each row prefixed with the plan-68 coverage letter (L–S) that owns it | grouping rules in the letters' § 2 |
| `test-output-before/<program>.txt` | the output of each C test program plan-68-G/H split, before the split | the instrumented programs, run with `LLVM_PROFILE_FILE=/dev/null` |
| `scov.sh` | the full app suite with coverage (unsigned build, plan-68-A § 2) | hand-written |
| `app-coverage.txt`, `app-missed.tsv` | the app tier's totals and `<uncovered>\t<lines>\t<file>` per Swift file | `scov.sh`, then `appcov.py` |
| `appcov.py` | turns `xccov view --report --json` into `app-missed.tsv` | hand-written |
