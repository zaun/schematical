---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct08q-q1",
  "title": "SN74AHCT08Q-Q1 automotive quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahct08q-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SGDS021D",
    "date": "2024-02",
    "pages": 27
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74AHCT08QDRG4Q1", "SN74AHCT08QDRQ1", "SN74AHCT08QPWRG4Q1", "SN74AHCT08QPWRQ1", "SN74AHCT08QWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-08"
}
---

# SN74AHCT08Q-Q1 automotive quadruple 2-input positive-AND gates

## Description

The SN74AHCT08Q-Q1 devices are quadruple 2-input positive-AND gates.
These devices perform the Boolean function Y = A × B or
Y = !{!{A} + !{B}} in positive logic.

Qualified for automotive applications. Inputs are TTL-voltage compatible;
the TTL inputs allow up translation from 3.3 V to 5 V. EPIC
(enhanced-performance implanted CMOS) process; latch-up performance
exceeds 250 mA per JESD 17. T~A~ = –40 °C to 125 °C.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Channel 1–4, Input A
1B, 2B, 3B, 4B: Channel 1–4, Input B
1Y, 2Y, 3Y, 4Y: Channel 1–4, Output Y
GND: Ground
VCC: Positive Supply
```

Thermal pad (BQA package only): the thermal pad can be connected to GND
or left floating. Do not connect to any other signal or supply.

## Functional description

The SN74AHCT08Q-Q1 devices are quadruple 2-input positive-AND gates with low
drive that will produce slow rise and fall times [1]. This slow transition
reduces ringing on the output signal. The device has TTL inputs that
allow up translation from 3.3 V to 5 V. The inputs are high impedance
when V~CC~ = 0 V.

[1] Source conflict: Overview (p.8) says low drive with slow rise and fall times; Design Requirements (p.9) says balanced output drive whose high drive will create fast edges into light loads. Both kept.

## Function tables

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
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge, human body model (HBM) [3] | AEC Q100-002 HBM ESD Classification Level 2 | | ±2000 | V |
| ^ | Electrostatic discharge, charged device model (CDM) | AEC Q100-011 CDM ESD Classification Level C4B | | ±1000 | V |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Application Report, Implications of Slow or Floating CMOS Inputs.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D (SOIC) | PW (TSSOP) | BQA (WQFN) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.6 | 113 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report. All packages 14 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | µA |
| ΔI~CC~ | Supply current change [1] | One input at 3.4 V, other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=noise caption="Noise characteristics, SN74AHCT08Q-Q1 [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | –40 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 5 | 6.9 | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 6.9 | 1 | 8 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.5 | 7.9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.9 | 1 | 9 | ns |
```

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4.5]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.94], [-40, 2.99], [0, 3.33], [25, 3.58], [50, 3.77], [75, 3.95], [100, 4.14], [125, 4.26]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. C~L~ is 15 pF or 50 pF,
as given in the [switching characteristics](#sw). Inputs swing 0 V to
3 V and are measured at 1.5 V; outputs are measured at 50 % V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50 % VCC (output)
```

The source figure also shows the 3-state and open-drain load circuit,
pulse-duration, setup-and-hold and enable/disable waveforms; all
parameters and waveforms are not applicable to all devices.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

The SN74AHCT08Q-Q1 devices are low-drive CMOS devices that can be used for a
multitude of bus-interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The TTL inputs can except [sic] voltages down
to 3.3 V and translate up to 5 V. Typical application (Figure 8-1): a
3.3-V bus driver drives the gate inputs; the gate runs from a 5 V
regulated V~CC~ bypassed with 0.1 µF, and its output drives a 5-V
accessory.

Design requirements: this device uses CMOS technology and has balanced
output drive. Care should be taken to avoid bus contention because it
can drive currents that would exceed maximum limits. The high drive will
also create fast edges into light loads, so routing and load conditions
should be considered to prevent ringing.

- Rise time and fall time: see Δt/Δv in the
  [recommended operating conditions](#recommended-operating-conditions).
- High and low levels: see V~IH~ and V~IL~ in the same table.
- Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at
  any valid V~CC~.
- Load currents should not exceed 25 mA per output and 50 mA total for
  the part.
- Outputs should not be pulled above V~CC~.

Power supply: any voltage between the MIN and MAX supply voltage rating
located in the Recommended Operating Conditions table. Each V~CC~ pin
should have a good bypass capacitor to prevent power disturbance. For
devices with a single supply, 0.1 µF is recommended. If there are
multiple V~CC~ pins, 0.01 µF or 0.022 µF is recommended for each power
pin. A 0.1 µF and 1 µF are commonly used in parallel. The bypass
capacitor should be installed as close to the power pin as possible for
best results.

Layout: when using multiple bit logic devices, inputs should never
float. All unused inputs of digital logic devices must be connected to a
high or low bias to prevent them from floating; generally they will be
tied to GND or V~CC~. It is generally acceptable to float outputs unless
the part is a transceiver. The layout example recommends a GND flood
fill, a 0.1 µF bypass capacitor placed close to the device, unused inputs
tied to V~CC~, unused outputs left floating, and no 90° corners on signal
lines.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHCT08QDRG4Q1 | Reel of 2500 | -40 to 125 | AHCT08Q
SN74AHCT08QDRQ1 | Reel of 2500 | -40 to 125 | AHCT08Q
SN74AHCT08QPWRG4Q1 | Reel of 2000 | -40 to 125 | HB08Q
SN74AHCT08QPWRQ1 | Reel of 2000 | -40 to 125 | HB08Q
SN74AHCT08QWBQARQ1 | Reel of 3000 | -40 to 125 | AHT08Q
```

## Conversion notes

- Source pages 1–11 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists (features kept as one
  sentence), package information table, simplified schematic and
  functional block diagram (one AND gate), the layout example drawing
  (summarized in one sentence).
- Dropped: Figure 8-2 (switching characteristics comparison, an
  oscilloscope-style plot of V~I~ and the ACT08, HCT08 and AHCT08 outputs
  at V~CC~ = 5 V, load 50 Ω/50 pF); it is not a characteristic of this
  part alone.
- The ESD ratings table is merged into Absolute maximum ratings.
- The TPD curve is digitized from Figure 5-1 to about 2 % of full scale;
  the source states no load or V~CC~ for it.
- The typical application circuit (Figure 8-1) is described in text.
- From images: 25, 75 (temperature points read from Figure 5-1); 0.1 µF,
  5 V, 3.3-V, 5-V (Figure 8-1, p.9); 50 Ω, 50 pF, ACT08, HCT08 (Figure
  8-2, p.10); 0 V, 3 V input levels (Figure 6-1, p.7).
- Source issues:
  - Source conflict: Overview "low drive ... slow rise and fall times" vs
    Design Requirements "high drive ... fast edges" (Functional
    description note).
