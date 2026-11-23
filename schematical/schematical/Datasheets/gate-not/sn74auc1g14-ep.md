---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc1g14-ep",
  "title": "SN74AUC1G14-EP single Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74auc1g14-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES673",
    "date": "2006-09",
    "pages": 14
  },
  "devices": ["gate:1G14"],
  "parts": ["SN74AUC1G14MDBVREP"],
  "packages": {"p5": "DBV"},
  "converted": "2026-11-21"
}
---

# SN74AUC1G14-EP single Schmitt-trigger inverter

## Description

Extended temperature performance of –55°C to 125°C. Optimized for 1.8-V
operation and is 3.6-V I/O tolerant to support mixed-mode signal
operation. [[1::1]]

This single Schmitt-trigger inverter is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation. [[1::5]]

The SN74AUC1G14-EP contains one inverter and performs the Boolean
function Y = !{A}. The device functions as an independent inverter, but
because of Schmitt action, it may have different input threshold levels
for positive-going (V~T+~) and negative-going (V~T–~) signals. [[1::6]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[2::2]]

For more information about AUC Little Logic devices, please refer to the
TI application report, Applications of Texas Instruments AUC Sub-1-V
Little Logic Devices, literature number SCEA027. [[2::3]]

## Pinout

```pinout pinout=p5
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions of A, Y, GND and VCC are the converter's.

```pins
A: Inverter input
Y: Inverter output
GND: Ground
VCC: Supply voltage
NC: No internal connection
```
[[1::table #1]]

## Functional description

Conversion note: the logic diagram (positive logic) shows one Schmitt-trigger inverter, A (pin 2) to Y (pin 4).

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[2::table #1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 3.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 3.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 3.6 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [3] | DBV package | | 206 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #2]]

Latch-up performance exceeds 100 mA per JESD 78, Class II. ESD
protection exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | -3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | -5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | -8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -9 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | ^ |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | ^ |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[3::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | < | 0.8 V | | 0.5 | | V |
| ^ | ^ | | < | 1.1 V | 0.51 | | 0.86 | ^ |
| ^ | ^ | | < | 1.4 V | 0.65 | | 1 | ^ |
| ^ | ^ | | < | 1.65 V | 0.79 | | 1.16 | ^ |
| ^ | ^ | | < | 2.3 V | 1.11 | | 1.56 | ^ |
| V~T–~ | Negative-going input threshold voltage | | < | 0.8 V | | 0.3 | | V |
| ^ | ^ | | < | 1.1 V | 0.22 | | 0.53 | ^ |
| ^ | ^ | | < | 1.4 V | 0.3 | | 0.58 | ^ |
| ^ | ^ | | < | 1.65 V | 0.39 | | 0.62 | ^ |
| ^ | ^ | | < | 2.3 V | 0.58 | | 0.87 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | < | 0.8 V | | 0.21 | | V |
| ^ | ^ | | < | 1.1 V | 0.25 | | 0.38 | ^ |
| ^ | ^ | | < | 1.4 V | 0.31 | | 0.5 | ^ |
| ^ | ^ | | < | 1.65 V | 0.37 | | 0.62 | ^ |
| ^ | ^ | | < | 2.3 V | 0.48 | | 0.77 | ^ |
| V~OH~ | | I~OH~ = –100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = –0.7 mA | < | 0.8 V | | 0.55 | | ^ |
| ^ | ^ | I~OH~ = –3 mA | < | 1.1 V | 0.8 | | | ^ |
| ^ | ^ | I~OH~ = –5 mA | < | 1.4 V | 1 | | | ^ |
| ^ | ^ | I~OH~ = –8 mA | < | 1.65 V | 1.2 | | | ^ |
| ^ | ^ | I~OH~ = –9 mA | < | 2.3 V | 1.8 | | | ^ |
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | ^ |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | ^ |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | ^ |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | ^ |
| I~I~ | A input | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 3.5 | | pF |

[1] All typical values are at T~A~ = 25°C.
```
[[4::table #1]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 14 | 15 | 15 | 16 | 19 | pF |
```
[[5::table #1]]

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 5.8 | 0.7 | 5.5 | 0.6 | 4.5 | 0.5 | 4.0 | 0.5 | 2.0 | ns |
```
[[4::table #2]]

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.7 | 1.6 | 3.0 | 0.5 | 3.0 | ns |
```
[[5::table #2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators have the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices. [[6::1]]

Source error: note C of Figure 1 prints "generators have the following characteristics"; kept as printed.

Conversion note: the source's test table sets S1 open for
t~PLH~/t~PHL~, which leaves R~L~ and C~L~ from the output to GND, and its
waveforms draw the input from 0 V to V~CC~ with the delays measured at
V~CC~/2.

```table id=pmi-load caption="Load values"
| V~CC~ | C~L~ | R~L~ | V~Δ~ |
|---|--:|--:|--:|
| 0.8 V | 15 pF | 2 kΩ | 0.1 V |
| 1.2 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.5 V ± 0.1 V | 15 pF | 2 kΩ | 0.1 V |
| 1.8 V ± 0.15 V | 15 pF | 2 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 15 pF | 2 kΩ | 0.15 V |
| 1.8 V ± 0.15 V | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | 30 pF | 500 Ω | 0.15 V |
```
[[6::table #1]]

```figure ref=load-rl-cl caption="Load circuit for t~PLH~/t~PHL~ (S1 open)"
RL: see table
CL: see table
```
[[6::Figure 1]]

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```
[[6::figure #1]]

## Package

```package footprint=SOT23-5
```

## Ordering information

```ordering
SN74AUC1G14MDBVREP | Reel of 3000 | -55 to 125 | U14
```
[[1::table #2]]

Conversion note: the marking above is from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –55°C to 125°C | SOT (SOT-23) – DBV | Reel of 3000 | SN74AUC1G14MDBVREP | U14_ |

[1] DBV: The actual top-side marking has one additional character that designates the assembly/test site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```
[[1::table #3]]

## Conversion notes

- Source pages 1–6 converted; the package option addendum (p.7–8), tape
  and reel and mechanical pages (9–14) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: the features list except the lines quoted, the
  qualification-pedigree footnote, the NanoStar and NanoFree package
  text, the enable/disable-time, pulse-duration and setup/hold parts of
  Figure 1 with their notes B, E and F (this device has no output
  enable), and the S1 positions for t~PLZ~/t~PZL~ (2 × V~CC~) and
  t~PHZ~/t~PZH~ (GND).
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The source's description (p.1) names the device "SN74AUC1G14"; the
  data sheet's title is SN74AUC1G14-EP, and the name is written with the
  suffix here.
- The source's electrical characteristics table gives symbols only for
  V~OH~, V~OL~, I~off~, I~CC~ and C~i~, so their Parameter cells are
  empty.
- The addendum also lists V62/06678-01XE, which the catalog doesn't
  carry.
- From images: overbar of Y = !{A} (p.1); "No internal connection"
  (legend of the pin drawing, p.1).
- Source issues:
  - Source error: "generators have the following characteristics" in
    note C of Figure 1 (parameter measurement information).
