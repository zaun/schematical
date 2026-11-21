---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74auc2g32",
  "title": "SN74AUC2G32 dual 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74auc2g32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES478C",
    "date": "2007-01",
    "pages": 16
  },
  "devices": ["gate:2G32"],
  "parts": ["SN74AUC2G32DCTR", "SN74AUC2G32DCUR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74AUC2G32 dual 2-input positive-OR gate

## Description

This dual 2-input positive-OR gate is operational at 0.8-V to 2.7-V
V~CC~, but is designed specifically for 1.65-V to 1.95-V V~CC~
operation. Optimized for 1.8-V operation and is 3.6-V I/O tolerant to
support mixed-mode signal operation.

The SN74AUC2G32 performs the Boolean function Y = A + B or
Y = !{!{A} × !{B}} in positive logic. Source error: the source prints
Y = !{A} × !{B}, without the bar over the whole product, which is not
the OR function of the function table, corrected.

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down.

For more information about AUC Little Logic devices, please refer to the
TI application report, Applications of Texas Instruments AUC Sub-1-V
Little Logic Devices, literature number SCEA027.

## Pinout

Conversion note: DCT and DCU share one pin order. Conversion note: the
source draws the YZP package in bottom view, and the drawing here is
generated from the catalog.

```pinout pinout=p8
```

```pinout pinout=dsbga
```

## Pin functions

Conversion note: the source has no pin table, and the descriptions are the converter's.

```pins
1A, 2A: Gate input A
1B, 2B: Gate input B
1Y, 2Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Functional description

Conversion note: the logic diagram (positive logic) shows two 2-input OR gates, 1A and 1B to 1Y and 2A and 2B to 2Y.

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
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
| θ~JA~ | Package thermal impedance [3] | DCT package | | 220 | °C/W |
| θ~JA~ | Package thermal impedance [3] | DCU package | | 227 | °C/W |
| θ~JA~ | Package thermal impedance [3] | YZP package | | 102 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

Latch-up performance exceeds 100 mA per JESD 78, Class II. ESD
protection exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 2.7 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
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
| Δt/ΔV | Input transition rise or fall rate | | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | < | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | < | 0.8 V to 2.7 V | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -0.7 mA | < | 0.8 V | | 0.55 | | V |
| ^ | ^ | I~OH~ = -3 mA | < | 1.1 V | 0.8 | | | V |
| ^ | ^ | I~OH~ = -5 mA | < | 1.4 V | 1 | | | V |
| ^ | ^ | I~OH~ = -8 mA | < | 1.65 V | 1.2 | | | V |
| ^ | ^ | I~OH~ = -9 mA | < | 2.3 V | 1.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | < | 0.8 V to 2.7 V | | | 0.2 | V |
| ^ | ^ | I~OL~ = 0.7 mA | < | 0.8 V | | 0.25 | | V |
| ^ | ^ | I~OL~ = 3 mA | < | 1.1 V | | | 0.3 | V |
| ^ | ^ | I~OL~ = 5 mA | < | 1.4 V | | | 0.4 | V |
| ^ | ^ | I~OL~ = 8 mA | < | 1.65 V | | | 0.45 | V |
| ^ | ^ | I~OL~ = 9 mA | < | 2.3 V | | | 0.6 | V |
| I~I~ | A or B inputs | V~I~ = V~CC~ or GND | < | 0 V to 2.7 V | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 2.7 V | < | 0 V | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, | I~O~ = 0 | 0.8 V to 2.7 V | | | 10 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | < | 2.5 V | | 2.5 | | pF |

[1] All typical values are at T~A~ = 25°C.
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 0.8 V | V~CC~ = 1.2 V | V~CC~ = 1.5 V | V~CC~ = 1.8 V | V~CC~ = 2.5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 13 | 13 | 13 | 13 | 14 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-15pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | < | V~CC~ = 1.5 V ± 0.1 V | < | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Typ | Min | Max | Min | Max | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 7.5 | 0.7 | 3.3 | 0.6 | 1.9 | 0.5 | 1 | 1.5 | 0.4 | 1.1 | ns |
```

```table id=sw-30pf caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | V~CC~ = 1.8 V ± 0.15 V | < | < | V~CC~ = 2.5 V ± 0.2 V | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 1.2 | 1.6 | 2.1 | 1 | 1.7 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, slew rate ≥ 1 V/ns. The outputs are measured one at a time,
with one transition per measurement. t~PLH~ and t~PHL~ are the same as
t~pd~. Conversion note: the source's test table sets S1 open for
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

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

## Ordering information

```ordering
SN74AUC2G32DCTR | Reel of 3000 | -40 to 85 | 2XC5, U32, R, Z
SN74AUC2G32DCUR | Reel of 3000 | -40 to 85 | U32Q, U32R, UR
```

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.1).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package | < | Orderable part number | Top-side marking [1] |
|---|---|---|---|---|
| –40°C to 85°C | NanoFree – WCSP (DSBGA) 0.23-mm Large Bump – YZP (Pb-free) | Reel of 3000 | SN74AUC2G32YZPR | _ _ _UG_ |
| ^ | SSOP – DCT | Reel of 3000 | SN74AUC2G32DCTR | U32_ _ _ |
| ^ | VSSOP – DCU | Reel of 3000 | SN74AUC2G32DCUR | U32_ |

[1] DCT: The actual top-side marking has three additional characters that designate the year, month, and assembly/test site. DCU: The actual top-side marking has one additional character that designates the assembly/test site. YZP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the assembly/test site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum (p.6), tape
  and reel and mechanical pages (6–16) dropped, replaced by the
  generated Package and Ordering sections.
- Dropped: the features list except the lines quoted, the NanoFree
  package text, the enable/disable-time, pulse-duration and setup/hold
  parts of Figure 1 with their notes B, E and F (this gate has no output
  enable), and the S1 positions for t~PLZ~/t~PZL~ (2 × V~CC~) and
  t~PHZ~/t~PZH~ (GND).
- The catalog does not carry SN74AUC2G32YZPR, which the source's
  ordering table on p.1 lists and the package option addendum does not.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- From images: overbars of Y = !{A} × !{B} (p.1; the text layer reads
  "Y = A × B").
- From images: V~Δ~ (load table of Figure 1, p.5; the text layer reads
  "VD").
- Source issues:
  - Source error: the Boolean function on p.1 is printed as
    Y = !{A} × !{B}, which is a NOR; the function table on p.2 is an OR,
    Y = !{!{A} × !{B}}. Corrected.
