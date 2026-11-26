# plan-79-F: MPD coin cell holders, retainers and contacts

Last updated: 2026-11-25
Effort: large (3h–1d)
Depends on: plan-79-E

F adds MPD's coin cell `in` rows (DigiKey functions "Coin Cell, Holder", "Coin Cell, Retainer",
"Coin Cell, Contact"), with D's tools and footprint rules. MPD has 249 PCB-mount parts in all
(planning census), under the cap, so every non-skipped MPD row is added across F and G.

References: plan-79-D § 3; plan-79-A § 2 (MPD's drawings are outlined vectors: `BCAA-datasheet.pdf`
has 42 words of text, its numbers drawn as outlines); plan-79-B `drawings.tsv` (each MPD PDF's kind).

## Prerequisites

plan-79-A § Prerequisites, plus:

| Must be true | Command | Status |
|---|---|---|
| plan-79-E complete | `ls planning/completed/plan-79-E-*.md` → exists | NOT MET (2026-11-25: E not started) |
| Goldens at F's start | `wc -l < backend/tests/golden/parts.txt`, `… footprints.txt` | F records its own |

Everything below is written against the world where these hold.

> **NOTE — the Status column is a snapshot; the Command column is the truth.**
> Re-run every command and update every status before you continue.

## 1. Goal

- Every MPD coin row of `verdicts.tsv` is `add` or `skip` (reason in `datasheets/skipped-parts.md`).
- `BAT-MPD-…` footprints in `footprints-mpd.json`, merged; `battery-mpd-…` devices and parts;
  `datasheets.json` entries with MPD's own URLs.

### Non-goals

- No change to D's tools or rules (a gap is a `skip` plus a Corrections line).

## 2. Design

- **Reading an outlined drawing:** `pdftoppm -r 600 -png <pdf> <out>` and read the dimension
  figures from the render; each number goes into the footprint `note` with the render's page.
  Where MPD prints a "PCB LAYOUT" or "RECOMMENDED PCB PATTERN" view, that view is the pad source
  (plan-79-D § 3.2); otherwise the fallback rules.
- **Series sharing:** MPD names drawings by series (`BCAA`, `BU2032SM-…`); parts of one series with
  one drawing share one footprint unless the drawing gives variant dimensions.

## Phases

> **NOTE — keep the checkboxes current as you go.** Tick `- [x]` in the same commit as the work;
> `- [~]` for partial; mark moot tasks with `~~text~~ — moot: <evidence>`; fill `Commit:` the moment
> a phase lands. **An unticked box means NOT DONE.**

### Phase 1 — Specs and footprints

- [ ] Renders of every MPD coin PDF; `specs.tsv` rows; `skip` rows.
- [ ] `footprints-mpd.json`; merge; regenerate `footprints.txt`.

Acceptance: the new footprints expand and match their drawings.
  Check: `/tmp/logic-p79/footprint_check` passes after regenerating; `git diff backend/tests/golden/footprints.txt | grep -c '^-[^-]'` → 0; the plan-79-D Phase 2 hand check on one holder and one retainer (est. 10 min).
  Result:
Commit:

### Phase 2 — Parts

- [ ] `datasheets.json`; `gen-battery.py --write`; regenerate `parts.txt`, `traces.txt`, `erc_pins.txt`.

Acceptance: the parts load; no other line moved.
  Check: `ctest --test-dir /tmp/logic-p79 -R 'listing_check|trace_check|erc_pins_check|footprint_check|library_check'` → `100% tests passed`; `git diff backend/tests/golden/parts.txt | grep -c '^-[^-]'` → 0 (est. 5 min).
  Result:
Commit:

## Validation Plan

- Phase checks above; `node scripts/check-datasheets.js --host memoryprotectiondevices.com` → every
  new entry answers with a PDF (est. 2 min). Final gate: plan-79-J.

## Decisions

- Author: **600 dpi renders for outlined drawings**, the same rule plan-74-G uses for Bel's image
  drawings: the number is read once, written in the `note`, and the footprint cites it.

## Corrections

## Summary

MPD's coin cell holders, retainers and contacts with footprints read from rendered drawings.
