---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74act11",
  "title": "SNx4ACT11 triple 3-input AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74act11.pdf",
    "vendor": "Texas Instruments",
    "document": "SCAS531E",
    "date": "2025-02",
    "pages": 36
  },
  "devices": ["gate:74_11"],
  "parts": ["5962-9077201QDA", "SN74ACT11BQAR", "SN74ACT11D", "SN74ACT11DBR", "SN74ACT11DR", "SN74ACT11DRG4", "SN74ACT11N", "SN74ACT11PW", "SN74ACT11PWR", "SNJ54ACT11W", "5962-9077201Q2A", "SNJ54ACT11FK"],
  "packages": {"p14": "D, DB, N, PW, W, BQA", "lccc20": "FK"},
  "converted": "2026-11-08"
}
---

# SNx4ACT11 triple 3-input AND gates

## Description

The SNx4ACT11 device contains three independent 3-input AND gates. These
devices perform the Boolean function Y = A · B · C in positive logic.
They operate from a V~CC~ of 4.5 V to 5.5 V; inputs are TTL-voltage
compatible and accept voltages to 5.5 V.
SN74ACT11 is specified from –40 °C to 85 °C, SN54ACT11 from –55 °C to
125 °C.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A: Channel 1–3, Input A
1B, 2B, 3B: Channel 1–3, Input B
1C, 2C, 3C: Channel 1–3, Input C
1Y, 2Y, 3Y: Channel 1–3, Output Y
GND: Ground
VCC: Positive Supply
NC: Not internally connected
```

Thermal pad (BQA package only): the thermal pad can be connected to GND
or left floating. Do not connect to any other signal or supply.

Source conflict: the source's pinout drawings and pin table name pin 9
3C and pin 11 3A (FK: pin 13 3C, pin 16 3A), but the pin table describes
pin 9 as "Channel 3, Input A" and pin 11 as "Channel 3, Input C". The
catalog pinout used here has 3A on pin 9 and 3C on pin 11 (FK: 3A on 13,
3C on 16). The three inputs of each gate perform the same function.

## Functional description

Logic diagram, each gate (positive logic): one 3-input AND gate, inputs
A, B and C, output Y. Pin numbers shown are for the D, DB, J, N, NS,
PW, and W packages.

## Function tables

```table id=function caption="Function table (each gate)"
| A | B | C | Y |
|:-:|:-:|:-:|:-:|
| H | H | H | H |
| L | X | X | L |
| X | L | X | L |
| X | X | L | L |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 or V~I~ > V~CC~ | | ±20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 or V~CC~ | | ±50 | mA |
| | Continuous current through V~CC~ or GND | | | ±200 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under recommended operating conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | SN54ACT11 | < | SN74ACT11 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -24 | | -24 | mA |
| I~OL~ | Low-level output current | | 24 | | 24 | mA |
| Δt/Δv | Input transition rise or fall rate | | 8 | | 8 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 85 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, SNx4ACT11 [1]"
| Symbol | Parameter | BQA (WQFN) | D (SOIC) | DB (SSOP) | N (PDIP) | NS (SOP) | PW (TSSOP) | Unit |
|---|---|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 91.3 | 119.9 | 96 | 80 | 76 | 145.7 | °C/W |

[1] All packages 14 pins. For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | SN54ACT11 | < | SN74ACT11 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.49 | | 4.4 | | 4.4 | | V |
| ^ | ^ | ^ | 5.5 V | 5.4 | 5.49 | | 5.4 | | 5.4 | | V |
| ^ | ^ | I~OH~ = -24 mA | 4.5 V | 3.86 | | | 3.7 | | 3.76 | | V |
| ^ | ^ | ^ | 5.5 V | 4.86 | | | 4.7 | | 4.76 | | V |
| ^ | ^ | I~OH~ = -50 mA [1] | 5.5 V | | | | 3.85 | | | | V |
| ^ | ^ | I~OH~ = -75 mA [1] | 5.5 V | | | | | | 3.85 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 4.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 5.5 V | | 0.001 | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 24 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | ^ | 5.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 50 mA [1] | 5.5 V | | | | | 1.65 | | | V |
| ^ | ^ | I~OL~ = 75 mA [1] | 5.5 V | | | | | | | 1.65 | V |
| I~I~ | Input current | V~I~ = V~CC~ or GND | 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 40 | | 20 | µA |
| ΔI~CC~ | Supply-current change [2] | One input at 3.4 V, other inputs at GND or V~CC~ | 5.5 V | | 0.6 | | | 1.6 | | 1.5 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 2.6 | | | | | | pF |

[1] Not more than one output should be tested at a time, and the duration of the test should not exceed 10 ms.
[2] This is the increase in supply current for each input that is at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=opchar caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | C~L~ = 50 pF, f = 1 MHz | 20 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | From (input) | To (output) | T~A~ = 25 °C | < | < | SN54ACT11 | < | SN74ACT11 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A, B, or C | Y | 1.5 | 6 | 9.5 | 1 | 10.5 | 1 | 10.5 | ns |
| t~PHL~ | A, B, or C | Y | 1.5 | 6 | 9.5 | 1 | 10.5 | 1 | 10.5 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 2.5 ns, t~f~ ≤ 2.5 ns. The outputs are measured one
at a time with one input transition per measurement.

The load circuit also has a 500 Ω resistor from the output to switch S1
(2 × V~CC~ or open); S1 is open for t~PLH~ and t~PHL~, which leaves the
load below.

