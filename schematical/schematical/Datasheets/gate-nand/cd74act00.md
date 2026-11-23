---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act00",
  "title": "CDx4ACT00 quadruple 2-input positive-NAND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nand/cd74act00.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS308C",
    "date": "2024-08",
    "pages": 20
  },
  "devices": ["gate:74_00"],
  "parts": ["CD54ACT00F3A", "CD74ACT00E", "CD74ACT00M", "CD74ACT00M96", "CD74ACT00M96G4"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CDx4ACT00 quadruple 2-input positive-NAND gates

## Description

The 'ACT00 devices contain four independent 2-input NAND gates. Each
gate performs the Boolean function of Y = !{A • B} in positive logic. [[1::1]]

Inputs are TTL-voltage compatible. Speed of bipolar F, AS, and S, with
significantly reduced power consumption. Balanced propagation delays.
±24mA output drive current. Fanout to 15 F devices.
SCR-latchup-resistant CMOS process and circuit design. Exceeds 2kV ESD
protection per MIL-STD-883, method 3015. [[1::2]] [[1::3]]

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
3A: 3A Input
3B: 3B Input
3Y: 3Y Output
4A: 4A Input
4B: 4B Input
4Y: 4Y Output
GND: Ground Pin
VCC: Power Pin
```
[[3::Table 3-1]]

## Functional description

Conversion note: the logic diagram, each gate (positive logic), shows
one 2-input NAND gate with inputs A and B and output Y.

## Function tables

Function Table (Each Gate) is the function table for the CDx4ACT00. [[7::2]]

```table id=function caption="Function table (each gate)"
| Inputs | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| H | H | L |
| L | X | H |
| X | L | H |
```
[[7::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6 | V |
| I~IK~ [2] | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) | | ±20 | mA |
| I~OK~ [2] | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) | | ±50 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±50 | mA |
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
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | CD74ACT00 | < | Unit |
| ^ | ^ | D (SOIC) 14 pins | N (PDIP) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 119.9 | 80 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Parameter | Test conditions | < | V~CC~ | T~A~ = 25 °C | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
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
| C~i~ | | < | | | 10 | | 10 | | 10 | PF |

[1] Test one output at a time, not exceeding 1-second duration. Measurement is made by forcing indicated current and measuring voltage to minimize power dissipation. Test verifies a minimum 50-Ω transmission-line drive capability at 85°C and 75-Ω transmission-line drive capability at 125°C.
```
[[5::table #1]]

Source error: the C~i~ unit is printed PF (p.5), kept as printed; picofarads (pF) are meant, as in the operating characteristics.

```table id=input-load caption="Act input load table" values=Value
| Input | Unit load | Unit |
|---|--:|---|
| A or B | 0.15 | |
```
[[5::Table 4-1]]

Unit load is ΔI~CC~ limit specified in electrical characteristics table
(e.g., 2.4 mA at 25°C). [[5::4]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 45 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF (unless otherwise noted) (see Figure 5-1)"
| Parameter | From (input) | To (output) | –40°C to 85°C | < | –55°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | 3.4 | 9.5 | 3.2 | 10.8 | ns |
| t~PHL~ | A or B | Y | 2.8 | 8 | 2.7 | 13.2 | ns |
```
[[5::table #3]]

Unconfirmed: the t~PHL~ maximum at –55°C to 125°C is printed 13.2 ns (p.5), above the t~PLH~ maximum of 10.8 ns, while at –40°C to 85°C the t~PHL~ maximum (8 ns) is below the t~PLH~ maximum (9.5 ns).

## Parameter measurement information

C~L~ includes probe and test-fixture capacitance. All input pulses are
supplied by generators having the following characteristics:
PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ = 3 ns, t~f~ = 3 ns. Phase relationships
between waveforms are arbitrary. The outputs are measured one at a time
with one input transition per measurement. t~PLH~ and t~PHL~ are the
same as t~pd~. [[6::Figure 5-1]]

Conversion note: the load circuit of Figure 5-1 also has R1 = 500 Ω
from the output to switch S1 (2 × V~CC~, Open or GND), and S1 is Open
for t~PLH~/t~PHL~, which leaves R2 and C~L~ below. Conversion note: in
the voltage waveforms the input swings 0 V to 3 V and is measured at
1.5 V, and the outputs are measured at 50% V~CC~ (transition times at
10% and 90%).

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```
[[6::Figure 5-1]]

```figure ref=waveforms-cmos-tpd-tt caption="Voltage waveforms, propagation delay and output transition times"
VM: 1.5 V (input), 50% V~CC~ (outputs)
```
[[6::Figure 5-1]]

## Application information

Information in the following applications sections is not part of the
TI component specification, and TI does not warrant its accuracy or
completeness. TI's customers are responsible for determining
suitability of components for their purposes, as well as validating and
testing their design implementation to confirm system functionality. [[8::1]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating located in the Section 4.1 table. Source
conflict: Section 4.1 is the
[absolute maximum ratings](#absolute-maximum-ratings) table; see
Conversion notes. [[8::2]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended.
If there are multiple V~CC~ pins, 0.01 µF or 0.022 µF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 µF and 1 µF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results. [[8::3]]

### Layout guidelines

When using multiple bit logic devices inputs should not ever float. [[8::4]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND
gate are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Specified
in Layout Example for the CD74ACT00 are the rules that must be observed
under all circumstances. All unused inputs of digital logic devices
must be connected to a high or low bias to prevent them from floating.
The logic level that must be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~; whichever makes more sense or is more convenient. It is
generally acceptable to float outputs unless the part is a transceiver. [[8::5]]

Conversion note: the layout example for the CDx4ACT00 (Figure 7-1) shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, a GND flood fill recommended for improved signal isolation, noise reduction, and thermal dissipation, unused inputs tied to V~CC~, an unused output left floating, and signal lines that avoid 90° corners.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=CDIP-14
```

## Ordering information

```ordering
CD54ACT00F3A | Tube of 25 | -55 to 125 | CD54ACT00F3A
CD74ACT00E | Tube of 25 | -55 to 125 | CD74ACT00E | NRND
CD74ACT00M | | -55 to 125 | ACT00M | Obsolete
CD74ACT00M96 | Reel of 2500 | -55 to 125 | ACT00M
CD74ACT00M96G4 | Reel of 2500 | -55 to 125 | ACT00M
```
[[10::table #1]]

## Conversion notes

- Source pages 1–8 converted; the table of contents (2), the support
  and revision history page (9), the package option addendum, tube and
  mechanical pages (10–19) and the notice page (20) dropped, replaced
  by the generated Package and Ordering sections.
- Dropped: device information table, the vendor's pinout drawing, the
  NC row of the pin functions table (it gives no pin number; the
  14-pin packages have no NC pin), the layout example figure
  (Figure 7-1, described in text) and the two transceiver sentences
  that end the layout guidelines; the logic diagram is described in
  text.
- Figure 5-1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this gate; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, the
  other rows of the S1 table (p.6) and their notes (B, D, G, H) are
  dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- From images: 500 Ω (R1, R2), 2 × V~CC~ (S1), 50 pF (C~L~), 0 V, 3 V
  and 1.5 V (input), 50% V~CC~, 10% and 90% (outputs) (Figure 5-1,
  p.6); 0.1 µF and 90° (Figure 7-1, p.8).
- From images: overbars of Y = !{A • B} (p.1).
- Ordering vendor columns are from the package option addendum (p.10).
- Source issues:
  - Source error: the C~i~ unit is printed PF in the electrical
    characteristics (p.5), kept as printed; the operating
    characteristics (p.5) print pF.
  - Source conflict: the power supply recommendations (p.8) point to
    the Section 4.1 table (absolute maximum ratings, V~CC~ –0.5 V to
    6 V) for the supply voltage range; the recommended operating
    conditions (Section 4.3) give 4.5 V to 5.5 V.
  - Unconfirmed: the t~PHL~ maximum at –55°C to 125°C is printed
    13.2 ns (p.5), above the t~PLH~ maximum of 10.8 ns; at –40°C to
    85°C the t~PHL~ maximum (8 ns) is below the t~PLH~ maximum
    (9.5 ns).
