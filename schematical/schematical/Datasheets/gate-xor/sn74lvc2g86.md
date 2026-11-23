---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74lvc2g86",
  "title": "SN74LVC2G86 dual 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74lvc2g86.pdf",
    "vendor": "Texas Instruments",
    "document": "SCES360I",
    "date": "2013-12",
    "pages": 21
  },
  "devices": ["gate:2G86"],
  "parts": ["SN74LVC2G86DCTR", "SN74LVC2G86DCUR", "SN74LVC2G86DCURG4", "SN74LVC2G86DCUT", "SN74LVC2G86YZPR"],
  "packages": {"p8": "DCT, DCU", "dsbga": "YZP"},
  "converted": "2026-11-21"
}
---

# SN74LVC2G86 dual 2-input exclusive-OR gate

## Description

This dual 2-input exclusive-OR gate is designed for 1.65-V to 5.5-V
V~CC~ operation. [[1::1]]

The SN74LVC2G86 performs the Boolean function Y = A ⊕ B or
Y = !{A}B + A!{B} in positive logic. [[1::3]]

A common application is as a true/complement element. If the input is
low, the other input is reproduced in true form at the output. If the
input is high, the signal on the other input is reproduced inverted at
the output. [[1::5]]

This device is fully specified for partial-power-down applications using
I~off~. The I~off~ circuitry disables the outputs, preventing damaging
current backflow through the device when it is powered down. [[1::6]]

Supports 5-V V~CC~ Operation. Inputs Accept Voltages to 5.5 V. Latch-Up
Performance Exceeds 100 mA Per JESD 78, Class II. [[1::2]]

## Pinout

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

## Functional description

An exclusive-OR gate has many applications, some of which can be
represented better by alternative logic symbols. [[2::2]]