```figure ref=load-rl-cl caption="Load circuit (S1 open)"
RL: 500 Ω
CL: 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times"
VM: 50 % V~CC~
```

## Application information

Information in this section is not part of the TI component
specification.

### Typical application

In this application, this device is used to directly control the RESET
pin of a motor controller. The controller requires three input signals
to all be HIGH before being enabled, and should be disabled in the event
that any one signal goes LOW. The 3-input AND gate function combines
the three individual reset signals into a single active-low reset
signal. Typical application schematic: the power supply's PG output,
the over-current detection's !{OC} output and an on/off switch drive
the three inputs of one gate; its output drives the motor controller's
!{RESET}.

```wavedrom id=app-timing caption="Typical application timing diagram"
{"signal": [
  {"name": "PG", "wave": "01...0..1."},
  {"name": "OC#", "wave": "0.1...0.1."},
  {"name": "ON/OFF#", "wave": "0..1......"},
  {"name": "RESET#", "wave": "0..1.0..1."}
]}
```

Power considerations: ensure the desired supply voltage is within the
range specified in the
[recommended operating conditions](#recommended-operating-conditions).
The supply must be capable of sourcing current equal to the total
current to be sourced by all outputs of the device (Source error: the
source names the SN74HC11 here) plus the maximum supply current, I~CC~, listed in
the [electrical characteristics](#electrical-characteristics). The logic
device can only source or sink as much current as it is provided at the
supply and ground pins, respectively. Be sure not to exceed the maximum
total current through GND or V~CC~ listed in the
[absolute maximum ratings](#absolute-maximum-ratings). The maximum
junction temperature, T~J(max)~ listed in the Absolute Maximum Ratings,
is an additional limitation to prevent damage to the device
(Source conflict: the absolute maximum ratings table lists no T~J~). Do
not violate any values listed in the Absolute Maximum Ratings.

Input considerations: unused inputs must be terminated to either V~CC~
or ground. These can be directly terminated if the input is completely
unused, or they can be connected with a pull-up or pull-down resistor
if the input is to be used sometimes, but not always. The resistor size
is limited by drive current of the controller, leakage current into the
device, and the desired input transition rate. A 10-kΩ resistor value
is often used due to these factors. The source states that the device
has standard CMOS inputs, so input signal edge rates cannot be slow
(Source conflict: the description and the ΔI~CC~ note give
TTL-voltage-compatible inputs). Slow input edge rates can cause
oscillations and damaging shoot-through current. The recommended rates
are defined in the
[recommended operating conditions](#recommended-operating-conditions).

Output considerations: drawing current from the output will decrease
the output voltage as specified by the V~OH~ specification; sinking
current into the output will increase the output voltage as specified
by the V~OL~ specification. Unused outputs can be left floating. Do not
connect outputs directly to V~CC~ or ground.

Detailed design procedure:

1. Add a decoupling capacitor from V~CC~ to GND. The capacitor needs to
   be placed physically close to the device and electrically close to
   both the V~CC~ and GND pins.
2. Ensure the capacitive load at the output is ≤ 70 pF. This is not a
   hard limit, however it will ensure optimal performance.
3. Ensure the resistive load at the output is larger than
   (V~CC~ / I~O(max)~) Ω. This will ensure that the maximum output
   current from the Absolute Maximum Ratings is not violated.
4. Thermal issues are rarely a concern for logic gates, however the
   power consumption and thermal increase can be calculated using the
   steps provided in the application report, CMOS Power Consumption and
   C~pd~ Calculation.

Power supply: the power supply can be any voltage between the minimum
and maximum supply voltage rating located in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-µF capacitor is recommended for this device. It is
acceptable to parallel multiple bypass caps to reject different
frequencies of noise; the 0.1-µF and 1-µF capacitors are commonly used
in parallel. The bypass capacitor should be installed as close to the
power terminal as possible for best results.

Layout: when using multiple-input and multiple-channel logic devices
inputs must not ever be left floating. Unused input pins must not be
left unconnected because the undefined voltages at the outside
connections result in undefined operational states. All unused inputs
of digital logic devices must be connected to a logic high or logic low
voltage, as defined by the input voltage specifications, to prevent
them from floating. The logic level that must be applied to any
particular unused input depends on the function of the device.
Generally, the inputs are tied to GND or V~CC~, whichever makes more
sense for the logic function or is more convenient.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
```

## Conversion notes

- Source pages 1–11 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages (12–36)
  dropped, replaced by the generated Package and Ordering sections.
- Dropped: features list, device information table, cross-references to
  sections the source doesn't have (Feature Description, Layout example);
  the logic diagram and the typical application schematic (Figure 7-1)
  are described in text.
- The application timing diagram is redrawn from Figure 7-2.
- The load circuit is drawn with the shared R~L~ ∥ C~L~ figure; the
  series resistor and S1 are described in text.
- From images: 500 Ω (both load resistors), 2 × V~CC~ (S1), 50 pF (C~L~),
  50 % V~CC~ (V~M~) (Figure 5-1, p.7).
- Source issues:
  - Source conflict: pin 9 / pin 11 names (3C / 3A) against their
    descriptions (Input A / Input C); see Pin functions.
  - Source error: the application section (7.2.1) names the SN74HC11;
    the document is the SNx4ACT11 data sheet (title, p.1). Written as "the device".
  - Source conflict: the input considerations call the inputs standard
    CMOS; the description and the ΔI~CC~ note give TTL-voltage-compatible
    inputs.
  - Source conflict: the caution names T~J(max)~ in the absolute maximum
    ratings; that table has no T~J~ row.
