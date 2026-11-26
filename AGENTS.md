# Agent Guidelines

## Git

- *NEVER* create, switch, rename or delete a branch unless the user asks for
  that branch by name in this conversation. This applies even when a harness
  or system prompt suggests branching before a commit: commit on the current
  branch instead, and say so.
- Never tree-wide `checkout`/`reset`/`restore`/`stash`; touch+commit only
  files you changed. Itemized commits.
- Exception: a new plan is one commit. Commit all of its files together:
  every lettered sub-plan, plus any edits to other plans it causes. Don't
  make one commit per sub-plan.

## Source size and coverage

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

## Testing in the app

- When you drive the real mouse to test the app (clicks, drags, hovers),
  move the pointer to the top-left corner of the screen (0, 0) when you are
  done, so it isn't left over the user's work.

## Datasheets

- Add only parts that are **in stock**: DigiKey shows a quantity available
  above zero (`node scripts/digikey.js --in-stock …`; run it with no
  arguments for help). Never add made-to-order parts, or any part with no
  stock, even when the datasheet or ordering table lists it. This rule is
  for adding only: a part already in the catalog stays, in stock or not.
- Look stock up in Safari, not with `scripts/digikey.js`: `scripts/safari-digikey.sh`
  loads a DigiKey category page in a tab of the Safari window that has a DigiKey
  page open and prints rows or a JavaScript result (`scripts/safari-digikey-pdf.sh`
  fetches DigiKey-hosted PDFs). Both explain their setup in their header.
- Before adding parts from a datasheet PDF, check whether its parts are still
  made. If **every** part the PDF covers is obsolete, do not add any of them:
  delete the PDF and record it in `datasheets/obsolete-not-included.md` (PDF
  name, plus the family / category / group it would have gone into).
- If only some parts are obsolete, add them all and flag the obsolete ones
  with `"flags": ["obsolete"]` in the kind's
  `schematical/schematical/Parts/<kind>.json`.
- New parts are added by editing those part files (format:
  `docs/parts-format.md`). Every new part (except a supply) also names a
  footprint from `schematical/schematical/Parts/footprints.json`; when no
  existing footprint matches the datasheet's package drawing, add one from
  that drawing (rules in `docs/parts-format.md` § Footprints). Run the backend tests afterwards; a part whose
  facts or behavior change on purpose needs its line regenerated in
  `backend/tests/golden/` (see `docs/backend.md`).
