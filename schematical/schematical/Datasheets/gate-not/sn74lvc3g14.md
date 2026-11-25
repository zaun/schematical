---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc3g14",
  "title": "SN74LVC3G14 triple Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc3g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES367J",
    "date": "2013-11",
    "pages": 21
  },
  "devices": ["gate:3G14"],
  "parts": ["SN74LVC3G14DCTR", "SN74LVC3G14DCUT", "SN74LVC3G14DCUTG4", "SN74LVC3G14YZPR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC3G14 triple Schmitt-trigger inverter

## Description

This triple Schmitt-trigger inverter is designed for 1.65-V to 5.5-V
V~CC~ operation. [[1::1]]

The SN74LVC3G14 contains three inverters and performs the Boolean
function Y = !{A}. The device functions as three independent inverters
but, because of Schmitt action, it may have different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals. [[1::3]]

NanoFree™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::4]]

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down. [[1::5]]

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. [[1::2]]

## Pinout

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A, 3A: Inverter input A, Schmitt trigger
1Y, 2Y, 3Y: Inverter output Y
GND: Ground
VCC: Supply voltage
```
[[1::table #1]]

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[2::table #1]]

Conversion note: the logic diagram (positive logic, p.2) shows three Schmitt-trigger inverters, 1A (pin 1) to 1Y (pin 7), 2A (pin 3) to 2Y (pin 5) and 3A (pin 6) to 3Y (pin 2).

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | DCT package | | 220 | °C/W |
| ^ | ^ | DCU package | | 227 | ^ |
| ^ | ^ | YZP package | | 102 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #2]]

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -16 | ^ |
| ^ | ^ | ^ | | -24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 16 | ^ |
| ^ | ^ | ^ | | 24 | ^ |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 1.65 V | 0.7 | | 1.4 | 0.7 | | 1.4 | V |
| ^ | ^ | ^ | 2.3 V | 1 | | 1.7 | 1 | | 1.7 | ^ |
| ^ | ^ | ^ | 3 V | 1.3 | | 2.2 | 1.3 | | 2.2 | ^ |
| ^ | ^ | ^ | 4.5 V | 1.9 | | 3.1 | 1.9 | | 3.1 | ^ |
| ^ | ^ | ^ | 5.5 V | 2.2 | | 3.7 | 2.2 | | 3.7 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 1.65 V | 0.3 | | 0.7 | 0.3 | | 0.7 | V |
| ^ | ^ | ^ | 2.3 V | 0.4 | | 1 | 0.4 | | 1 | ^ |
| ^ | ^ | ^ | 3 V | 0.6 | | 1.3 | 0.6 | | 1.3 | ^ |
| ^ | ^ | ^ | 4.5 V | 1.1 | | 2 | 1.1 | | 2 | ^ |
| ^ | ^ | ^ | 5.5 V | 1.4 | | 2.5 | 1.4 | | 2.5 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 1.65 V | 0.3 | | 0.8 | 0.3 | | 0.8 | V |
| ^ | ^ | ^ | 2.3 V | 0.4 | | 0.9 | 0.4 | | 0.9 | ^ |
| ^ | ^ | ^ | 3 V | 0.4 | | 1.1 | 0.4 | | 1.1 | ^ |
| ^ | ^ | ^ | 4.5 V | 0.6 | | 1.3 | 0.6 | | 1.3 | ^ |
| ^ | ^ | ^ | 5.5 V | 0.7 | | 1.4 | 0.7 | | 1.4 | ^ |
| V~OH~ | | I~OH~ = -100 µA | 1.65 V to 4.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | ^ |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | ^ |
| ^ | ^ | I~OH~ = -24 mA | 3 V | 2.3 | | | 2.3 | | | ^ |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 4.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | | | 0.75 | ^ |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.75 | ^ |
| I~I~ | A inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | | V~I~ = 5.5 V or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 4.5 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[4::table #1]]

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 17 | 18 | 19 | 22 | pF |
```
[[5::table #1]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | SN74LVC3G14, –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.9 | 9.2 | 1.9 | 5.7 | 2.3 | 5.4 | 1.5 | 4.3 | ns |
```
[[4::table #2]]

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | SN74LVC3G14, –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 3.9 | 9.7 | 1.9 | 6.2 | 2.3 | 5.9 | 1.5 | 4.7 | ns |
```
[[5::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices. [[6::1]]

Conversion note: in the load circuit of Figure 1 one R~L~ goes from the output to ground and a second R~L~ from the output to switch S1, which goes to V~LOAD~, open or GND; the figure below draws the load with S1 open.

```table id=pmi-s1 caption="Figure 1. Switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```
[[6::table #1]]

```figure ref=load-rl-cl caption="Figure 1, load circuit (S1 open)"
RL: see table
CL: see table
```
[[6::Figure 1]]

```table id=pmi-load caption="Figure 1. Load values"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|--:|--:|--:|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #2]]

```figure ref=waveforms-tpd caption="Figure 1, voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[6::Figure 1]]

Conversion note: Figure 1 also shows pulse-duration, setup-and-hold and enable and disable waveforms with their notes B, E and F, which do not apply to this part.

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

```package footprint=DSBGA-8-YZP
```

## Ordering information

```ordering
SN74LVC3G14DCTR | Reel of 3000 | -40 to 125 | 2X15, C14, R, Z
SN74LVC3G14DCUT | Reel of 250 | -40 to 125 | C14J, C14Q, C14R, CR
SN74LVC3G14DCUTG4 | Reel of 250 | -40 to 125 | C14R
SN74LVC3G14YZPR | Reel of 3000 | -40 to 125 | CFN
```
[[8::table #1]]

## Conversion notes

- Source pages 1–6 converted; the revision history (p.7), the package
  option addendum (p.8–9), tape and reel and mechanical pages (10–21)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are from the package option addendum.
- Dropped: the features list except the lines quoted, the ESD caution,
  the vendor pin drawings, the logic diagram (described in text), and
  the pulse-duration, setup-and-hold and enable and disable waveforms of
  Figure 1 with its notes B, E and F.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The electrical characteristics table of the source gives symbols only
  for V~OH~ to C~i~, so those Parameter cells are empty except for
  I~I~. The source prints the C~i~ typical value 4.5 once, between the
  two temperature groups.
- The package option addendum (p.8) also lists SN74LVC3G14DCTRE4,
  SN74LVC3G14DCUR and SN74LVC3G14DCURG4, which the catalog doesn't
  carry.
- From images: overbar of Y = !{A} (p.1).
- From images: ≤2 ns, ≤2.5 ns, 1 kΩ, 500 Ω, 50 Ω, PRR ≤ 10 MHz and
  V~Δ~ in the Figure 1 table and notes (p.6; the text layer has other
  glyphs).
- Source issues:
  - Unconfirmed: the first V~OH~ and V~OL~ rows (p.4) print
    V~CC~ = 1.65 V to 4.5 V, and the recommended supply voltage goes to
    5.5 V (p.3).
