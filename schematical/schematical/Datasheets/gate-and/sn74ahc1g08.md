---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g08",
  "title": "SN74AHC1G08 single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahc1g08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS314R",
    "date": "2024-01",
    "pages": 26
  },
  "devices": ["gate:1G08"],
  "parts": ["SN74AHC1G08DBV3", "SN74AHC1G08DBVR", "SN74AHC1G08DBVRE4", "SN74AHC1G08DBVRG4", "SN74AHC1G08DBVT", "SN74AHC1G08DBVTG4", "SN74AHC1G08DCKR", "SN74AHC1G08DCKRE4", "SN74AHC1G08DCKRG4", "SN74AHC1G08DCKT", "SN74AHC1G08DCKTE4", "SN74AHC1G08DCKTG4", "SN74AHC1G08DRLR", "SN74AHC1G08DRLRG4"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-08"
}
---

# SN74AHC1G08 single 2-input positive-AND gate

## Description

The SN74AHC1G08 device is a single 2-input positive-AND gate. The device
performs the Boolean function Y = A ● B in positive logic [1].

The SN74AHC1G08 device has a wide operating V~CC~ range of 2 V to 5.5 V,
which allows it to be used in a broad range of systems. The low
propagation delay allows fast switching and higher operation speeds. In
addition, the low-power consumption makes this device a good choice for
portable and battery power-sensitive applications. The source lists
Schmitt-trigger action at all inputs, which makes the circuit tolerant
for slower input rise and fall time.

[1] Source error: the Description (p.1) and Overview (p.8) print the second form as Y = !{A + B} (a NOR); the function table (Table 7-1) gives Y = H only when A and B are both H. Kept out.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A, B: Data Input
Y: Data Output
GND: Ground
VCC: Power
```

## Function tables

```table id=function caption="Function table"
| A [1] | B [1] | Y [2] |
|:-:|:-:|:-:|
| H | H | H |
| L | X | L |
| X | L | L |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge, human body model (HBM), all pins [3] | ANSI/ESDA/JEDEC JS-001 | | ±2000 | V |
| ^ | Electrostatic discharge, charged-device model (CDM), all pins [4] | JEDEC JESD22-C101 | | ±1000 | V |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[4] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
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
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (SOT-23) | DCK (SC70) | DRL (SOT) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 142 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953. All packages 5 pins.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25 °C | < | < | T~A~ = –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = -50 µA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | 3.8 | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | µA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | pF |
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | T~A~ = –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 6.2 | 8.8 | 1 | 10.5 | 1 | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.2 | 8.8 | 1 | 10.5 | 1 | 11 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 8.7 | 12.3 | 1 | 14 | 1 | 14.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.7 | 12.3 | 1 | 14 | 1 | 14.5 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25 °C | < | < | T~A~ = –40 °C to 85 °C | < | T~A~ = –55 °C to 125 °C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | | 4.3 | 5.9 | 1 | 7 | 1 | 7.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.3 | 5.9 | 1 | 7 | 1 | 7.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | | 5.8 | 7.9 | 1 | 9 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.8 | 7.9 | 1 | 9 | 1 | 9.5 | ns |
```

## Typical characteristics

```curve id=voh caption="AHC family V~OH~ vs I~OH~, T~A~ = 25 °C, V~CC~ = 5.5 V, all outputs switching"
{"x": {"label": "I_OH", "unit": "mA", "range": [-55, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 5.5]},
 "series": [
  {"label": "AHC", "points": [[-55, 1.79], [-50, 2.62], [-45, 3.13], [-40, 3.49], [-35, 3.83], [-30, 4.1], [-25, 4.34], [-20, 4.57], [-15, 4.8], [-10, 5.02], [-5, 5.24], [0, 5.5]]},
  {"label": "AHCT", "points": [[-55, 0], [-50, 0], [-45, 0.61], [-40, 1.27], [-35, 1.79], [-30, 2.27], [-25, 2.75], [-20, 3.21], [-15, 3.62], [-10, 4.06], [-5, 4.5], [0, 5.02]]}
 ]}
```

```curve id=vol caption="AHC family V~OL~ vs I~OL~, T~A~ = 25 °C, V~CC~ = 5.5 V, all outputs switching"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 40]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 3]},
 "series": [
  {"label": "AHC", "points": [[0, 0], [10, 0.31], [20, 0.66], [30, 1.07], [35, 1.33], [40, 1.65]]},
  {"label": "AHCT", "points": [[0, 0], [10, 0.21], [20, 0.44], [30, 0.72], [40, 1.0]]}
 ]}
```

Figure 5-1 (response time vs output voltage, T~A~ = 25 °C, V~A~ = 5 V) is
not converted: it shows two output edges rising from 0 V to 5 V near
20 ns and 26 ns on a 0–50 ns axis, with no labels for the traces.

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. C~L~ is 15 pF or 50 pF,
as given in the [switching characteristics](#switching-characteristics).

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50 % VCC
```

