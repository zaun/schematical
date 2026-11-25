# plan-47-O: Landing tier

Last updated: 2026-11-16
Effort: small (< 1h)
Depends on: plan-47-N

O runs plan-47's landing tier once, on the landed tree on `main`, ticks the
todo item and archives the plan. Every gate's command is in the table; the
observed values are recorded under it.

References:

- plan-47-A § 1 (the plan's outcome), plan-47-I § 1, J § 1, K § 1, L § 1,
  M § 1, N § 1.
- `planning/completed/plan-40-I-transistor-landing.md` (the precedent).

## Prerequisites

plan-47-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-47-N complete | `ls planning/completed/plan-47-N-*.md` → exists, or every N box ticked | MET with one exception (2026-11-25: every N box is ticked except the hand check by ear, `[~]`: nobody here can hear; the owner's) |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

Every gate of Phase 1's table shows its expected value in one run.

### Non-goals (explicit constraints)

- No new work: a failing gate is fixed in the letter that owns the area (and
  recorded in that letter's § Corrections), then O re-runs.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work; fill `Commit:` the moment a phase lands.
> **An unticked box means NOT DONE.**

### Phase 1 — Landing tier

- [x] (2026-11-25: every gate matches; the observed values are under the table) Run every gate of the table on a fresh build dir
      (`/tmp/logic-p47-land`) and record the observed values here.
- [x] (2026-11-25) `planning/todo.md`: mark `Add inductors, fuses / polyfuses, relays and
      buzzers.` done (`[x]`) with a note on the same item: "(plan-47: top
      three makers per type at DigiKey; no generic parts; panel-mount,
      AC-coil and plug-in parts out of scope)", touching only that item.
- [x] (2026-11-25) Move `plan-47-A` … `plan-47-O` to `planning/completed/` (A–N are
      archived as each completes; O with this commit).

| Gate | Command | Expected |
|---|---|---|
| Backend tests | `cmake -S backend -B /tmp/logic-p47-land && cmake --build /tmp/logic-p47-land -j8 && ctest --test-dir /tmp/logic-p47-land` | `100% tests passed`, T0 + 4 programs (41 with T0 = 37), plus any program other plans added meanwhile (`git log --oneline <plan-47-I first commit>^.. -- backend/CMakeLists.txt \| grep -v plan-47`) |
| Goldens vs plan-47 start | `git diff <plan-47-I first commit>^ -- backend/tests/golden \| grep -c '^-[^-]'` | 0 (only added lines), counting only plan-47's commits |
| Part counts | `for k in inductor fuse relay buzzer; do grep -c "\|$k\|" backend/tests/golden/parts.txt; done` | the `add` rows per kind (`awk -F'\t' '$23=="add"{print $3}' planning/plan-47-data/candidates.tsv \| sort \| uniq -c`, `fuse` + `ptc` together for the `fuse` kind); 600 / 1200 / 600 / 553 at planning time |
| Parts total | `wc -l < backend/tests/golden/parts.txt` | P0 (32897) + the `add` rows, plus any part other plans landed meanwhile (`git log --oneline <plan-47-K first commit>^.. -- schematical/schematical/Parts \| grep -v plan-47`) |
| Footprints | `python3 -I -c "import json;print(sum(f['id'].startswith(('IND-','FUSE-','PTC-','RELAY-','BUZZER-')) for f in json.load(open('schematical/schematical/Parts/footprints.json'))['footprints']))"` | the distinct new ids the map names (`awk -F'\t' 'NR>1{print $10}' planning/plan-47-data/footprint-map.tsv \| grep -E '^(IND\|FUSE\|PTC\|RELAY\|BUZZER)-' \| sort -u \| wc -l`); `footprint_check` (in `ctest`) passes, so none is unused |
| Datasheets | `for f in inductor fuse relay buzzer; do find datasheets/$f -iname '*.pdf' \| wc -l; done` | per family, the distinct PDFs the `add` rows cite (`awk -F'\t' '$23=="add"{print $26}' planning/plan-47-data/candidates.tsv \| sort -u`, by directory), `fuse` + 5 for the clip PDFs; `python3 -I planning/plan-47-data/census/place.py --verify` → `0 missing, 0 unused` |
| Datasheet URLs | `node scripts/check-datasheets.js --host <host>` for the hosts K and L recorded as answering its client | exits 0; the hosts that refuse it are the ones listed in `docs/parts-format.md` § Datasheets, with the statuses recorded there |
| Skipped and obsolete | `grep -c 'plan-47' datasheets/skipped-parts.md datasheets/obsolete-not-included.md` | at least 1 in the first; in the second the number of PDFs C deleted for status |
| Stock check | `awk -F'\t' '$23=="add" && $29!=1' planning/plan-47-data/candidates.tsv \| wc -l` | 0: every `add` row was checked part by part |
| In the ordering table | `python3 -I planning/plan-47-data/census/scheme-check.py` | `<n> rows: <k> ok, 0 unchecked`, and `awk -F'\t' '$23=="add"' … \| wc -l` at most `<k>` |
| App | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -configuration Debug -derivedDataPath /tmp/p47-dd CODE_SIGNING_ALLOWED=NO build` | `** BUILD SUCCEEDED **` |
| App tests | `xcodebuild -project schematical/schematical.xcodeproj -scheme schematical -derivedDataPath /tmp/p47-dd test -only-testing:schematicalTests/InductorFuseTests -only-testing:schematicalTests/RelayBuzzerTests -only-testing:schematicalTests/DrawerPerformanceTests -only-testing:schematicalTests/PartEditorWindowTests -only-testing:schematicalTests/PartKindRoundTripTests -only-testing:schematicalTests/PartKindKeysTests -only-testing:schematicalTests/KeyboardHelpTests -only-testing:schematicalTests/CapacitorTests -only-testing:schematicalTests/ResistorTests CODE_SIGNING_ALLOWED=NO` | `** TEST SUCCEEDED **` |
| Runtime proof | `APP=/tmp/p47-dd/Build/Products/Debug/schematical.app scripts/harness/build.sh scripts/harness/inductor-fuse-app.swift /tmp/p47-h1 && /tmp/p47-h1/H.app/Contents/MacOS/harness`; the same with `relay-buzzer-app.swift` and `/tmp/p47-h2` | every line `ok`: the drawer paths and value order, `L1` / `F1` / `K1` / `LS1`, the LED lit through an inductor and a fuse and dark with the fuse blown; the relay operated by a gate, the LED and the buzzer voice behind its contact, the 500 Hz transducer voice |


Observed (2026-11-25, on the integration branch after merging main, `3c6d5370`; the landing run is on this tree, then merged):

| Gate | Observed |
|---|---|
| Backend tests | `cmake --build /tmp/logic-p47-land -j6 && ctest --test-dir /tmp/logic-p47-land -j4` → `100% tests passed, 0 tests failed out of 65` (45 before the merge of main's tests; plan-47 added `inductor_test`, `fuse_test`, `relay_test`, `buzzer_test`) |
| Goldens vs plan-47 start | against `main` (the merge base: the drawer regroup is main's): `git diff main -- backend/tests/golden \| grep -c '^-[^-]'` → 0; plan-47 adds footprints.txt 896, parts.txt 2813, traces.txt 2813 lines. Against `1c9f6088^` the count is 7626 removed lines, which are main's own changes since then (plan-47 removes none: the count against main is 0) |
| Part counts | `grep -c '\|inductor\|'` etc. on `parts.txt`: inductor 600, fuse 1199, relay 504, buzzer 510; `add` rows: inductor 600, fuse 600 + ptc 599, relay 504, buzzer 510 (planning-time figures: 600 / 1200 / 600 / 553) |
| Parts total | `wc -l < backend/tests/golden/parts.txt` on the merged tree: 39903 = main's 37090 + plan-47's 2813 (`git diff main` adds 6522 golden lines: 896 + 2813 + 2813) |
| Footprints | 896 ids with the five prefixes in `Parts/footprints.json`; `footprint_check` passes (no unused footprint; 4 unused ones were removed) |
| Datasheets | inductor 354, fuse 276 (271 + 5 clip PDFs), relay 124, buzzer 510 PDFs = the distinct `add`-row PDFs (inductor 354, fuse 271, relay 124, buzzer 510); `place.py --verify` → `0 missing, 0 unused` |
| Datasheet URLs | the seven hosts of K and L checked on 2026-11-22 (all serve a PDF); on 2026-11-25 the four entries the final pick added (`v23700-x0000-a001`, `cmt-1614-85t`, `cpt-2475-92t`, the Littelfuse AGRF) → `4 of 4 URLs serve a PDF`; `mspindy.com`'s eight unconfirmed URLs: `0 of 8`, the site does not answer from this machine (plan-47-L § Corrections) |
| Skipped and obsolete | `grep -c 'plan-47'`: skipped-parts.md 2, obsolete-not-included.md 9 (the nine PDFs C deleted) |
| Stock check | `awk -F'\t' '$23=="add" && $29!=1' … \| wc -l` → 0 |
| In the ordering table | `scheme-check.py inductor fuse ptc relay buzzer` → `3513 rows: 3245 ok, 0 unchecked`; 2813 `add` rows ≤ 3245 |
| App | `** BUILD SUCCEEDED **` (`/tmp/p47-dd-land`, and again on the merged tree, `/tmp/p47-dd-merge`) |
| App tests | the nine listed classes on the pre-merge tree → `Executed 116 tests, with 0 failures`, `** TEST SUCCEEDED **`; on the merged tree with `KindGuideTests`, `KindInfoTests`, `WizardStepsTests` added → 152 tests, 0 failures, `** TEST SUCCEEDED **` |
| Runtime proof | both harnesses on the merged build: `inductor-fuse-app.swift` 18 `ok` lines, `relay-buzzer-app.swift` 13 `ok` lines, each ending `all checks ok`, exit 0; the self-interrupting relay's voice 501.3 Hz. The hand check by ear is plan-47-N's `[~]` box |

Acceptance: every gate's observed value matches.
  Check: the table's commands (est. 60 min: the fresh backend build, the app
  tests and the two harnesses; this is the plan's one landing run, and
  nothing smaller covers the backend, the four part files and the app
  together).
Commit: 1db5cff6

## Validation Plan

- The table above is the plan's landing tier; the full suites run per the
  project's background policy, not as a plan step.

## Decisions

- Author: **CapacitorTests and ResistorTests are in the app gate**: M changes
  `functionGroups` (the drawer's value-ordered leaves are the capacitors' and
  resistors'), which those kinds' tests exercise.
- Author: **the todo item is ticked although no generic part was added**: the
  item asks for the four part types; generics were offered and not chosen
  (plan-47-A § Decisions).

## Corrections

## Summary

One run of every gate; no new work.
