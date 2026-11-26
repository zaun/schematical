# plan-80 data: addressable LEDs

Files of plan-80 (A writes them; later letters read them):

| File | Written by | Content |
|---|---|---|
| `census/rows.py` | planning | prints a saved census page's rows: part, maker, own In Stock, chosen parameters |
| `census/list.sh` | A | the listing: one page of DigiKey category 126 per maker, saved to `raw-A/` |
| `census/build.py` | A | `raw-A/*.json` → `candidates.tsv` |
| `raw-A/<maker>.json` | A | the saved listing pages (every product row kept) |
| `candidates.tsv` | A | one row per (maker, part): stock, status, DigiKey description, interface facet, size, datasheet URL, `scope`, `reason` |
| `stock-check.tsv` | A | one row per `in` row: DigiKey's own `In Stock` quantity and status, read off the row |
| `verdicts.tsv` | A | one row per `in` row: `add` or `skip` and the reason (§ 3.3 of plan-80-A) |
| `specs.tsv` | A | one row per (maker, datasheet, package) of the `add` rows: pins, `order`, `leds`, timing, `vcc`, drawing page |
| `footprints-needed.txt` | A | one line per distinct package drawing of `specs.tsv`, with the id C gives it |
| `gen-smartled.py` | C | `specs.tsv` + `verdicts.tsv` → `schematical/schematical/Parts/smartled.json` devices |
| `footprints-smartled.json` | C | the new footprints before `fp-merge` into `footprints.json` |

Census (planning, 2026-11-25): `planning/plan-76-data/README.md` § Plan 80 and the saved page
`planning/plan-76-data/raw/d126.json` (category 126, Configuration Discrete, In Stock, Active,
Marketplace excluded; its 100 product rows kept). The four makers of plan-80 have 49 rows on that
page (`python3 -I planning/plan-80-data/census/rows.py planning/plan-76-data/raw/d126.json | grep -c -E 'Inolux|Dialight|Würth|Lite-On'` → 49), the
same total as their facet counts (Inolux 28 + Dialight 10 + Würth 6 + Lite-On 5 = 49), so the page
holds every one of them.

Datasheets read at planning (scratchpad copies, not committed; A downloads the set into
`datasheets/smartled/<maker>/`):

| Datasheet | Pins | Protocol | Order | T0H / T1H | Tolerance | Reset | DIN→DOUT |
|---|---|---|---|---|---|---|---|
| Inolux `IN-PI55TAT(X)R(X)G(X)B_v1.7.pdf` (14 p.) | 1 GND, 2 DOUT, 3 DIN, 4 VDD | single wire, 800 kHz, 24 bit | G R B (`G7 … B0`) | 0.3 / 0.6 µs (T0L 0.9, T1L 0.6) | typical only | ≥ 200 µs | TPHL 80 ns typ |
| Inolux `IN-PI554FCH.pdf` (19 p.) | 1 VSS, 2 DIN, 3 VDD, 4 DO | single wire, 800 kHz, 24 bit | (image) | (image) | — | — | — |
| Inolux `IN-PIS63BTPW_v1.1.pdf` (15 p.) | GND, VDD, DIN, DOUT | single wire, 800–1100 kHz, 24 bit, one white die on the middle byte | – W – | 0.3 / 0.6 µs (T0L 0.6, T1L 0.3) | typical only | ≥ 80 µs | tPZL ≤ 200 ns |
| Inolux `IN-PIS85ATPR_v1.0.pdf` (13 p.) | 2 DOUT, 3 DIN | as IN-PIS63, red die, middle byte | – R – | 0.3 / 0.6 µs | typical only | ≥ 80 µs | ≤ 200 ns |
| Dialight `587-2056-147F` (6 p., 5050, 6 pins) | 1 NC, 2 GND, 3 DO, 4 DI, 5 VDD, 6 VDD | single wire, 24 bit, MSB first | R G B | 0.3 / 0.9 µs (T0L 0.9, T1L 0.3) | ±0.15 µs | ≥ 200 µs | — |
| Dialight `587-1024-147F` (6 p., 2020, 4 pins) | DIN, GND, VDD, DOUT | single wire, 24 bit, MSB first | G R B | 0.3 / 0.9 µs | ±0.15 µs | ≥ 250 µs | — |
| Lite-On `LTST-E563CEGBW` (14 p., 5050) | 1 VSS, 2 DIN, 3 DOUT, 4 VDD (the level table also names a `SET` input that the pin table does not have) | single wire, 24 bit | (image) | 300 / 900 ns (T0L 900, T1L 300) | ±80 ns | > 50 µs | — |

Würth's `we-online.com` returns `Access Denied` to curl (226-byte HTML page, 2026-11-25); A fetches
Würth PDFs with `scripts/safari-digikey-pdf.sh` (it loads any URL in the Safari DigiKey window) or
from DigiKey's `mm.digikey.com` copy.

None of the seven datasheets has a clock input (CI / CKI) or a backup data input (BIN / FDI).
The 6-pin Dialight packages carry two VDD pads and one NC pad, not a second signal.
