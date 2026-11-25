---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup2g04",
  "title": "SN74AUP2G04 low-power dual inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74aup2g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES747B",
    "date": "2010-03",
    "pages": 26
  },
  "devices": ["gate:2G04"],
  "parts": ["SN74AUP2G04DCKR", "SN74AUP2G04DRYR", "SN74AUP2G04DSFR", "SN74AUP2G04YFPR"],
  "packages": {"p6": "DCK, DRY, DSF", "dsbga": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP2G04 low-power dual inverter gate

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static- and dynamic-power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in increased battery life. This
product also maintains excellent signal integrity. [[1::4]]

The SN74AUP2G04 performs the Boolean function Y = !{A} in positive
logic. [[1::6]]

NanoStar™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[2::1]]

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down. [[2::2]]

Optimized for 3.3-V Operation. 3.6-V I/O Tolerant to Support Mixed-Mode
Signal Operation. Suitable for Point-to-Point Applications. [[1::2]]

## Pinout

```pinout pinout=p6
```

```pinout pinout=dsbga
```

## Pin functions

```pins
1A, 2A: Inverter input
1Y, 2Y: Inverter output
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

Conversion note: the logic diagram (positive logic, p.2) shows two inverters, 1A on pin 1 to 1Y on pin 6 and 2A on pin 3 to 2Y on pin 4.

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
| θ~JA~ | Package thermal impedance [3] | DCK package | | 252 | °C/W |
| ^ | ^ | DRY package | | 234 | ^ |
| ^ | ^ | DSF package | | 300 | ^ |
| ^ | ^ | YFP package | | 132 | ^ |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[3::table #1]]

Latch-Up Performance Exceeds 100 mA Per JESD 78, Class II. ESD
Performance Tested Per JESD 22: 2000-V Human-Body Model (A114-B, Class
II), 1000-V Charged-Device Model (C101). [[1::2]]

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
| T~A~ | Operating free-air temperature | | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow of Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –20 µA | 0.8 V to 3.6 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | V |
| ^ | ^ | I~OH~ = –1.1 mA | 1.1 V | 0.75 × V~CC~ | | | 0.7 × V~CC~ | | ^ |
| ^ | ^ | I~OH~ = –1.7 mA | 1.4 V | 1.11 | | | 1.03 | | ^ |
| ^ | ^ | I~OH~ = –1.9 mA | 1.65 V | 1.32 | | | 1.3 | | ^ |
| ^ | ^ | I~OH~ = –2.3 mA | 2.3 V | 2.05 | | | 1.97 | | ^ |
| ^ | ^ | I~OH~ = –3.1 mA | ^ | 1.9 | | | 1.85 | | ^ |
| ^ | ^ | I~OH~ = –2.7 mA | 3 V | 2.72 | | | 2.67 | | ^ |
| ^ | ^ | I~OH~ = –4 mA | ^ | 2.6 | | | 2.55 | | ^ |
| V~OL~ | | I~OL~ = 20 µA | 0.8 V to 3.6 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 1.1 mA | 1.1 V | | | 0.3 × V~CC~ | | 0.3 × V~CC~ | ^ |
| ^ | ^ | I~OL~ = 1.7 mA | 1.4 V | | | 0.31 | | 0.37 | ^ |
| ^ | ^ | I~OL~ = 1.9 mA | 1.65 V | | | 0.31 | | 0.35 | ^ |
| ^ | ^ | I~OL~ = 2.3 mA | 2.3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 3.1 mA | ^ | | | 0.44 | | 0.45 | ^ |
| ^ | ^ | I~OL~ = 2.7 mA | 3 V | | | 0.31 | | 0.33 | ^ |
| ^ | ^ | I~OL~ = 4 mA | ^ | | | 0.44 | | 0.45 | ^ |
| I~I~ | A or B input [1] | V~I~ = GND to 3.6 V | 0 V to 3.6 V | | | 0.1 | | 0.5 | µA |
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6 V [2], I~O~ = 0 | 3.3 V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6 V | | 1.5 | | | | ^ |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | pF |

[1] Source error: the source prints "A or B input", kept as printed; each inverter has one input, A (function table, p.2).
[2] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND.
```
[[5::table #1]]

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
[[6::table #1]]

## Switching characteristics

```table id=sw-5pf caption="Switching characteristics, C~L~ = 5 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 5 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B [1] | Y | 0.8 V | | 18 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.3 | 12.8 | 2.1 | 15.6 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.2 | 8.7 | 0.9 | 10.3 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.2 | 6.6 | 0.5 | 8.2 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 4.4 | 0.5 | 5.5 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 3.5 | 0.5 | 4.3 | ^ |

[1] Source error: the source prints "A or B" as the input, kept as printed; each inverter has one input, A (function table, p.2).
```
[[5::table #2]]

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B [1] | Y | 0.8 V | | 21 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.5 | 14.7 | 1 | 17.2 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.2 | 10 | 0.5 | 11.3 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 7.7 | 0.5 | 9 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 5.2 | 0.5 | 6.1 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 4.2 | 0.5 | 4.7 | ^ |

[1] Source error: the source prints "A or B" as the input, kept as printed; each inverter has one input, A (function table, p.2).
```
[[6::table #2]]

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B [1] | Y | 0.8 V | | 24 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.9 | 16.3 | 3.1 | 19.9 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 7.2 | 11.1 | 1.8 | 13.2 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.8 | 8.7 | 1.1 | 10.6 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4.3 | 5.9 | 0.5 | 7.3 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.4 | 4.8 | 0.5 | 5.9 | ^ |

[1] Source error: the source prints "A or B" as the input, kept as printed; each inverter has one input, A (function table, p.2).
```
[[6::table #3]]

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B [1] | Y | 0.8 V | | 32.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 13.1 | 20.9 | 4.4 | 25.5 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9.5 | 14.2 | 2.9 | 16.9 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.7 | 11 | 2 | 13.5 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.7 | 7.6 | 1.3 | 9.4 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.7 | 6.2 | 1 | 7.5 | ^ |

[1] Source error: the source prints "A or B" as the input, kept as printed; each inverter has one input, A (function table, p.2).
```
[[6::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω, for propagation delays t~r~/t~f~ = 3 ns, for setup and
hold times and pulse width t~r~/t~f~ = 1.2 ns. The outputs are measured
one at a time, with one transition per measurement. t~PLH~ and t~PHL~
are the same as t~pd~. All parameters and waveforms are not applicable
to all devices. [[7::2]] [[7::4]] [[7::5]] [[7::6]]

```table id=pmi-conditions caption="Load circuit test conditions"
| | V~CC~ = 0.8 V | V~CC~ = 1.2 V ± 0.1 V | V~CC~ = 1.5 V ± 0.1 V | V~CC~ = 1.8 V ± 0.15 V | V~CC~ = 2.5 V ± 0.2 V | V~CC~ = 3.3 V ± 0.3 V |
|---|---|---|---|---|---|---|
| C~L~ | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF | 5, 10, 15, 30 pF |
| V~M~ | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 | V~CC~/2 |
| V~I~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ | V~CC~ |
```
[[1::table #2]]

```figure ref=load-rl-cl caption="Load circuit"
RL: 1 MΩ
CL: 5, 10, 15, 30 pF
```
[[7::Figure 3]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: VCC/2
```
[[7::Figure 3]]

## Package

```package footprint=SC70-6
```

```package footprint=SON-6-DRY
```

```package footprint=SON-6-DSF
```

```package footprint=DSBGA-6-YFP
```

## Ordering information

```ordering
SN74AUP2G04DCKR | Reel of 3000 | -40 to 85 | H45, H4F
SN74AUP2G04DRYR | Reel of 5000 | -40 to 85 | H4
SN74AUP2G04DSFR | Reel of 5000 | -40 to 85 | H4
SN74AUP2G04YFPR | Reel of 3000 | -40 to 85 | HCN
```
[[2::table #2]]

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information [1]"
| T~A~ | Package [2] | < | Orderable part number | Top-side marking [3] |
|---|---|---|---|---|
| –40°C to 85°C | NanoStar™ – WCSP (DSBGA) 0.23-mm Large Bump – YFP (Pb-free) | Reel of 3000 | SN74AUP2G04YFPR | ___HC_ |
| ^ | QFN – DRY | Reel of 5000 | SN74AUP2G04DRYR | H4 |
| ^ | uQFN – DSF | Reel of 5000 | SN74AUP2G04DSFR | H4 |
| ^ | SOT (SC-70) – DCK | Reel of 3000 | SN74AUP2G04DCKR | H4_ |

[1] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI web site at www.ti.com.
[2] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
[3] DCK: The actual top-side marking has one additional character that designates the wafer fab/assembly site. YFP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the wafer fab/assembly site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```
[[2::table #3]]

## Conversion notes

- Source pages 1–7 converted; page 8 (Figure 4, the load circuit and
  voltage waveforms for enable and disable times, which this gate does
  not have), the package option addendum (p.9–10), tape and reel and
  mechanical pages (11–26) dropped, replaced by the generated Package
  and Ordering sections. The ordering lines are from the package option
  addendum.
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
- The source's ordering table calls the DRY package QFN and DSF uQFN;
  the package option addendum (p.9) calls both SON.
- The addendum also lists SN74AUP2G04DCKRG4, SN74AUP2G04DRYRG4 and
  SN74AUP2G04DSFRG4, which the catalog doesn't carry.
- From images: the overbar of Y = !{A} (p.1); µA as the unit of I~OH~
  and I~OL~ at 0.8 V (p.4) and of the –20 µA and 20 µA conditions and
  the I~I~, I~off~, ΔI~off~, I~CC~ and ΔI~CC~ rows (p.5; the text layer
  reads "mA"); θ~JA~ (p.3; the text layer reads "qJA").
- Source issues:
  - Source error: the I~OH~ condition "V~CC~ = 1.65" has no unit (p.4);
    the I~OL~ row prints 1.65 V.
  - Source error: note 1 of the recommended operating conditions prints
    the report's title as "Implications of Slow of Floating CMOS
    Inputs" (p.4); kept as printed.
  - Source error: the I~I~ row (p.5) and the four switching tables
    (p.5–6) print "A or B" as the input; each inverter has one input,
    A (kept as printed).
