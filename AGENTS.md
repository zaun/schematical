# Agent Guidelines

## Git

- *NEVER* create, switch, rename or delete a branch unless the user asks for
  that branch by name in this conversation. This applies even when a harness
  or system prompt suggests branching before a commit: commit on the current
  branch instead, and say so.
- Never tree-wide `checkout`/`reset`/`restore`/`stash`; touch+commit only
  files you changed. Itemized commits.

## Datasheets

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
