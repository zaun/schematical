# Datasheet file format (spike, version 1)

A datasheet file is a **Markdown file with JSON front matter and typed
fenced blocks**. It replaces one vendor PDF from `datasheets/`. The app
renders it on screen and prints it. Facts the part catalog already has
(part numbers, pinouts, pin numbers, footprints) are **linked, not
repeated**: the renderer draws them from `schematical/schematical/Parts/`.

Spike status: this is a proposal. Nothing in the app, the backend or the
part files reads it yet. `check.py` (beside this file) validates files
against everything below.

## Why this shape

| Option | Verdict |
|---|---|
| Pure JSON | Rejected. Prose (functional description, application notes) is a third to half of a datasheet, and prose in JSON strings is unreadable and unreviewable in diffs. |
| Markdown with YAML front matter | Rejected for the front matter only. Neither the Swift app nor the C backend has a YAML parser; adding one for ten keys is not worth it, and YAML's implicit typing (`NO` → false, `1.10` → 1.1) bites part numbers and versions. |
| **Markdown, JSON front matter, typed fenced blocks** | **Chosen.** Prose stays Markdown. Front matter is JSON, like every other data file in the repo, read with `JSONSerialization` and checked with the same "no unknown key, no duplicate key" rules as the part files. Things Markdown can't do (multi-level tables, timing diagrams, curves, catalog-generated drawings) are fenced blocks with a type, so a plain Markdown viewer still shows readable text and the app's renderer gets structure. |

WaveDrom is used as-is (WaveJSON, which also covers register bit-field
diagrams via its `reg` form), so the renderer can bundle the stock
WaveDrom script.

## File

- Name: `<id>.md`, where `<id>` is the source PDF's base name
  (`sn74hc00.md` for `datasheets/gate-nand/sn74hc00.pdf`). One file per
  vendor PDF, never one per part: a PDF that covers 20 orderables becomes
  one file.
