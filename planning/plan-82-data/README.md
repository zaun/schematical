# plan-82 data: indicator LEDs

The files of plan-82. The planning census (all seven plans 76–82) is
`planning/plan-76-data/README.md` § Plan 82; its raw page is
`planning/plan-76-data/raw/m105.json`.

| Path | Written by | Content |
|---|---|---|
| `census/list.sh` | A | one census page per (maker, piece); retries a page that `check.py` rejects |
| `census/split.py` | A | splits a (maker) listing over 100 rows into pieces by Mounting Type, then Color |
| `census/check.py` | A | a page is valid: `productCount` set, every row of the page's maker |
| `census/build.py` | A | `candidates.tsv` from `raw-A/*.json` |
| `census/scope.py` | A | the scope verdict of each row (§ 3.2 of plan-82-A) |
| `raw-A/<maker>_<piece>.json` | A | the census pages, product rows kept |
| `candidates.tsv` | A | every row of the four makers: `maker part stock status digikey_id description mount config color package supplier_package lens datasheet_url scope reason` |
| `stock-check.tsv` | A | per `in` row: DigiKey's own `In Stock` quantity and status |
| `verdicts.tsv` | A, then C–E | `add` / `skip` with a reason; C–E change only a row whose datasheet turns out unusable |
| `census/fetch.py` | B | downloads every datasheet URL of the `add` rows (Kingbright, SunLED: curl-like `urllib`; Broadcom, QTB: as B § 3 measures) |
| `datasheets.tsv` (one per folder of `datasheets/led/<maker>/`) | B | file, URL, bytes, SHA-256 |
| `specs.tsv` | B skeleton, C–E fill | one row per (maker, datasheet, package drawing): the pin map, dies, colours, facts and footprint id |
| `footprints-<maker>.json` | C–E | the new footprints of each maker, merged into `footprints.json` by `census/fp-merge.py` |
| `census/fp-match.py` | C | compares a drawing's land pattern with the 11 generic `LED-` footprints (0.05 mm) |
| `census/fp-merge.py` | C | merges a `footprints-<maker>.json`, skipping ids already present |
| `gen-led.py` | H | writes the plan's devices and parts into `Parts/led.json` from `specs.tsv` + `verdicts.tsv` |

Makers (DigiKey ids): Kingbright 754, Broadcom Limited 516, SunLED 1497, QT Brightek (QTB) 1516.
