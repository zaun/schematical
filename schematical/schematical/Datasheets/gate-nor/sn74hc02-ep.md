---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hc02-ep",
  "title": "SN74HC02-EP quadruple 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74hc02-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS478A",
    "date": "2004-01",
    "pages": 12
  },
  "devices": ["gate:74_02"],
  "parts": ["SN74HC02QPWREP", "V62/04687-02YE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HC02-EP quadruple 2-input positive-NOR gate

## Description

The SN74HC02 contains four independent 2-input NOR gates. It performs
the Boolean function Y = !{A + B} or Y = !{A} • !{B} in positive logic. [[1::4]]

Controlled Baseline. One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of Up To −55°C to 125°C. 2-V to 6-V
V~CC~ Operation. Outputs Can Drive Up To 10 LSTTL Loads. [[1::1]] [[1::2]]

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. This
includes, but is not limited to, Highly Accelerated Stress Test (HAST)
or biased 85/85, temperature cycle, autoclave or unbiased HAST,
electromigration, bond intermetallic life, and mold compound life. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits. [[1::3]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Gate input A
1B, 2B, 3B, 4B: Gate input B
1Y, 2Y, 3Y, 4Y: Gate output Y
GND: Ground
VCC: Positive supply
```
[[1::table #1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |
```
[[1::table #2]]

Conversion note: the logic diagram (positive logic) shows one 2-input NOR gate with the inputs A and B and the output Y.

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| I~IK~ | Input clamp current [2] | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current [2] | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| θ~JA~ | Package thermal impedance [3] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Nom | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5 | 6 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | | V |
| ^ | ^ | V~CC~ = 4.5 V | 3.15 | | | V |
| ^ | ^ | V~CC~ = 6 V | 4.2 | | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | | 0.5 | V |
| ^ | ^ | V~CC~ = 4.5 V | | | 1.35 | V |
| ^ | ^ | V~CC~ = 6 V | | | 1.8 | V |
| V~I~ | Input voltage | | 0 | | V~CC~ | V |
| V~O~ | Output voltage | | 0 | | V~CC~ | V |
| Δt/Δv | Input transition rise/fall time | V~CC~ = 2 V | | | 1000 | ns |
| ^ | ^ | V~CC~ = 4.5 V | | | 500 | ns |
| ^ | ^ | V~CC~ = 6 V | | | 400 | ns |
| T~A~ | Operating free-air temperature | Q-suffix device | -40 | | 125 | °C |
| ^ | ^ | M-suffix device | -55 | | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | < | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | V~I~ = V~IH~ or V~IL~ | I~OH~ = −20 µA | 2 V | 1.9 | 1.998 | | 1.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.499 | | 4.4 | | V |
| ^ | ^ | ^ | 6 V | 5.9 | 5.999 | | 5.9 | | V |
| ^ | ^ | I~OH~ = −4 mA | 4.5 V | 3.98 | 4.3 | | 3.7 | | V |
| ^ | ^ | I~OH~ = −5.2 mA | 6 V | 5.48 | 5.8 | | 5.2 | | V |
| V~OL~ | V~I~ = V~IH~ or V~IL~ | I~OL~ = 20 µA | 2 V | | 0.002 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 6 V | | 0.001 | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 4.5 V | | 0.17 | 0.26 | | 0.4 | V |
| ^ | ^ | I~OL~ = 5.2 mA | 6 V | | 0.15 | 0.26 | | 0.4 | V |
| I~I~ | V~I~ = V~CC~ or 0 | < | 6 V | | ±0.1 | ±100 | | ±1000 | nA |
| I~CC~ | V~I~ = V~CC~ or 0, | I~O~ = 0 | 6 V | | | 2 | | 40 | µA |
| C~i~ | | < | 2 V to 6 V | | 3 | 10 | | 10 | pF |
```
[[3::table #1]]

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 22 | pF |
```
[[3::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 50 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 2 V | | 45 | 90 | | 135 | ns |
| ^ | ^ | ^ | 4.5 V | | 9 | 18 | | 27 | ns |
| ^ | ^ | ^ | 6 V | | 8 | 15 | | 23 | ns |
| t~t~ | | Y | 2 V | | 38 | 75 | | 110 | ns |
| ^ | ^ | ^ | 4.5 V | | 8 | 15 | | 22 | ns |
| ^ | ^ | ^ | 6 V | | 6 | 13 | | 19 | ns |
```
[[3::table #3]]

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. Phase relationships
between waveforms were chosen arbitrarily. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 6 ns, t~f~ = 6 ns. The outputs are
measured one at a time with one input transition per measurement.
t~PLH~ and t~PHL~ are the same as t~pd~. [[4::1]]

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```
[[4::Figure 1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, input rise and fall times, propagation delay and output transition times"
VM: 50%
```
[[4::Figure 1]]

## Package

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HC02QPWREP | Reel of 2000 | -40 to 125 | SHC02EP
V62/04687-02YE | Reel of 2000 | -40 to 125 | SHC02EP
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted; the package option addendum (p.5–6), tape
  and reel and mechanical pages (7–12) dropped, replaced by the
  generated Package and Ordering sections. The ordering lines are from
  the package option addendum.
- Dropped: the feature list (some lines are kept as sentences in the
  Description), the logic diagram (described in text), the vendor's pin
  drawing (D or PW package) and the ordering table on p.1.
- The ordering table on p.1 also lists the SN74HC02MDREP (SOIC − D,
  −55°C to 125°C, marking HC02MEP), which the catalog doesn't carry and
  the package option addendum doesn't list; the D package and M-suffix
  rows of the tables are kept as the source prints them.
- The source's description names the device SN74HC02, without the -EP
  suffix; the sentence is kept as printed.
- The source prints the package thermal impedance in the absolute
  maximum ratings list; it is kept there, and the file has no Thermal
  information section.
- The source has no pin table; the pin descriptions are the
  converter's.
- Figure 1 (p.4): the input rise and fall time waveform and the
  propagation delay and output transition time waveforms are shown by
  one shared waveform figure.
- From images: overbars of Y = !{A + B} and Y = !{A} • !{B} (p.1; the
  text layer reads "Y = A + B or Y = A • B").
- Source issues: none found.
