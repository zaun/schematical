---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g04-q1",
  "title": "SN74AHCT1G04-Q1 automotive single 4.5V to 5.5V inverter with TTL-compatible CMOS inputs",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct1g04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS510D",
    "date": "2024-10",
    "pages": 20
  },
  "devices": ["gate:1G04"],
  "parts": ["CAHCT1G04QDCKRG4Q1", "CAHCT1G04QDCKRQ1", "CAHCT1G04WDTXRQ1"],
  "packages": {"p5": "DCK, DTX"},
  "converted": "2026-11-21"
}
---

# SN74AHCT1G04-Q1 automotive single 4.5V to 5.5V inverter with TTL-compatible CMOS inputs

## Description

The SN74AHCT1G04-Q1 contains one gate. The device performs the Boolean
function Y = !{A}. [[1::2]]

Qualified for automotive applications. ESD protection exceeds 1500V per
MIL-STD-883, method 3015. Operating range of 4.5V to 5.5V. Max tpd of
7.5ns at 5V. Low power consumption, 10µA max I~CC~. 8mA output drive at
5V. Inputs are TTL-voltage compatible. [[1::3]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No Connection
A: Input A
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```
[[3::Table 4-1]]

## Functional description

Conversion note: the functional block diagram (§ 7.1) shows one inverter, input A, output Y.

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[7::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under [Section 5.3](#recommended-operating-conditions) is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output negative-voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±1500 | V |

[1] AEC Q100-002 indicates that HBM stressing must be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | V |
| V~IL~ | Low-level Input voltage | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | mA |
| Δt/Δv | Input Transition rise or fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information"
| Symbol | Thermal metric [1] | DCK (SOT-SC70) 5 pins | DTX (X2SON) 5 pins | Unit |
|---|---|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 293.4 | 184.7 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[4::table #4]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to +125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | I~OH~ = −8 mA | 4.5 V | 3.94 | | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | µA |
| ΔI~CC~ ^1^ | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | pF |
```
[[5::table #1]]

Source error: the source prints a note marker 1 on ΔI~CC~ (p.5) and no note under the table; the marker is kept as printed.

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 14 | pF |
```
[[5::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature rangee, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see load circuit and voltage wave forms)"
| Symbol | From (input) | To (output) | Test conditions | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4.7 | 6.7 | 1 | 7.5 | ns |
| t~PHL~ | A | Y | C~L~ = 15 pF | | 4.7 | 6.7 | 1 | 7.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.5 | 7.7 | 1 | 8.5 | ns |
| t~PHL~ | A | Y | C~L~ = 50 pF | | 5.5 | 7.7 | 1 | 8.5 | ^ |
```
[[5::table #3]]

Conversion note: the source prints no temperature heading over the last Min and Max columns of this table.

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.4], [-40, 4.6], [0, 4.78], [25, 4.9], [50, 5.1], [85, 5.4], [125, 5.6]]}
 ]}
```
[[5::figure #1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. t~PHL~ and t~PLH~ are
the same as t~pd~. All parameters and waveforms are not applicable to
all devices. [[6::Figure 6-1]]

Conversion note: in Figure 6-1 the inputs swing 0 V to V~CC~, inputs and outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[6::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[6::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1
table, and the pulse-duration, setup-and-hold and enable/disable
waveforms with their notes B, E and F, which are not drawn here.

## Package

```package footprint=SC70-5
```

```package footprint=X2SON-5-DTX
```

## Ordering information

```ordering
CAHCT1G04QDCKRG4Q1 | Reel of 3000 | -40 to 125 | BCS | NRND
CAHCT1G04QDCKRQ1 | Reel of 3000 | -40 to 125 | BCS
CAHCT1G04WDTXRQ1 | Reel of 3000 | -40 to 125 | 1
```
[[10::table #1]]

## Conversion notes

- Source pages 1–7 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.10–11). The support and revision history page (8), the
  mechanical information page (9), tape and reel and mechanical pages
  (12–20) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the vendor
  pin drawings (p.3) with their note "NC − No internal connection", the
  simplified schematic (p.1, one inverter); the functional block diagram
  (p.7) is described in text.
- The source has no Overview, no Feature Description and no Application
  and Implementation chapter; none is added.
- Section 5.3, named in absolute maximum ratings note 1, is the source's
  Recommended Operating Conditions.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- The curve is digitized from the unnumbered, uncaptioned figure under
  Typical Characteristics (p.5), a picture, to about 2 % of full scale;
  the source states no load or V~CC~ for it.
- From images: page 6 (Figure 6-1 and its notes A to H) is a picture
  with no text layer: the Parameter measurement information sentences,
  "C~L~ includes probe and jig capacitance.", "All input pulses are
  supplied by generators having the following characteristics:
  PRR ≤ 1 MHz, Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns.", "The outputs
  are measured one at a time, with one input transition per
  measurement.", "t~PHL~ and t~PLH~ are the same as t~pd~." and "All
  parameters and waveforms are not applicable to all devices." are read
  from it, with 0 V, 50% V~CC~, R~L~ = 1 kΩ, S1 and the drawing.
- From images: the overbar of Y = !{A} (p.1); the axis labels TPD (ns)
  and Temperature (°C), the legend "TPD in ns", the curve points -55,
  4.4, -40, 4.6, 0, 4.78, 25, 4.9, 50, 5.1, 85, 5.4, 125, 5.6 and the
  axis ranges -100 to 150 and 0 to 6 (figure on p.5).
- Source issues:
  - Source error: ΔI~CC~ carries a note marker 1 and the table has no
    note; kept as printed (Electrical characteristics note).
  - Source error: the I~OK~ row of the absolute maximum ratings prints
    ". ±20" (p.4), with a stray dot, which is kept out.
  - Source error: the switching characteristics line prints "rangee" and
    "wave forms" (p.5); kept as printed.
  - Source error: Parameter Measurement Information (p.6) has a heading
    "6.1" with no title; kept out.
  - Source conflict: Figure 4-1 (p.3) is captioned "DBV Package, 5-Pin
    SOT-23; DCK Package"; the package information table (p.1), the
    thermal information (p.4) and the addendum list no DBV package.
  - Unconfirmed: Figure 6-1 (p.6) draws the inputs swinging to V~CC~ and
    measured at 50% V~CC~, while the inputs are TTL-voltage compatible
    (V~IH~ 2 V, V~IL~ 0.8 V, p.4).
