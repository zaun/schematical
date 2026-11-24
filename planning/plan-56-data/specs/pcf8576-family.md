# PCF8576D / PCF85176 / PCA85176 / PCA85276: 40 x 4 I2C LCD drivers (plan-56-E)

Datasheets (all NXP, `datasheets/lcd-drivers/`; the four maker URLs `https://www.nxp.com/docs/en/data-sheet/<NAME>.pdf` answer a PDF to
a browser User-Agent and are byte-identical to the local files):

| id | file | revision | pages |
|---|---|---|---|
| `pcf8576d` | pcf8576d.pdf | Rev. 15.1, 1 September 2021 | 54 |
| `pcf85176` | pcf85176.pdf | Rev. 5.1, 15 September 2021 | 54 |
| `pca85176` | pca85176.pdf | Rev. 6.1, 13 September 2021 | 52 |
| `pca85276` | pca85276.pdf | Rev. 4, 29 September 2021 | 50 |

Page numbers below are the printed = PDF page numbers (`<id> p.N`). One chip family: the same command set, RAM map and I2C
protocol in all four; they differ in packages, one pin (PCA85276), the subaddress width and the internal clock.

Devices written (`lcddriver.json`, driver `lcd`):

| device | parts (ordering table) | pinouts | why one device |
|---|---|---|---|
| `PCF85176` | PCF8576DT/2,118, PCF8576DT/2,518, PCF8576DT/S400/2,1, PCF8576DT/S400/2Y, PCF85176H/1,518, PCF85176T/1,118, PCF85176T/1Y, PCA85176H/Q900/1,5 | `p56` TSSOP56, `p64` TQFP64 | identical pinout per package, command set, RAM map and 77 Hz typical internal frame (fclk(int) 1850 Hz typ in pcf8576d p.32, pcf85176 p.33, pca85176 p.32 for the H) |
| `PCA85176` | PCA85176T/Q900/1,1, PCA85176T/Q900/1Y | `p56` | same logic and pinout, but the T runs a faster internal clock (fclk(int) 1920 / 2640 / 3600 Hz, ffr 80 / 110 / 150 Hz, pca85176 p.32-33) so its frame time differs (one device per frame time, the sim frame is a device key) |
| `PCA85276` | PCA85276ATT/AJ, PCA85276ATT/AY | `p56` | pin 52 is T1 (no A2), a 2-bit subaddress, fclk(int) 4800 Hz typ (ffr 200 Hz typ, pca85276 p.32) |

## Pins and roles

Supply 1.8-5.5 V (VDD); VLCD 2.5-6.5 V (PCF8576D, PCF85176) or 2.5-8.0 V (PCA85176, PCA85276; 6.5 V while VDD < 2.5 V)
(pcf8576d p.31, pcf85176 p.32, pca85176 p.30-31, pca85276 p.30-31). I2C pins are 5 V tolerant (the same notes). `vcc` [1.8, 5.5];
VLCD is the LCD supply, listed in `analog`.

TSSOP56 SOT364-1 (pcf8576d p.4-6 Figure 2 / Table 3; pcf85176 p.4-5; pca85176 p.5; pca85276 p.4-5):

| pins | name | role |
|---|---|---|
| 1, 2, 3, 56 | BP2, BP1, BP3, BP0 | output |
| 4-43 | S0-S39 (S0 = 4 ... S39 = 43) | output |
| 44 | SDA | inout |
| 45 | SCL | input |
| 46 | SYNC (active low, open-drain output with pull-up, input otherwise; cascade) | inout |
| 47 | CLK (clock in or out) | inout |
| 48 | VDD | power |
| 49 | OSC (VSS: internal oscillator, VDD: external clock on CLK) | input |
| 50, 51, 52 | A0, A1, A2 (hardware subaddress); PCA85276: A0, A1, **T1** (test, tie to VSS) | input |
| 53 | SA0 (I2C address bit 0) | input |
| 54 | VSS | ground |
| 55 | VLCD | power (analog) |

