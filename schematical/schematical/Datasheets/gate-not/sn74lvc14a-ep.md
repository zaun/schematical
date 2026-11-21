---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc14a-ep",
  "title": "SN74LVC14A-EP hex Schmitt-trigger inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74lvc14a-ep.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS732C",
    "date": "2006-06",
    "pages": 15
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74LVC14AQDREP", "SN74LVC14AQPWREP"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74LVC14A-EP hex Schmitt-trigger inverter

## Description

The hex Schmitt-trigger inverter is designed for 2.7-V to 3.6-V V~CC~
operation.

The device contains six independent inverters and performs the Boolean
function Y = !{A}.

Inputs can be driven from either 3.3-V or 5-V devices. This feature
allows the use of this device as a translator in a mixed 3.3-V/5-V
system environment.

ESD Protection Exceeds 2000 V Per MIL-STD-883, Method 3015; Exceeds
200 V Using Machine Model (C = 200 pF, R = 0). Operates From 2 V to
3.6 V. Inputs Accept Voltages to 5.5 V. Latch-Up Performance Exceeds
100 mA Per JESD 78, Class II.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Inverter input A
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Inverter output Y
GND: Ground
VCC: Positive supply
```

## Function tables

```table id=function caption="Function table (each inverter)"
| Input A | Output Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Output voltage range [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | D package | | 133.5 | °C/W |
| ^ | ^ | PW package | | 113 | °C/W |
| T~stg~ | Storage temperature range [5] | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
[5] Long-term high-temperature storage and/or extended use at maximum recommended operating conditions may result in a reduction of overall device life. See http://www.ti.com/ep_quality for additional information on enhanced plastic packaging.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 2 | 3.6 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2.7 V | | -12 | mA |
| ^ | ^ | V~CC~ = 3 V | | -24 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2.7 V | | 12 | mA |
| ^ | ^ | V~CC~ = 3 V | | 24 | mA |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ [1] | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~T+~ | Positive-going threshold | | 2.7 V | 0.8 | | 2 | V |
| ^ | ^ | | 3 V | 0.9 | | 2 | V |
| ^ | ^ | | 3.6 V | 1.1 | | 2 | V |
| V~T–~ | Negative-going threshold | | 2.7 V | 0.4 | | 1.4 | V |
| ^ | ^ | | 3 V | 0.6 | | 1.5 | V |
| ^ | ^ | | 3.6 V | 0.8 | | 1.7 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 2.7 V | 0.3 | | 1.1 | V |
| ^ | ^ | | 3 V | 0.3 | | 1.2 | V |
| ^ | ^ | | 3.6 V | 0.3 | | 1.2 | V |
| V~OH~ | | I~OH~ = –100 µA | 2.7 V to 3.6 V | V~CC~ - 0.2 | | | V |
| ^ | | I~OH~ = –12 mA | 2.7 V | 2.2 | | | V |
| ^ | | ^ | 3 V | 2.4 | | | V |
| ^ | | I~OH~ = –24 mA | 3 V | 2.2 | | | V |
| V~OL~ | | I~OL~ = 100 µA | 2.7 V to 3.6 V | | | 0.2 | V |
| ^ | | I~OL~ = 12 mA | 2.7 V | | | 0.4 | V |
| ^ | | I~OL~ = 24 mA | 3 V | | | 0.55 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 3.6 V | | | ±5 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 3.6 V | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ – 0.6 V, Other inputs at V~CC~ or GND | 2.7 V to 3.6 V | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```

```table id=operating caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 2.5 V | V~CC~ = 3.3 V | Unit |
| ^ | ^ | ^ | Typ | Typ | ^ |
|---|---|---|--:|--:|---|
| C~pd~ | Power dissipation capacitance per inverter | f = 10 MHz | 12 | 15 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | V~CC~ = 2.7 V | < | V~CC~ = 3.3 V ± 0.3 V | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~pd~ | A | Y | | 7.5 | 1 | 6.4 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. All parameters
and waveforms are not applicable to all devices.

Conversion note: the load circuit also has a second R~L~, from the output
to switch S1, which goes to V~LOAD~, open or GND; the figure below draws
the load with S1 open.
Conversion note: Figure 1 also shows pulse-duration, setup-and-hold and
enable/disable waveforms, which are not drawn here.

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
SN74LVC14AQDREP | Reel of 2500 | -40 to 125 | LVC14AE
SN74LVC14AQPWREP | Reel of 2000 | -40 to 125 | LVC14AE
```

## Conversion notes

- Source pages 1–4 converted. The ordering lines are from the package
  option addendum (p.5). The rest of the addendum (p.6), tape and reel
  (pp.7–8), mechanical pages (9–14) and the notice page (15) are left
  out; the generated Package section replaces them.
- Dropped: features list (except the ESD, supply, input-voltage and
  latch-up lines) with its qualification-pedigree note, the ordering
  information table of p.1, the vendor pinout drawing and the logic
  diagram (one inverter with a Schmitt-trigger symbol, input A and
  output Y, p.2).
- Not claimed here: V62/04658-01XE and V62/04658-01YE, which the package
  option addendum lists and the catalog does not carry.
- The source has no pin functions table; the pin descriptions are the
  converter's, written from the pin names and the function table.
- The electrical characteristics table of the source gives no parameter
  name for V~OH~, V~OL~, I~I~, I~CC~, ΔI~CC~ and C~i~, so those
  Parameter cells are empty.
- The thermal impedance rows are part of the source's absolute maximum
  ratings table and stay there.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text. Of the lettered notes of
  Figure 1, A, C, D, G and H are kept as sentences; B, E and F are for
  the enable and disable waveforms, which are not drawn.
- The description (p.1) names the device "SN74LVC14A"; the name is left
  out of that sentence here.
- From images: overbars of Y = !{A} (p.1).
- Source issues:
  - Source conflict: the description gives the device as designed for
    2.7-V to 3.6-V V~CC~ operation (p.1); the feature list (p.1,
    "Operates From 2 V to 3.6 V") and the recommended operating
    conditions (p.2) give an operating supply voltage of 2 V minimum.
  - Source conflict: the ordering information table (p.1) lists
    SN74LVC14AMDREP for –55°C to 125°C; the recommended operating
    conditions (p.2) give an operating free-air temperature of –40 to
    125 °C only, and the package option addendum (p.5) does not list
    that part.
