# plan-72 data — IO expanders and I2C buffers/muxes

Written at planning, 2026-11-25. Letters: `planning/plan-72-A-ioexp-data.md` … `plan-72-J-ioexp-landing.md`.

## At planning

- `raw/` — DigiKey category pages through Safari, In Stock + Active (facets and the first page of products):
  `c749.json` (I/O Expanders, 453), `c756.json` (Signal Buffers, Repeaters, Splitters, 496),
  `i2cbuf.json` (756 with the I2C Applications filter, 148), `c743.json` (Signal Switches,
  Multiplexers, Decoders, 2,778; no I2C facet). Tools and the full census table:
  `planning/plan-69-data/census/` (`README.md`, `facets.sh`, `show.py`, `kw.js`, `url.js`).
- Maker counts (plan-72-A § 2): expanders TI 101, ADI 78, NXP 65, Microchip 62; buffers ADI 92,
  NXP 23, TI 13; muxes by series keyword TCA954 11 (TI), PI4MSD 10 (Diodes), PCA984 4 (NXP).
  Upper bound 459 parts.

## Written by the letters

| File | Letter | What |
|---|---|---|
| `census/` (`make-urls.js`, `urls.tsv`, `list.sh`, `check.py`, `build.py`, `fetch.py`, `textscan.py`, `decide.py`, `place.py`, `specs-skeleton.py`) | A | listing, stock, datasheets, verdicts |
| `candidates.tsv` | A | every listed part with `decision` and `reason` (columns: plan-72-A § 4.2) |
| `specs.tsv` | A (skeleton), B | one row per (family, maker, PDF, package) |
| `census/pinout-check.py`, `census/template-check.py`, `census/footprint-lines.py` | B | checks |
| `templates/*.json`, `address.tsv`, `footprints-needed.txt` | B | device skeletons, I2C address tables, the footprint list |
| `census/fp-check.py`, `census/fp-load.sh`, `census/fp-merge.py`, `footprints-C.json`, `footprint-map.tsv` | C | new footprints and the map |
| `gen-parts.py` | F (used by G, H) | part-file generator |
