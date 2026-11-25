---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74aup2g00",
  "title": "SN74AUP2G00 low-power dual 2-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74aup2g00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES752C",
    "date": "2010-05",
    "pages": 25
  },
  "devices": ["gate:2G00", "gate:2G00_RSE"],
  "parts": ["SN74AUP2G00DCUR", "SN74AUP2G00DCURG4", "SN74AUP2G00DQER", "SN74AUP2G00YFPR", "SN74AUP2G00RSER"],
  "packages": {"p8": "DCU, DQE, RSE", "dsbga": "YFP"},
  "converted": "2026-11-21"
}
---

# SN74AUP2G00 low-power dual 2-input positive-NAND gate

## Description

The AUP family is TI's premier solution to the industry's low-power
needs in battery-powered portable applications. This family ensures a
very low static- and dynamic-power consumption across the entire V~CC~
range of 0.8 V to 3.6 V, resulting in increased battery life. This
product also maintains excellent signal integrity. [[1::4]]

The SN74AUP2G00 performs the Boolean function Y = !{A • B} or
Y = !{A} + !{B} in positive logic. [[2::1]]

NanoStar™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[2::2]]

This device is fully specified for partial-power-down applications
using I~off~. The I~off~ circuitry disables the outputs, preventing
damaging current backflow through the device when it is powered down. [[2::3]]

Optimized for 3.3-V Operation. 3.6-V I/O Tolerant to Support Mixed-Mode
Signal Operation. Suitable for Point-to-Point Applications. [[1::2]]

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
[[1::table #1]]

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
[[2::table #1]]

Conversion note: the logic diagram (positive logic, p.2) shows two 2-input NAND gates, 1A and 1B to 1Y and 2A and 2B to 2Y, with the pin numbers of the DCU and DQE packages.

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

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
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
| I~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V | | | 0.2 | | 0.6 | µA |
| ΔI~off~ | | V~I~ or V~O~ = 0 V to 3.6 V | 0 V to 0.2 V | | | 0.2 | | 0.6 | µA |
| I~CC~ | | V~I~ = GND or (V~CC~ to 3.6 V), I~O~ = 0 | 0.8 V to 3.6 V | | | 0.5 | | 0.9 | µA |
| ΔI~CC~ | | V~I~ = V~CC~ – 0.6 V [1], I~O~ = 0 | 3.3 V | | | 40 | | 50 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 0 V | | 1.5 | | | | pF |
| ^ | ^ | ^ | 3.6 V | | 1.5 | | | | ^ |
| C~o~ | | V~O~ = GND | 0 V | | 3 | | | | pF |

[1] One input at V~CC~ – 0.6 V, other input at V~CC~ or GND
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
| t~pd~ | A or B | Y | 0.8 V | | 19.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 2.6 | 7.8 | 18.8 | 2.1 | 19.8 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1.4 | 5.4 | 11.8 | 0.9 | 13.9 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 4.3 | 9 | 0.5 | 11.1 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3 | 5.9 | 0.5 | 7.8 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.4 | 4.9 | 0.5 | 5.9 | ^ |
```
[[5::table #2]]

```table id=sw-10pf caption="Switching characteristics, C~L~ = 10 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 10 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 23.1 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 1.5 | 8.9 | 21.1 | 1 | 22 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 1 | 6.3 | 13.2 | 0.5 | 15.1 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1 | 5 | 10.1 | 0.5 | 12.2 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 3.6 | 7.4 | 0.5 | 9 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 2.9 | 5.2 | 0.5 | 6.5 | ^ |
```
[[6::table #2]]

```table id=sw-15pf caption="Switching characteristics, C~L~ = 15 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 15 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 24.7 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 3.6 | 9.8 | 21.7 | 3.1 | 22.7 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 2.3 | 4.6 | 14 | 1.8 | 15.7 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 1.6 | 5.5 | 10.6 | 1.1 | 12.6 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1 | 4 | 7 | 0.5 | 8.9 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1 | 3.3 | 5.6 | 0.5 | 6.9 | ^ |
```
[[6::table #3]]

```table id=sw-30pf caption="Switching characteristics, C~L~ = 30 pF" conditions="Over recommended operating free-air temperature range, C~L~ = 30 pF (unless otherwise noted) (see Figure 3 and Figure 4)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 0.8 V | | 31.8 | | | | ns |
| ^ | ^ | ^ | 1.2 V ± 0.1 V | 4.9 | 12.6 | 26.3 | 4.4 | 27 | ^ |
| ^ | ^ | ^ | 1.5 V ± 0.1 V | 3.4 | 9 | 16.6 | 2.9 | 18.3 | ^ |
| ^ | ^ | ^ | 1.8 V ± 0.15 V | 2.5 | 7.3 | 12.9 | 2 | 14.8 | ^ |
| ^ | ^ | ^ | 2.5 V ± 0.2 V | 1.8 | 5.4 | 8.8 | 1.3 | 10.5 | ^ |
| ^ | ^ | ^ | 3.3 V ± 0.3 V | 1.5 | 4.5 | 6.7 | 1 | 8.2 | ^ |
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
SN74AUP2G00DCUR | Reel of 3000 | -40 to 85 | H00R
SN74AUP2G00DCURG4 | Reel of 3000 | -40 to 85 | H00R
SN74AUP2G00DQER | Reel of 5000 | -40 to 85 | PN
SN74AUP2G00RSER | Reel of 5000 | -40 to 85 | PN
SN74AUP2G00YFPR | Reel of 3000 | -40 to 85 | HAN
```
[[9::table #1]]

Conversion note: the markings above are from the package option addendum, and the table below is the source's ordering information table (p.2).

```table id=ordering-table caption="Ordering information"
| T~A~ | Package [1] [2] | < | Orderable part number | Top-side marking [3] |
|---|---|---|---|---|
| –40°C to 85°C | NanoStar™ – WCSP (DSBGA) 0.23-mm Large Bump – YFP (Pb-free) | Reel of 3000 | SN74AUP2G00YFPR | ___HA_ |
| ^ | uQFN – DQE | Reel of 5000 | SN74AUP2G00DQER | PN |
| ^ | QFN – RSE | Reel of 5000 | SN74AUP2G00RSER | PN |
| ^ | SSOP – DCU | Reel of 3000 | SN74AUP2G00DCUR | H00_ |

[1] Package drawings, thermal data, and symbolization are available at www.ti.com/packaging.
[2] For the most current package and ordering information, see the Package Option Addendum at the end of this document, or see the TI website at www.ti.com.
[3] DCU: The actual top-side marking has one additional character that designates the wafer fab/assembly site. YFP: The actual top-side marking has three preceding characters to denote year, month, and sequence code, and one following character to designate the wafer fab/assembly site. Pin 1 identifier indicates solder-bump composition (1 = SnPb, • = Pb-free).
```
[[2::table #2]]

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
- From images: overbars of Y = !{A • B} and Y = !{A} + !{B} (p.2);
  µA as the unit of I~OH~ and I~OL~ at 0.8 V (p.4) and of the –20 µA
  and 20 µA conditions and the I~I~, I~off~, ΔI~off~, I~CC~ and ΔI~CC~
  rows (p.5; the text layer reads "mA"); θ~JA~ (p.3; the text layer
  reads "qJA").
- Source issues:
  - Source error: the I~OH~ condition "V~CC~ = 1.65" has no unit (p.4);
    the I~OL~ row prints 1.65 V.
  - Unconfirmed: the C~L~ = 15 pF switching table (p.6) prints a
    typical t~pd~ of 4.6 ns at 1.5 V, below the 5.5 ns it prints at
    1.8 V.
