---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct1g08",
  "title": "SN74AHCT1G08 single 2-input positive-AND gate",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahct1g08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS315S",
    "date": "2024-02",
    "pages": 27
  },
  "devices": ["gate:1G08"],
  "parts": ["74AHCT1G08DBVRG4", "74AHCT1G08DCKRE4", "74AHCT1G08DCKRG4", "74AHCT1G08DRLRG4", "SN74AHCT1G08DBV3", "SN74AHCT1G08DBVR", "SN74AHCT1G08DBVT", "SN74AHCT1G08DCK3", "SN74AHCT1G08DCKR", "SN74AHCT1G08DCKT", "SN74AHCT1G08DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-08"
}
---

# SN74AHCT1G08 single 2-input positive-AND gate

## Description

The SN74AHCT1G08 device is a single 2-input positive-AND gate. The device
performs the Boolean function Y = A × B in positive logic [1]. Low I~CC~
current allows this device to be used in power-sensitive or
battery-powered applications.

[1] Source error: the Description (p.1) and Overview (p.8) print the second form as Y = !{A + B} (a NOR); the function table (Table 7-1) gives Y = H only when A and B are both H. Kept out.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
Y: Output
GND: Ground Pin
VCC: Supply Pin
```

## Functional description

Robust inputs allow the device to up-translate with a propagation delay
of 20 ns [1]. The V~CC~ for the device is optimized at 5 V. Up voltage
translation from 3.3 V to 5 V is allowed. The inputs accept V~IH~ levels
of 2 V. Output ringing is minimized by slow edge rates. Inputs are
TTL-Voltage compatible.

[1] Source conflict: the switching characteristics (p.6) give t~PLH~ and t~PHL~ of at most 10 ns.

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
| T~J~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge, human-body model (HBM) [3] | ANSI/ESDA/JEDEC JS-001 | | ±2500 | V |
| ^ | Electrostatic discharge, charged-device model (CDM) [4] | JEDEC JESD22-C101 | | ±1000 | V |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[4] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
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
| Δt/Δv | Input transition rise and fall rate | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information [1]"
| Symbol | Parameter | DBV (SOT-23) | DCK (SC70) | DRL (SOT) | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 242.9 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | 77.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | 77.5 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | 9.6 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | 77.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953. All packages 5 pins. R~θJC(bot)~ (junction-to-case, bottom) is N/A for all of them.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | T~A~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA, V~CC~ = 4.5 V | 25 °C | 4.4 | 4.5 | | V |
| ^ | ^ | ^ | -40 °C to 125 °C | 4.4 | | | V |
| ^ | ^ | I~OH~ = -8 mA, V~CC~ = 4.5 V | 25 °C | 3.94 | | | V |
| ^ | ^ | ^ | -40 °C to 125 °C | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA, V~CC~ = 4.5 V | | | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA, V~CC~ = 4.5 V | 25 °C | | | 0.36 | V |
| ^ | ^ | ^ | -40 °C to 125 °C | | | 0.44 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND, V~CC~ = 0 V to 5.5 V | 25 °C | | | ±0.1 | µA |
| ^ | ^ | ^ | -40 °C to 125 °C | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0, V~CC~ = 5.5 V | 25 °C | | | 1 | µA |
| ^ | ^ | ^ | -40 °C to 125 °C | | | 10 | µA |
| ΔI~CC~ | Change in supply current [1] | One input at 3.4 V, other inputs at V~CC~ or GND, V~CC~ = 5.5 V | 25 °C | | | 1.35 | mA |
| ^ | ^ | ^ | -40 °C to 125 °C | | | 1.5 | mA |
| C~I~ | Input capacitance | V~I~ = V~CC~ or GND, V~CC~ = 5 V | | | 4 | 10 | pF |

[1] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25 °C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 18 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted)"
| Symbol | Parameter | From (input) | To (output) | Output capacitance | Test conditions | Min | Typ | Max | Unit |
|---|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~ | Propagation delay, low to high transition | A or B | Y | C~L~ = 15 pF | T~A~ = 25 °C | | 5 | 6.2 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 85 °C | 1 | | 7.1 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 125 °C | 1 | | 7.5 | ns |
| t~PHL~ | Propagation delay, high to low transition | A or B | Y | C~L~ = 15 pF | T~A~ = 25 °C | | 5 | 6.2 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 85 °C | 1 | | 7.1 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 125 °C | 1 | | 7.5 | ns |
| t~PLH~ | Propagation delay, low to high transition | A or B | Y | C~L~ = 50 pF | T~A~ = 25 °C | | 5.5 | 7.9 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 85 °C [1] | 1 | | 9 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 125 °C | 1 | | 10 | ns |
| t~PHL~ | Propagation delay, high to low transition | A or B | Y | C~L~ = 50 pF | T~A~ = 25 °C | | 5.5 | 7.9 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 85 °C | 1 | | 9 | ns |
| ^ | ^ | ^ | ^ | ^ | T~A~ = -40 °C to 125 °C | 1 | | 10 | ns |

[1] Source error: this condition cell is printed "Propagation delay, high to low transition"; the row sits between the 25 °C and –40 °C to 125 °C rows, and the matching t~PHL~ row and both 15 pF groups give T~A~ = –40 °C to 85 °C there.
```

