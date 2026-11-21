---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc14",
  "title": "SN74AUC14 hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74auc14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES473A",
    "date": "2010-05",
    "pages": 10
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74AUC14RGYR"],
  "packages": {"p14": "RGY"},
  "converted": "2026-11-21"
}
---

# SN74AUC14 hex Schmitt-trigger inverter

## Description

Optimized for 1.8-V operation and is 3.6-V I/O tolerant to support
mixed-mode signal operation.

This hex Schmitt-trigger inverter is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation.

The SN74AUC14 contains six independent inverters and performs the
Boolean function Y = !{A}. The device functions as six independent
inverters, but because of Schmitt action, it may have different input
threshold levels for positive-going (V~T+~) and negative-going (V~T–~)
signals.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

## Pinout

```pinout pinout=p14
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
GND: Ground
VCC: Supply voltage
```

## Functional description

Conversion note: the logic diagram (each inverter, positive logic) shows one inverter with input A and output Y, drawn with a hysteresis symbol.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

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
| θ~JA~ | Package thermal impedance [3] | | | 47 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-5.
```

Latch-up performance exceeds 100 mA per JESD 78, Class II. ESD
protection exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | -3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | -8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -9 | mA |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 0.7 | mA |
| ^ | ^ | V~CC~ = 1.1 V | | 3 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 5 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 8 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 9 | mA |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | < | 0.8 V | | 0.5 | | V |
| ^ | ^ | | < | 1.1 V | 0.51 | | 0.86 | V |
| ^ | ^ | | < | 1.4 V | 0.65 | | 1 | V |
| ^ | ^ | | < | 1.65 V | 0.79 | | 1.16 | V |
| ^ | ^ | | < | 2.3 V | 1.11 | | 1.56 | V |
| V~T–~ | Negative-going input threshold voltage | | < | 0.8 V | | 0.3 | | V |
| ^ | ^ | | < | 1.1 V | 0.22 | | 0.53 | V |
| ^ | ^ | | < | 1.4 V | 0.3 | | 0.58 | V |
| ^ | ^ | | < | 1.65 V | 0.39 | | 0.62 | V |
| ^ | ^ | | < | 2.3 V | 0.58 | | 0.87 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | < | 0.8 V | | 0.21 | | V |
| ^ | ^ | | < | 1.1 V | 0.25 | | 0.38 | V |
| ^ | ^ | | < | 1.4 V | 0.31 | | 0.5 | V |
| ^ | ^ | | < | 1.65 V | 0.37 | | 0.62 | V |
| ^ | ^ | | < | 2.3 V | 0.48 | | 0.77 | V |
| V~OH~ | High-level output voltage | I~OH~ = -100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | < | 0.8 V | | 0.55 | | V |
| ^ | ^ | I~OH~ = -3 mA | < | 1.1 V | 0.8 | | | V |
| ^ | ^ | I~OH~ = -5 mA | < | 1.4 V | 1 | | | V |
| ^ | ^ | I~OH~ = -8 mA | < | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -9 mA | < | 2.3 V | 1.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A inputs | V~I~ = V~CC~ or GND | < | 0 to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 2.5 | | pF |

[1] All typical values are at T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 17 | 18 | 19 | 20 | 22 | pF |
```

## Switching characteristics

Conversion note: Figure 1 is in [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 8.8 | 1.2 | 5.7 | 0.7 | 3.9 | 0.6 | 1.2 | 3 | 0.5 | 1.8 | ns |
```

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.7 | 1.7 | 3.5 | 0.7 | 2.7 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. All parameters and waveforms are not applicable to all devices.

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

```figure ref=load-rl-cl caption="Load circuit for t~PLH~/t~PHL~ (S1 open)"
RL: see table
CL: see table
```

```figure ref=waveforms-tpd caption="Propagation delay times"
VM: VCC/2
```

## Package

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74AUC14RGYR | Reel of 3000 | -40 to 85 | MS14
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6), tape
  and reel and mechanical pages (7–10) dropped, replaced by the generated
  Package and Ordering sections.
- Dropped: the features list except the lines quoted, the source's
  ordering information table on p.1 (–40°C to 85°C, QFN – RGY, Tape and
  reel, SN74AUC14RGYR, marking MS14) with its notes, the
  enable/disable-time, pulse-duration and setup/hold parts of Figure 1
  with their notes B, E and F (this inverter has no output enable), and
  the S1 positions for t~PLZ~/t~PZL~ (2 × V~CC~) and t~PHZ~/t~PZH~
  (GND).
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- From images: overbar of Y = !{A} (p.1).
- From images: θ~JA~ (absolute maximum ratings, p.2; the text layer
  prints "qJA"); µA in the I~OH~ = –100 µA and I~OL~ = 100 µA conditions
  and as the unit of the I~I~, I~off~ and I~CC~ rows (p.3; the text
  layer prints them as mA).
