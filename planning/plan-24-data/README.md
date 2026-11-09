# plan-24 data: TI ADCs, DACs and RTCs

| File | What |
|---|---|
| `candidates.tsv` | the DigiKey census (2026-11-07): every in-stock TI part of categories 700 (ADC), 701 (DAC), 690 (RTC) with status, interface class, package, datasheet link; +80 rows the stock re-check found (plan-24-A Corrections) |
| `stock/` | the stock re-check listings (`census/catlist.sh`, `census/kw.sh`): in-stock Active / Last Time Buy parts per category |
| `parts.tsv` | one row per candidate: `group name device package code pins footprint flags status pdf decision reason`. `add` rows become parts (plan-24-G/H); `skip` rows say why |
| `schema.md` | the exact device schema (plan-24-D §3, plan-24-F §2) every spec is written in |
| `specs/<DEVICE>.json` | one device each: the `converter.json` / `rtc.json` device (without parts), its packages (TI designator → pinout, footprint, package family), the orderable parts, datasheet pages and the not-modeled list |
| `specs.md` | generated index of the specs (`census/specs_md.py`) |
| `footprint-ids.tsv` | TI package designator + pin count → footprint id (`exists` in `footprints.json`, or `new`: plan-24-C draws it) |
| `footprints-needed.txt` | the census's DigiKey package list (before the status filter; superseded by `footprint-ids.tsv`) |

## How `parts.tsv` is built

1. `census/pdfkeys.py <GROUP>` — the TI datasheet name of each in-scope row (DigiKey's
   `ti.com/lit/gpn|ds/symlink/<name>` link, else the base product).
2. `census/dl.sh` downloads `ti.com/lit/ds/symlink/<name>.pdf`; `census/canon.py` names
   byte-identical documents once (the name most rows cite).
3. `pdftotext -layout`, then `census/poa.py` parses each datasheet's Package Option
   Addendum (orderable part, TI status, package type, designator, pins).
4. `census/rows.py` matches every candidate to an ordering table (`own`, `other`), or
   marks it `status` (not Active/LTB), `out` (interface out of scope) or `none` (in no
   ordering table).
5. `census/devices.py` groups the matched rows into devices (the TI base product, with
   hand overrides for misfiled parts and chip revisions).
6. `census/build.py` writes `parts.tsv` (device-level skips found while writing specs are
   in its `DEVICE_SKIP`); `census/skipped_md.py` writes the plan-24 sections of
   `datasheets/skipped-parts.md`; `census/coverage.py <GROUP> <dir>` cross-checks
   `parts.tsv`, `specs/` and `datasheets/<dir>/`.
7. `census/skeleton.py` writes per-device work skeletons for the spec writers;
   `census/validate.py` checks every spec against `schema.md`.