TQFP64 SOT357-1, 10 x 10 x 1.0 mm (PCF85176H, PCA85176H; pcf85176 p.4-5, p.39; pca85176 p.4-6): SDA 10, SCL 11, SYNC 12, CLK 13,
VDD 14, OSC 15, A0-A2 16-18, SA0 19, VSS 20, VLCD 21, BP0 25, BP2 26, BP1 27, BP3 28, S0-S3 29-32, S4-S17 34-47, S18-S33 49-64,
S34-S39 2-7; n.c. (role `nc`, "do not connect and do not use as feed through") 1, 8, 9, 22, 23, 24, 33, 48.

Footprints (no new ones): TSSOP56 = existing `TSSOP-56` (6.1 x 14.0 mm, 0.5 mm; SOT364-1 body D 13.9-14.1, E 6.0-6.2, e 0.5:
pcf8576d p.37, pcf85176 p.40, pca85176 p.38, pca85276 p.37); TQFP64 = existing `LQFP-64-10X10` (10 x 10 mm body, 0.5 mm pitch,
tip span HD/HE 11.85-12.15; pcf85176 p.39, pca85176 p.37).

## I2C bus

Write-only slave; address byte `0 1 1 1 0 0 SA0 R/W` (7-bit 0x38 with SA0 = VSS, 0x39 with SA0 = VDD); R/W must be 0, the chip "will
not respond to a read access" (pcf8576d p.27, pcf85176 p.28, pca85176 p.27-28, pca85276 p.27). Bus model: `address: ["SA0"]`,
`table [56, 57, null, null]`. Up to 16 chips on one bus (SA0 x 3-bit subaddress; PCA85276: 8, SA0 x 2-bit; cascade Table 20
pcf8576d p.34). 400 kHz, SDA 3 mA sink (not modeled).

A transfer: START, address byte, then **command bytes** and **display data bytes**:

```
S | 0111 00 SA0 0 | A | C command | A | ... | 0 last command | A | display data byte | A | ... | P
```

Every command byte has the continuation bit C in bit 7 (pcf8576d p.22 Table 10, p.27-28 Figures 18-19): C = 1: the next byte is another
command; C = 0: this is the last command, every following byte is display data. At least one command byte comes first.

## Commands (pcf8576d p.22-24 Tables 9-15; pcf85176 p.5-8 Tables 4-10; pca85176 p.6-9; pca85276 p.5-8)

| command | bits 7..0 | effect |
|---|---|---|
| mode-set | C 1 0 - E B M1 M0 | E = display enable (0: blank, "all backplane and segment outputs set to VLCD"); B = bias (0: 1/3, 1: 1/2; not for static); M: 01 static (BP0), 10 1:2 (BP0, BP1), 11 1:3 (BP0-BP2), 00 1:4 (BP0-BP3). Defaults E = 0, B = 0, M = 00 |
| load-data-pointer | C 0 P5..P0 | P 0-39 = the display RAM column the next data byte goes to. Default 0 |
| device-select | C 1 1 0 0 A2 A1 A0 (PCA85276: C 1 1 0 0 0 A1 A0) | the subaddress counter (0-7; PCA85276 0-3). Default 0 |
| bank-select | C 1 1 1 1 0 I O | I = input bank (where data is stored), O = output bank (what is shown). Static: 0 = RAM row 0, 1 = row 2; 1:2: 0 = rows 0, 1, 1 = rows 2, 3. No effect in 1:3 and 1:4. Defaults 0, 0 |
| blink-select | C 1 1 1 0 AB BF1 BF0 | AB = 1 alternate-RAM-bank blinking (static and 1:2 only); BF: 00 off, 01, 10, 11 = blink frequency 1, 2, 3 (Table 8: 2, 1, 0.5 Hz at fclk 1536 Hz). Defaults 0 |

Bytes whose bits match none of these are not defined by the sheets (the model drops them).

