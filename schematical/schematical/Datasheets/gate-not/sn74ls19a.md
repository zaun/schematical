---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ls19a",
  "title": "SN74LS19A, SN74LS24A Schmitt-trigger positive-NAND gates and inverters with totem-pole outputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ls19a.pdf",
    "vendor": "Texas Instruments",
    "document": "SDLS138",
    "date": "1988-03",
    "pages": 15
  },
  "devices": ["gate:74_19"],
  "parts": ["SN74LS19ADR", "SN74LS19AN"],
  "packages": {"p14": "D, J, N"},
  "converted": "2026-11-21"
}
---

# SN74LS19A, SN74LS24A Schmitt-trigger positive-NAND gates and inverters with totem-pole outputs

## Description

Each circuit functions as a NAND gate or inverter, but because of the
Schmitt action, it has different input threshold levels for
positive-going (V~T+~) and for negative-going (V~T–~) signals. The
hysteresis or backlash, which is the difference between the two
threshold levels (V~T+~ – V~T–~), is typically 800 millivolts. [[1::2]]

These circuits are temperature-compensated and can be triggered from the
slowest of input ramps and still give clean, jitter-free output signals. [[1::3]]

Improved Line-Receiving Characteristics. P-N-P Inputs Reduce System
Loading. Excellent Noise Immunity with Typical Hysteresis of 0.8 V. [[1::1]]

## Pinout

```pinout pinout=p14
```