These are five equivalent exclusive-OR symbols valid for an SN74LVC2G86
gate in positive logic; negation may be shown at any two ports.
Conversion note: the five symbols (exclusive OR, two logic-identity
element, even-parity element and odd-parity element symbols) are not
drawn here. [[2::3]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[2::table #1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6.5 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high-impedance or power-off state [2] | | -0.5 | 6.5 | V |
| V~O~ | Voltage range applied to any output in the high or low state [2] [3] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -50 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 | | -50 | mA |
| I~O~ | Continuous output current | | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance [4] | DCT package | | 220 | °C/W |
| ^ | ^ | DCU package | | 227 | °C/W |
| ^ | ^ | YZP package | | 102 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input negative-voltage and output voltage ratings may be exceeded if the input and output clamp-current ratings are observed.
[3] The value of V~CC~ is provided in the recommended operating conditions table.
[4] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[3::table #1]]

ESD Protection Exceeds JESD 22: 2000-V Human-Body Model (A114-A), 200-V
Machine Model (A115-A), 1000-V Charged-Device Model (C101). [[1::2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | Operating | 1.65 | 5.5 | V |
| ^ | ^ | Data retention only | 1.5 | | V |
| V~IH~ | High-level input voltage | V~CC~ = 1.65 V to 1.95 V | 0.65 × V~CC~ | | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | 2 | | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | 0.7 × V~CC~ | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 1.65 V to 1.95 V | | 0.35 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.3 V to 2.7 V | | 0.7 | V |
| ^ | ^ | V~CC~ = 3 V to 3.6 V | | 0.8 | V |
| ^ | ^ | V~CC~ = 4.5 V to 5.5 V | | 0.3 × V~CC~ | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 1.65 V | | -4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | -8 | mA |
| ^ | ^ | V~CC~ = 3 V | | -16 | mA |
| ^ | ^ | ^ | | -24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | -32 | mA |
| I~OL~ | Low-level output current | V~CC~ = 1.65 V | | 4 | mA |
| ^ | ^ | V~CC~ = 2.3 V | | 8 | mA |
| ^ | ^ | V~CC~ = 3 V | | 16 | mA |
| ^ | ^ | ^ | | 24 | mA |
| ^ | ^ | V~CC~ = 4.5 V | | 32 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 1.8 V ± 0.15 V, 2.5 V ± 0.2 V | | 20 | ns/V |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 10 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 5 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[4::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | –40°C to 85°C | < | < | –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ [1] | Max | Min | Typ [1] | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -100 µA | 1.65 V to 5.5 V | V~CC~ - 0.1 | | | V~CC~ - 0.1 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 1.65 V | 1.2 | | | 1.2 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 2.3 V | 1.9 | | | 1.9 | | | V |
| ^ | ^ | I~OH~ = -16 mA | 3 V | 2.4 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = -24 mA | ^ | 2.3 | | | 2.3 | | | V |
| ^ | ^ | I~OH~ = -32 mA | 4.5 V | 3.8 | | | 3.8 | | | V |
| V~OL~ | | I~OL~ = 100 µA | 1.65 V to 5.5 V | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 1.65 V | | | 0.45 | | | 0.45 | V |
| ^ | ^ | I~OL~ = 8 mA | 2.3 V | | | 0.3 | | | 0.3 | V |
| ^ | ^ | I~OL~ = 16 mA | 3 V | | | 0.4 | | | 0.4 | V |
| ^ | ^ | I~OL~ = 24 mA | ^ | | | 0.55 | | | 0.55 | V |
| ^ | ^ | I~OL~ = 32 mA | 4.5 V | | | 0.55 | | | 0.55 | V |
| I~I~ | A or B inputs | V~I~ = 5.5 V or GND | 0 to 5.5 V | | | ±5 | | | ±5 | µA |
| I~off~ | | V~I~ or V~O~ = 5.5 V | 0 | | | ±10 | | | ±10 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 1.65 V to 5.5 V | | | 10 | | | 10 | µA |
| ΔI~CC~ | | One input at V~CC~ - 0.6 V, Other inputs at V~CC~ or GND | 3 V to 5.5 V | | | 500 | | | 500 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 3.3 V | | 5 | | | | | pF |

[1] All typical values are at V~CC~ = 3.3 V, T~A~ = 25°C.
```
[[5::table #1]]

### Operating characteristics

```table id=opchar caption="Operating characteristics" conditions="T~A~ = 25°C"
| Symbol | Parameter | Test conditions | V~CC~ = 1.8 V | V~CC~ = 2.5 V | V~CC~ = 3.3 V | V~CC~ = 5 V | Unit |
| ^ | ^ | ^ | Typ | Typ | Typ | Typ | ^ |
|---|---|---|--:|--:|--:|--:|---|
| C~pd~ | Power dissipation capacitance | f = 10 MHz | 20 | 20 | 20 | 22 | pF |
```
[[5::table #2]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-85 caption="Switching characteristics, –40°C to 85°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | SN74LVC2G86, –40°C to 85°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 4.1 | 9.9 | 2 | 5.7 | 1.6 | 4.7 | 1.4 | 3.6 | ns |
```
[[5::table #3]]

```table id=sw-125 caption="Switching characteristics, –40°C to 125°C" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 1)"
| Symbol | From (input) | To (output) | SN74LVC2G86, –40°C to 125°C | < | < | < | < | < | < | < | Unit |
| ^ | ^ | ^ | V~CC~ = 1.8 V ± 0.15 V | < | V~CC~ = 2.5 V ± 0.2 V | < | V~CC~ = 3.3 V ± 0.3 V | < | V~CC~ = 5 V ± 0.5 V | < | ^ |
| ^ | ^ | ^ | Min | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~pd~ | A or B | Y | 4.1 | 11.0 | 2 | 6.7 | 1.6 | 5.6 | 1.4 | 4.2 | ns |
```
[[5::table #4]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 10 MHz,
Z~O~ = 50 Ω. The outputs are measured one at a time, with one transition
per measurement. t~PLH~ and t~PHL~ are the same as t~pd~. [[6::1]]

All parameters and waveforms are not applicable to all devices. [[6::1]]

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
[[6::table #1]]

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: see table
CL: see table
```
[[6::Figure 1]]

```table id=test-conditions caption="Test conditions (Figure 1)"
| V~CC~ | Inputs | < | V~M~ | V~LOAD~ | C~L~ | R~L~ | V~Δ~ |
| ^ | V~I~ | t~r~/t~f~ | ^ | ^ | ^ | ^ | ^ |
|---|---|---|---|---|---|---|---|
| 1.8 V ± 0.15 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 1 kΩ | 0.15 V |
| 2.5 V ± 0.2 V | V~CC~ | ≤2 ns | V~CC~/2 | 2 × V~CC~ | 30 pF | 500 Ω | 0.15 V |
| 3.3 V ± 0.3 V | 3 V | ≤2.5 ns | 1.5 V | 6 V | 50 pF | 500 Ω | 0.3 V |
| 5 V ± 0.5 V | V~CC~ | ≤2.5 ns | V~CC~/2 | 2 × V~CC~ | 50 pF | 500 Ω | 0.3 V |
```
[[6::table #2]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: see table
```
[[6::Figure 1]]

## Package

```package footprint=SSOP-8
```

```package footprint=VSSOP-8
```

```package footprint=DSBGA-8-YZP
```

NanoFree™ package technology is a major breakthrough in IC packaging
concepts, using the die as the package. [[1::4]]

## Ordering information

```ordering
SN74LVC2G86DCTR | Reel of 3000 | -40 to 125 | 2WV5, C86, R, Z
SN74LVC2G86DCUR | Reel of 3000 | -40 to 125 | C86J, C86Q, C86R
SN74LVC2G86DCURG4 | Reel of 3000 | -40 to 125 | C86R
SN74LVC2G86DCUT | Reel of 250 | -40 to 125 | C86J, C86Q, C86R
SN74LVC2G86YZPR | Reel of 3000 | -40 to 85 | CHN
```
[[8::table #1]]

## Conversion notes

- Source pages 1–6 converted. The ordering lines are from the package
  option addendum (pp.8–9). The revision history page (7), tape and reel
  and mechanical pages (10–21) are left out; the generated Package
  section replaces them.
- Dropped: features list (the 5-V, input-voltage, latch-up and ESD lines
  are kept in text), the ESD caution box, the five exclusive-OR logic
  symbols (p.2).
- The pin descriptions are added; the source has no pin function table.
  The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty except for I~I~.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  second R~L~ and S1 are described in text.
- From images: overbars of Y = !{A}B + A!{B} (p.1); ≤ and Ω in Figure 1
  (p.6; the text layer has other glyphs).
- The package option addendum also lists SN74LVC2G86DCUT1G4.B, which the
  catalog doesn't carry.
- Source issues:
  - Source conflict: the recommended operating conditions give
    T~A~ –40°C to 125°C for every package (p.4); the package option
    addendum gives –40 to 85 for SN74LVC2G86YZPR (p.8).