Power-on reset (pcf8576d p.7 6.1, pcf85176 p.9, pca85176 p.9): every backplane and segment output at VLCD, mode 1:4 with 1/3 bias,
blinking off, banks reset, I2C interface initialized, data pointer and subaddress counter 0, display disabled (E = 0); no I2C traffic for
1 ms (POR threshold VP(POR) 1.0 / 1.3 / 1.6 V, pcf8576d p.31). The sheets do not say what the RAM holds after power-on: the model
starts it cleared. **PCA85276** has no POR (pca85276 p.15-16 6.4): its registers and I2C interface are undefined until a START, a
dummy 0x00 byte and a STOP, then mode-set / load-data-pointer / device-select / bank-select / blink-select are sent; the model starts
in the PCF reset state and has no initialization sequence.

## Display RAM (pcf8576d p.17-21, pcf85176 p.20-26, pca85276 p.9-14)

40 x 4 bits: column g (0-39) is segment output Sg, row k (0-3) is backplane BPk; 1 = on. In a multiplexed mode the four rows are
time-multiplexed with BP0-BP3 (1:3 uses rows 0-2, 1:2 rows 0-1 (or 2-3), static row 0 (or 2)).

**Data byte placement** (Figure 13 pcf8576d p.18, pcf85176 p.21, pca85176 p.21, pca85276 p.11; filling text pcf8576d p.19): with rows = 1 (static), 2,
3, 4 and the data pointer at column P, the byte's bits, **most significant first**, go in groups of `rows` down a column:
bit j (j = 0 for the MSB) to column P + floor(j / rows), row j mod rows (plus 2 when the input bank is 1 in static or 1:2).
So static fills row 0 of 8 columns (c b a f g e d DP of the drawing), 1:2 fills 4 columns x 2 rows (a b f g e c d DP), 1:3 fills 2 columns x 3 rows and
the first two rows of a third column (b DP c a d g f e; "bit 3 of the third address left unchanged", p.19), 1:4 fills 2 columns x 4 rows
(a c b DP f e g d). Tables 6 and 7 (pcf8576d p.20, pcf85176 p.22-23, pca85276 p.12-13) are this rule: three bytes at pointers 0, 3, 6 fill 1:3 as
a7 a4 a1 / a6 a3 a0 / a5 a2 - for columns 0-2; a second byte written at the address of a1 (pointer 2) overwrites a1, a0 with b7, b6.

**Data pointer** after each byte (p.19): + 8 static, + 4 in 1:2, + 3 in 1:3, + 2 in 1:4 (columns). If an I2C access ends early the pointer is
unknown; load it again.

