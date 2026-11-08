---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc08a-ep",
  "title": "SN74LVC08A-EP quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74lvc08a-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS731D",
    "date": "2007-09",
    "pages": 16
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74LVC08AMDREP", "SN74LVC08AMPWREP", "SN74LVC08AQDREP", "SN74LVC08AQPWREP", "V62/04655-01XE", "V62/04655-01YE", "V62/04655-02XE", "V62/04655-02YE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-08"
}
---

# SN74LVC08A-EP quadruple 2-input positive-AND gate

## Description

The SN74LVC08A quadruple 2-input positive-AND gate is designed for 2.7-V
to 3.6-V V~CC~ operation. The device performs the Boolean function
Y = A · B or Y = !{!{A} + !{B}} in positive logic.

Inputs can be driven from either 3.3 V or 5 V devices. This feature
allows the use of this device as a translator in a mixed 3.3 V/5 V
system environment.

This enhanced product (EP) version has a controlled baseline (one
assembly, one test site, one fabrication site) and extended temperature
performance: Q suffix –40 °C to 125 °C, M suffix –55 °C to 125 °C.

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

## Function tables

Logic diagram: four 2-input AND gates.

```table id=function caption="Function table (each gate)"
| A | B | Y |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 V | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 V | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | D package | | 86 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```

ESD protection exceeds 2000 V per MIL-STD-883, Method 3015; exceeds
200 V using machine model (C = 200 pF, R = 0).

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 2.7 V to 3.6 V | 2 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2.7 V to 3.6 V | | 0.8 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 0 | 8 | ns/V |
| T~A~ | Operating free-air temperature | M suffix | -55 | 125 | °C |
| ^ | ^ | Q suffix | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~OH~ | I~OH~ = -100 µA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | I~OH~ = -12 mA | 2.7 V | 2.2 | | | V |
| ^ | ^ | 3 V | 2.4 | | | V |
| ^ | I~OH~ = -24 mA | 3 V | 2.2 | | | V |
| V~OL~ | I~OL~ = 100 µA | 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | I~OL~ = 12 mA | 2.7 V | | | 0.4 | V |
| ^ | I~OL~ = 24 mA | 3 V | | | 0.55 | V |
| I~I~ | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | One input at V~CC~ - 0.6 V, other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25 °C.
```

```table id=operating caption="Operating characteristics, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 9.8 | 10 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics (see Figure 1)" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | | 4.8 | 1 | 4.1 | ns |
```

## Parameter measurement information

Figure 1, load circuit and voltage waveforms. For t~PLH~/t~PHL~ the
switch S1 is open, which leaves R~L~ and C~L~ from the output to ground.
C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

```table id=pmi caption="Load circuit and input conditions"
| V~CC~ | V~I~ | t~r~/t~f~ | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
|---|---|---|---|---|---|---|---|
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=load-rl-cl caption="Load circuit for t~pd~ (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Propagation delay times, inverting and noninverting outputs"
VM: 1.5 V
```

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LVC08AQDREP | Reel of 2500 | -40 to 125 | LVC08AE
SN74LVC08AQPWREP | Reel of 2000 | -40 to 125 | LVC08AE
SN74LVC08AMDREP | Reel of 2500 | -55 to 125 | LVC08AM
SN74LVC08AMPWREP | Reel of 2000 | -55 to 125 | LVC08AM
V62/04655-01XE | Reel of 2500 | -40 to 125 | LVC08AE
V62/04655-01YE | Reel of 2000 | -40 to 125 | LVC08AE
V62/04655-02XE | Reel of 2500 | -55 to 125 | LVC08AM
V62/04655-02YE | Reel of 2000 | -55 to 125 | LVC08AM
```

## Conversion notes

- Source pages 1–5 converted; the package option addendum, tape and
  reel and mechanical pages dropped, replaced by the generated Package
  and Ordering sections. Packing, temperature and marking of the V62
  parts come from the package option addendum.
- Dropped: features list (the qualification pedigree and controlled
  baseline are summarised in the description), the enable/disable and
  setup/hold waveforms of the generic measurement figure (they do not
  apply to this part).
- The Boolean function is garbled in the PDF text layer
  ("Y + A • B or Y + A ) B"); written as Y = A · B.
- Source issues:
  - Source conflict: the feature list says "Operates From 1.65 V to
    3.6 V" (p.1), the description says the device is designed for 2.7-V
    to 3.6-V V~CC~ operation (p.1), and the recommended operating
    conditions give an operating V~CC~ of 2 V to 3.6 V (p.2).
