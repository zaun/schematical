---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "cd74act04",
  "title": "CDx4ACT04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/cd74act04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCHS310D",
    "date": "2024-08",
    "pages": 23
  },
  "devices": ["gate:74_04"],
  "parts": ["CD74ACT04E", "CD74ACT04M96"],
  "packages": {"p14": "D, N, J"},
  "converted": "2026-11-21"
}
---

# CDx4ACT04 hex inverters

## Description

The 'ACT04 devices contain six independent inverters. The devices
perform the Boolean function Y = !{A}. [[1::1]]

Inputs are TTL-voltage compatible. Speed of Bipolar F, AS, and S, with
significantly reduced power consumption. Balanced propagation delays.
± 24mA output drive current. Fanout to 15 F Devices.
SCR-latchup-resistant CMOS process and circuit design. Exceeds 2kV ESD
protection per MIL-STD-883, method 3015. [[1::2]]

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: Channel 1, Input A
1Y: Channel 1, Output Y
2A: Channel 2, Input A
2Y: Channel 2, Output Y
3A: Channel 3, Input A
3Y: Channel 3, Output Y
GND: Ground
4Y: Channel 4, Output Y
4A: Channel 4, Input A
5Y: Channel 5, Output Y
5A: Channel 5, Input A
6Y: Channel 6, Output Y
6A: Channel 6, Input A
VCC: Positive Supply
```
[[3::Table 3-1]]

## Functional description

Conversion note: the logic diagram, each inverter (positive logic)
(Figure 6-1), shows one inverter with input A and output Y.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[7::Table 6-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 6 | V |
| I~IK~ | Input clamp current | (V~I~ < 0 or V~I~ > V~CC~) [2] | | ±20 | mA |
| I~OK~ | Output clamp current | (V~O~ < 0 or V~O~ > V~CC~) [2] | | ±50 | mA |
| I~O~ | Continuous output current | (V~O~ = 0 to V~CC~) | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±100 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

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
[[4::table #2]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | CD74ACT04 | < | Unit |
| ^ | ^ | N (PDIP) 14 pins | D (SOIC) 14 pins | ^ |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 80 | 89.9 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[4::table #3]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Parameter | Test conditions | < | V~CC~ | T~A~ = 25°C | < | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | < | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | V~I~ = V~IH~ or V~IL~ | I~OH~ = -50 µA | 4.5 V | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.94 | | 3.8 | | 3.7 | | V |
| ^ | ^ | I~OH~ = -50 mA | 5.5 V | | | | | 3.85 | | V |
| ^ | ^ | I~OH~ = -75 mA | 5.5 V | | | 3.85 | | | | V |
| V~OL~ | V~I~ = V~IH~ or V~IL~ | I~OL~ = 50 µA | 4.5 V | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | 0.36 | | 0.44 | | 0.5 | V |
| ^ | ^ | I~OL~ = 50 mA | 5.5 V | | | | | | 1.65 | V |
| ^ | ^ | I~OL~ = 75 mA | 5.5 V | | | | 1.65 | | | V |
| I~I~ | V~I~ = V~CC~ or GND | < | 5.5 V | | ±0.1 | | ±0.1 | | ±0.1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND | I~O~ = 0 | 5.5 V | | 4 | | 40 | | 80 | µA |
| ΔI~CC~ | V~I~ = V~CC~ – 2.1 V | < | 4.5 V to 5.5 V | | 2.4 | | 2.8 | | 3 | mA |
| C~i~ | | < | | | 10 | | 10 | | 10 | pF |
```
[[5::table #1]]

Source error: the V~OH~ and V~OL~ rows at 5.5 V carry a note marker (1) (p.5), and the source prints no note 1 under the electrical characteristics table; the markers are kept out.

```table id=input-load caption="Act input load table [1]"
| Input | Value | Unit |
|---|--:|---|
| A | 0.18 | unit load |

[1] Unit load is ΔI~CC~ limit specified in electrical characteristics table (e.g., 2.4 mA at 25°C).
```
[[5::Table 4-1]]

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Typ | Unit |
|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | 105 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF (unless otherwise noted) (see Load Circuit and Voltage Wave Forms)"
| Parameter | From (input) | To (output) | -40°C to 85°C | < | -55°C to 125°C | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | 2.4 | 8.5 | 2.3 | 9.3 | ns |
| t~PHL~ | ^ | ^ | 2.4 | 8.5 | 2.3 | 9.3 | ns |
```
[[5::table #3]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at
a time, with one input transition per measurement. t~PHL~ and t~PLH~
are the same as t~pd~. All parameters and waveforms are not applicable
to all devices. [[6::1]]

Conversion note: the load circuit of Figure 5-1 also has R1 = 500 Ω
from the output to switch S1 (2 × V~CC~, Open or GND), and S1 is Open
for t~PLH~/t~PHL~, which leaves R2 and C~L~ below.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω (R2)
CL: 50 pF
```
[[6::Figure 5-1]]

Conversion note: in the voltage waveforms the input swings between 0 V
and 3 V and is measured at 1.5 V, and the outputs are measured at 50%
V~CC~, with transition times between 10% and 90%.

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

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the
[Recommended Operating Conditions](#recommended-operating-conditions)
table. [[8::2]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 µF is recommended.
If there are multiple V~CC~ pins, 0.01 µF or 0.022 µF is recommended
for each power pin. It is acceptable to parallel multiple bypass caps
to reject different frequencies of noise. A 0.1 µF and 1 µF are
commonly used in parallel. The bypass capacitor should be installed as
close to the power pin as possible for best results. [[8::3]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such
input pins should not be left unconnected because the undefined
voltages at the outside connections result in undefined operational
states. [[8::4]]

Specified in Layout Example are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The
logic level that should be applied to any particular unused input
depends on the function of the device. Generally they will be tied to
GND or V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I/Os so they also cannot float when disabled. [[8::5]]

Conversion note: the layout diagram (Figure 7-1) shows a 0.1 µF bypass capacitor placed close to the device between V~CC~ and GND, a GND flood fill recommended for improved signal isolation, noise reduction, and thermal dissipation, unused inputs tied to V~CC~, an unused output left floating, and signal lines that avoid 90° corners.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

## Ordering information

```ordering
CD74ACT04E | Tube of 25 | -55 to 125 | CD74ACT04E | NRND
CD74ACT04M96 | Reel of 2500 | -55 to 125 | ACT04M
```
[[11::table #1]]

## Conversion notes

- Source pages 1–8 converted; the table of contents (2), the support
  and revision history pages (9–10), the package option addendum, tape
  and reel, tube and mechanical pages (11–22) and the notice page (23)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: device information table, the vendor's pinout drawing and
  the layout example figure (Figure 7-1, described in text); the logic
  diagram is described in text.
- Figure 5-1 is TI's generic load circuit and waveform set. Only the
  propagation-delay parts apply to this inverter; the pulse-duration,
  recovery-time, setup/hold and output-enable/disable waveforms, the
  other rows of the S1 table (p.6) and their notes (B, E, F) are
  dropped.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; R1 and
  S1 are described in text.
- The Act input load table's "Unit Load" column is written as a Value
  column with the unit "unit load".
- The catalog does not carry CD54ACT04F3A (J package), CD74ACT04M and
  CD74ACT04M96G4, which the package option addendum lists.
- From images: 500 Ω (R1, R2), 2 × V~CC~ (S1), 50 pF (C~L~), 0 V, 3 V,
  1.5 V, 50% V~CC~, 10% and 90% (waveform levels) (Figure 5-1, p.6);
  0.1 µF and 90° (Figure 7-1, p.8).
- From images: overbar of Y = !{A} (p.1).
- Ordering vendor columns are from the package option addendum (p.11).
- Source issues:
  - Source error: the V~OH~ and V~OL~ rows at 5.5 V (p.5) carry a note
    marker (1), and no note 1 is printed under the electrical
    characteristics table; the markers are kept out.
