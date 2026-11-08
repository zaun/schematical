# Datasheet format spike: findings

## Result

- **Format:** Markdown with JSON front matter and typed fenced blocks
  (`table`, `pins`, `pinout`, `wavedrom`, `curve`, `figure`, `package`,
  `ordering`, `equation`). Spec: [format.md](format.md). Checker:
  [check.py](check.py).
- **Size:** the six hand-converted samples are 5–18 KB each, so every
  non-controller IC fits the 20–30 KB target with room to spare. The big
  UARTs (16C550 class) won't: they come to about 40–55 KB.
- **Catalog links:** part numbers, pin numbers, pinout drawings,
  footprints and the ordering table come from the catalog. A datasheet
  holds only what the catalog lacks: descriptions, tables, waveforms.
- **WaveDrom:** all 16 diagrams in the samples render with the stock
  WaveDrom 3 library (checked with the npm package and rsvg-convert).
  Every digital timing diagram seen in the review fits WaveDrom. Analog
  measurement figures don't, and they are shared figures instead.
- **Next step:** a write-plan plan covering the renderer, the figure
  library, the checker in the test suite and a staged conversion
  (see [Recommendation](#recommendation)).

## What was reviewed

### The whole collection (scripted)

`pdfinfo` and `pdftotext` over all 2,085 PDFs in `datasheets/`:

| | |
|---|---|
| Files, pages | 2,085 PDFs, 50,129 pages (median 24), 2.4 GB |
| Extracted text | 63 MB in total |
| Vendors (by text) | TI 1,858 (89 %), ECS 68, MaxLinear/Exar 28, Alliance Memory 17, Lite-On 16 (+11 unattributed Lite-On-style displays), Maxim/ADI 14, Microchip 12, Same Sky/CUI 6, NXP 5, WDC 5, Würth 4, Renesas/Intersil/Harris 4, Zilog 3, Newhaven 3, others and unknown about 45 |
| Scanned, no text layer | **131 PDFs** (pages 1–4 carry under 400 characters a page): mostly old TI TTL/CD4000 sheets in shifters (16), counters (13), transceiver (12), controller-uart (11), buffer and latch (9 each), plus 2 LED displays and 2 LCD modules |
| Variant PDFs | **413** `-q1`, `-ep` and `-sp` files: automotive, enhanced and space versions whose content mostly repeats a base PDF |
| Catalog references | 1,826 of the 2,085 PDF base names appear in the part files' `note` fields |

There are **no Nexperia or onsemi PDFs** in `datasheets/`: the logic
library is TI. The review covered every vendor that is present.

### Sampled PDFs (read in full)

About 55 PDFs from 30 of the 34 categories (all but gate-and, gate-nor,
gate-or and gate-xnor, which share the other gates' TI templates): 6
converted by hand, the rest reviewed by three parallel reviewers using text extraction and page
renders. One line per PDF; "Tables" lists the hardest shape.

| PDF | Vendor | Pages | Sections beyond the basics | Tables | Timing diagrams | Hard to express |
|---|---|--:|---|---|---|---|
| gate-nand/sn74hc00 | TI 2021 | 39 | ESD, thermal, curves, app, layout | TA group over Min/Typ/Max, VCC sub-rows | app S-R timing | curves, load circuit, clamp diagram |
| gate-nand/cd74hc00 | TI 2021 | 26 | as sn74hc00 | 3-level: TA group / Min-Typ-Max, conditions then VCC | measurement only | gate symbol |
| gate-nand/sn74ls00 | TI 2017 | 40 | one table per family (6) | ROC rows split by family subsets | measurement only | overbar formula Y = !{A·B} |
| gate-not/sn74lvc1g04 | TI 2025 | 47 | 8 pinouts (incl. DSBGA) | 4 near-identical switching tables, VCC as column group | measurement only | formula cells (V~CC~ – 0.1) |
| gate-xor/cd4070b | Harris/TI | 34 | transistor schematic, 8 curves | temperature columns –55/–40/85/125 + 25 °C Min/Typ/Max | none | ±10⁻⁵ values, metadata junk in text |
| ff-d/sn74hc74 | TI 2021 | 41 | SN74 and SN54 tables separate | parameter → condition → VCC nesting | setup/hold, pulse | Q vs !{Q} lost in text |
| ff-d/000036269 | ICPDF copy of TI SCLS094D | 20 | duplicate of sn74hc74 (2003 rev.) | TA 25 °C / SN54 / SN74 groups | — | **duplicate: skip** |
| ff-jk/sn54ls76a | TI 1988 scan | 13 | logic diagrams, I/O schematics | per-family ROC groups | — | **scan**, † ‡ § footnotes, refs to a databook |
| counters/sn74hc161 | TI 2003 | 31 | logic diagram, sequence, cascade app | TA / SN54 / SN74 groups, 3-deep nesting | clear/load/count/inhibit sequence | logic diagram, glitch diagram |
| counters/cd4510b | Harris scan | 20 | two parts, die photo | temperature groups | full count sequences | **scan** |
| counters/sn74ls90 | TI 1988 scan | 27 | 12 parts, 3 pinouts | column group per part | sideways measurement figure | **scan** |
| latch/tpic6259 | TI 1995 | 19 | power DMOS, avalanche | conditions per row | operation mode (with 24 V levels) | SOA-type curves that matter |
| shifters/sn74hc595 | TI 2021 | 41 | | FROM → TO → C~L~ → VCC | full timing diagram (x, z, hatch) | 3-state hatch |
| buffer/sn74abt125 | TI 2002 | 30 | 3 pinouts | rows split by output state, `*` notes | measurement only | S1 switch table |
| transceiver/cy74fct245t | TI/Cypress | 21 | 4 speed grades | column group per grade | measurement only | I~C~ formula in a footnote |
| translator/txu0104-q1 | TI 2022 | 40 | 11 feature subsections | 6 switching tables × 6 V~CCB~ groups (~18 numeric columns) | measurement only | widest table seen |
| mux/sn74hc151 | TI 2022 | 28 | | TA / SN54 / SN74, VCC rows | measurement only | !{W} lost |
| mux/cd74hc670 | Harris/TI | 16 | HC and HCT | temperature groups | setup/hold | |
| decoder/sn74hc138 | TI | 35 | app curve, layout | SN74 and SN54 separate | measurement only | |
| decoder/sn7447a | TI 1974 scan | 31 | 6 types, 4 pinouts | 20-row function table with notes | — | **scan**, 7-segment glyph strip |
| encoder/sn74hc148 | TI | 25 | | 15-column function table | — | every pin active low, no overbars in text |
| parity/sn74ls280 | TI 1972 scan | 21 | cascading app | | — | **scan**, Σ in pin names |
| divider/cd4017b, comparator/sn74ls85, arith/sn74ls283 | TI scans | 12–23 | | | decoded-counter timing (4017) | **scans** |
| eeprom/AT28C256 | Microchip 2022 | 32 | SDP flowcharts, normalized curves | grade column, part-number key | read, write ×2, page, SDP, polling, toggle, erase | V~H~ 12 V level on a waveform |
| eeprom/AT28C64B | Microchip | 33 | as AT28C256 | | ~10 cycles | overbars lost |
| eeprom/AT28C010-military | Microchip | 31 | SMD numbers | **4 speed grades × Min/Max** | as above | errors in the source's ordering table |
| sram/AS6C62256 | Alliance | 13 | data retention | per-temperature-grade sub-rows | read ×2, write ×2, retention | |
| sram/AS7C4098A | Alliance | 11 | | 4 grades × Min/Max; truth table with row spans | read/write | 2-D ordering grid |
| osc/ECS-2520MV | ECS 2026 | 2 | current-by-frequency table, reflow | part-number key, frequency codes | — | a suspect 40 mA column |
| osc/ECS-3225MV | ECS | 2 | as ECS-2520MV | 22 frequency codes | — | example code contradicts the table |
| osc/SiT1534 | SiTime | 13 | 8 curves, 2 packages | banner rows, stacked condition rows | output waveform | coloured ordering text lost |
| osc/lmk6c | TI | 66 | 4 output types, BAW | IDD × 5 outputs × 2 VDD × freq | analog differential levels | 211 orderables |
| osc/OM-7605-C8 | Micro Crystal | 2 | f-vs-T parabola | "typ/max" in one cell | — | opaque order numbers |
| osc/NJU6366 | Nisshinbo | 6 | IC, die pad table | same table ×3 VDD | measurement | |
| led-7/LTS-4301JR | Lite-On 2010 | 6 | 6 curves | | — | µ, λ, Δ lost in text; vector drawings with no text |
| led-7/LTC-4627JR, C4727JR, LSHD-7503 | Lite-On | 6–10 | cautions (9 KB) | two-row I~V~ | — | same |
| module-disp-char/NHD-0216K1Z | Newhaven | 13 | DDRAM map, commands, init code | RS, R/W, DB7…DB0 instruction table | **raster images only** | CGROM font table (raster) |
| module-disp-char/AMC1602G | Orient | 22 | HD44780 description | 13-field part-number key | write/read with level labels | GB-encoded font garbles symbols |
| controller-i2c/sc18is604 | NXP 2021 | 29 | registers, SPI commands | Standard/Fast mode groups, [n] notes | SPI, SPI-to-I²C, I²C | equations are images |
| controller-uart/sc16is750 | NXP | 61 | 3 parts, ~25 registers, I²C tutorial | access-condition map | 44 figures | **55–70 KB** |
| controller-uart/tl16c550d | TI 2008 | 60 | pin table ×3 packages | transposed register summary | 19 figures | RD1/RD2 differ only by overbar |
| controller-uart/xr16v2550 | Exar | 46 | | **shading = meaning** | 10 figures | `#` suffix convention (good) |
| controller-uart/max3100 | Maxim | 24 | 16-bit command words | DIN/DOUT bit rows | serial timing | two-column text interleaves |
| controller-uart/82c52 | Intersil | 21 | burn-in, die | | ~9 figures | bitfield register figures |
| controller-uart/w65c51n | WDC | 26 | erratum (TDRE) | 1 MHz / 2 MHz groups | read/write/TX/RX | `B` suffix convention |
| pio/w65c22 | WDC | 50 | caveats section | variant-split AC tables | **all bitmaps** (1,226 images) | redraw by hand |
| connector-header/61300211021 | Würth | 7 | materials, certifications | key/value | — | units cut off in extraction |
| connector-barrel/pj-002a, KLDX_0202 | Same Sky, Kycon | 1–3 | materials, switch schematic | part-number builder | — | drawings are raster or outlined art |
| switch/ (2 TE catalogs) | TE | 30–44 | ~20 series, 440–560 part numbers | fill-down blanks, bold = preferred | — | catalog, not datasheet: split per series |

### Patterns

1. **TI logic comes in three generations.** Pre-1990 scans and Harris
   layouts; 1995–2003 "SPDF" sheets (ordering table first, dot-leader
   absolute maximum lists); and the 2016+ numbered template. All three
   end in a package addendum, tape-and-reel pages, mechanical drawings
   and the legal notice: **40–70 % of the text**, all dropped.
2. **Engineer content is 5–15 % of the extracted text.** A 2 MB,
   39-page PDF becomes 12 KB.
3. **Sharing is the norm.** 14 of 15 logic PDFs cover several parts:
   SN54 plus SN74, families (00/LS00/S00), sibling functions
   (4070B/4077B), speed grades, 2–8 pinouts. Oscillator PDFs are
   configurators: frequency × stability × temperature codes, up to 211
   orderables in one PDF. Hence one file per PDF, with a claimed-parts
   list and globs.
4. **Table shapes beyond GFM:**
   - three-level headers (TA group or family, over Min/Typ/Max);
   - row nesting 3–4 deep (parameter → condition → sub-condition → VCC);
   - a column group per part or speed grade;
   - banner rows;
   - "same as above" blanks;
   - meaning carried by shading or bold.

   `<`, `^`, banner rows and the "no meaning in formatting" rule cover
   all of these (format.md § table).
5. **Text extraction can't be trusted for symbols.**
   - Overbars vanish everywhere. TL16C550D's RD1/RD2 differ only by
     polarity.
   - µ, λ and Δ drop out, which makes units wrong (Lite-On "cd" means
     µcd).
   - Equations are images (NXP), and one font is GB-encoded (Orient).

   Conversion has to read page images, not just text.
6. **The sources contain errors:**
   - sn74hc00's I~I~ unit;
   - sc18is604's I2CTO address and a cut-off sentence;
   - AT28C010's ordering rows;
   - ECS's 40 mA column;
   - TE lists one part number twice.

   Conversion notes record each one; a conversion is never a blind
   copy.
7. **Figures fall into five groups:**
   - Digital timing: WaveDrom. Fits everything seen.
   - Standard measurement figures (load circuits, 50 %/10 %/90 %
     waveforms): identical across a vendor's family, so they go in a
     shared figure library.
   - Typical curves: digitized into `curve` blocks.
   - Block, logic and application diagrams: described in a sentence,
     or a sidecar SVG.
   - Package drawings: generated from the catalog footprint plus a
     dimension table.

## What the format can't express yet

1. **Big controllers exceed 30 KB.** SC16IS750, TL16C550D, XR16V2550
   and W65C22 come to 35–55 KB even trimmed: 15–25 registers with
   enumerated bit values, plus 10–20 timing diagrams. A decision is
   needed: a 60 KB budget for `controller`, or a split into several
   files.
2. **Gate-level logic diagrams, block diagrams and application
   schematics** are described in a sentence. An editor that draws
   schematics could embed a `.schematical` snippet instead. That is a
   later feature.
3. **Mechanical drawings** of connectors, displays and LCD modules
   (side views, connector bodies) have no form beyond a dimension table
   or a hand-drawn sidecar SVG. None was drawn in this spike, so the
   effort is unknown.
4. **Parts the catalog doesn't carry.** Pinout, package and ordering
   come from the catalog, so a package the catalog lacks (SN54HC00W/FK
   in CFP and LCCC) loses its drawing and package rows. Only a sentence
   in Ordering keeps it.
5. **Analog content in waveforms.** WaveDrom has no voltage levels: the
   AT28C256 chip erase's 12 V level is a labelled bus segment, and
   power-on ramps and 10 %/90 % points go to shared figures.
6. **Equations** are one plain-text line each, with no fractions or
   radicals. That was enough for the four formulas the samples needed.
7. **Very wide tables.** TXU0104's 18-column switching tables won't fit
   a portrait page. The renderer needs a rule: rotate the table,
   shrink it, or split it by column group.
8. **Overbars inside WaveDrom labels.** The rule is to draw a trailing
   `#` as an overline, which WaveDrom's `tspan` labels can do. The
   samples render the plain `CE#`; the conversion is untested.
9. **Character ROM tables** (HD44780 CGROM): best as one shared
   figure per controller ROM, not per module.
10. **Part-number keys** are plain tables. Nothing checks that the
    catalog's part numbers decode under the key.
11. **Subscript syntax.** Single-tilde `~x~` is subscript here, but
    strikethrough in GFM. The renderer must use a parser with
    strikethrough off and subscript on (markdown-it with the sub/sup
    plugins, or a Swift parser configured the same way).
12. **Values aren't verified against the PDF.** The checker validates
    structure and catalog links, not that 3.15 really is 3.15.
13. **Provenance after the PDFs are gone.** `source.pdf` becomes a dead
    path once the PDFs are removed. It stays as provenance; the
    catalog's `"<pdf> p.N"` notes need rewording to point at sections.

## Size results

| Sample (category, vendor) | Source PDF | PDF pages, KB | Layout text KB | Converted file | Biggest sections |
|---|---|--:|--:|--:|---|
| `samples/ECS-2520MV.md` (osc, ECS) | osc/ECS-2520MV.pdf | 2, 512 | 7.5 | **5.1 KB** | Electrical 1.3 |
| `samples/LTS-4301JR.md` (led-7, Lite-On) | led-7/LTS-4301JR.pdf | 6, 352 | 5.2 | **5.8 KB** | Curves 2.0 |
| `samples/sn74hc00.md` (gate-nand, TI) | gate-nand/sn74hc00.pdf | 39, 2,088 | 118 | **11.9 KB** | Electrical 2.9, Application 1.3 |
| `samples/sn74hc161.md` (counters, TI) | counters/sn74hc161.pdf | 31, 1,396 | 86 | **14.8 KB** | Switching 3.1, Application 2.2 |
| `samples/AT28C256.md` (eeprom, Microchip) | eeprom/AT28C256.pdf | 32, 736 | 82 | **17.4 KB** | Timing diagrams 3.9, Switching 2.7 |
| `samples/sc18is604.md` (controller-i2c, NXP) | controller-i2c/sc18is604.pdf | 29, 500 | 129 | **17.5 KB** | Registers and commands 5.2 |

From the reviewers' estimates for the rest of the sample:

| Group | Expected size |
|---|---|
| LED digits, connectors, simple oscillators | 2–5 KB |
| TI gates, flip-flops, mux/decoders | 6–16 KB |
| Counters, shift registers, TPIC | 10–15 KB |
| TXU translators, TI LMK6x family | 25–40 KB |
| Memories, LCD modules | 8–18 KB |
| Small controllers (SC18IS604, MAX3100, 82C52, W65C51N) | 15–25 KB |
| W65C22 | 28–35 KB |
| 16C550-class UARTs | 40–55 KB |

**So the 20–30 KB target is realistic, and generous**, for everything
except register-heavy UARTs. The limits are now 60 KB for `controller`
and 140 KB for `microprocessor`/`microcontroller`, measured in
[large-parts.md](large-parts.md): controllers and microprocessors fit;
full AVR microcontroller datasheets (190–310 KB) don't, and need a
decision.

## Effort to convert all PDFs

Agent time assumes a converter that reads page images and text and
then runs `check.py`. The six samples took 10–30 minutes each,
including reading the source.

| Group | Files | Per file | Agent hours |
|---|--:|---|--:|
| TI logic with a text layer, base PDFs | ~1,170 | 12–18 min | 230–350 |
| -Q1/-EP/-SP variants (convert as a diff of the base) | 413 | 5 min | 35 |
| Scans: transcribe from page images | 131 | 30–45 min | 65–100 |
| Memories, controllers, PIO, CPU | ~120 | 45–90 min | 90–180 |
| Oscillators, LED displays, connectors, LCD modules, switches | ~140 | 8–15 min | 20–35 |
| Shared figure library (~15 SVGs) and the controller tier decision | — | — | 10 |
| **Total** | **~2,085** | | **~450–700** |

Ways to cut it:

- Run 8–10 conversion agents in parallel: about 50–80 hours of wall
  time.
- Skip exact duplicates such as `ff-d/000036269.pdf`.
- Write a deterministic table pre-extractor for TI's 2016+ template.
  Its tables are regular enough to draft `table` blocks, which could
  halve the largest group.

Human review: spot-check about 10 % per batch against the PDF, plus
every scan and controller. Roughly 60–100 hours.

## Recommendation

Write a **plan with write-plan** ("datasheet viewer and conversion"),
phased so each phase lands on its own:

1. **Format v1 and checker in the tree.**
   - Freeze format.md as `docs/datasheet-format.md`.
   - Move check.py to `scripts/`.
   - Run it in the backend test suite over `Datasheets/`, including the
     cross-file "claimed twice" check.
   - Settle the microcontroller limit (large-parts.md § Options) and the
     home folder (`schematical/schematical/Datasheets/<category>/`).
2. **Renderer.**
   - Show the Markdown in a WKWebView with markdown-it (sub/sup on,
     strikethrough off) and WaveDrom bundled locally.
   - Have Swift generate the `pinout`, `package`, `ordering`, `pins`,
     `curve` and `table` HTML/SVG from the catalog.
   - Print through the web view.
   - Add a "Show Datasheet" command on a placed part and in the parts
     drawer.
3. **Shared figure library:** the TI/Harris load circuits and
   measurement waveforms, the EEPROM/SRAM test load, the clamp diagram.
4. **Pilot conversion:** one category end to end (`osc`, 80 files: small
   and varied), then `gate-nand` (88 TI files) to measure real
   per-file time before scaling.
5. **Batch conversion by category**, one commit per batch:
   - each batch is checker-clean;
   - Conversion notes record dropped content and source errors;
   - a review agent compares every table against the PDF page images
     (open problem 12).
6. **Scans and controllers last**, with the extra review.
7. **Retiring the PDFs:**
   - Separate and decided by the user.
   - Keep them until every file that cites them is converted and
     reviewed.
   - Reword the catalog's `"<pdf> p.N"` notes to point at sections.

Decisions the plan should ask the user for:

- the controller size tier;
- whether parts the catalog doesn't carry may add their own package
  rows (open problem 4);
- whether typical curves are worth digitizing for every logic part, or
  only where they carry design information (power devices, LEDs,
  oscillators).
