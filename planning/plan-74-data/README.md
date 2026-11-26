# plan-74 data: Ethernet PHYs, single-port Ethernet controllers, RJ45 jacks with magnetics

Written at planning time (2026-11-25). A–K add to it; each file says which letter writes it.

## At planning

- `raw/` — the planning census pages (DigiKey through Safari; In Stock + Active), saved without
  their product rows beyond page one. How they were made: `planning/plan-69-data/census/README.md`
  (the shared census of plans 69–74; tools `url.js`, `facets.sh`, `show.py`, `kw.js` there).
  - `eth710.json` — category 710 (Drivers, Receivers, Transceivers), Protocol (`578`) one of the
    13 Ethernet ids `345107` Ethernet, `429199` Ethernet IEEE 802.3, `433140` Ethernet IEEE 802.3
    MII RMII PHY2, `861690` Ethernet IEEE 802.3 PHY, `873074` Ethernet PHY, `350612` Gigabit
    Ethernet, `868792` Gigabit Ethernet IEEE 802.3 PHY, `861813` Gigabit Ethernet PHY, `375626` MII,
    `375628` MII RMII, `375634` MII RMII SMII, `397303` RMII, `358006` IEEE 802.3: 267 parts,
    first 100 rows.
  - `eth753.json` — category 753 (Controllers), Protocol Ethernet: 314 parts.
  - `ethc.json` — 753, Ethernet, Function Controller / MAC Controller / Physical Layer Controller: 134.
  - `ethc2.json` — `ethc` plus Interface Parallel, SPI, SPI+Parallel, I2C+SPI, Bus: **74, every row
    saved** (the whole controller listing).
  - `c365.json` — category 365 (Modular Connectors, Jacks With Magnetics): 1,112 parts, first 100 rows.
- `census/ctl-scope.py` — the controller scope count from `raw/ethc2.json`:
  `python3 -I planning/plan-74-data/census/ctl-scope.py planning/plan-74-data/raw/ethc2.json` →
  `rows 74 in 59 {'wiznet': 4, 'microchip': 53, 'renesas': 2} out {'EtherCAT (2-3 ports)': 11,
  'single pair (10BASE-T1S)': 2, 'not a maker of the pick / no DigiKey stock': 2}`.

## Written by the letters

| File | Letter | What |
|---|---|---|
| `census/list.sh`, `census/split.py`, `census/build.py` | A | the listing: one Safari page per (family, maker, split) under 100 rows; `candidates.tsv` |
| `raw-A/` | A | A's listing pages |
| `candidates.tsv` | A | one row per listed part: family, maker, part number, stock, status, scope verdict, datasheet URL |
| `verdicts.tsv`, `stock-check.tsv` | A | the per-part stock check and the ordering-table verdicts |
| `census/fetch.py`, `census/textscan.py` | A | downloads (Python `urllib`, Safari user agent plus Accept and Accept-Language) and the PDF text scan |
| `specs-ic.tsv` | A (skeleton), B, C | one row per (maker, datasheet, package) of the IC `add` rows: pinout, facts |
| `pinouts/<datasheet>-<package>.json` | B, C | the pinout of each `specs-ic.tsv` row, in the part-file pinout format |
| `footprints-needed.txt` | C (final) | the IC packages with no footprint in `footprints.json` |
| `footprints-ic.json` | D | the new IC footprints (`schematical-footprints` format) |
| `census/fp-merge.py` | D | merges a `footprints-*.json` into `footprints.json`, skipping ids already present |
| `census/gen-ethernet.py` | F | writes `Parts/ethernet.json` from `specs-ic.tsv`, the pinouts and `candidates.tsv` (idempotent, `--check`) |
| `specs-rj45.tsv` | A (skeleton), G, H, I | one row per (maker, drawing) of the jack `add` rows |
| `footprints-rj45-{bel,pulse,te}.json` | G, H, I | the jack footprints |
| `census/gen-rj45.py` | G (H, I extend its table) | writes the `rj45-` blocks of `Parts/connector.json` (idempotent, `--check`) |
