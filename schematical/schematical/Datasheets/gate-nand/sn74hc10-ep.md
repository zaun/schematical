---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74hc10-ep",
  "title": "SN74HC10-EP triple 3-input positive-NAND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/sn74hc10-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS558",
    "date": "2004-01",
    "pages": 15
  },
  "devices": ["gate:74_10"],
  "parts": ["SN74HC10QDREP", "SN74HC10QPWREP", "V62/04688-01XE", "V62/04688-01YE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74HC10-EP triple 3-input positive-NAND gate

## Description

The SN74HC10 device contains three independent 3-input NAND gates. It
performs the Boolean function Y = !{A • B • C} or Y = !{A} + !{B} + !{C}
in positive logic.

Controlled Baseline. One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of −40°C to 125°C. Wide Operating
Voltage Range of 2 V to 6 V. Outputs Can Drive Up To 10 LSTTL Loads.
Low Power Consumption, 20-µA Max I~CC~. Typical t~pd~ = 9 ns. ±4-mA
Output Drive at 5 V. Low Input Current of 1 µA Max.

Component qualification in accordance with JEDEC and industry standards
to ensure reliable operation over an extended temperature range. This
includes, but is not limited to, Highly Accelerated Stress Test (HAST)
or biased 85/85, temperature cycle, autoclave or unbiased HAST,
electromigration, bond intermetallic life, and mold compound life. Such
qualification testing should not be viewed as justifying use of this
component beyond specified performance and environmental limits.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A: Gate input A
1B, 2B, 3B: Gate input B
1C, 2C, 3C: Gate input C
1Y, 2Y, 3Y: Gate output Y
GND: Ground
VCC: Positive supply
```

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | < | Output |
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | L |
| L | X | X | H |
| X | L | X | H |
| X | X | L | H |
```

Conversion note: the logic diagram (positive logic) shows one 3-input NAND gate with the inputs A, B and C and the output Y.

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
| T~A~ | Operating free-air temperature | | -40 | | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

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

```table id=op caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance per gate | No load | 25 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, C~L~ = 50 pF (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~pd~ | A, B, or C | Y | 2 V | | 35 | 95 | | 145 | ns |
| ^ | ^ | ^ | 4.5 V | | 10 | 19 | | 29 | ns |
| ^ | ^ | ^ | 6 V | | 9 | 16 | | 25 | ns |
| t~t~ | | Y | 2 V | | 23 | 75 | | 110 | ns |
| ^ | ^ | ^ | 4.5 V | | 6 | 15 | | 22 | ns |
| ^ | ^ | ^ | 6 V | | 5 | 13 | | 19 | ns |
```

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. Phase relationships
between waveforms were chosen arbitrarily. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 6 ns, t~f~ = 6 ns. The outputs are
measured one at a time with one input transition per measurement.
t~PLH~ and t~PHL~ are the same as t~pd~.

```figure ref=load-cmos-50pf caption="Load circuit"
CL: 50 pF
```

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, input rise and fall times, propagation delay and output transition times"
VM: 50%
```

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74HC10QDREP | Reel of 2500 | -40 to 125 | SHC10EP
SN74HC10QPWREP | Reel of 2000 | -40 to 125 | SHC10EP | NRND
V62/04688-01XE | Reel of 2500 | -40 to 125 | SHC10EP
V62/04688-01YE | Reel of 2000 | -40 to 125 | SHC10EP | NRND
```

## Conversion notes

- Source pages 1–4 converted; the package option addendum (p.5–6), tape
  and reel and mechanical pages (7–15) dropped, replaced by the
  generated Package and Ordering sections. The ordering lines are from
  the package option addendum.
- Dropped: the feature list (some lines are kept as sentences in the
  Description), the logic diagram (described in text), the vendor's pin
  drawing (D or PW package) and the ordering table on p.1.
- The source's description names the device SN74HC10, without the -EP
  suffix; the sentence is kept as printed.
- The source prints the package thermal impedance in the absolute
  maximum ratings list; it is kept there, and the file has no Thermal
  information section.
- The source has no pin table; the pin descriptions are the
  converter's.
- Figure 1 (p.4): the input rise and fall time waveform and the
  propagation delay and output transition time waveforms are shown by
  one shared waveform figure.
- From images: overbars of Y = !{A • B • C} and Y = !{A} + !{B} + !{C}
  (p.1; the text layer reads "Y = A • B • C or Y = A + B + C").
- Source issues: none found.
