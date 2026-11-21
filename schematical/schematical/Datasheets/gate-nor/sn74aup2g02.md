---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup2g02",
  "title": "SN74AUP2G02 low-power dual 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74aup2g02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES753C",
    "date": "2010-05",
    "pages": 25
  },
  "devices": ["gate:2G02", "gate:2G02_RSE"],
  "parts": ["SN74AUP2G02DCUR", "SN74AUP2G02DCURG4", "SN74AUP2G02DQER", "SN74AUP2G02YFPR", "SN74AUP2G02RSER"],
  "packages": {"p8": "DCU, DQE, RSE", "dsbga": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP2G02 low-power dual 2-input positive-NOR gate

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static- and dynamic-power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in increased battery life. This
product also maintains excellent signal integrity.

The SN74AUP2G02 performs the Boolean function Y = !{A + B} or
Y = !{A} · !{B} in positive logic.

NanoStar™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package.

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down.

Optimized for 3.3-V Operation. 3.6-V I/O Tolerant to Support Mixed-Mode
Signal Operation. Suitable for Point-to-Point Applications.

## Pinout

Conversion note: DCU and DQE share one pin order, and the RSE package has its own (1Y on pin 1, 1A on pin 7).

```pinout pinout=p8
```

```pinout pinout=dsbga
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

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
```

Conversion note: the logic diagram (positive logic, p.2) shows two 2-input NOR gates, 1A and 1B to 1Y and 2A and 2B to 2Y, with the pin numbers of the DCU and DQE packages.

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
| θ~JA~ | Package thermal impedance [3] | DCU package | | 220 | °C/W |
| ^ | ^ | RSE package | | 253 | °C/W |
| ^ | ^ | YFP package | | 132 | °C/W |
| ^ | ^ | DQE package | | 261 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Performance Tested Per JESD 22: 2000-V Human-Body Model (A114-B, Class
II), 1000-V Charged-Device Model (C101).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 0.8 | 3.6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 0.8 V | V~CC~ | | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.6 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 0.8 V | | 0 | V |
| ^ | ^ | V~CC~ = 1.1 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.9 | V |
| V~I~ | Input voltage | | 0 | 3.6 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 0.8 V | | -20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | -1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | -1.7 | mA |
| ^ | ^ | V~CC~ = 1.65 | | -1.9 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | -4 | mA |
| I~OL~ | Low-level output current | V~CC~ = 0.8 V | | 20 | µA |
| ^ | ^ | V~CC~ = 1.1 V | | 1.1 | mA |
| ^ | ^ | V~CC~ = 1.4 V | | 1.7 | mA |
| ^ | ^ | V~CC~ = 1.65 V | | 1.9 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 3.1 | mA |
| ^ | ^ | V~CC~ = 3 V | | 4 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 0.8 V to 3.6 V | | 200 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –20 µA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = –1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | V |
| ^ | ^ | I~OH~ = –1.7 mA | 1.4 V | 1.11 | | | 1.03 | | V |
| ^ | ^ | I~OH~ = –1.9 mA | 1.65 V | 1.32 | | | 1.3 | | V |
| ^ | ^ | I~OH~ = –2.3 mA | 2.3 V | 2.05 | | | 1.97 | | V |
| ^ | ^ | I~OH~ = –3.1 mA | ^ | 1.9 | | | 1.85 | | V |
| ^ | ^ | I~OH~ = –2.7 mA | 3 V | 2.72 | | | 2.67 | | V |
| ^ | ^ | I~OH~ = –4 mA | ^ | 2.6 | | | 2.55 | | V |
| V~OL~ | | I~OL~ = 20 µA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | V |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | V |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | V |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | V |
| ^ | ^ | I~OL~ = 3.1 mA | ^ | | | 0.44 | | 0.45 | V |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | V |
| ^ | ^ | I~OL~ = 4 mA | ^ | | | 0.44 | | 0.45 | V |
| I~I~ | A or B input | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6 V [1], I~O~ = 0 | 3.3 V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6 V | | 1.5 | | | | pF |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | pF |

[1] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND
```

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ | Typ | Unit |
|---|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 0.8 V | 4 | pF |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4 | pF |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4 | pF |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 4 | pF |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 4.1 | pF |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 4.3 | pF |
```

## Switching characteristics

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 5 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 18 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.3 | 12.8 | 2.1 | 15.6 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.2 | 8.7 | 0.9 | 10.3 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.2 | 6.6 | 0.5 | 8.2 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 4.4 | 0.5 | 5.5 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 3.5 | 0.5 | 4.3 | ns |
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 21 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.5 | 14.7 | 1 | 17.2 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.2 | 10 | 0.5 | 11.3 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 7.7 | 0.5 | 9 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 5.2 | 0.5 | 6.1 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 4.2 | 0.5 | 4.7 | ns |
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 24 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.9 | 16.3 | 3.1 | 19.9 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 7.2 | 11.1 | 1.8 | 13.2 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.8 | 8.7 | 1.1 | 10.6 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4.3 | 5.9 | 0.5 | 7.3 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.4 | 4.8 | 0.5 | 5.9 | ns |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 32.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 13.1 | 20.9 | 4.4 | 25.5 | ns |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9.5 | 14.2 | 2.9 | 16.9 | ns |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.7 | 11 | 2 | 13.5 | ns |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.7 | 7.6 | 1.3 | 9.4 | ns |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.7 | 6.2 | 1 | 7.5 | ns |
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

```package footprint=X2SON-8
```

```package footprint=UQFN-8
```

```package footprint=DSBGA-8-YFP
```

## Ordering information

```ordering
SN74AUP2G02DCUR | Reel of 3000 | -40 to 85 | H02R
SN74AUP2G02DCURG4 | Reel of 3000 | -40 to 85 | H02R
SN74AUP2G02DQER | Reel of 5000 | -40 to 85 | PP
SN74AUP2G02RSER | Reel of 5000 | -40 to 85 | PP
SN74AUP2G02YFPR | Reel of 3000 | -40 to 85 | HBN
```

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information [1]"
| T~A~ | Package [2] | < | Orderable part number | Top-side marking [3] |
|---|---|---|---|---|
| –40°C to 85°C | NanoStar™ – WCSP (DSBGA) 0.23-mm Large Bump – YFP (Pb-free) | Reel of 3000 | SN74AUP2G02YFPR | ___HB_ |
| ^ | uQFN – DQE | Reel of 5000 | SN74AUP2G02DQER | PP |
| ^ | QFN – RSE | Reel of 5000 | SN74AUP2G02RSER | PP |
| ^ | SSOP – DCU | Reel of 3000 | SN74AUP2G02DCUR | H02_ |

[1] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI web site at www.ti.com.
[2] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
[3] DCU: The actual top-side marking has one additional character that designates the wafer fab/assembly site. YFP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the wafer fab/assembly site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```

## Conversion notes

- Source pages 1–7 converted; page 8 (Figure 4, the load circuit and
  voltage waveforms for enable and disable times, which this gate does
  not have), the package option addendum (p.9–10), tape and reel and
  mechanical pages (11–25) dropped, replaced by the generated Package
  and Ordering sections. The ordering lines are from the package option
  addendum.
- Dropped: the features list except the lines quoted, Figure 1 (family power comparison) and
  Figure 2 (signal integrity at 25 MHz) on p.1, the logic diagram
  (described in text), and the pulse-duration and setup-and-hold
  waveforms of Figure 3 with its note B on output-control waveforms.
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The source has no pin table; the pin descriptions are the
  converter's.
- The source's ordering table calls the DCU package SSOP, DQE uQFN and
  RSE QFN; the package option addendum (p.9) calls them VSSOP, X2SON
  and UQFN.
- From images: overbars of Y = !{A + B} and Y = !{A} · !{B} (p.2; the
  text layer reads "Y = A + B or Y = A × B"); µA as the unit of I~OH~
  and I~OL~ at 0.8 V (p.4) and of the –20 µA and 20 µA conditions and
  the I~I~, I~off~, ΔI~off~, I~CC~ and ΔI~CC~ rows (p.5; the text layer
  reads "mA"); θ~JA~ (p.3; the text layer reads "qJA").
- Source issues:
  - Source error: the I~OH~ condition "V~CC~ = 1.65" has no unit (p.4);
    the I~OL~ row prints 1.65 V.
