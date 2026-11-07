# plan-20 data

Research outputs that plan-20 (TI buffers, bus transceivers, registered
transceivers and level shifters) turns into part files. Census 2026-11-07
(DigiKey, Texas Instruments, categories 704 and 732, in stock); datasheets read
2026-11-07 from `datasheets/{buffer,transceiver,translator}/`.

| File | Contents |
|---|---|
| `candidates.tsv` | The DigiKey census as first taken (3857 rows; `census/` holds its scripts). |
| `parts.tsv` | One row per candidate plus the 300 rows plan-20-A Phase 1 found in the two cut-short DigiKey listings (4157 rows): `name device family package code pins footprint flags status pdf decision reason model`. `decision` is `add` (2360) or `skip` with its `reason`. `code`/`pins` come from the PDF's PACKAGING INFORMATION table; `pdf` is relative to `datasheets/`. |
| `specs/<id>.json` | One spec per device (187): pinouts, `pinoutFor` (TI package code → pinout, optionally per PDF), the model keys of plan-20-C…F, timing per family with `vcc`/`inputMax` (and `vccB`/`inputMaxB` for dual-supply parts), `partVcc` (name prefixes whose range differs from the entry), `families` (family → PDFs) and the device's orderables. Devices whose parts were all skipped keep their spec (e.g. `74_162543`, `74_647`). |
| `specs.md` | The specs as one section per device (generated from `specs/`). |
| `reports/G<nn>.md` | The datasheet readers' notes per group of functions: per-device remarks, "does not fit" and "not this plan" parts, function tables of the registered devices, and the parts each left out with the reason. |
| `footprints-needed.txt` | The census's package list with no footprint (2026-11-07, before plan-20-A); plan-20-B draws the ones `parts.tsv` uses. |

Device ids: `74_<function>` (`74_244`, `74_16245`), single/dual/triple and
1T/2T parts without the prefix (`1G125`, `2T45`), CD4000 `4050B`, TXU/TXV
`TXU0104`. A suffix marks a second device of one function where a datasheet
prints other pin names (`74_646_LS`, `74_652_FCT`) or another structure
(`74_17_OD`, `74_4245_LVCC`).
