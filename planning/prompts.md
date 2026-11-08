Write a new plan (use the write-plan skill) to add tactile switches to the backend and the app, including downloading the datasheets they need.

Before writing:
1. Ask me your scope questions in one batch (which sub-types, interfaces and extras are in or out). After I answer, ask nothing more: decide everything else yourself and record it under Decisions.
2. Measure the population with `node scripts/digikey.js` (load DIGIKEY_* from ~/.zshrc first; run it with no arguments for help). Record every count with the command that produced it. If a listing hits DigiKey's 300-result cap, say so, and make phase A re-check stock part by part.
3. Inclusion rule: a part must be in stock (quantity > 0), in the datasheet's ordering table, and have status Active or Last Time Buy. Leave out obsolete, discontinued, "Not For New Designs" and made-to-order parts. Parts with no vendor datasheet (ti.com/product/<PART> returns 404, or only Rochester copies exist) go in datasheets/skipped-parts.md. PDFs where every part is obsolete go in datasheets/obsolete-not-included.md.
4. Check existing kinds, models, families and footprints first. Reuse what already exists, and list only the gaps: new kind/model, new families, and new footprints, which come from the datasheet package drawings.
5. Re-check the next free plan-NN number right before writing, and again before committing, because other sessions land plans on main.

Plan shape:
- Lettered sub-plans, where letter order is implementation order: data/census, then footprints, then backend kind and tests, then parts and goldens, then app (drawer, sidebar, kindName, any window), then the landing tier.
- Each sub-plan has: a Prerequisites table, measured Goal numbers, Phases with checkboxes, acceptance commands, `Commit: —`, Decisions, Corrections and Summary.
- Put the census scripts, candidates.tsv and footprints-needed.txt in planning/plan-NN-data/.
- If the plan touches shared things (families, footprint ids, PART_* enums, app lines) that another open plan also adds: add them only if absent, otherwise mark that step moot with a grep check.
- No open questions in the plan.

Rules:
- Write every file yourself, in this session. Don't hand the writing to subagents or forks.
- Stay on main. Never branch.
- Commit all of the plan's files together as one commit. Touch only your own files.
- Plan only. Don't start implementing.
- When done, tell me the commit hash, the sub-plans, the part counts and what's out of scope.