**Subaddress counter** (p.19): storage happens only while the counter (set by device-select) equals the hardware subaddress on
A0-A2 (PCA85276: A0, A1). A non-matching chip still increments its data pointer as if it had stored. The counter is also incremented
when the data pointer overflows (past column 39, to the next chip of a cascade; the sheets do not say what value the pointer then
takes: the model wraps it modulo 40, so the next byte starts where the previous one's bits ended). Bits that run past column 39 in a
byte "fall into the next device that also generates the acknowledge"; a single device or the last in a cascade discards them and does
not acknowledge (p.20 6.10.4). The 3-bit counter wraps 7 -> 0 (2-bit counter 3 -> 0 in the PCA85276), an assumption (the sheets
give no modulus).

**Banks** (p.21 6.10.5-6.10.6): static and 1:2 only. Input bank: where arriving data is stored (rows 0 / 0, 1 or rows 2 / 2, 3).
Output bank: which rows are shown (a bank holds RAM row 0 (or rows 0, 1) of the display, bank 1 rows 2 (or 2, 3)); in the shown slot k the bank-1
row k + 2 is displayed on BPk.

## Outputs and frame

BP signals by mode (pcf8576d p.16 6.9): static: all four BP pins carry one signal; 1:2: BP0 = BP2, BP1 = BP3; 1:3: BP3 = BP1; 1:4: four
signals. Segment output Sg in slot k carries RAM(g, k) XOR the backplane level (the `lcd` driver's convention, docs `lcd`); the
`GENERIC-LCD-*` glass lights the cell whose segment pin differs from its driven backplane. Mode `static` is one slot per frame.

Frame (p.16 6.6): Tfr = 24 / fclk (a full AC cycle: two passes over the backplanes, Figure 11 p.15), the same in every mode. The model's
frame is one pass (one polarity), `frameUs` = Tfr / 2 at the typical internal clock: PCF8576D / PCF85176 / PCA85176H fclk 1850 Hz
-> Tfr 12 973 us -> **6487**; PCA85176T 2640 Hz -> Tfr 9091 us -> **4545**; PCA85276 4800 Hz -> Tfr 5000 us -> **2500**. A slot is the frame
divided by the mode's slot count (the datasheet's frame is fixed per clock in every mode). Bench: BP0's slot recurs every Tfr / 2.

## Ordering tables

| source | type | orderable part | package | notes |
|---|---|---|---|---|
| pcf8576d p.2 Table 2 | PCF8576DT/2 | PCF8576DT/2,118 | TSSOP56 reel, non dry pack | discontinued (202107021DN), drop-in PCF8576DT/2,518; `obsolete` |
| | | PCF8576DT/2,518 | TSSOP56 reel, dry pack | "not for new designs" (replacement PCF85176T/1); kept unflagged |
| | PCF8576DT/S400/2 | PCF8576DT/S400/2,1 | TSSOP56 reel, non dry pack | AEC-Q100 (p.1, p.36); discontinued, drop-in PCF8576DT/S400/2Y; `obsolete`, `automotive`; the table prints "2,1" (a truncation of ",118"?), spelled as printed |
| | | PCF8576DT/S400/2Y | TSSOP56 reel, dry pack | `automotive` |
| | PCF8576DU/DA/2 | PCF8576DU/DA/2,026 | wire-bond die | skipped (below) |
| | PCF8576DU/2DA/2 | PCF8576DU/2DA/2,02 | bare die, 59 bumps | skipped (below) |
| pcf85176 p.2 | PCF85176H/1 | PCF85176H/1,518 | TQFP64 reel, dry pack | |
| | PCF85176T/1 | PCF85176T/1,118 | TSSOP56 reel, non dry pack | discontinued (202107021DN); `obsolete` |
| | | PCF85176T/1Y | TSSOP56 reel, dry pack | |
| pca85176 p.2 | PCA85176H/Q900/1 | PCA85176H/Q900/1,5 | TQFP64 reel, dry pack | AEC-Q100, 95 C; `automotive`; `obsolete` per DigiKey (the task), the sheet does not discontinue it |
| | PCA85176T/Q900/1 | PCA85176T/Q900/1,1 | TSSOP56 reel, non dry pack | discontinued (202107021DN); `obsolete`, `automotive` |
| | | PCA85176T/Q900/1Y | TSSOP56 reel, dry pack | `automotive` |
| pca85276 p.2 | PCA85276ATT/A | PCA85276ATT/AJ | TSSOP56 reel, non dry pack | AEC-Q100 grade 2, 105 C; discontinued (202107021DN); `obsolete`, `automotive` |
| | | PCA85276ATT/AY | TSSOP56 reel, dry pack | `automotive` |

(The table headings print "PCF85178T" / "PCA85178T" for the TSSOP56 type; the topside mark and every other mention say 85176.)

## Not modeled

- LCD bias levels, VLCD, contrast, the analog waveform (the glass reads 0 and 1; display disabled is the pins at VLCD on the chip, released here).
- Blinking: blink-select and the bias bit are stored only.
- CLK, SYNC and OSC: no cascade clock, no synchronization, no external clock; the frame always runs from the internal oscillator.
- The acknowledge of a data byte: a chip whose subaddress does not match does not acknowledge the byte; the engine acknowledges every byte.
- The refusal of a read address byte (the chip is write-only); the engine acknowledges it.
- The bare dies PCF8576DU/DA/2 and PCF8576DU/2DA/2 (no board package; three SDA pads and two SCL pads, which this engine cannot treat as one pin
  each, pad table pcf8576d p.40-42).

## Engine

The `lcd` driver of the converter engine had a fixed mode and no data-pointer RAM load. `pcf8576-lcd-mode.patch` adds (tested with the
bench `pcf8576_family_test`): `mode` as a register field with per-mode backplane `slots`, `outputBank`, a `load` section and the protocol action
`lcdload` (docs section in the patch).
