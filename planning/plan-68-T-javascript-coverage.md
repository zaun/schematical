# plan-68-T: Scripts — `datasheet-edit.js` split, and all JavaScript at 100%

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-68-S and plan-68-F (both tracks; plan-68-A § the dependency graph)

The JavaScript in scope (plan-68-A § 3.3): the datasheet renderer and editor
that run inside the app's datasheet window, the node scripts in `scripts/`,
and the two node tools the backend tests run. `datasheet-edit.js` (1,079 code
lines) is split into its two halves — the pure model that node already tests
and the DOM editor that nothing tests — and every in-scope JavaScript file is
brought to 100% line coverage under `node --test`.

Prerequisites: plan-68-A's table, plan-68-F complete (`ls
schematical/schematical/Canvas/SchematicModel.swift` → no such file), and plan-68-S complete (every backend row is
gone from the coverage ratchet: `grep -c 'backend/' scripts/coverage-ratchet.tsv`
→ 0).

References:

- `schematical/schematical/Datasheet/Renderer/datasheet-edit.js`,
  `datasheet.js`, `datasheet.html` (script order: `markdown-it.min.js`,
  `wavedrom.min.js`, `datasheet.js`, `datasheet-edit.js`).
- `schematical/schematical/Datasheet/MarkdownDatasheetWindow.swift`,
  `VerifyDatasheetWindow.swift` (the Swift side: the page, the
  `webkit.messageHandlers.edit`/`cite` messages).
- `scripts/datasheet-edit.test.js` (tests the model half through
  `module.exports`), `scripts/pin-polarity.test.js` (spawn style).
- `scripts/check-datasheets.js`, `digikey.js`, `gen-headers.js`,
  `gen-sockets.js`, `gen-terminals.js`, `pin-polarity.js`;
  `backend/tools/avr8js-oracle/oracle.js`; `backend/tools/jsmooch-gen/run.js`.

## 1. Goal

- `datasheet-edit.js` is two files, each at or under 800 code lines, loaded
  by `datasheet.html` in order; the window behaves as before.
- `COV --scripts` reports 100% of lines for every JavaScript file in § 2, and
  their rows are gone from the coverage ratchet.

### Non-goals

- The datasheet window's behavior, the messages it posts to Swift, and every
  script's command line and output are unchanged.
- No npm dependency (no jsdom): the DOM editor is tested against a small fake
  DOM in `scripts/test-support/fake-dom.js` that implements exactly the DOM
  surface the editor uses (§ 2).

## 2. Current State

Line coverage under `node --test --experimental-test-coverage
scripts/*.test.js` at `e2f6811a` (56 tests pass):

| File | Code lines | Line coverage |
|---|---|---|
| `Datasheet/Renderer/datasheet-edit.js` | 1,079 | 35.04% (uncovered 442–1189: the editor half) |
| `Datasheet/Renderer/datasheet.js` | 770 | no test |
| `scripts/pin-polarity.js` | 807 | 91.72% |
| `scripts/gen-sockets.js` | 373 | 98.00% |
| `scripts/gen-terminals.js` | 208 | 100.00% |
| `scripts/digikey.js` | 507 | no test |
| `scripts/check-datasheets.js` | 108 | no test |
| `scripts/gen-headers.js` | 129 | no test |
| `backend/tools/avr8js-oracle/oracle.js` | 336 | run by `avr_core_test`, not by `node --test` |
| `backend/tools/jsmooch-gen/run.js` | 11 | run by `cpu_core_test`, not by `node --test` |
| `scripts/source-size.js`, `scripts/coverage.js` | — | 100% (plan-68-A) |

(Code lines: `node planning/plan-68-data/loc.js <file>`.)

`datasheet-edit.js` is one IIFE: lines 1–439 build `model` (pure functions:
cites, notes, table cells, the line diff; exported to node through
`module.exports` and to the page as `window.dsEdit`), lines 440–1191 are the
editor (`state`, `h`, `post`, the per-block editors `proseEditor`,
`listEditor`, `codeEditor`, `gridEditor`, `tableEditor`, `openEditor`,
`decorate`, `onClick`). The editor's DOM surface (`grep -oE` over lines
440–1191): `.value` 21, `.focus` 16, `.textContent` 9, `.addEventListener` 5,
`.style` 4, `.dataset` 4, `.querySelector` 3, `.classList.add` 3,
`.setAttribute` 2, `.closest` 2, `webkit.messageHandlers.{edit,cite}
.postMessage`, `setSelectionRange`, `selectionStart`, `selectionEnd`,
`scrollIntoView`, `scrollHeight`, `remove`, `querySelectorAll`, `innerHTML`,
`getElementById`, `createElement`, `classList.toggle`, `classList.remove`.

The app bundles the Renderer folder through a synchronized group: the built
app has `Contents/Resources/datasheet-edit.js` beside `datasheet.html`
(checked in `/tmp/…/Debug/schematical.app` built from this tree), so a new
`.js` there ships with no project edit.

## 3. Design

- **Split.** `datasheet-edit-model.js`: the model half, unchanged, still
  exporting `model` to node and `window.dsEditModel` to the page.
  `datasheet-edit.js`: the editor, reading the model from
  `window.dsEditModel`. `datasheet.html` loads the model file before the
  editor. `scripts/datasheet-edit.test.js` requires the model file.
- **The fake DOM** (`scripts/test-support/fake-dom.js`): elements with
  `tagName`, `children`, `parentNode`, `textContent`, `innerHTML` (as text),
  `value`, `dataset`, `style`, `classList`, attributes, `focus` (records the
  focused element), `selectionStart/End` and `setSelectionRange`,
  `scrollIntoView` and `scrollHeight` (no-ops/constants),
  `addEventListener`/`dispatch`, `closest`/`querySelector(All)` for the tag,
  `.class` and `[data-x]` selectors the editor uses; a `window` with
  `webkit.messageHandlers.edit/cite.postMessage` that records messages; and a
  `window.dsRender` stub returning the HTML the editor decorates.
- **The editor tests** (`scripts/datasheet-editor.test.js`): load the model
  and the editor into a `vm` context with the fake DOM; for each editor kind
  (prose, list, code, grid, table), open a block of a real datasheet from
  `schematical/schematical/Datasheets/`, type, apply, cancel, undo, and check
  the posted `edit` message's Markdown and the `cite` messages.
- **`datasheet.js`** (`scripts/datasheet-render.test.js`): load
  `markdown-it.min.js`, `wavedrom.min.js` (vendored, out of the size and
  coverage scope) and `datasheet.js` into a `vm` context with the fake DOM;
  render every datasheet in `schematical/schematical/Datasheets/` and check
  each one's headings, tables, wavedrom blocks and citations against the
  Markdown (the same per-file walk `datasheet-edit.test.js` does).
- **The node tools**: `scripts/oracle.test.js` spawns `oracle.js` with the
  commands `avr_core_test` uses and checks the output format;
  `scripts/jsmooch-gen.test.js` runs `run.js <tmp>` and compares with the
  committed `wdc65816_generated_opcodes.c`. `digikey.js` is tested with an
  injected `fetch` (the module reads `globalThis.fetch`; the test replaces it
  with a stub serving recorded responses from `scripts/fixtures/digikey/`),
  never the network. `check-datasheets.js`, `gen-headers.js`: spawned on
  temporary directories, as `gen-sockets.test.js` does.
- **The rest of `pin-polarity.js` and `gen-sockets.js`**: cases for the
  listed uncovered lines (§ 2) in their existing test files.

### Rejected alternatives

- **jsdom** — a new npm dependency tree for 25 DOM members.
- **Testing the editor through the app** (a Swift test evaluating JS in a
  `WKWebView`) — the coverage would be invisible to `node --test`, so the
  scripts tier could not count it.

### Where the risk is

The split's page side: if `datasheet.html` loaded the editor before the
model, the window would show the datasheet but Edit would do nothing. Phase 1
checks the built page in the app.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same
> commit as the work it describes; `- [~]` with what remains; moot tasks are
> struck through with evidence, never deleted; fill `Commit:` when a phase
> lands. **An unticked box means NOT DONE.**

### Phase 1 — the split

- [x] `datasheet-edit-model.js` and `datasheet-edit.js` (§ 3);
  `datasheet.html`; `scripts/datasheet-edit.test.js` requires the model file.
  `MarkdownDatasheetTests` also checks the model file is in the bundle.
- [x] The size ratchet row for `datasheet-edit.js` removed (432 and 772 code
  lines now; `node scripts/source-size.js --check` → exit 0).

Acceptance: the model tests pass on the new file, and the app's datasheet
window still edits.
  Check: `node --test scripts/datasheet-edit.test.js` → all pass; `XB
  -only-testing:schematicalTests/MarkdownDatasheetTests
  -only-testing:schematicalTests/VerifyDatasheetLifetimeTests` → pass (the
  window's page loads its scripts) (est. 4 min).
  Observed: `node --test scripts/datasheet-edit.test.js` → pass 10, fail 0;
  `xcodebuild … -only-testing` MarkdownDatasheetTests,
  VerifyDatasheetLifetimeTests → 7 tests, 0 failures.
Commit: COMMIT_T1

### Phase 2 — the DOM editor and the renderer

- [x] `scripts/test-support/fake-dom.js`; `scripts/datasheet-editor.test.js`;
  `scripts/datasheet-render.test.js` (plus `datasheet-editor-blocks.test.js`,
  `test-support/datasheet-page.js`, `test-support/datasheet-editor-fixture.js`:
  Corrections). `node --test --experimental-test-coverage
  scripts/datasheet-render.test.js scripts/datasheet-editor-blocks.test.js
  scripts/datasheet-editor.test.js scripts/datasheet-edit.test.js` → 36 pass,
  0 fail; `datasheet-edit-model.js 100.00`, `datasheet-edit.js 100.00`,
  `datasheet.js 100.00` lines.

Acceptance: both browser files fully covered.
  Check: `node --test --experimental-test-coverage scripts/*.test.js` →
  `datasheet-edit-model.js`, `datasheet-edit.js`, `datasheet.js` each
  `100.00` lines (est. 30 s).
Commit: —

### Phase 3 — the node scripts and tools

- [x] `oracle.test.js`, `jsmooch-gen.test.js`, `digikey.test.js` (with
  `scripts/fixtures/digikey/`), `check-datasheets.test.js`,
  `gen-headers.test.js`; the missing cases in `pin-polarity.test.js` and
  `gen-sockets.test.js`. `LOGIC_COV_WORK=/tmp/logic-cov-T node
  scripts/coverage.js --scripts` → exit 0, no `.js` path:line printed, no
  `.js` per-file row (`scripts: 6744 lines, 94 uncovered, 98.61%`, all
  Python, plan-68-U); `node --test scripts/*.test.js` → `tests 163`,
  `pass 163`, `fail 0`. For the ratchet box: the nine `.js` rows still in
  `scripts/coverage-ratchet.tsv` (oracle.js 336, run.js 9,
  datasheet-edit.js 773, datasheet.js 768, check-datasheets.js 108,
  digikey.js 507, gen-headers.js 129, gen-sockets.js 9, pin-polarity.js 73)
  all go to 0 and are deleted.
- [x] `COV --scripts --write-ratchet`: the JavaScript rows gone. (The
  parent edits the ratchet: the nine `.js` rows and the two
  `backend/tools/*.js` rows are removed in this letter's commit.) Observed:
  `LOGIC_COV_WORK=/tmp/p68-covS node scripts/coverage.js --scripts` → exit
  0, `scripts: 6750 lines, 0 uncovered, 100.00%`; `node --test
  scripts/*.test.js` → `tests 163`, `pass 163`, `fail 0`.

Acceptance: every JavaScript file in § 2 at 100%.
  Check: `COV --scripts --only .js` → no uncovered line; `NT` → all pass
  (est. 1 min).
Commit: —

## Validation Plan

- Tests: the new test files above, in `NT`.
- Coverage check: `COV --scripts --check` → exit 0, no JavaScript row left.
- Runtime proof: Phase 1's app tests load the split page.
- Final gate (once, after Phase 3): `NT`; `SZ`; `COV --scripts --check`; the
  two Phase 1 app test classes (est. 5 min).

## Decisions

- (author) A hand-written fake DOM over jsdom; node tests over in-app tests
  for the page scripts (§ 3).

## Corrections

- (Phase 2, bug fixed) `datasheet.js` `outline()` matched only `<h2 id="…">`;
  in edit mode a heading is `<h2 id="…" data-l0="…" data-l1="…">`, so the
  Verify window's outline was empty while editing. Proved by
  `datasheet-render.test.js` "edit mode: … the outline stays" (actual `[]`,
  expected `['Description', 'Pinout']`); the regex now allows attributes after
  the id.
- (Phase 2) The fake DOM's `innerHTML` setter parses HTML into elements (a
  small tag/attribute/entity parser) rather than keeping it as text: the editor
  finds blocks with `querySelectorAll("#page [data-l0]")` and `citeClick`
  clones a paragraph, so the page must be a real tree. No `window.dsRender`
  stub: the tests load the real `datasheet.js`, so the editor decorates what
  the renderer drew.
- (Phase 2) The editor tests are two files — `datasheet-editor.test.js` (the
  frame, prose, citations, markup bar, undo/revert/state, a real datasheet:
  `gate-nand/sn74hc00.md`'s function table) and
  `datasheet-editor-blocks.test.js` (list, text fence, pins and ordering
  grids, tables) — sharing a made-up datasheet in
  `test-support/datasheet-editor-fixture.js`; the page set-up and the payload
  (devices from `Parts/*.json`, figures from `Datasheets/figures/`) are in
  `test-support/datasheet-page.js`. The payload's footprints are expanded in
  the test only for the `dual` and `explicit` styles (the others need the
  backend's expansion; their pinouts render as pin tables); the drawing
  branches (round body, balls, slots, bare holes, all four sides) are covered
  by a hand-made footprint case.
- (Phase 2) The renderer test checks per file: no `pre.error`, the `h2` count
  against the Markdown's `##` lines, the outline links against the headings,
  one `figure.ds-table` / `ds-wave svg` / `ds-curve svg` / `table.ordering`
  per fence of that kind, a chip for every citation in the prose, and the
  document title.
- (Phase 3, bug fixed) `scripts/gen-headers.js` did not write
  `"manufacturer": "Generic"`, which `00fcd849` added by hand to all 80 parts
  of `header.json`, so rerunning the generator would have dropped it. Proved by
  `gen-headers.test.js` "the generated header.json and PINHDR- lines are the
  checked-in ones"; a one-line fix makes the output identical to the checked-in
  file.
- (Phase 3, unreachable line removed) `scripts/pin-polarity.js`
  `Library.kindOf` had a branch for a kind already loaded. Callers read:
  `deviceOrNull` ← `device`, used once per command (`sources`, `pins`, `set`,
  `record`, lines ~561–647) on a fresh `new Library(root)` (line ~867) before
  any kind loads, so the branch never ran; removed, and a loaded kind still
  comes from the `kinds` cache.
- (Phase 3) `check-datasheets.js` and `gen-headers.js` read fixed paths and
  call `fetch` themselves, so they are not run on temporary directories as § 3
  says: each test preloads a `--require` file (`scripts/fixtures/check-datasheets/fetch-stub.js`,
  `scripts/fixtures/gen-headers/redirect-parts.js`) that stubs `fetch` or
  points the `Parts/` paths at a temp folder; neither script changed for this.
  `check-datasheets.test.js` reads the real `datasheets.json` (no write) and
  picks entries at run time. The generated `footprints.json` moves the 160
  `PINHDR-` lines to the end (by design), so the test compares those lines and
  the order of the rest, not the whole file.
- (Phase 3) `digikey.test.js` spawns `digikey.js` with a preloaded stub
  (`scripts/test-support/digikey-stub.js`: a queue of responses for
  `globalThis.fetch`, a request log, a faked `process.platform`, retry waits
  of 1 s or more run at once) and a fake `osascript`
  (`test-support/digikey-osascript.js`, wrapped in a shell file written to the
  test's temp `PATH`) for the Safari path. The fixtures in
  `scripts/fixtures/digikey/` are hand-written in the shape of DigiKey's v4
  answers, not recorded (no network allowed). `digikey.js` is unchanged.
- (Phase 3) `oracle.test.js` runs `oracle.js` the way `avr_core_test` and
  `avr_periph_test` do: the step output and all 21 scenario runs match the
  hashes in `golden.txt`, and `asm` / `build` match the committed `.hex` /
  `.json`. Its layout files (`scripts/fixtures/avr8js-oracle/`) come from
  `avr_periph_test --layout`. It takes about 21 s.
- (Phase 3) `pin-polarity.test.js`'s `fetch` cases use `file://` URLs (curl
  copies them without the network) and clean up `/tmp/pin-polarity/<id>`.
  `pin-polarity.js` is 803 code lines (an existing file, under 1000).
- **(parent) One commit for all three phases.** The letter's phases landed
  together, so all three Commit lines name the same commit.

## Summary

One split along an existing seam, then test writing. The page-side risk of the
split is checked in the app; the rest is node-only.