- Proposed home (next plan's decision): `schematical/schematical/Datasheets/<category>/<id>.md`,
  same category folders as `datasheets/`. Sidecar figures, when a file has
  any, go in `<id>.figures/`.
- UTF-8, LF line endings, no tabs. Use the real characters `µ Ω ° ± × ≤ ≥
  θ Ψ λ Δ` rather than spelled-out names.

## Front matter

The file starts with a line `---`, a JSON object, and a line `---`.

```json
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hc00",
  "title": "SNx4HC00 quadruple 2-input NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hc00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS181H",
    "date": "2021-08",
    "pages": 39
  },
  "devices": ["gate:74_00"],
  "parts": ["SN74HC00N", "SN74HC00DR", "SN54HC00J"],
  "packages": {"p14": "D, DB, N, NS, PW, J, W", "lccc20": "FK"},
  "converted": "2026-11-07"
}
```

| Key | Value |
|---|---|
| `format` | always `"schematical-datasheet"` |
| `version` | always `1` |
| `id` | the file's base name; the source PDF's base name |
| `title` | the datasheet title, one line, without the vendor's marketing adjectives |
| `class` | one of `logic`, `memory`, `controller`, `oscillator`, `display`, `module`, `connector`, `switch`, `microprocessor`, `microcontroller`. Picks the required sections and the size limit (below). `controller` is a peripheral controller (UART, I²C/SPI bridge, PIO); a part that runs its own program is `microprocessor` (external memory) or `microcontroller` (on-chip memory and peripherals). |
| `source` | the PDF it was converted from: `pdf` (path under `datasheets/`), `vendor`, `document` (literature number, revision letter included), `date` (`YYYY` or `YYYY-MM`), `pages`. `document` and `date` may be `""` when the PDF has none. |
| `devices` | catalog devices this datasheet describes, each `"<kind>:<device id>"` (`"gate:74_00"`, `"eeprom:AT28C256"`). At least one, unless `pending`. |
| `parts` | the catalog parts this datasheet is *the* datasheet for: exact part names from the part files, each in one of `devices`, or globs with `*` matched against the parts of `devices` (`"ECS-2520MV-*"` claims all 49 ECS-2520MV parts, and parts added later). Orderables the PDF lists but the catalog doesn't carry are not listed here (they may appear in the Ordering table as text). A part is claimed by at most one datasheet. May be empty only for a PDF whose catalog parts all live under another datasheet. |
| `packages` | optional: catalog pinout name → the vendor's package designators that use it (`"p14": "D, DB, N, NS, PW, J, W"`). Labels the pinout drawings and the per-package columns of the pin table. |
| `converted` | date of conversion, `YYYY-MM-DD` |
| `pending` | optional: why none of the PDF's parts is in the catalog yet (`"AVR microcontrollers are not in the catalog yet"`). Only with `"devices": []` and `"parts": []`; see [Pending datasheets](#pending-datasheets). |

No other key is allowed, and no key may appear twice.

### Linking to the catalog

- **Parts.** `parts` is the only list of orderables to maintain. The app
  builds the reverse index (part → datasheet) at load, so a placed
  part's "Show Datasheet" opens the right file. The Ordering section is
  generated from these parts' catalog entries.
- **Pins.** Pin numbers, ball names and roles come from the catalog
  pinouts of the claimed parts. The datasheet only supplies, per pin
  *name*, a description (`pins` block). Names must be catalog names
  (`CE#`, `CLR`), so the datasheet can never disagree with the simulation
  about which pin is which.
- **Pinout drawings** are generated from the catalog pinout (`pinout`
  block); nobody draws them by hand.
- **Footprints.** The Package section's land pattern and body outline are
  drawn from `footprints.json` for every footprint the claimed parts use
  (`package` block). Vendor package drawings are not copied; only the
  dimensions the catalog footprint lacks (height, lead dimensions) go in
  a table.
- The catalog's `note` fields keep their `"<pdf> p.N"` references; those
  become `"<id>.md § <section>"` once the PDFs are gone (later plan).

### Pending datasheets

A PDF may be converted before its parts are in the catalog (the AVR
microcontrollers today). Such a file has `"devices": []`, `"parts": []`
and a `pending` reason, and then:

- `pins` names are not checked against a catalog pinout, and are not
  required to cover one.
- The Pinout section holds a `table` instead of `pinout` blocks: pin name,
  then one column of pin numbers (or balls) per package.
- The Package section holds `table`s (package, body, pitch, height)
  instead of `package` blocks; the Ordering section holds a `table`
  (part, package, temperature range, speed or grade) instead of an
  `ordering` block.

When the parts are added to the catalog, the tables are replaced by the
generated blocks, `pending` is removed, and `devices`/`parts` are filled
in; everything else stays.

## Body

After the front matter: exactly one `# ` heading (the title, usually the
same as `title`), then the sections, each a `## ` heading with one of the
fixed titles below, in this order. Inside a section, `###` and deeper
headings are free.

| # | `##` title | Anchor | logic | memory | controller | oscillator | display | module | connector | switch | µproc. | µctrl. |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | Description | `description` | R | R | R | R | R | R | R | R | R | R |
| 2 | Pinout | `pinout` | R | R | R | R | R | R | R | R | R | R |
| 3 | Pin functions | `pin-functions` | R | R | R | R | R | R | O | O | R | R |
| 4 | Functional description | `functional-description` | O | R | R | O | O | O | O | O | R | R |
| 5 | Programming model | `programming-model` | – | – | – | – | – | – | – | – | R | R |
| 6 | Instruction set | `instruction-set` | – | – | – | – | – | – | – | – | R | R |
| 7 | Memory map | `memory-map` | – | O | O | – | – | O | – | – | O | R |
| 8 | Peripherals | `peripherals` | – | – | – | – | – | – | – | – | O | R |
| 9 | Function tables | `function-tables` | R | R | O | O | O | O | – | O | O | O |
| 10 | Registers and commands | `registers` | – | O | R | – | – | O | – | – | O | R |
| 11 | Absolute maximum ratings | `absolute-maximum-ratings` | R | R | R | R | R | R | O | O | R | R |
| 12 | Recommended operating conditions | `recommended-operating-conditions` | R | R | R | O | O | O | – | – | R | R |
| 13 | Thermal information | `thermal-information` | O | O | O | O | – | – | – | – | O | O |
| 14 | Electrical characteristics | `electrical-characteristics` | R | R | R | R | R | R | R | R | R | R |
| 15 | Switching characteristics | `switching-characteristics` | R | R | R | O | – | O | – | O | R | R |
| 16 | Timing diagrams | `timing-diagrams` | O | R | R | O | – | O | – | – | R | O |
| 17 | Typical characteristics | `typical-characteristics` | O | O | O | O | O | O | – | – | O | O |
| 18 | Parameter measurement information | `parameter-measurement` | O | O | O | O | – | – | – | – | O | O |
| 19 | Application information | `application-information` | O | O | O | O | O | O | O | O | O | O |
| 20 | Package | `package` | R | R | R | R | R | R | R | R | R | R |
| 21 | Ordering information | `ordering` | R | R | R | R | R | R | R | R | R | R |
| 22 | Conversion notes | `conversion-notes` | R | R | R | R | R | R | R | R | R | R |

R = required, O = optional, – = not allowed for that class.

What goes where:

- **Description**: what the part does, in one to three short paragraphs.
  No feature bullets; a feature that matters (5 V tolerant inputs,
  Schmitt trigger) is stated once where it applies.
- **Electrical characteristics** holds DC characteristics, capacitances,
  supply currents, and for displays the optical characteristics.
  Connector ratings (current, voltage, contact resistance, mating
  cycles) go here too.
- **Switching characteristics** holds both switching characteristics
  (what the part does) and timing requirements (what the part needs);
  give them separate tables under `###` headings.
- **Timing diagrams** are WaveDrom. Functional sequences (a counter's
  clear/load/count/inhibit run, a shift register's load and shift) and
  cycle diagrams (read, write, page write, SPI frames) go here, in that
  order.
- **Registers and commands**: a register map table (address, name, bits
  7…0, access, reset value), then one bit table per register (bits
  `7:6`, name, description with values `00 = …`). The bit table has
  Access and Reset columns only when they differ between the register's
  bits; otherwise the map row says them once. A register holding one
  field (a counter, a compare value, a data register) gets one sentence,
  not a bit table. Identical instances (USART0–3, TCB0–3, UART channels
  0–3) and registers that differ only in what a write does
  (DIR/DIRSET/DIRCLR) share one table that names them all. Reserved bits
  are left out of bit tables; one sentence per file says how to write
  them. Prose describes behavior and refers to bits by name; it does not
  restate what a bit table already says (no "bit 3, RXEN, enables the
  receiver" paragraphs). When a
  register is only reachable under a condition (`LCR = 0xBF`, `DLAB = 1`,
  `EFR[4] = 1`), say so in an `Access` column, never with shading or bold.
  A WaveDrom `reg` diagram may follow a bit table; the table stays the
  source of truth. Command protocols (SPI command bytes, LCD instruction
  words) are tables of byte sequences, plus one WaveDrom frame per
  command when the PDF draws one.
- **Programming model** (processors): the programmer-visible registers
  (a WaveDrom `reg` diagram or a table per register: width, reset
  value, bit meanings), the status/flag register bit by bit, the stack,
  reset and interrupt behavior, and processor modes (65C816
  emulation/native, AVR sleep modes belong in Peripherals).
- **Instruction set**: an addressing-mode table (syntax, operand, how
  the effective address is formed) and **one instruction summary
  table**: mnemonic, operation, flags affected, then per addressing mode
  the opcode, bytes and cycles (or, for a fixed-format ISA like AVR,
  operands, opcode pattern, words, cycles). A 16 × 16 opcode matrix
  repeats the summary and is dropped. Cycle-by-cycle bus tables
  (65xx "what is on the bus in each cycle") stay: they are what a
  hardware designer needs. Long per-instruction prose (one page per
  instruction, as in instruction-set manuals) is not a datasheet's job
  and is dropped; the summary's Operation column carries the meaning.
- **Memory map**: address ranges (program, data, I/O, reserved), the
  interrupt/reset vector table, and the I/O register address list when
  it isn't given in Registers and commands.
- **Peripherals** (microcontrollers): one `###` subsection per
  peripheral (clock system, power management and sleep, reset and
  watchdog, interrupts, I/O ports, each timer, USART, SPI, TWI/I²C, USI,
  ADC, analog comparator, EEPROM, self-programming, debug, memory
  programming), in the PDF's order. Each holds its description, its
  register bit tables (as under Registers and commands) and its own
  timing and electrical tables. Registers and commands then holds only
  the register summary (address, name, bits 7…0), so a register is
  described once, next to the hardware it controls.
- Fuses, lock bits and signature bytes (microcontrollers) go in Memory
  map, as bit tables like registers but with the fuse byte's name and
  default instead of an address and reset value.
- Current errata go in a last `### Errata` subsection of Functional
  description.
- Tables of characteristics that belong to one peripheral (ADC accuracy,
  TWI timing, oscillator accuracy) stay in that peripheral's subsection;
  they follow the spec-table rules, and the checker treats them as spec
  tables.
- **Conversion notes**: what was dropped beyond the standard drop list,
  what was approximated (digitized curves, redrawn waveforms), and known
  errors in the source (e.g. a unit typo).

Always dropped (never converted): feature bullet lists, marketing text,
applications lists, table of contents, revision history, support /
E2E / trademark / glossary / legal and "important notice" pages, ESD
caution boxes, tape-and-reel and tray dimensions, soldering process
essays, stencil and solder-mask drawings, package marking diagrams, and
the vendor's own pinout drawings and package outlines (both generated
from the catalog instead).

Processor classes drop more: the opcode matrix (it repeats the
instruction summary), code examples (C and assembly snippets; one short
sequence may stay where a register write needs a timed sequence, such
as the AVR watchdog or clock-prescaler change), "data sheet / errata /
device ID" history beyond the current errata, and **typical-
characteristic curves beyond twelve**. The twelve kept are those that
size a design: active, idle and power-down supply current against
frequency or V~CC~, I/O pin output voltage against current (source and
sink), internal oscillator frequency against V~CC~ and temperature,
and brown-out/bandgap levels against temperature. The others are named
in Conversion notes.

## Inline markup

Plain CommonMark plus GFM, with three additions:

| Write | Renders | Use |
|---|---|---|
| `V~CC~`, `t~PLH~`, `R~θJA~` | subscript | every symbol with a subscript |
| `10^6^` | superscript | exponents |
| `!{CLR}` | CLR with an overline | active-low signal names in prose and tables |

Pin names inside `pins` blocks and WaveDrom names are catalog names
(`CE#`); the renderer draws a trailing `#` as an overline.
Cross references are ordinary links to anchors. Every heading has the
GitHub-style anchor of its text (lowercase, punctuation removed, spaces
to `-`), `###` subsections included, and every block with an `id` has
that id: `[recommended operating conditions](#recommended-operating-conditions)`,
`[USART0](#usart0)`, `[the function table](#function)`. Headings inside
one file must give distinct anchors.

## Fenced blocks

A block's info string is its type, optionally followed by attributes
`key=value` or `key="value with spaces"`. Unknown types are errors.

### `table`

```table id=roc caption="Recommended operating conditions" parts=SN74*
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5 | 6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | | V |
| ^ | ^ | V~CC~ = 4.5 V | 3.15 | | | ^ |

[1] Unused inputs must be held at V~CC~ or GND.
```

A GFM pipe table, with these additions:

- **Several header rows.** Every row before the `|---|` delimiter row is a
  header row.
- **`<` merges a cell into the one on its left** (column span), **`^`
  merges it into the one above** (row span). This gives multi-level
  headers (`25 °C` over Min/Typ/Max) and parameters that span several
  condition rows (V~OH~ over five I~OH~/V~CC~ rows). Every row has the
  same number of cells; a row's first cell can't be `<`, and the first
  header row and first body row can't hold `^`.
- **Notes.** After the rows, a blank line, then one note per line,
  `[n] text`. Cells, headers and the caption reference them as `[n]`,
  with a space or the start of the cell before the `[`. `ISR[2]` and
  `TO[7:1]` (a name directly followed by brackets) are bit references,
  not markers. Every marker
  needs a note and every note a marker. Notes are printed under their
  table (not at the end of the document).
- Attributes: `id` (anchor), `caption`, `values` (see spec tables), `parts` (comma-separated part
  name globs this table applies to, such as `SN54*`; the renderer marks
  the tables that don't apply to the part the user opened the datasheet
  from), `conditions` (the "over recommended operating free-air
  temperature range" line, printed under the caption).

**Spec tables** are the tables in sections 7–11. Rules:

- The last header row names the columns. Recognized names
  (case-insensitive): `Symbol`, `Parameter`, `Conditions` / `Test
  conditions`, `From`, `To`, `Min`, `Typ`, `Nom`, `Max`, `Value`, `Unit`.
  Any other name is a condition column (`V~CC~`, `T~A~`, a package name,
  a speed grade).
- A spec table has a `Unit` column and at least one of `Min`, `Typ`,
  `Nom`, `Max`, `Value`. A table whose value columns are conditions
  (current per frequency band, θ~JA~ per package) says what its values
  are with `values=Max` (or `Typ`, …): then every column except the
  first and the named ones holds values. Thermal tables need only the
  `Unit` column. The unit is the column, never part of a value:
  `±20` with unit `mA`, not `±20 mA`.
- Min/Typ/Nom/Max/Value cells are a number (ASCII `-` for negative,
  optional `±`, optional `<`, `>`, `≤` or `≥` before it for values the
  source gives only as a bound, such as a typical "< 10"), a rail name
  (`GND`, `VCC`, `VDD`, `VSS`, `AVCC`, `AREF`), an expression on a symbol (`V~CC~`, `V~CC~ + 0.5`,
  `0.7 × V~DD~`), a ratio (`45/55`, `2:1`), or empty. A dash for "none"
  is written as an empty cell.
- Column groups (temperature ranges, SN54 vs SN74, speed grades -15/-20,
  standard vs fast mode) are header groups made with `<`: one table, not
  one table per group.
- One parameter, several conditions: one row per condition, with `^` in
  the Symbol and Parameter cells. Never pack two values into one cell.
- Parameters in lists ("Supply voltage range, V~CC~ . . . −0.5 V to 7 V",
  the old TI style) become ordinary rows with Min and Max.
- A banner row (a heading inside a table, "SPI slave interface") is a
  row whose first cell holds the text and every other cell is `<`.
- Meaning is never carried by formatting: a "preferred" part, a shaded
  bit or a bold value becomes a column or a note.
- A blank cell meaning "same as above" in the source is written `^`.

### `pins`

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Positive supply
NC: Not internally connected
```

One line per description: catalog pin names, comma-separated, `:`, the
description. Rendered as a table: name, the pin number (or ball) in
every pinout of the claimed parts, I/O type from the catalog role, and
the description. Every catalog pin name of every claimed part's pinout
must be described, except `nc` and `absent` pins (optional); a name may
come from any pinout of the listed devices.

### `pinout`

```pinout pinout=p14
```

Empty body. Draws the catalog pinout as a top-view package drawing (dual
row, four-sided or grid, picked from the part's footprint style),
labelled with `packages`. Without `pinout=`, one drawing per pinout of
the claimed parts. Any pinout of the listed devices may be drawn, even
one no claimed part uses (the FK pinout of a device whose FK parts the
catalog doesn't carry). For 7-segment and character-LCD devices the
drawing adds the segment identification and the internal circuit (which
anode and cathode pins drive which segment), from the kind's segment
keys, so neither is transcribed.

### `wavedrom`

Strict-JSON WaveJSON: an object with `signal` (timing diagram) or `reg`
(register bit fields). Attributes `id`, `caption`. Rules:

- Signal names are catalog pin names where the signal is a pin.
- Node names are **uppercase** letters: WaveDrom prints lowercase node
  names on the diagram and hides uppercase ones.
- `config` may set `hscale` (2 or 3 for diagrams with many edge labels
  or long bus labels); no other WaveDrom option (skins, `head`/`foot`
  text) is used, so every diagram looks alike.
- Timing parameters are `edge` labels between `node`s, named exactly as
  in the switching tables (`t~su~` is written `tsu` here, since
  WaveDrom labels are plain text).
- The number of `=`/`2`–`9` data cells in a wave equals the length of
  its `data` array.
- Analog detail (input levels, 10 %/90 % points, slew) is not drawn;
  it lives in Parameter measurement information.

### `curve`

```curve id=voh caption="Typical V~OH~ vs I~OH~, T~A~ = 25 °C"
{"x": {"label": "I_OH", "unit": "mA", "range": [0, 6]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 7]},
 "series": [
   {"label": "VCC = 6 V", "points": [[0, 6], [2, 5.92], [4, 5.84], [6, 5.76]]}
 ]}
```

A typical-characteristics graph, digitized from the PDF: axis `label`,
`unit`, `range` and optional `"log": true`, then one or more series of
`[x, y]` points (x ascending, at most 24 points per series). Rendered as
a line chart. Typical curves are informative only, so digitizing to
about 2 % of full scale is enough; say so in Conversion notes.

### `figure`

```figure ref=load-cmos-50pf caption="Load circuit"
```

```figure src=at28c256.figures/block.svg caption="Block diagram"
```

Anything that is neither a table, a waveform nor a curve: test/load
circuits, block diagrams, logic diagrams, application circuits,
mechanical drawings of connectors and displays. Two forms:

- `ref=`: a **shared figure** from the app's figure library
  (`Datasheets/figures/<ref>.svg`), with optional parameters in the body
  as `key: value` lines (`CL: 50 pF`). Most logic datasheets use the same
  few load circuits and measurement waveforms (TI's 50 pF CMOS load,
  15 pF/50 pF bus-interface load, open-drain load, the 50 %/10 %/90 %
  waveforms), so each is drawn once for the whole library. The library
  starts with the refs the samples use: `load-cmos-50pf`,
  `waveforms-cmos-tpd-tt`, `waveforms-cmos-tsu-th-tw`,
  `load-eeprom-ttl`, `clamp-diodes`.
- `src=`: a sidecar SVG in `<id>.figures/`, for a figure only this part
  has. Sidecars are counted in the file's size.

A figure that is neither worth drawing nor shared is described in one
sentence of text instead (`Logic diagram: four 2-input NAND gates.`).

### `package`

```package footprint=SOIC-14
```

Empty body. Draws the catalog footprint (body outline and pads, with
the pad size, pitch and span dimensioned) and lists which claimed parts
use it. One block per footprint the claimed parts use; the checker
reports any footprint left out. Dimensions the footprint doesn't hold
(seated height, lead width, standoff) follow as an ordinary `table`.

### `ordering`

```ordering
SN74HC00N | Tube of 25 | -40 to 85 | SN74HC00N
SN74HC00DR | Reel of 2500 | -40 to 85 | HC00
```

Generated: one row per claimed part with its package, pinout, footprint
and flags from the catalog. Optional body lines add vendor columns,
`<part> | <packing> | <temp range °C> | <marking>`; every part named must
be claimed. A part-numbering key (oscillator frequency/stability codes,
EEPROM option letters) is an ordinary `table` after the block.

### `equation`

```equation
f~SCL~ = 15 000 000 / (8 × I2CClk)
```

One formula per line, plain text with the inline markup above, rendered
centred. (No TeX: the samples needed four formulas in total.)

## Size

The size of a file is its `.md` file's UTF-8 byte count plus any
sidecar SVGs; generated blocks cost nothing.

| Class | Limit | Checker |
|---|--:|---|
| `controller` | 60 KB | error above |
| `microprocessor` | 120 KB | error above |
| `microcontroller` | 400 KB | error above |
| every other class | target 20–30 KB | warning above 30 KB |

`check.py` reports the size of every section so the big ones are easy to
find. A file over its limit is cut by the drop rules above, never by
leaving out ratings, characteristics, timing or register meanings; if
the rules can't bring it under, that is a format problem to raise, not
a reason to drop facts.

## Checker

`python3 -I planning/spike-datasheet-format/check.py <file.md>…` checks:
front matter keys and types; catalog links (devices, parts, part ↔
device, a part claimed twice across the files given, pin names in
`pins` blocks and full coverage, `pinout`/`package` names, `ordering`
rows); section titles, order and the required set for the class; the
`table` grid (cell counts, merges, notes ↔ markers, spec-table columns
and value cells); WaveJSON (parses, `signal`/`reg`, data cells vs
`data`, edge nodes exist, no node named twice); `curve` JSON; `figure` refs and sidecars;
inline `!{…}` balance; and the size report. Exit status 1 on any error.
