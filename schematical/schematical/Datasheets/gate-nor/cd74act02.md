---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act02",
  "title": "CD54ACT02, CD74ACT02 quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/cd74act02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS309B",
    "date": "2002-05",
    "pages": 24
  },
  "devices": ["gate:74_02"],
  "parts": ["CD54ACT02F3A", "CD74ACT02BQAR", "CD74ACT02E", "CD74ACT02EE4", "CD74ACT02M", "CD74ACT02M96", "CD74ACT02PWR"],
  "packages": {"p14": "D, N, PW, BQA, J"},
  "converted": "2026-11-21"
}
---

# CD54ACT02, CD74ACT02 quadruple 2-input positive-NOR gates

## Description

The 'ACT02 devices contain four independent 2-input NOR gates that
perform the Boolean function Y = !{A} • !{B} or Y = !{A + B} in
positive logic. [[1::2]]

Inputs are TTL-voltage compatible. Speed of bipolar F, AS, and S, with
significantly reduced power consumption. Balanced propagation delays.
±24-mA output drive current. Fanout to 15 F devices.
SCR-latchup-resistant CMOS process and circuit design. Exceeds 2-kV ESD
protection per MIL-STD-883, Method 3015. [[1::1]]

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

## Functional description

Conversion note: the logic diagram (positive logic) shows the four
2-input NOR gates with their pin numbers.

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

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 6 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) (see Note 1) [2] | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) (see Note 1) [2] | | ±50 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| θ~JA~ | Package thermal impedance (see Note 2) [3] | E package | | 80 | °C/W |
| ^ | ^ | M package | | 86 | °C/W |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] The package thermal impedance is calculated in accordance with JESD 51-7.
```
[[2::table #1]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions (see Note 3) [1]"
| Symbol | Parameter | T~A~ = 25°C | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 10 | | 10 | | 10 | ns/V |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```
[[2::table #2]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Parameter | Test conditions | < | V~CC~ | T~A~ = 25°C | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | V~I~ = V~IH~ or V~IL~ | I~OH~ = -50 µA | 4.5 V | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | 3.8 | | 3.7 | | V |
| ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | | | 3.85 | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | 3.85 | | | | V |
| V~OL~ | V~I~ = V~IH~ or V~IL~ | I~OL~ = 50 µA | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.36 | | 0.44 | | 0.5 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | | | 1.65 | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | 1.65 | | | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, | I~O~ = 0 | 5.5 V | | 4 | | 40 | | 80 | µA |
| ΔI~CC~ | V~I~ = V~CC~ - 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 2.8 | | 3 | mA |
| C~i~ | | < | | | 10 | | 10 | | 10 | pF |

[1] Test one output at a time, not exceeding 1-second duration. Measurement is made by forcing indicated current and measuring voltage to minimize power dissipation. Test verifies a minimum 50-Ω transmission-line drive capability at 85°C and 75-Ω transmission-line drive capability at 125°C.
```
[[2::table #3]]

```table id=input-load caption="ACT input load table [1]" values=Value
| Input | Unit load | Unit |
|---|--:|---|
| A or B | 0.32 | |

[1] Unit load is ΔI~CC~ limit specified in electrical characteristics table (e.g., 2.4 mA at 25°C).
```
[[3::table #1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 55 | pF |
```
[[3::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF (unless otherwise noted) (see Figure 1)"
| Parameter | From (input) | To (output) | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 3.1 | 11.1 | 3.1 | 12.2 | ns |
| t~PHL~ | A or B | Y | 3.1 | 11.1 | 3.1 | 12.2 | ns |
```
[[3::table #3]]

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. Phase relationships
between waveforms are arbitrary. The outputs are measured one at a time
with one input transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~. [[4::1]]

Conversion note: the load circuit of Figure 1 also has R1 = 500 Ω
from the output to switch S1 (2 × V~CC~, Open or GND), and S1 is Open
for t~PLH~/t~PHL~, which leaves R2 and C~L~ below. Conversion note: in
the voltage waveforms the input swings 0 V to 3 V and is measured at
1.5 V, and the outputs are measured at 50% V~CC~ (transition times at
10% and 90%).

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```
[[4::Figure 1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output transition times"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```
[[4::Figure 1]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
CD54ACT02F3A | Tube of 25 | -55 to 125 | CD54ACT02F3A
CD74ACT02BQAR | Reel of 3000 | -55 to 125 | AD02
CD74ACT02E | Tube of 25 | -55 to 125 | CD74ACT02E
CD74ACT02EE4 | Tube of 25 | -55 to 125 | CD74ACT02E
CD74ACT02M | | -55 to 125 | ACT02M | Obsolete
CD74ACT02M96 | Reel of 2500 | -55 to 125 | ACT02M
CD74ACT02PWR | Reel of 3000 | -55 to 125 | AD02
```
[[5::table #1]]

## Conversion notes

- Source pages 1–4 converted (the 2002 data sheet); the package option
  addendum, tape and reel, tube and mechanical pages (5–23) and the
  notice page (24) dropped, replaced by the generated Package and
  Ordering sections.
- Dropped: the vendor's pinout drawing and the 2002 ordering
  information table (p.1), which the package option addendum replaces;
  the logic diagram is described in text. The pin descriptions are the
  converter's: the source has no pin table.
- The data sheet names the packages F (CDIP), E (PDIP) and M (SOIC);
  the package option addendum names them J, N and D and adds the PW
  (TSSOP) and BQA (WQFN) parts, which the 2002 pages don't describe.
- Figure 1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this gate; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, the
  other rows of the S1 table (p.4) and their notes (B, D, G, H) are
  dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- From images: overbars of Y = !{A} • !{B} and Y = !{A + B} (p.1).
- From images: ΔI~CC~ and Δt/Δv (recommended operating conditions and
  electrical characteristics, p.2; input load table, p.3), θ~JA~
  (absolute maximum ratings, p.2).
- Ordering vendor columns are from the package option addendum (p.5).
- Source issues: none found.
