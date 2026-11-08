---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc08q-q1",
  "title": "SN74AHC08Q-Q1 automotive quadruple 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahc08q-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SGDS010E",
    "date": "2024-05",
    "pages": 25
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74AHC08QDRG4Q1", "SN74AHC08QDRQ1", "SN74AHC08QPWRG4Q1", "SN74AHC08QPWRQ1", "SN74AHC08QWBQARQ1"],
  "packages": {"p14": "D, PW, BQA"},
  "converted": "2026-11-08"
}
---

# SN74AHC08Q-Q1 automotive quadruple 2-input positive-AND gate

## Description

This device is a quadruple 2-input positive-AND gate that performs the
Boolean function Y = A • B in positive logic [1].

Qualified for automotive applications. Operating range 2 V to 5.5 V
V~CC~; EPIC (Enhanced-Performance Implanted CMOS) process; latch-up
performance exceeds 250 mA per JESD 17.

[1] Source error: the source prints "Y + A • B or Y = !{A} + !{B}". The function table (Table 6-1) gives Y = H only when A and B are both H, so the first form is Y = A • B, and !{A} + !{B} (a NAND) is kept out.

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

BQA package: the thermal pad can be connected to GND or left floating. Do
not connect to any other signal or supply.

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
| V~(ESD)~ | Electrostatic discharge, human body model (HBM), per AEC Q100-002 [3] | | | ±2000 | V |

[1] Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed. Source error: the V~I~ and V~O~ rows carry marker 1; this note (2) is the voltage-rating note and has no other marker.
[3] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V [2] | | -50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V [2] | | 50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
[2] Unconfirmed: the source prints the unit mA for the V~CC~ = 2 V rows; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = –50 µA and I~OL~ = 50 µA.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | D (SOIC) | PW (TSSOP) | BQA (WQFN) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 147.7 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report. All packages 14 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | T~A~ = –40 °C to 85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | V |
| I~I~ | Input current, A or B inputs | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | | pF |
```

```table id=noise caption="Noise characteristics [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.8 | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | T~A~ = –40 °C to 85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 6.2 | 8.8 | 1 | 10.5 | 1 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.2 | 8.8 | 1 | 10.5 | 1 | 10.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8.7 | 12.3 | 1 | 14 | 1 | 14 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.7 | 12.3 | 1 | 14 | 1 | 14 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 125 °C | < | T~A~ = –40 °C to 85 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 4.3 | 5.9 | 1 | 7 | 1 | 7 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.3 | 5.9 | 1 | 7 | 1 | 7 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.8 | 7.9 | 1 | 9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.8 | 7.9 | 1 | 9 | 1 | 9 | ns |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. C~L~ is 15 pF or 50 pF,
as given in the [switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50 % VCC
```

The source figure also shows the 3-state and open-drain load circuit,
pulse-duration, setup-and-hold and enable/disable waveforms, which do
not apply to this part.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHC08QDRG4Q1 | Reel of 2500 | | AHC08Q
SN74AHC08QDRQ1 | Reel of 2500 | -40 to 125 | AHC08Q
SN74AHC08QPWRG4Q1 | Reel of 2000 | -40 to 125 | HA08Q
SN74AHC08QPWRQ1 | Reel of 2000 | -40 to 125 | HA08Q
SN74AHC08QWBQARQ1 | Reel of 3000 | -40 to 125 | AHC08Q
```

The package option addendum lists SN74AHC08QDRG4Q1 as Last Time Buy,
with no operating temperature.

## Conversion notes

- Source pages 1–9 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features list (kept as one sentence), package information
  table, logic diagram (one AND gate).
- The ESD ratings table is merged into Absolute maximum ratings.
- Source issues:
  - Source error: Description formula "Y + A • B or Y = !{A} + !{B}"
    (Description note).
  - Source error: V~I~/V~O~ marker 1 instead of 2 in the absolute maximum
    ratings (table note).
  - Unconfirmed: I~OH~/I~OL~ at V~CC~ = 2 V printed as ±50 mA (ROC table
    note).