Conversion note: the source (p.1) draws this pinout for the SN74LS19A D, J, or N package; it also draws the SN74LS24A, which the catalog carries under another device.

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Output Y
GND: Ground
VCC: Supply voltage
```
[[1::figure #3]]

## Functional description

Conversion note: the logic diagram (positive logic) of the SN74LS19A shows six Schmitt-trigger inverters, 1A to 1Y through 6A to 6Y, with Y = !{A}; the logic symbol gives their pin numbers.

Conversion note: the schematic (each gate) shows inputs A, B, C and D with clamp diodes to GND, the transistor stages between V~CC~ and GND, and the totem-pole output Y, with resistors of 25 kΩ NOM and 120 Ω NOM.

## Function tables

Conversion note: the source prints no function table.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage (see Note 1) | | 7 | V |
| | Input voltage | | 7 | V |
| | Operating free-air temperature range | 0 | 70 | °C |
| | Storage temperature range | -65 | 150 | °C |
```
[[2::table #1]]

Source error: the supply voltage line refers to Note 1, which the page (p.2) does not print.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Min | Nom | Max | Unit |
|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.75 | 5 | 5.25 | V |
| I~OH~ | High-level output current | | | -400 | µA |
| I~OL~ | Low-level output current | | | 8 | mA |
| T~A~ | Operating free-air temperature | 0 | | 70 | °C |
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions [1] | < | < | Min | Typ [2] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~T+~ | V~CC~ = 5 V | | | 1.65 | 1.9 | 2.15 | V |
| V~T–~ | V~CC~ = 5 V | | | 0.75 | 1.0 | 1.25 | V |
| Hysteresis (V~T+~ – V~T–~) | V~CC~ = 5 V | | | 0.4 | 0.9 | | V |
| V~IK~ | V~CC~ = MIN, | I~I~ = –18 mA | | | -1.5 [4] | | V |
| V~OH~ | V~CC~ = MIN, | V~I~ = V~T–min~ | I~OH~ = –0.4 mA | 2.7 | 3.4 | | V |
| V~OL~ | V~CC~ = MIN, | V~I~ = V~T+max~ | I~OL~ = 4 mA | | 0.25 | 0.4 | V |
| ^ | ^ | ^ | I~OL~ = 8 mA | | 0.35 | 0.5 | ^ |
| I~T+~ | V~CC~ = 5 V, | V~I~ = V~T+~ | | | -2 | -20 | µA |
| I~T–~ | V~CC~ = 5 V, | V~I~ = V~T–~ | | | -5 | -30 | µA |
| I~I~ | V~CC~ = MAX, | V~I~ = 7 V | | | 0.1 [4] | | mA |
| I~IH~ | V~CC~ = MAX, | V~I~ = 2.7 V | | | | 20 | µA |
| I~IL~ | V~CC~ = MAX, | V~I~ = 0.4 V | | | | -50 | µA |
| I~OS~ [3] | V~CC~ = MAX, | V~I~ = V~O~ = 0 V | | -20 | | -100 | mA |
| I~CCH~ | V~CC~ = MAX, | V~I~ = 0 V | ’LS19A | | 9.9 | 18 | mA |
| ^ | ^ | ^ | ’LS24A | | 6.6 | 12 | ^ |
| I~CCL~ | V~CC~ = MAX, | V~I~ = 4.5 V | ’LS19A | | 17 | 30 | mA |
| ^ | ^ | ^ | ’LS24A | | 11 | 20 | ^ |

[1] For conditions shown as MIN or MAX, use the appropriate value specified under recommended operating conditions.
[2] All typical values are at V~CC~ = 5 V, T~A~ = 25°C.
[3] Not more than one output should be shorted at a time, and the duration of the short-circuit should not exceed one second.
[4] Unconfirmed: the source prints this value under TYP; kept where printed.
```
[[3::table #1]]

## Switching characteristics

```table id=sw caption="Switching characteristics, V~CC~ = 5 V, T~A~ = 25°C (see Figure 1)"
| Symbol | From (input) | To (output) | Test conditions | SN74LS19A | < | < | SN74LS24A | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | Any | Y | R~L~ = 2 kΩ, C~L~ = 15 pF | | 13 | 20 | | 13 | 20 | ns |
| t~PHL~ | Any | Y | ^ | | 18 | 30 | | 25 | 40 | ns |
```
[[3::table #2]]

t~PLH~ = Propagation delay time, low-to-high-level output [[3::2]]

t~PHL~ = Propagation delay time, high-to-low-level output [[3::2]]

## Parameter measurement information

Conversion note: the load circuit of Figure 1 connects the output under test to the test point, to C~L~ to ground, and through a diode to R~L~ from V~CC~ and to a chain of diodes to ground.

Conversion note: in the voltage waveforms of Figure 1 the input swings from 0 V to 3 V, and t~PHL~ is measured from the 1.9 V point of the rising input and t~PLH~ from the 1.0 V point of the falling input, each to the 1.3 V point of the output between V~OH~ and V~OL~.

All diodes are 1N3064 or equivalent. C~L~ includes probe and circuit
capacitance. The generator characteristics are: PRR = 1 MHz, t~r~ =
15 ns, t~p~ = 6 ns, Z~o~ = 50 Ω. [[4::1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

## Ordering information

```ordering
SN74LS19ADR | Reel of 2500 | 0 to 70 | LS19A
SN74LS19AN | Tube of 25 | 0 to 70 | SN74LS19AN
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum (pp.5–6),
  tape and reel, tube and mechanical pages and the notice (7–15)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are from the package option addendum (p.5).
- Pages 1–4 are scans with no text layer; every value on them was read
  from the page images (110 to 400 dpi) and checked there.
- Dropped: the first feature line, the vendor's pin drawings, the logic
  symbols with their note (p.1), and the SN74LS24A logic diagram with
  its formula. The SN74LS19A logic diagram, the schematic, the load
  circuit and the voltage waveforms are described in text; the
  waveforms match no shared figure, because the input is measured at
  two levels.
- The data sheet also covers the SN74LS24A, whose columns and rows are
  kept in the tables; the catalog carries it under another device. The
  package option addendum also lists SN74LS19ANSR, which this file
  doesn't claim.
- The source has no pin table; the pin descriptions are the
  converter's, after the schematic's inputs A and output Y.
- The electrical characteristics table of the source gives symbols
  only, so it has no Parameter column. The absolute maximum ratings and
  recommended operating conditions print the symbol after the name.
- From images: overbars of Y = !{A} (logic diagram, p.1); 800, 0.8
  (p.1); 25, 120, 7, 0, 70, -65, 150, 4.75, 5, 5.25, -400, 8 (p.2);
  1.65, 1.9, 2.15, 0.75, 1.0, 1.25, 0.4, 0.9, -18, -1.5, -0.4, 2.7,
  3.4, 4, 0.25, 0.35, 0.5, -2, -20, -5, -30, 0.1, 20, -50, -100, 4.5,
  9.9, 18, 6.6, 12, 17, 30, 11, 13, 25, 40, 2, 15 (p.3); 3, 1.9, 1.0,
  1.3, 1N3064, 1, 15, 6, 50 (p.4).
- Source issues:
  - Source error: the absolute maximum ratings (p.2) refer to Note 1,
    which the page does not print.
  - Unconfirmed: V~IK~ –1.5 V and I~I~ 0.1 mA (p.3) are printed under
    TYP, not MAX.
  - Unconfirmed: the first feature line (p.1) says "Functionally and
    Mechanically Identical to ’LS13, ’LS14, and ’LS132, Respectively"
    for the two devices of this data sheet; left out of Description.
  - Unconfirmed: the description (p.1) gives a typical hysteresis of
    800 millivolts and the feature line 0.8 V; the electrical
    characteristics (p.3) give 0.9 V typical.
  - Unconfirmed: note C of Figure 1 (p.4) prints t~p~ = 6 ns after
    t~r~ = 15 ns.