The source figure also shows the 3-state and open-drain load circuit,
pulse-duration, setup-and-hold and enable/disable waveforms; all
parameters and waveforms are not applicable to all devices.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

A common application for AND gates is their use in power sequencing.
Using the SN74AHC1G08 to verify that the processor has turned on can
protect it from any harmful signals. Power sequencing application
(Figure 8-1): input A is tied to V~CC~ = 5 V, the MCU (MSP43x) drives
input B, Y drives the EN input of a temperature sensor, and the sensor's
V~O~ returns to the MCU.

Design requirements: the SN74AHC1G08 device uses CMOS technology and has
balanced output drive. Take care to avoid bus contention because it can
drive currents that would exceed maximum limits. All input signals
should remain as close to either 0 V or V~CC~ as possible for optimal
operation.

- Rise time and fall time: see Δt/Δv in the
  [recommended operating conditions](#recommended-operating-conditions).
- High and low levels: see V~IH~ and V~IL~ in the same table.
- Inputs and outputs are overvoltage tolerant and can therefore go as
  high as 5.5 V at any valid V~CC~ [1].
- Load currents should not exceed ±50 mA [2].
- The effects of frequency upon the device's power consumption should be
  studied in CMOS Power Consumption and CPD Calculation, SCAA035.
- Added trace resistance and capacitance can reduce maximum frequency
  capability.

[1] Source conflict: the absolute maximum ratings limit V~O~ to V~CC~ + 0.5 V and the recommended operating conditions limit V~O~ to V~CC~; only the inputs are rated to 5.5 V.
[2] Source conflict: the absolute maximum continuous output current is ±25 mA; ±50 mA is the rating for the current through V~CC~ or GND.

Power supply: any voltage between the minimum and maximum supply voltage
rating listed in the
[recommended operating conditions](#recommended-operating-conditions).
Each V~CC~ terminal should have a bypass capacitor to prevent power
disturbance. A 0.1-µF bypass capacitor is recommended for devices with a
single supply. Capacitors with values of 0.1 µF and 1 µF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power terminal as possible for best results.

Layout: when a PCB trace turns a corner at a 90° angle, a reflection can
occur, primarily because of the change of width of the trace; at the
apex of the turn, the trace width increases to 1.414 times the width.
Not all PCB traces can be straight and therefore some traces must turn
corners. Only rounded corners (BEST in Figure 8-4) maintain constant
trace width and minimize reflections.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1G08DBVR | Reel of 3000 | -55 to 125 | 38EH, 3BVF, A083, A08G, A08J, A08L, A08S
SN74AHC1G08DBVRE4 | Reel of 3000 | -55 to 125 | A08G
SN74AHC1G08DBVRG4 | Reel of 3000 | -55 to 125 | A08G
SN74AHC1G08DBVTG4 | Reel of 250 | -55 to 125 | A08G
SN74AHC1G08DCKR | Reel of 3000 | -55 to 125 | 1B9, AE3, AEG, AEJ, AEL, AES
SN74AHC1G08DCKRE4 | Reel of 3000 | -55 to 125 | AE3
SN74AHC1G08DCKRG4 | Reel of 3000 | -55 to 125 | AE3
SN74AHC1G08DCKTE4 | Reel of 250 | -55 to 125 | AE3
SN74AHC1G08DCKTG4 | Reel of 250 | -55 to 125 | AE3
SN74AHC1G08DRLR | Reel of 4000 | -55 to 125 | AEB, AES
SN74AHC1G08DRLRG4 | Reel of 4000 | -55 to 125 | AEB, AES
```

The package option addendum lists SN74AHC1G08DBV3, SN74AHC1G08DBVT and
SN74AHC1G08DCKT as obsolete. Where several markings are listed, only one
appears on a part.

## Conversion notes

- Source pages 1–11 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists (Schmitt-trigger inputs kept
  as one sentence), package information table, logic diagram (one AND
  gate), the trace example drawing (Figure 8-4, summarized in one
  sentence), the generic multiple-V~CC~ and dual-supply bypass advice.
- The ESD ratings table is merged into Absolute maximum ratings.
- Curves digitized from Figures 8-2 and 8-3 to about 2 % of full scale.
- From images: MCU (MSP43x), EN, V~O~ (Figure 8-1, p.9); 20, 26 (edge
  times read from Figure 5-1, p.6).
- Source issues:
  - Source error: Description and Overview print Y = !{A + B} as the
    second form (Description note).
  - Source conflict: "inputs and outputs are overvoltage tolerant" to
    5.5 V vs the V~O~ ratings (Application information note).
  - Source conflict: "load currents should not exceed ±50 mA" vs I~O~
    ±25 mA (Application information note).
