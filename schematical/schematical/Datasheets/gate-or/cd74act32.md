---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act32",
  "title": "CDx4ACT32 quadruple 2-input positive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-or/cd74act32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS342A",
    "date": "2024-08",
    "pages": 21
  },
  "devices": ["gate:74_32"],
  "parts": ["CD54ACT32F3A", "CD74ACT32E", "CD74ACT32M", "CD74ACT32M96", "CD74ACT32M96G4"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CDx4ACT32 quadruple 2-input positive-OR gates

## Description

The 'ACT32 devices are quadruple 2-input positive-OR gates. These
devices perform the Boolean function Y = !{!{A} • !{B}} or Y = A + B in
positive logic. Source error: the source prints one bar over the whole
product, Y = !{A • B}, which is the NAND function; the bars over A and
over B are added, as Y = A + B and the function table require. [[1::1]]

Inputs are TTL-voltage compatible. Buffered inputs. Speed of bipolar F,
AS, and S, with significantly reduced power consumption. Balanced
propagation delays. ±24mA output drive current. Fanout to 15 F devices.
SCR-latchup-resistant CMOS process and circuit design. Exceeds 2k V ESD
protection per MIL-STD-883, method 3015. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1B: 1B Input
1Y: 1Y Output
2A: 2A Input
2B: 2B Input
2Y: 2Y Output
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
4Y: 4Y Output
4A: 4A Input
4B: 4B Input
GND: Ground Pin
VCC: Power Pin
```
[[3::Table 3-1]]

## Functional description

Logic Diagram (Positive Logic). Conversion note: the diagram shows one
2-input OR gate, inputs A and B, output Y. [[8::1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[8::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6 | V |
| I~IK~ [2] | Input clamp current | V~I~ < 0 V or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ [2] | Output clamp current | V~O~ < 0 V or V~O~ > V~CC~ | | ±50 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | ±2000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 500-V HBM is possible with the necessary precautions.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | T~A~ = 25°C | < | –55°C to 125°C | < | –40°C to 85°C | < | Unit |
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
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, CD74ACT32"
| Symbol | Thermal metric [1] | D (SOIC) 14 pins | N (PDIP) 14 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 119.9 | 80 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | < | V~CC~ | T~A~ = 25 °C | < | –55°C to 125°C | < | –40°C to 85°C | < | Unit |
| ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | V~I~ = V~IH~ or V~IL~ | I~OH~ = -50 µA | 4.5 V | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | 3.7 | | 3.8 | | V |
| ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | | 3.85 | | V |
| V~OL~ | V~I~ = V~IH~ or V~IL~ | I~OL~ = 50 µA | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | 1.65 | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, | I~O~ = 0 | 5.5 V | | 4 | | 80 | | 40 | µA |
| ΔI~CC~ [2] | V~I~ = V~CC~ - 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 3 | | 2.8 | mA |
| C~i~ | | < | | | 10 | | 10 | | 10 | pF |

[1] Test one output at a time, not exceeding 1-second duration. Measurement is made by forcing indicated current and measuring voltage to minimize power dissipation. Test verifies a minimum 50-Ω transmission-line drive capability at 85°C and 75-Ω transmission-line drive capability at 125°C.
[2] Additional quiescent supply current per input pin, TTL inputs high, 1 unit load
```
[[5::table #1]]

```table id=input-load caption="Act input load table"
| Input | Value | Unit |
|---|--:|---|
| All | 0.42 | unit load |
```
[[5::Table 4-1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 47 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | -55°C to 125°C | < | -40°C to 85°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 3 | 12.1 | 3.1 | 11 | ns |
| t~PHL~ | A or B | Y | 3 | 12.1 | 3.1 | 11 | ns |
```
[[5::table #3]]

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. Phase relationships
between waveforms are arbitrary. The outputs are measured one at a time
with one input transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~. All parameters and waveforms are not applicable to all
devices. [[6::1]]

Conversion note: the load circuit of Figure 5-1 also has R1 = 500 Ω
from the output to switch S1 (2 × V~CC~, Open or GND), and S1 is Open
for t~PLH~/t~PHL~, which leaves R2 and C~L~ below. Conversion note: in
the voltage waveforms the input swings 0 V to 3 V and is measured at
1.5 V, and the outputs are measured at 50 % V~CC~ (transition times at
10 % and 90 %).

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```
[[6::Figure 5-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output transition times"
VM: 1.5 V (input), 50 % V~CC~ (outputs)
```
[[6::Figure 5-1]]

## Application information

Information in the following applications sections is not part of the
TI component specification. [[9::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Section 4.1 table. Source
conflict: Section 4.1 is the
[absolute maximum ratings](#absolute-maximum-ratings) table; see
Conversion notes. [[9::2]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 μF and 1 μF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results. [[9::3]]

### Layout guidelines

When using multiple bit logic devices inputs should not ever float. [[9::4]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND
gate are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Specified
in Figure 7-1 are the rules that must be observed under all
circumstances. All unused inputs of digital logic devices
must be connected to a high or low bias to prevent them from floating.
The logic level that must be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~; whichever makes more sense or is more convenient. It is
generally acceptable to float outputs unless the part is a transceiver.
If the transceiver has an output enable pin, it will disable the
outputs section of the part when asserted. This will not disable the
input section of the IOs, so they cannot float when disabled. [[9::5]]

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
CD54ACT32F3A | Tube of 25 | -55 to 125 | CD54ACT32F3A
CD74ACT32E | Tube of 25 | -55 to 125 | CD74ACT32E | NRND
CD74ACT32M | | -55 to 125 | ACT32M | Obsolete
CD74ACT32M96 | Reel of 2500 | -55 to 125 | ACT32M
CD74ACT32M96G4 | Reel of 2500 | -55 to 125 | ACT32M
```
[[11::table #1]]

## Conversion notes

- Source pages 1–9 converted; the support and revision history page
  (10), the package option addendum, tube and mechanical pages (11–20)
  and the notice page (21) dropped, replaced by the generated Package
  and Ordering sections.
- Dropped: device information table, the vendor's pinout drawing, the
  layout example figure (Figure 7-1) and the NC row of the pin
  functions table (the 14-pin packages have no NC pin); the logic
  diagram is described in text.
- Figure 5-1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this gate; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, the
  other rows of the S1 table (p.7) and their notes (B, D, G, H) are
  dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- From images: 500 Ω (R1, R2), 2 × V~CC~ (S1), 50 pF (C~L~), 3 V, 0 V,
  1.5 V, 50 % V~CC~, 10 % and 90 % (waveform levels) (Figure 5-1, p.6).
- From images: overbars of Y = !{!{A} • !{B}} and of Y = !{A • B} (p.1).
- Ordering vendor columns are from the package option addendum (p.11).
- Source issues:
  - Source error: the description prints Y = !{A • B}, one bar over the
    whole product (p.1), a NAND function. Written Y = !{!{A} • !{B}},
    as Y = A + B and the function table (p.8) require.
  - Source conflict: the power supply recommendations point to the
    Section 4.1 table (absolute maximum ratings, V~CC~ –0.5 V to 6 V)
    for the supply voltage range; the recommended operating conditions
    (Section 4.3) give 4.5 V to 5.5 V.