## Typical characteristics

```curve id=tpd-temp caption="T~pd~ vs temperature, C~L~ = 15 pF"
{"x": {"label": "Temperature", "unit": "°C", "range": [-25, 135]},
 "y": {"label": "Tpd", "unit": "ns", "range": [4, 8]},
 "series": [
  {"label": "Tpd vs. Temp", "points": [[0, 5.89], [15, 6.07], [35, 6.36], [55, 6.67], [75, 6.96], [95, 7.21], [115, 7.38], [125, 7.51]]}
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

The SN74AHCT1G08 device is a single AND gate, which is often used for
many common functions like power sequencing or an on LED indicator.
Because the device is configured to output LOW unless all inputs are
HIGH, an LED tied to the output of the device will only turn HIGH when
all systems connected are sending a HIGH, or ready signal. Typical
application (Figure 8-1): as an AND logic function, A and B come from a
µC or logic and Y drives a µC or logic; as a basic LED driver, Y drives
an LED with a series resistor to ground.

Design requirements: this device uses CMOS technology and has balanced
output drive. Take care to avoid bus contention because it can drive
currents that would exceed maximum limits. The high drive also creates
fast edges into light loads, so routing and load conditions must be
considered to prevent ringing.

- Rise time and fall time: see Δt/Δv in the
  [recommended operating conditions](#recommended-operating-conditions).
- High and low levels: see V~IH~ and V~IL~ in the same table.
- Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at
  any valid V~CC~.
- Load currents must not exceed 25 mA per output and 50 mA total for the
  part.
- Outputs must not be pulled above V~CC~.

Power supply: any voltage between the minimum and maximum supply voltage
rating located in Recommended Operating Conditions. Each V~CC~ pin must
have a good bypass capacitor to prevent power disturbance. TI recommends
a 0.1-µF capacitor for devices with a single supply. 0.1-µF and 1-µF
capacitors are commonly used in parallel. The bypass capacitor must be
installed as close to the power pin as possible for best results.

Layout: when using multiple bit logic devices inputs must not ever
float. All unused inputs of digital logic devices must be connected to a
high or low bias to prevent them from floating; generally they will be
tied to GND or V~CC~. The layout example recommends a GND flood fill, a
0.1 µF bypass capacitor placed close to the device, and no 90° corners
on signal lines.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
74AHCT1G08DBVRG4 | Reel of 3000 | -40 to 125 | B08G
74AHCT1G08DCKRE4 | Reel of 3000 | -40 to 125 | BE3, BEG, BEL, BES
74AHCT1G08DCKRG4 | Reel of 3000 | -40 to 125 | BE3, BEG, BEL, BES
74AHCT1G08DRLRG4 | Reel of 4000 | -40 to 125 | BEB, BES
SN74AHCT1G08DBV3 | Reel of 3000 | -40 to 125 | B08Y
SN74AHCT1G08DBVR | Reel of 3000 | -40 to 125 | 38WH, 3C8F, B083, B08G, B08J, B08L, B08S
SN74AHCT1G08DCK3 | Reel of 3000 | -40 to 125 | BEY
SN74AHCT1G08DCKR | Reel of 3000 | -40 to 125 | 1PJ, BE3, BEG, BEJ, BEL, BES
SN74AHCT1G08DRLR | Reel of 4000 | -40 to 125 | BEB, BES
```

The package option addendum lists SN74AHCT1G08DBVT and SN74AHCT1G08DCKT
as obsolete, and SN74AHCT1G08DBV3 and SN74AHCT1G08DCK3 as Last Time Buy.
Where several markings are listed, only one appears on a part.

## Conversion notes

- Source pages 1–11 converted; support pages, revision history, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, package information table,
  logic diagram (one AND gate), the layout example drawing (summarized in
  one sentence), the multiple-V~CC~-pin bypass advice.
- Dropped: Figure 8-2 (typical switching characteristics, an
  oscilloscope-style plot of V~I~ and the AHCT1G08, AC and HC outputs at
  V~CC~ = 5 V, load 50 Ω / 50 pF); it compares families rather than
  characterizing this part.
- The ESD ratings table is merged into Absolute maximum ratings.
- The T~pd~ curve is digitized from Figure 5-1 to about 2 % of full scale.
- The typical application diagram (Figure 8-1) is described in text.
- From images: 5.89, 6.07, 6.36, 6.67, 6.96, 7.21, 7.38, 7.51 (T~pd~
  points, Figure 5-1, p.6); µC, LED (Figure 8-1, p.9); 0 V, 3 V, 1.5 V
  input levels and 50 % V~CC~ (Figure 6-1, p.7, image only).
- Source issues:
  - Source error: Description and Overview print Y = !{A + B} as the
    second form (Description note).
  - Source error: t~PLH~ 50 pF condition cell misprinted (switching table
    note).
  - Source conflict: "propagation delay of 20 ns" vs at most 10 ns in the
    switching characteristics (Functional description note).
