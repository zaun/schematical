# Sample correction pass: issue list

The samples were checked against their source PDFs with
[validate.py](spike-datasheet-format/validate.py) and by reading the flagged pages, and
corrected under [format.md § Source fidelity](spike-datasheet-format/format.md#source-fidelity).
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

`validate.py --quiet samples/*.md` (in `spike-datasheet-format/`) after the pass:

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

## Validator limits after the AND-gate review (2026-11-21)

The 83 `gate-and` files were reviewed by hand against their PDFs in
several rounds; every kind of mistake a round found became a check in
`check.py` or `validate.py`, the files were corrected, and the round was
repeated. The list above describes the validator before that work; much
of it no longer holds (there are no warnings; a row's values are
compared by their position on the page, column by column; prose is held
to the source's words and order). What a person still finds that the
scripts do not report, from the last rounds:

- **Words added inside a kept sentence.** A sentence passes with up to a
  fifth of its words not in the source's order, so "It is" in front of a
  feature line, or two words added to the end of a note, pass.
- **Conditions.** A condition cell is checked for its numbers and for
  its words being near the row; a condition the row doesn't print whose
  words are near (`I~O~ = 0` beside an `I~CC~` row) passes.
- **Note markers.** That a note exists, whole, is checked; which cell
  carries its marker is not (a marker on the caption that the source
  prints on INPUTS, or on a condition that the source prints on a value).
- **Tables without Min/Typ/Max columns** (load-condition tables, pin
  tables): their headers are not compared with the source's, so a
  dropped group header there passes. Function tables are covered.
- **Scanned sources** (cd4081b, sn74ls08, sn74ls11, sn74ls21 here): the
  position checks need a text layer. Their findings are listed to check
  by eye; table count, joined tables and repeated notes are not checked.
- **A figure's values.** A shared figure's parameter must be printed
  somewhere in the PDF with its unit, not necessarily on the figure's
  page.
- **The converter's own statements.** `Conversion note:`, `Source
  error:` and the Conversion notes list are not checked, except page
  numbers given for a figure, a table or the addendum. A stale note
  ("four tables joined into one" after they were split) passes.
- **Pin descriptions** when the PDF has no pin table are the
  converter's words and are not compared.
- **Meaning.** A source typo kept without a `Source error:` ("SMOS"),
  a value that looks wrong in the source, and a bar the source draws in
  the wrong place need a person. Waveform and curve shapes are by eye.
- **The other samples.** `spike-datasheet-format/samples/sn74hc00.md` was brought to 0 errors
  under the strict validator; the other samples in `spike-datasheet-format/samples/` predate it
  and still report errors there (they pass `check.py`).

`mutate.py` measures the rest: it plants one mistake at a time and
counts how many the scripts report as errors. Weakest kinds: a dropped
table note when a near-identical note remains, and a changed number in
prose when the same number appears elsewhere in the passage.

## Logic gate conversion (2026-11-21)

All seven gate groups are converted with the procedure in
converting.md: gate-and 83, gate-nand 88, gate-nor 53, gate-not 168,
gate-or 52, gate-xnor 4, gate-xor 39 (487 files). All pass `check.py` and are registered in
`datasheets.json`.

**Not converted.** gate-nor/cd54hc4002.pdf: every part it lists is in
the catalog under cd74hc4002 or sn54hc4002, so no part names it.

**Parts added for the conversion.** 126 orderables that eleven PDFs
list and the catalog had left out (military numbers and G4/E4
variants), with an FK pinout for 74_05, 74_14 and 74_U04, the
SN54LS05/SN54S05 flat-pack pinout `w14_5405`, and a TTL timing entry
for 74_86. They were added without a stock check.

**Validator errors left: 27, in 7 files.** A second reviewer compared
every remaining error with the page image; in each the file is right
and the script is wrong. What is left after the fixes that followed:

- Old Harris sheets that set two dot-leader lists side by side
  (gate-not/cd74hcu04, gate-xor/cd4070b): a row of the left list and a
  row of the right list share a line, so each gets the other's values.
- The same row printed in two tables on two pages, placed on the wrong
  one (gate-not/sn74as04 and sn54als04b: the V~IK~ row of the ALS04B
  and AS04 tables).
- A value top-aligned in a tall cell, which the text layer puts a line
  above its row (gate-nand/sn74hct00).
- A symbol printed over its condition, "VICi=" (gate-not/sn74aup3g06).
- A figure note whose subscripts the text layer sets apart, "t PHL",
  matched to a table cell instead (gate-or/sn74lv32a).

Also open: a feature bullet that wraps from the foot of one column to
the head of the next ("Latch-up performance exceeds 250mA / per JESD
17", gate-xor/sn74lv1t86, gate-or/sn74lv1t32) fails the order check, so
those files leave the sentence out; an old-format package addendum
(quantity only, gate-xor/sn74ls86) can't be read by the script and its
ordering lines are a by-eye item.

**Scanned sources.** About 20 files come from scans (old CD4000 and
LS/S TTL sheets); their tables are in the by-eye list, about 1,680
items over all files.

**The other samples.** `spike-datasheet-format/samples/sn74hc00.md` passes the strict
validator. The other ten samples predate it and report 25 to 1,463
errors each (the three AVR samples over 900); the validator's position
checks were built on TI logic sheets and have not been tried on those
layouts, so those counts mix real rewordings with script limits.

**Catalog facts checked against the PDFs and corrected.**

- Datasheet ids renamed to the documents they are: sn74ls226 →
  sn74ls266, sn74ac02 → sn74ac02-q1, sn74act02 → sn74act02-q1 (the old
  URLs return 404 at ti.com, the new ones the PDF).
- 74_86 "LS" timing was the SN7486's (15/11, 18/13 ns); it is now the
  SN74LS86A's (12/10, 20/13 ns, sn74ls86a p.4).
- 8404401CA is an orderable of the SN54HC4002 sheet, not of
  cd74hc4002 (where it is only a marking).
- 1G10, 1G11, 1G27, 1G332, 1G386: "6-pin SON (DRY, DSF)" removed from
  the description; their parts also come in DBV, DCK and YZP.
- 74_14: the note said the group has no FK part; it now has the FK
  pinout and the SN54ACT14 FK parts.

**Catalog doubts left as they are.**

- 74_10: sn74ac10's J/W, BQA and FK drawings and its pin table name
  pins 9/11 (13/16 on FK) 3C/3A where the catalog has 3A/3C; its
  D/DB/N/NS/PW drawing agrees with the catalog. 74_00: sn74ls00 swaps
  A/B of gates 3 and 4 against the catalog p14 pinout. Inputs of one
  gate are interchangeable, so nothing behaves differently.
- sn74lvc1g32 DPW: Figure 4-2 draws A=1, B=2; the catalog has B=1, A=2
  (the catalog note already records the conflict).
- Parts the addenda list as Active that the catalog doesn't carry
  (never stock-checked): SN74AC04WBQARQ1, SN74AHCT14PWR,
  SN74AHCT14BQAR, SN74LVC2GU04DCKR, CLVC2GU04QDCKRQ1, SN74LS19ANSR,
  SN74HCS86DYYR, and the FK orderables of sn54ahc14 and sn54ahct14-sp.
- The catalog has no flag for NRND or Last Time Buy; such parts
  (JM38510/17401BCA, several SN54HC4002 and SN54ACT14 military parts)
  carry the status only in the ordering block.
- Military drawing numbers whose device the PDF never names
  (JM38510/37006, /07003, /07004, /00105, /30003, /07501, /30502) are
  filed by the MIL-M-38510 slash-sheet family (001xx TTL, 070xx/075xx
  S, 300xx/305xx LS, 370xx ALS).

## Citations (2026-11-22)

Every prose paragraph and every table, pins, ordering, wavedrom, curve and
figure block of the 487 gate files now ends with a citation to the page of
its PDF (format.md § Citations; the renderer does not show them): 16,148
in all, 9,315 paragraph numbers, 1,855 labels (`Table 6-3`, `Figure 8-1`),
4,978 unlabelled-table or figure ordinals (`table #2`).

Tools: `cite.py list` numbers a page's paragraphs, `cite.py fill` writes
the citations of a file from where `validate.py` finds its words,
`cite.py scans` lists the ones on picture pages. check.py requires them in
the app's files, validate.py checks them, mutate.py plants five kinds of
wrong citation (page, paragraph number, label, a deleted citation, a
citation in the wrong place: 88–100 % caught on eight files).

**How they were placed.** `fill` placed all but 179 of them in 48 files;
those, and every citation on a picture page (416, in 46 files), were
counted by eye from the page images by agents, which also corrected the
script's citations on those pages (it had put tables on the wrong page, a
pins block under a table, and off-by-one paragraph numbers wherever the
page's text layer is partial).

**What the tools can't check.**

- A paragraph number on a picture page (a scan, or a figure page such as
  the load-circuit page) and every `table #N` / `figure #N` ordinal. The
  validator lists the pages for a person. Conventions the by-eye pass kept
  to: the dot-leader absolute-maximum-ratings list of an old TI sheet
  counts as a table; table ordinals count tables only and figure ordinals
  drawings only, each in reading order; a pins block of an old sheet cites
  its pin drawings as figures; the lettered notes under a load-circuit
  figure are one paragraph, or cite the figure's label.
- A paragraph number depends on poppler's layout analysis. A page whose
  layout is messy (words set one to a block, a cover page over a picture)
  has numbers that are not the visible paragraphs; `cite.py list` prints
  the numbering the validator uses, and that is the reference. A change of
  poppler version can renumber such a page.
- A citation on a page read by OCR (tesseract) is checked for the page,
  not the paragraph.
- `fill` cannot be re-run on a file that has hand-placed citations: it
  strips and redoes all of them.
