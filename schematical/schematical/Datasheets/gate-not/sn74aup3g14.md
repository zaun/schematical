---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup3g14",
  "title": "SN74AUP3G14 low-power triple Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup3g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES764D",
    "date": "2012-08",
    "pages": 27
  },
  "devices": ["gate:3G14", "gate:3G14_RSE"],
  "parts": ["SN74AUP3G14DCUR", "SN74AUP3G14DQER", "SN74AUP3G14YFPR", "SN74AUP3G14RSER"],
  "packages": {"p8": "DCU, DQE, RSE", "dsbga": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP3G14 low-power triple Schmitt-trigger inverter

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static- and dynamic-power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in increased battery life. This
product also maintains excellent signal integrity.

The SN74AUP3G14 contains three inverters and performs the Boolean
function Y = !{A}. The device functions as three independent inverters
but, because of Schmitt action, it may have different input threshold
levels for positive-going (V~T+~) and negative-going (V~T–~) signals.

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
1A, 2A, 3A: Inverter input A
1Y, 2Y, 3Y: Inverter output Y
GND: Ground
VCC: Supply voltage
```

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

Conversion note: the logic diagram (positive logic, p.3) shows three Schmitt-trigger inverters, 1A to 1Y, 2A to 2Y and 3A to 3Y.

Pin numbers shown are for the DCU and DQE package.

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
| ^ | ^ | DQE package | | 261 | ^ |
| ^ | ^ | RSE package | | 253 | ^ |
| ^ | ^ | YFP package | | 132 | ^ |
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
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 0.8 V | 0.3 | | 0.6 | 0.3 | 0.6 | V |
| ^ | ^ | ^ | 1.1 V | 0.53 | | 0.9 | 0.53 | 0.9 | ^ |
| ^ | ^ | ^ | 1.4 V | 0.74 | | 1.11 | 0.74 | 1.11 | ^ |
| ^ | ^ | ^ | 1.65 V | 0.91 | | 1.29 | 0.91 | 1.29 | ^ |
| ^ | ^ | ^ | 2.3 V | 1.37 | | 1.77 | 1.37 | 1.77 | ^ |
| ^ | ^ | ^ | 3 V | 1.88 | | 2.29 | 1.88 | 2.29 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 0.8 V | 0.1 | | 0.6 | 0.1 | 0.6 | V |
| ^ | ^ | ^ | 1.1 V | 0.26 | | 0.65 | 0.26 | 0.65 | ^ |
| ^ | ^ | ^ | 1.4 V | 0.39 | | 0.75 | 0.39 | 0.75 | ^ |
| ^ | ^ | ^ | 1.65 V | 0.47 | | 0.84 | 0.47 | 0.84 | ^ |
| ^ | ^ | ^ | 2.3 V | 0.69 | | 1.04 | 0.69 | 1.04 | ^ |
| ^ | ^ | ^ | 3 V | 0.88 | | 1.24 | 0.88 | 1.24 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 0.8 V | 0.07 | | 0.5 | 0.07 | 0.5 | V |
| ^ | ^ | ^ | 1.1 V | 0.08 | | 0.46 | 0.08 | 0.46 | ^ |
| ^ | ^ | ^ | 1.4 V | 0.18 | | 0.56 | 0.18 | 0.56 | ^ |
| ^ | ^ | ^ | 1.65 V | 0.27 | | 0.66 | 0.27 | 0.66 | ^ |
| ^ | ^ | ^ | 2.3 V | 0.53 | | 0.92 | 0.53 | 0.92 | ^ |
| ^ | ^ | ^ | 3 V | 0.79 | | 1.31 | 0.79 | 1.31 | ^ |
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
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | µA |
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

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 5 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 18 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.3 | 12.8 | 2.1 | 15.6 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.2 | 8.7 | 0.9 | 10.3 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.2 | 6.6 | 0.5 | 8.2 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 4.4 | 0.5 | 5.5 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 3.5 | 0.5 | 4.3 | ^ |
```

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A | Y | 0.8 V | | 18.4 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.6 | 7.9 | 13.4 | 1.3 | 16.7 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 4 | 6 | 9.6 | 2.2 | 11.8 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 3.6 | 5 | 7.9 | 2.4 | 9.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 3.2 | 4 | 5.5 | 2.3 | 6.8 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 2.9 | 3.5 | 4.6 | 2.1 | 5.6 | ^ |
```

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 24 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.9 | 16.3 | 3.1 | 19.9 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 7.2 | 11.1 | 1.8 | 13.2 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.8 | 8.7 | 1.1 | 10.6 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4.3 | 5.9 | 0.5 | 7.3 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.4 | 4.8 | 0.5 | 5.9 | ^ |
```

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 32.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 13.1 | 20.9 | 4.4 | 25.5 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9.5 | 14.2 | 2.9 | 16.9 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.7 | 11 | 2 | 13.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.7 | 7.6 | 1.3 | 9.4 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.7 | 6.2 | 1 | 7.5 | ^ |
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
SN74AUP3G14DCUR | Reel of 3000 | -40 to 85 | H14R
SN74AUP3G14DQER | Reel of 5000 | -40 to 85 | TX
SN74AUP3G14RSER | Reel of 5000 | -40 to 85 | TX
SN74AUP3G14YFPR | Reel of 3000 | -40 to 85 | HFN
```

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information [1]"
| T~A~ | Package [2] | < | Orderable part number | Top-side marking [3] |
|---|---|---|---|---|
| –40°C to 85°C | NanoStar™ – WCSP (DSBGA) 0.23-mm Large Bump – YFP (Pb-free) | Reel of 3000 | SN74AUP3G14YFPR | ___HF_ |
| ^ | uQFN – DQE | Reel of 5000 | SN74AUP3G14DQER | TX |
| ^ | QFN – RSE | Reel of 5000 | SN74AUP3G14RSER | TX |
| ^ | SSOP – DCU | Reel of 3000 | SN74AUP3G14DCUR | H14_ |

[1] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI web site at www.ti.com.
[2] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
[3] DCU: The actual top-side marking has one additional character that designates the wafer fab/assembly site. YFP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the wafer fab/assembly site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```

## Conversion notes

- Source pages 1–8 converted; page 9 (Figure 4, the load circuit and
  voltage waveforms for enable and disable times, which this inverter
  does not have), the revision history (p.10), the package option
  addendum (p.11–12), tape and reel and mechanical pages (13–27)
  dropped, replaced by the generated Package and Ordering sections. The
  ordering lines are from the package option addendum.
- Dropped: the features list except the lines quoted, Figure 1 (family
  power comparison) and Figure 2 (signal integrity at 25 MHz) on p.1,
  the logic diagram (described in text), and the pulse-duration and
  setup-and-hold waveforms of Figure 3 with its note B on
  output-control waveforms.
- The source prints the package thermal impedance in the absolute
  maximum ratings table; it is kept there, and the file has no Thermal
  information section.
- The ESD and latch-up ratings come from the source's features list
  (the source has no ESD ratings table).
- The source has no pin table; the pin descriptions are the
  converter's.
- The source also lists SN74AUP3G14DCURG4 in the package option
  addendum (p.11); the catalog doesn't carry it.
- The source's ordering table calls the DCU package SSOP, DQE uQFN and
  RSE QFN; the package option addendum (p.11) calls them VSSOP, X2SON
  and UQFN.
- From images: overbar of Y = !{A} (p.2).
- Source issues:
  - Source error: the I~OH~ condition "V~CC~ = 1.65" has no unit (p.4);
    the I~OL~ row prints 1.65 V.
  - Unconfirmed: the I~I~ row and the C~L~ = 5 pF, 15 pF and 30 pF
    switching tables name an "A or B" input (p.5–6); the device has no
    B input, and the C~L~ = 10 pF table prints "A".
  - Unconfirmed: the C~L~ = 10 pF switching table (p.6) prints minimum
    t~pd~ values (4.6 ns at 1.2 V at 25°C) above those of the
    C~L~ = 15 pF table (3.6 ns), and a minimum over temperature at
    1.2 V (1.3 ns) below the one at 1.5 V (2.2 ns).
