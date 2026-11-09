# Datasheet format spike, part 2: controllers, microprocessors, microcontrollers

Follow-up to [findings.md](findings.md). The question: do per-class size
limits hold for real datasheets converted under [format.md](format.md)
without cutting facts? The first limits asked for were 60 KB for
`controller` and 140 KB for `microprocessor` and `microcontroller`.
Microcontrollers didn't fit 140 KB, so the limits are now **60 KB for
`controller`**, **120 KB for `microprocessor`** and **400 KB for
`microcontroller`** (option B below).

## Answer

| Class | Limit | Measured | Verdict |
|---|--:|---|---|
| `microprocessor` | 120 KB | W65C816S **72.1 KB** (largest processor datasheet in the repo) | **Works**, with room to spare |
| `controller` | 60 KB | MAX14830 **63.6 KB** first pass, **58.3 KB** after the de-duplication rules (largest controller in the repo) | **Works**, just: it needed the de-duplication rules, with no facts cut |
| `microcontroller` | 400 KB | ATtiny85 **192.7 KB**, ATmega328P **310.1 KB**, ATmega4809 **309.0 KB** | **Works**. At 140 KB it didn't: full AVR datasheets are 1.4–2.2× over |

The microcontroller limit was decided from the options in
[Options for microcontrollers](#options-for-microcontrollers): option B,
400 KB. ATmega2560 and ATmega32U4, the largest AVR datasheets in the
repo, are estimated at 330–380 KB, so they should fit too.

## Method

- Six popular AVR datasheets were downloaded from microchip.com into
  `datasheets/mcu/`: ATtiny13A, ATtiny25/45/85, ATmega48A–328P,
  ATmega16U4/32U4, ATmega640–2561 and ATmega4808/4809. The repo had no
  microcontroller PDFs before.
- format.md gained the two classes, four sections (Programming model,
  Instruction set, Memory map, Peripherals), processor drop rules,
  per-class limits and "pending" datasheets (parts not in the catalog
  yet). check.py enforces all of it.
- Five datasheets were converted in full, one per agent, under the rule
  "convert faithfully, never cut facts to fit, report the true size". The
  picks were the worst or most typical cases:
  - MAX14830, the largest controller PDF (68 pages, quad UART);
  - W65C816S, the largest processor datasheet (55 pages);
  - ATtiny85, a small AVR (234 pages);
  - ATmega328P, the most popular AVR (653 pages, about 250 of them
    curves);
  - ATmega4809, the most peripheral-heavy AVR (551 pages).
- Every sample passes check.py apart from the size limit, and every
  WaveDrom block renders with the WaveDrom 3 library.

## Results

### Microprocessor: W65C816S, 72.1 KB

| Section | KB |
|---|--:|
| Instruction set | 42.2 (bus activity per cycle 22.2, instruction summary 12.8, assembler syntax 3.7, addressing modes 3.5) |
| Programming model | 7.5 |
| Functional description | 4.1 |
| Pin functions | 2.8 |
| Switching and electrical characteristics | 4.7 |
| Everything else | about 11 |

- The cycle-by-cycle bus table (330 rows) is the biggest single item and
  stays, because it is what a hardware designer needs.
- The instruction summary has one row per opcode (256 rows). An "Adders"
  column holds letter codes for the M, X, DL and E cycle penalties.
- Estimates for the other processor PDFs:
  - W65C02S: 28–35 KB.
  - Z84C00 (`cpu/ps0178.pdf`, a scan): 50–65 KB.
- `cpu/um0080.pdf` is the 332-page Z80 *user manual*, about 230 pages of
  it one page per instruction. It is a reference manual, not a
  datasheet: leave it out of the conversion, or fold its two unique
  chapters (interrupt response, hardware/software implementation) into
  the Z84C00 file.

120 KB leaves about 1.7× headroom over the largest real case.

### Controller: MAX14830

| Section | KB, first pass | KB, after de-duplication |
|---|--:|--:|
| Registers and commands | 25.3 | 20.6 |
| Functional description | 14.4 | 13.7 |
| Electrical characteristics | 4.6 | 4.6 |
| Timing diagrams | 4.4 | 4.4 |
| Switching characteristics | 2.8 | 2.8 |
| Pin functions | 2.3 | 2.3 |
| Conversion notes | 2.1 | 2.1 |
| Application information | 2.0 | 2.0 |
| Everything else | 5.7 | 5.8 |
| **Total** | **63.6** | **58.3** |

Saved per rule: Access/Reset columns only where bits differ 1.6 KB;
one-field registers as one sentence 1.6 KB; the four interrupt-enable
registers described as mirrors of their status registers 1.1 KB;
reserved bits left out 0.35 KB; prose no longer restating bit tables
0.8 KB.

- The register section (about 22–25 KB) is the fixed cost of every
  MAX310x/MAX14830 sibling.
- First-pass estimates for the siblings (before the de-duplication
  rules):
  - MAX3107 and MAX3108: 50–55 KB.
  - MAX3109: 58–62 KB.
- The 16C550-class UARTs reviewed in part 1 (SC16IS750, TL16C550D,
  XR16V2550) were estimated at 40–55 KB, under the limit.
- With the same rules the siblings should land about 4–5 KB lower than those estimates: MAX3109 around 54–57 KB. 60 KB holds for every controller in the repo, but with little headroom for the MAX310x family; a future, larger controller (an 8-channel UART with its own DMA, say) could exceed it.

### Microcontrollers

| | ATtiny85 | ATmega328P | ATmega4809 |
|---|--:|--:|--:|
| PDF pages | 234 | 653 | 551 |
| Converted size | **192.7 KB** | **310.1 KB** | **309.0 KB** |
| Peripherals section | 130.2 | 237.5 | 213.8 |
| Registers and commands (summary only) | 4.8 | 9.5 | 23.6 |
| Instruction set | 8.9 | 10.0 | 12.8 |
| Typical characteristics (12 curves) | 8.0 | 10.9 | 10.9 |
| Tables / prose / other blocks | 90 / 81 / 22 | 133 / 140 / 37 | 160 / 110 / 39 |

Largest peripheral subsections:

| Part | Subsections (KB) |
|---|---|
| ATtiny85 | memory programming 22.4, ADC 18.8, Timer/Counter0 14.3, USI 11.2, clock 11.1, I/O ports 11.1 |
| ATmega328P | TWI 29.9, USART0 24.8, Timer/Counter1 21.5, Timer/Counter2 18.6, ADC 18.3, I/O ports 16.1, memory programming 15.7, Timer/Counter0 15.1 |
| ATmega4809 | TWI 27.6, USART 23.1, ADC 19.7, UPDI 19.1, TCA 17.2, SPI 14.2, CLKCTRL 11.8, TCB 10.5 |

The ATmega328P converter applied the Access/Reset and reserved-bit
rules (−3.4 KB) but not the "prose doesn't restate bit tables" rule,
which needs a hand rewrite; on the MAX14830 that rule saved about 4 %.
Applied in full, it might take the ATmega328P to about 280–295 KB.

Why they are big: tables and prose are about equal. Neither is padding.

- **Tables** are mode tables, bit tables, start-up and clock tables,
  ADC channel tables, pin-override tables, TWI status tables and
  programming instruction tables.
- **Prose** describes behavior a bit table can't: timer modes, the TWI
  state machine, ADC conversion timing, programming sequences.
- The drop rules already remove most of what could go:
  - typical curves, 40 of 52 (ATtiny85), 61 of 73 (ATmega4809) and
    549 of 563 (ATmega328P);
  - code examples;
  - the opcode matrix;
  - reserved bits;
  - restated bit meanings.

Estimates for the AVRs not converted, by the same method:

| Part | Estimate | Basis |
|---|--:|---|
| ATtiny13A | 145–155 KB | fewer peripherals than ATtiny85, same fixed costs |
| ATmega16U4/32U4 | 330–380 KB | ATmega328P plus the USB device controller, PLL and 4 timers |
| ATmega640–2561 | 330–380 KB | ATmega328P plus external memory interface, 6 timers and 4 USARTs |

## Options for microcontrollers

The first 140 KB limit failed by 1.4× (ATtiny85) to 2.2× (ATmega4809).
Facts can't be cut to fit, so one of these had to give. **Decision: B,
400 KB.**

| Option | What changes | Effect on the samples | Cost |
|---|---|---|---|
| **A. Shared family modules** | A peripheral chapter that recurs across a family becomes one shared file, referenced from each device file like a shared figure. The device file keeps its instances, pin and event maps, vectors, fuses, characteristics and device-specific tables. Examples: the megaAVR-0/tinyAVR-0/1/AVR Dx peripherals; classic AVR USART, SPI, TWI and timers; the AVR instruction set; memory programming. Each file is still counted on its own. | ATmega4809 to about 120 KB, under the limit. ATmega328P to about 120–150 KB. ATtiny85 to about 160–170 KB, still over: its USI, high-speed Timer/Counter1 and ADC are tiny-specific. | A new block type (`module ref=`), a module checker, and module files kept in sync. Modules need instance placeholders (USART*n*, register suffixes). |
| **B. Raise the limit** to about 400 KB | Nothing else changes. | Every sample fits; ATmega2560/32U4 are estimated at 330–380 KB. | The app renders and prints a 300 KB Markdown page: about 150 printed pages of dense tables. That's fine for a viewer with an outline sidebar, heavy for a single scroll. |
| **C. Split per chapter** | An MCU datasheet becomes a core file (description, pins, programming model, memory map, ratings, characteristics, package, ordering) plus one file per peripheral, each under 140 KB. | Same total size. Every file is easily under 140 KB; ATmega4809's TWI is the largest chapter at 27.6 KB. | One datasheet becomes 15–25 files, and the app shows them as one document with chapters. |
| **D. Limit with A**, 200 KB | A and B together. | All three samples fit (about 120–170 KB). ATmega2560/32U4 would be about 180–220 KB with classic-AVR modules, borderline. | As A. |

The spike recommended **D** (shared family modules plus a 200 KB
microcontroller limit); B was chosen instead. Shared modules are still
worth adding later, since they would shrink the files without changing
the limit:

- **The library isn't duplicated.** The six AVR PDFs repeat the same
  USART, SPI, TWI, timer and programming chapters six times, about
  60–70 % of each file. Shared modules mean one fix reaches every
  device.
- **Families stay consistent.**
- **The device file stays readable.** It holds what is particular to
  that chip.

Nothing else in the format depends on the number. The microprocessor
limit is 120 KB, separate from the microcontroller limit.

## Format changes made for these classes

All are in format.md and enforced by check.py:

- **Classes.** `microprocessor` and `microcontroller` are new; the
  `controller` definition is now "peripheral controller".
- **Sections.**
  - Programming model, Instruction set, Memory map and Peripherals are
    new, each with per-class requirements.
  - For microcontrollers, each peripheral is a `###` subsection under
    Peripherals, holding its own register bit tables and
    characteristics. Registers and commands holds only the register
    summary.
- **Instruction set.** An addressing-mode table and one instruction
  summary. The opcode matrix and per-instruction prose are dropped; the
  cycle-by-cycle bus tables are kept.
- **Processor drop rules.**
  - Code examples are dropped, except timed register sequences.
  - At most 12 typical curves, chosen by a fixed list: supply currents,
    pin drive, oscillator, BOD/bandgap. The checker counts them.
- **Register rules** (from the MAX14830 pass, apply to every class):
  - Access/Reset columns appear only when they vary within a register.
  - A single-field register gets one sentence instead of a bit table.
  - Identical instances share one table.
  - Reserved bits are left out.
  - Prose does not restate bit tables.
- **Where things go.**
  - Fuses, lock bits and signature bytes go in Memory map.
  - Errata go in `### Errata` under Functional description.
  - Peripheral characteristic tables stay with their peripheral, and are
    checked as spec tables.
- **Pending datasheets.** `"pending": "<reason>"` with empty `devices`
  and `parts` lets a PDF be converted before its parts are in the
  catalog. Pinout, Package and Ordering then use tables, which are
  replaced by the generated blocks when the parts land.
- **Per-class size limits.** controller 60 KB, microprocessor
  120 KB and microcontroller 400 KB are errors above the limit; the other classes
  warn above 30 KB.
- **Fixes the large samples exposed:**
  - Note markers no longer match bit references: `ISR[2]` and
    `TO[7:1]` are not markers.
  - A marker in a column header no longer hides the column's name.
  - Value cells may be a bound (`<10`) or a rail (`GND`).
  - Every heading has an anchor, `###` subsections included; `##`
    sections keep their fixed anchors.
  - Every in-file link must resolve, and block ids must not clash with
    heading anchors.

## Still open

1. **Rendering 300–400 KB pages.** The 400 KB microcontroller limit
   means the viewer must handle about 150–200 printed pages in one
   document; it needs an outline sidebar and should be tested with
   ATmega328P. Shared family modules (option A) remain a way to shrink
   these files later.
2. **Analog drawings in MCU datasheets.** Reset ramps, the external
   clock drive waveform and timer sawtooth/triangle mode plots have no
   WaveDrom form. They are drawn as digital levels or described in
   prose; a small "analog sketch" figure type may be worth adding.
3. **Instruction-set shapes differ.**
   - AVR PDFs give no opcodes or word counts in their summary; the
     converters added word counts and flagged them.
   - The 65C816 needed an "Adders" column for mode-dependent cycles.
   - The format says what the summary must hold; it does not fix the
     columns.
4. **Programming frames.** AVR high-voltage serial programming (three
   pins, several frames per instruction) was squeezed into a table.
5. **Registers spread over several peripherals** (AVR MCUCR) can't be
   described "once, next to its hardware". The converter put the full
   table under one peripheral and cross-referenced it from the others.
6. **Errors in the sources.**
   - ATmega4809: about 25, logged in its Conversion notes.
   - W65C816S: about 10 in its instruction table.
   - ATmega328P: many, in the notes.

   Large datasheets need the review pass from findings.md even more.
7. **Effort.** The large conversions took one agent about 8–16 minutes
   each when the PDF was split into chunks converted in parallel; the
   agent-hours in findings.md stand. The 96 controller and processor
   PDFs in the repo (80 UARTs, 1 I²C bridge, 2 PIOs, 7 CPUs, 6 MCUs) are
   the slowest group.
