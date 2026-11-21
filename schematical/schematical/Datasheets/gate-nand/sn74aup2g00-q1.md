---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup2g00-q1",
  "title": "SN74AUP2G00-Q1 low-power dual 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74aup2g00-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES812",
    "date": "2010-06",
    "pages": 16
  },
  "devices": ["gate:2G00"],
  "parts": ["SN74AUP2G00QDCURQ1"],
  "packages": {"p8": "DCU"},
  "converted": "2026-11-21"
}
---

# SN74AUP2G00-Q1 low-power dual 2-input positive-NAND gate

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static- and dynamic-power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in increased battery life. This
product also maintains excellent signal integrity.

The SN74AUP2G00-Q1 performs the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic. Source error: the source prints this
sentence without the -Q1 suffix (p.1); this datasheet is for the
SN74AUP2G00-Q1.

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down.

Qualified for Automotive Applications. Optimized for 3.3-V Operation.
3.6-V I/O Tolerant to Support Mixed-Mode Signal Operation. Suitable for
Point-to-Point Applications.

## Pinout

```pinout pinout=p8
```

## Pin functions

```pins
1A, 2A: Gate input A
1B, 2B: Gate input B
1Y, 2Y: Gate output Y
GND: Ground
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | H |
| L | X | H |
| X | L | H |
| H | H | L |
```

Conversion note: the logic diagram (positive logic, p.2) shows two 2-input NAND gates, 1A and 1B to 1Y and 2A and 2B to 2Y, and a line that the pin numbers shown are for DCU and DQE packages.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 4.6 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 4.6 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 4.6 | V |
| V~O~ | Output voltage range in the high or low state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±20 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance, junction to free air | DCU package [3] | | 220 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

```table id=esd caption="ESD protection"
| Symbol | Parameter | Conditions | Max | Unit |
|---|---|---|--:|---|
| ESD | Electrostatic discharge rating | Human-Body Model (HBM) | 1000 | V |
```

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II.

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | ^ |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | ^ |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | ^ |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 | | -1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | -4 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | ^ |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | ^ |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | ^ |
| ^ | ^ | V~CC~ = 3 V | | 4 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -20 µA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = -1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | ^ |
| ^ | ^ | I~OH~ = -1.7 mA | 1.4 V | 1.11 | | | 1.03 | | ^ |
| ^ | ^ | I~OH~ = -1.9 mA | 1.65 V | 1.32 | | | 1.3 | | ^ |
| ^ | ^ | I~OH~ = -2.3 mA | 2.3 V | 2.05 | | | 1.97 | | ^ |
| ^ | ^ | I~OH~ = -3.1 mA | ^ | 1.9 | | | 1.85 | | ^ |
| ^ | ^ | I~OH~ = -2.7 mA | 3 V | 2.72 | | | 2.67 | | ^ |
| ^ | ^ | I~OH~ = -4 mA | ^ | 2.6 | | | 2.55 | | ^ |
| V~OL~ | | I~OL~ = 20 µA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | ^ |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | ^ |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | ^ |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1 mA | ^ | | | 0.44 | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4 mA | ^ | | | 0.44 | | 0.45 | ^ |
| I~I~ | A or B input | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 1.3 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 2 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 1.7 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6 V [1], I~O~ = 0 | 3.3 V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6 V | | 1.5 | | | | ^ |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | pF |

[1] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 4 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 4.1 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.3 | ^ |
```

## Switching characteristics

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF [1]" conditions="Over recommended operating free-air temperature range, C~L~ = 5 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 19.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.8 | 18.8 | 2.1 | 20.9 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.4 | 11.8 | 0.9 | 12.7 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.3 | 9 | 0.5 | 9.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 5.9 | 0.5 | 6.4 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 5.2 | 0.5 | 5.7 | ^ |

[1] Specified by design. Not production tested.
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF [1]" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 23.1 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.9 | 21.1 | 1 | 22.1 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.3 | 13.2 | 0.5 | 13.7 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 10.1 | 0.5 | 10.6 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 7.4 | 0.5 | 7.9 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 5.5 | 0.5 | 6 | ^ |

[1] Specified by design. Not production tested.
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF [1]" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 24.7 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.8 | 21.7 | 3.1 | 24.8 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 4.6 | 14 | 1.8 | 15.8 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.5 | 10.6 | 1.1 | 11.7 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4 | 7 | 0.5 | 7.5 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.3 | 5.9 | 0.5 | 6.4 | ^ |

[1] Specified by design. Not production tested.
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 31.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 12.6 | 26.3 | 4.4 | 29 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9 | 16.6 | 2.9 | 20 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.3 | 12.9 | 2 | 15.7 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.4 | 8.8 | 1.3 | 11.4 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.5 | 7 | 1 | 9.5 | ^ |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, for propagation delays t~r~/t~f~ = 3 ns, for setup and
hold times and pulse width t~r~/t~f~ = 1.2 ns. The outputs are measured
one at a time, with one transition per measurement. t~PLH~ and t~PHL~
are the same as t~pd~. All parameters and waveforms are not applicable
to all devices.

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | V~CC~ = 1.5 V ± 0.1 V | V~CC~ = 1.8 V ± 0.15 V | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|---|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~M~ | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 |
| V~I~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ |
```

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```

## Package

```package footprint=VSSOP-8
```

## Ordering information

```ordering
SN74AUP2G00QDCURQ1 | Reel of 3000 | -40 to 125 | SBTQ
```

Conversion note: the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information [1]"
| T~A~ | Package [2] | < | Orderable part number | Top-side marking |
|---|---|---|---|---|
| –40°C to 125°C | VSSOP – DCU | Reel of 3000 | SN74AUP2G00QDCURQ1 | SBTQ |

[1] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI web site at www.ti.com.
[2] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
```

## Conversion notes

- Source pages 1–7 converted; page 8 (Figure 4, the load circuit and
  voltage waveforms for enable and disable times, which this gate does
  not have), the package option addendum (p.9–10), tape and reel and
  mechanical pages (11–16) dropped, replaced by the generated Package
  and Ordering sections. The ordering line is from the package option
  addendum.
- Dropped: the features list except the lines quoted, Figure 1 (family power comparison) and
  Figure 2 (signal integrity at 25 MHz) on p.1, the NanoStar package
  sentence (no NanoStar part is offered), the logic diagram
  (described in text), and the pulse-duration and setup-and-hold
  waveforms of Figure 3 with its note B on output-control waveforms.
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The latch-up rating comes from the source's features list.
- The source has no pin table; the pin descriptions are the
  converter's.
- The package option addendum (p.9) names the catalog version of this
  device, without the -Q1 suffix.
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.1);
  µA as the unit of I~OH~ and I~OL~ at 0.8 V (p.4) and of the –20 µA
  and 20 µA conditions and the I~I~, I~off~, ΔI~off~, I~CC~ and ΔI~CC~
  rows (p.5; the text layer reads "mA"); θ~JA~ (p.3; the text layer
  reads "qJA").
- Source issues:
  - Source error: the description (p.1) names the device without the
    -Q1 suffix; this datasheet is for the SN74AUP2G00-Q1.
  - Source error: the I~OH~ condition "V~CC~ = 1.65" has no unit (p.4);
    the I~OL~ row prints 1.65 V.
  - Unconfirmed: the C~L~ = 15 pF switching table (p.6) prints a
    typical t~pd~ of 4.6 ns at 1.5 V, below the 5.5 ns it prints at
    1.8 V.
  - Unconfirmed: the logic diagram's line (p.2) mentions the DQE
    package, which this datasheet does not offer.
