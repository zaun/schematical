# Sample correction pass: issue list

The samples were checked against their source PDFs with
[validate.py](validate.py) and by reading the flagged pages, and
corrected under [format.md § Source fidelity](format.md#source-fidelity).
This list is the record of that pass. A sample may be used as a fixture
or template only once its rows here are closed, and `validate.py` reports
no errors for it, or only the known false positives listed at the end.

Kinds:

- **Conversion error**: the file said something the PDF doesn't. Fixed.
- **Source error**: the PDF is wrong, and the PDF itself proves it. The
  file corrects it, names the evidence in a `Source error:` note, and
  lists it under Conversion notes → Source issues.
- **Source conflict**: two places in the PDF disagree and the PDF doesn't
  settle it. The file keeps both, as printed, in a `Source conflict:` note.
- **Unconfirmed**: a vendor value looks wrong but nothing proves it. The
  file keeps the value, flags it `Unconfirmed:`, and suggests no
  replacement.

## Status

`validate.py --quiet samples/*.md` after the pass:

| Sample | Errors | Warnings | Numbers found | Rows matched | Source issues flagged |
|---|--:|--:|--:|--:|--:|
| sn74hc00 | 0 | 12 | 265/265 | 65/65 | 0 |
| sn74hc161 | 0 | 10 | 402/402 | 88/91 | 0 |
| AT28C256 | 0 | 13 | 176/176 | 73/76 | 1 |
| ECS-2520MV | 0 | 6 | 147/147 | 33/37 | 2 |
| LTS-4301JR | 0 | 10 | 56/56 | 18/26 | 0 |
| sc18is604 | 0 | 16 | 316/316 | 93/99 | 10 |
| max14830 | 0 | 49 | 1216/1216 | 228/253 | 3 |
| w65c816s | 0 | 50 | 3641/3641 | 700/728 | 4 |
| attiny85 | 1 † | 204 | 4382/4382 | 797/970 | 50 |
| atmega4809 | 0 | 197 | 6759/6759 | 1537/1717 | 47 |
| atmega328p | 0 | 267 | 6345/6345 | 1225/1376 | 59 |

† A known false positive (see the end): the package row's only label in
the PDF is the letter "C"; the value was checked on the drawing.

The remaining warnings were read. They are reworded sentences that keep
the source's meaning, table notes carried in shorter wording, rows whose
PDF layout splits a cell across lines, and terms the format adds (catalog
pin names). Prose "reworded" items (info level) were not all reviewed in
the three microcontroller files.

## Issues reported in review

All six were confirmed against the PDFs.

| Sample | Kind | Where | Finding | PDF | Action |
|---|---|---|---|---|---|
| sc18is604 | Source conflict | Register map, I2CStat reset | Map gives 0x00; bits 7:4 are fixed at 1 and Table 9 says 0xF0 is the default after reset. | p.4, 7 | Kept 0x00 as printed with a `Source conflict:` note in the map and in the status table. |
| sc18is604 | Source conflict | I²C bus; I²C timing t~SP~ | Table 13 note [2]: SCL and SDA "do not have glitch suppression circuits"; Table 15 lists t~SP~, "spikes that must be suppressed by the input filter", 50 ns. | p.15, 17 | Both kept, each with a note. |
| sc18is604 | Conversion error | I²C timing t~SP~ | The file said "spikes suppressed by the input filter", turning a requirement into a claim. | p.17 | Restored "that must be suppressed". |
| w65c816s | Conversion error | Instruction summary, BIT # | Immediate BIT showed NV....Z.; Table 5-5 has one flag entry for all modes, with note 1: immediate changes only Z. | p.34 | `#` row is ......Z.; the other modes keep NV....Z. |
| w65c816s | Source error | Table 5-7, 16b direct indexed RMW | The wrapped note "(3),(17)" pushes note (1) onto cycle 6 (data low); every other RMW mode puts (1) on the data-high cycle. The file had copied the misplacement. | p.40 | (1) on 6a; `Source issues` entry. |
| AT28C256 | Conversion error | !{DATA} polling waveform | I/O7 was driven while OE# was high and high-impedance while OE# was low; WE# stayed low; CE# was out of step. | p.19 | Redrawn from Figure 6.15 with tDH, tOEH, tOE and tWR. |
| AT28C256 | Conversion error | Toggle bit waveform | Same faults as the polling diagram (CE# held low, WE# as a pulse), no edges. | p.19 | Redrawn from Figure 6.17. |
| max14830 | Source conflict | Auto software flow control | p.26: "An ISR[1] interrupt is generated when XON or XOFF are received"; p.27 (transmitter flow control): "An interrupt is not generated". SpclCharInt bits are set only with special character detection on (MODE2[4]). | p.26–27, 47 | Both statements kept in source wording with a `Source conflict:` note; no reading chosen. |
| ECS-2520MV | Unconfirmed | Current consumption, 160 MHz | 40 mA at every supply voltage, 5–10 times the 60 MHz column. The old note proposed 4.0 mA without evidence. | p.1 | Kept 40 mA, `Unconfirmed:`; replacement value removed. |

## Further issues found in the pass

### sc18is604

| Kind | Where | Finding | PDF | Action |
|---|---|---|---|---|
| Conversion error | SPI slave timing waveform | Edges joined points in the same column (zero-length tv(Q), tDS); data changed on the wrong edge. | p.16 | Redrawn from Figure 17, two columns per half clock. |
| Conversion error | SPI to I²C waveform | tSPILAG1 had zero length; tCSW missing. | p.16 | Redrawn from Figure 18. |
| Conversion error | I²C timing waveform | The "STOP" had SDA rising while SCL was low (not a STOP); tSU;STA and tSU;STO nodes were off their edges. | p.17 | Redrawn from Figure 19. |
| Conversion error | Limiting values | An ESD row with an invented symbol V~ESD~; the PDF gives ESD only in its features list. | p.1 | Symbol removed; note quotes the features list. |
| Conversion error | Operating conditions | Symbol f~SCLK~ invented. | p.15 | Symbol removed; note quotes the source. |
| Conversion error | Package | A comparison with a TI footprint in the body. | — | Moved to Conversion notes. |
| Unconfirmed | SPI slave timing | Figure 17 labels the first bit LSB and the last MSB; MSB first is the default (Table 10). | p.11, 16 | Kept as printed. |
| Source error (already corrected) | I2CTO address, 0xF2 text, 0xF9 text | Table 8 says 0x04 (map: 0x03); 0xF2's heading contradicts its own second sentence; 0xF9 is cut off. | p.6–8 | Now listed under Source issues. |

### AT28C256

| Kind | Where | Finding | PDF | Action |
|---|---|---|---|---|
| Conversion error | Write cycle waveforms (WE#, CE# controlled) | tOEH ended one column before OE# falls. | p.13–14 | Node moved to the OE# edge. |
| Conversion error | Package dimensions | PDIP values converted to mm (6.35, 0.38); the drawing is in inches. | p.23 | Kept in inches as printed, one table per drawing. |
| Conversion error | Description, DC table | "electrically erasable PROM", V~OH~ (source: V~OH1~), invented symbol T~C~. | p.1, 7–8 | Source terms restored. |
| Source error | !{DATA} polling figure | Address bus labelled A0–A16; this part has A0–A14. | p.19 | Corrected, listed. |
| — | Supply and erase voltages | 4.5–5.5 V and 11.5–12.5 V are calculated from "5V ± 10%" and "12.0 V ± 0.5V". | p.7, 15 | Listed under `Derived:`. |

### w65c816s

| Kind | Where | Finding | PDF | Action |
|---|---|---|---|---|
| Source error | Instruction summary, STA a,y | Table 5-5 has no opcode; the opcode matrix gives 99. | p.31, 34 | Kept 99; `Derived:` and Source issues. |
| Unconfirmed | DC table, C~TS~ | One row of values (5 pF) is printed between the C~IN~ and C~TS~ labels; the file gives both rows 5 pF. | p.25 | `Unconfirmed:` note. |
| Conversion error | Pin functions, RESB | "Hold low…" for the source's "must be held low". | p.15 | Source wording restored. |

### max14830

| Kind | Where | Finding | PDF | Action |
|---|---|---|---|---|
| Source error | Register map, GlobalIRQ reset | The map's list of nonzero resets omits GlobalIRQ (implying 0x00); its register page gives 0x0F. The file used 0x0F silently. | p.29, 53 | `Source error:` note with the evidence. |
| Conversion error | Baud-rate example | 189 463 baud; the source prints 189,463.087. | p.20 | Source value. |
| Conversion error | Transmitter sync waveform | Two time-origin nodes sat on signal levels. | p.37 | Moved to the SCLK edge and spacer lanes. |

### ECS-2520MV, LTS-4301JR

| Sample | Kind | Where | Finding | PDF | Action |
|---|---|---|---|---|---|
| ECS-2520MV | — | V~OL~/V~OH~, body, reel | 0.1/0.9 × V~dd~ come from "10% / 90% Vdd"; ± 0.1 body tolerances and 3,000 per reel are only in images. | p.1–2 | `Derived:` / `From images:`, checked on the page images. |
| LTS-4301JR | Conversion error | Package dimensions | "Lead diameter 0.5": the drawing gives 0.5 and 0.3 lead widths, no diameter. | p.3 | Two rows, as drawn; all dimensions listed under `From images:`. |

### attiny85 (agent pass)

About 50 items, all listed in the file's Conversion notes → Source issues.
Main ones:

| Kind | Where | Finding | PDF |
|---|---|---|---|
| Conversion error | Instruction set | A Words column and an operand legend taken from the general AVR instruction set; not in this PDF. | p.202–203 |
| Conversion error | Crystal start-up table | A "recommended usage" column of invented text. | p.30 |
| Conversion error | Waveforms (power-on reset, ADC single conversion, USI three-wire, external clock) | Edges measured the wrong points; tCLCL drawn rise to rise (figure: fall to fall). | p.40, 126, 110, 164 |
| Conversion error | Pin change interrupt | Timing of PCIF stated from the wrong edge. | p.50 |
| Conversion error | About 30 sentences | Negations and obligations differed from the PDF (must/should/not). | various |
| Conversion error | Programming | "low voltage", "12 V" not in the PDF (11.5–12.5 V). | p.151, 156 |
| Conversion error | Several places | A plausible reading chosen silently where the PDF contradicts itself (CLKO during reset, timer register access types, POR revision lists). | p.31/61, 77/90/101, 165 |
| Source error | 9 items | E.g. SRAM sizes, data-memory location counts, CKSEL "00", 32K = 32764 cycles, OC1A described as Compare Match B. | various |
| Source conflict | 11 items | E.g. SPH bit names, unipolar saturation, bipolar resolution 10 vs 9 bits + sign, 256-word vs 256-byte page. | various |
| Unconfirmed | 4 items | E.g. "below 1 LSW", "0000 000aa" (nine bit positions). | p.123, 153 |

### atmega4809 (agent pass)

About 47 items, all listed in the file's Conversion notes → Source issues.
Main ones:

| Kind | Where | Finding | PDF |
|---|---|---|---|
| Conversion error | UPDI bit rates | 900/1800 kbps were unit conversions of 0.9/1.8 Mbps; a note guessed "probably 2.7 V" for a conflicting row. | p.481 |
| Source conflict | UPDI bit rates | Two rates for the same V~DD~ range. | p.481 |
| Conversion error | Package dimensions | Tables transposed and incomplete; UQFN48 values only in a scanned image. | p.522–541 |
| Conversion error | Thermal table | A shifted TQFP48 row placed by guess. Now left out with an `Unconfirmed:` note quoting it. | p.463 |
| Conversion error | Waveforms (TWI, SPI slave, SPI buffer, USART XDIR, UPDI enable) | Edges measured the wrong points; a lost-byte sequence drawn wrong. | p.473, 472, 315, 288, 481 |
| Conversion error | ASI_CTRLA reset | "Corrected" to 0x03 silently; the header says 0x02. | p.450 |
| Conversion error | TCB 8-bit PWM | One reading of a self-contradicting paragraph picked silently. | p.239 |
| Conversion error | Memory map | Extended I/O "4032 B"; the figure says 960. | p.37 |
| Source conflict | 12 items | E.g. VPORT.INTFLAGS access, USART STATUS reset vs DREIF reset, T~DebZ~ 14000 µs vs 13.5 ms. | various |
| Source error | 10 items | E.g. STS operation, TCA split-mode compare names, TWI and AC bit names. | various |

### atmega328p (agent pass)

About 59 items, all listed in the file's Conversion notes → Source issues.
Main ones:

| Kind | Where | Finding | PDF |
|---|---|---|---|
| Conversion error | Waveforms (parallel programming ×3, reset ×4, SPI master/slave, SPI CPHA 0, external clock, TWI, timers, ADC) | Edges on the wrong points, missing parameters, wrong start levels, hi-Z drawn as x, t~CLCL~ rise to rise (figure: fall to fall), clk~T~ drawn running. | p.57–59, 86, 138–160, 175, 250–251, 313–321 |
| Conversion error | DC table | "must not exceed" for "should not exceed"; typicals moved into notes; a condition not in the PDF. | p.322–323 |
| Conversion error | About 20 sentences | Obligations changed (CKDIV8, BODS sequence, WDE timed sequence, USART set-up, serial programming, …). | various |
| Conversion error | Fuse read table | Invented names EFB7…EFB0; the PDF prints FHB7…FHB0 (now kept, flagged). | p.266 |
| Conversion error | Timer2 phase-correct | Modes silently "corrected" to 1/5; the PDF says 3/7. | p.157 |
| Conversion error | Package tables | Notes (BSC/REF, pin 1), drawing numbers and reference dimensions missing; image-only values now listed under `From images:` and checked. | p.636–644 |
| Source conflict | 12 items | E.g. JMP/CALL availability, EEPROM 3.4 vs 3.3 ms, CKSEL codes in text vs tables, 28M1 exposed pad 2.40 vs 2.45 mm. | various |
| Source error | 12 items | E.g. PD2 "PCIE1", "PUO", "OC0A disconnected" in the Timer2 tables, "104,35". | various |
| Unconfirmed | 2 items | E.g. the USART error table's −4.54 % at D = 8; a 0.20 mm thermal-via pitch (the old note suggested 1.20 without evidence). | p.194, 643 |

## Validator limits found in the pass

- **What it caught of the six reported issues.** Run on the samples as
  they were before this pass, validate.py reports the I2CStat reset
  contradiction as an error, and the t~SP~ obligation change and the
  dropped SC18IS604 note as warnings; the AT28C256 polling waveform shows
  only as a node not on a transition. It does not catch the BIT flags,
  the 16b note row, the XON/XOFF contradiction or the ECS replacement
  value: those need a person reading the PDF. It is a filter that
  points at likely problems, not a proof of correctness.
- **Wrapped cells and labels.** A PDF cell that wraps ("VDDx0." / "2", a
  condition on the next line inside a merged group), or a row whose only
  label is one letter ("C 0.127"), can make a correct row an error.
- **`Derived:` scope.** A number listed under `Derived:` or
  `From images:` is accepted anywhere in the file, so listing a common
  value can hide a wrong one elsewhere. Scoping each entry to a table
  would be safer.
- **Output-enable rule.** It skips bus phases the host drives (labels
  with "addr", "cmd", "in", "write"), and columns within two of a write
  strobe; a bidirectional bus drawn without such labels can still be
  flagged.
- **Paraphrase.** The prose check measures shared words; it can't tell a
  fair condensation from a changed meaning, except for negation and
  obligation. Those two checks sometimes compare against the neighbouring
  sentence.
- **Text layer gaps.** Characters missing from the text layer (Δ, µ, ⊕,
  equation glyphs) show up as terms warnings; values only in images must
  be listed under `From images:`.
- **Figures.** Waveform shape and curve points can't be checked from text;
  `--info` lists each with its likely PDF page for a person to compare.
