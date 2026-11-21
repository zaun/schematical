---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc32a-ep",
  "title": "SN74LVC32A-EP quadruple 2-input positive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74lvc32a-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS733C",
    "date": "2007-03",
    "pages": 15
  },
  "devices": ["gate:74_32"],
  "parts": ["SN74LVC32AMDREP", "SN74LVC32AMPWREP", "SN74LVC32AMPWREPG4", "SN74LVC32AQDREP", "SN74LVC32AQPWREP", "V62/04661-01XE", "V62/04661-01YE", "V62/04661-02XE", "V62/04661-02YE"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74LVC32A-EP quadruple 2-input positive-OR gate

## Description

Controlled Baseline – One Assembly/Test Site, One Fabrication Site.
Extended Temperature Performance of –40°C to 125°C and –55°C to 125°C.
Inputs Accept Voltages to 5.5 V.

The SN74LVC32A quadruple 2-input positive-OR gate is designed for 2.7-V
to 3.6-V V~CC~ operation.

The device performs the Boolean function Y = A + B or
Y = !{!{A} • !{B}} in positive logic.

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of this device as a translator in a mixed 3.3-V/5-V
system environment.

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

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1] [2]"
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
| Δt/Δv | Input transition rise or fall rate | | | 7 | ns/V |
| T~A~ | Operating free-air temperature | M suffix | -55 | 125 | °C |
| ^ | ^ | Q suffix | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
[2] Long-term high-temperature storage and/or extended use at maximum recommended operating conditions may result in a reduction of overall device life. See http://www.ti.com/ep_quality for additional information on enhanced plastic packaging.
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
| ΔI~CC~ | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per gate | f = 10 MHz | 10.6 | 12.5 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | | 4.4 | 1 | 3.8 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~.

All parameters and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: the source figure also shows pulse-duration,
setup-and-hold and enable/disable waveforms, which are not drawn here.

```table id=s1 caption="Load circuit switch S1"
| Test | S1 |
|---|---|
| t~PLH~/t~PHL~ | Open |
| t~PLZ~/t~PZL~ | V~LOAD~ |
| t~PHZ~/t~PZH~ | GND |
```

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```table id=test-conditions caption="Test conditions (Figure 1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 2.7 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 3.3 V ± 0.3 V | 2.7 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V
```

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74LVC32AMDREP | Reel of 2500 | -55 to 125 | LVC32AM
SN74LVC32AMPWREP | Reel of 2000 | -55 to 125 | LVC32AM
SN74LVC32AMPWREPG4 | Reel of 2000 | -55 to 125 | LVC32AM
SN74LVC32AQDREP | Reel of 2500 | -40 to 125 | LVC32AE
SN74LVC32AQPWREP | Reel of 2000 | -40 to 125 | LVC32AE
V62/04661-01XE | Reel of 2500 | -40 to 125 | LVC32AE
V62/04661-01YE | Reel of 2000 | -40 to 125 | LVC32AE
V62/04661-02XE | Reel of 2500 | -55 to 125 | LVC32AM
V62/04661-02YE | Reel of 2000 | -55 to 125 | LVC32AM
```

## Conversion notes

- Source pages 1–4 converted. The ordering lines are from the package
  option addendum (p.5). The rest of the addendum (p.6), tape and reel
  (pp.7–8), mechanical pages (9–14) and the notice page (15) are left
  out; the generated Package section replaces them.
- Dropped: features list (except the controlled baseline, the
  temperature ranges and the input voltage), the ordering information
  table of p.1 and the logic diagram (one 2-input OR gate, p.2).
- The source has no pin functions table; the pin descriptions are the
  converter's, written from the pin names and the function table.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- The text layer prints the Boolean function as "Y + A ) B or
  Y + A • B"; the page image shows Y = A + B or Y = !{!{A} • !{B}}
  (p.1).
- From images: overbars of Y = !{!{A} • !{B}} (p.1).
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column.
- The thermal impedance rows are part of the source's absolute maximum
  ratings table and stay there.
- Source issues:
  - Source conflict: the description gives the device as designed for
    2.7-V to 3.6-V V~CC~ operation (p.1); the features list says
    "Operates From 2 V to 3.6 V" (p.1) and the recommended operating
    conditions give an operating supply voltage of 2 V minimum (p.2).
