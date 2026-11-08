---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc08",
  "title": "SNx4AHC08 quadruple 2-input positive-AND gates",
  "class": "logic",
  "source": {
    "pdf": "gate-and/sn74ahc08.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS236L",
    "date": "2024-02",
    "pages": 42
  },
  "devices": ["gate:74_08"],
  "parts": ["SN74AHC08BQAR", "SN74AHC08D", "SN74AHC08DBR", "SN74AHC08DGVR", "SN74AHC08DR", "SN74AHC08DR1G4", "SN74AHC08N", "SN74AHC08NSR", "SN74AHC08PWR", "SN74AHC08PWRG4", "SN74AHC08RGYR", "SN74AHC08RGYRG4", "5962-9682001Q2A", "SNJ54AHC08FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA", "lccc20": "FK"},
  "converted": "2026-11-08"
}
---

# SNx4AHC08 quadruple 2-input positive-AND gates

## Description

The SNx4AHC08 devices are quadruple 2-input positive-AND gates. These
devices perform the Boolean function Y = A · B in positive logic.

SN74AHC08 is specified from –40 °C to 125 °C, SN54AHC08 from –55 °C to
125 °C. Operating range 2 V to 5.5 V; inputs are overvoltage tolerant up
to 5.5 V at any valid V~CC~.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
```

## Pin functions

```pins
1A, 2A, 3A, 4A: Input A
1B, 2B, 3B, 4B: Input B
1Y, 2Y, 3Y, 4Y: Output
GND: Ground
VCC: Power
NC: No connection
```

## Functional description

The SNx4AHC08 devices are quadruple 2-input positive-AND gates with low
drive that will produce slow rise and fall times [1]. This slow transition
reduces ringing on the output signal. The inputs are high impedance when
V~CC~ = 0 V.

[1] Source conflict: Overview (p.12) says low drive with slow rise and fall times; Design Requirements (p.13) says balanced output drive whose high drive will create fast edges into light loads. Both kept.

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
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge, human body model (HBM) [3] | ANSI/ESDA/JEDEC JS-001 | | ±2000 | V |
| ^ | Electrostatic discharge, charged device model (CDM) [4] | JEDEC JESD22-C101 | | ±1000 | V |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 500-V HBM is possible with the necessary precautions.
[4] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process. Manufacturing with less than 250-V CDM is possible with the necessary precautions.
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
| I~OH~ | High-level output current | V~CC~ = 2 V [2] | | -50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V [2] | | 50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | SN54AHC08 | -55 | 125 | °C |
| ^ | ^ | SN74AHC08 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
[2] Unconfirmed: the source prints the unit mA for the V~CC~ = 2 V rows; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = –50 µA and I~OL~ = 50 µA.
```

## Thermal information

```table id=thermal caption="Thermal information (SN74AHC08) [1]"
| Symbol | Parameter | D (SOIC) | DB (SSOP) | DGV (TVSOP) | N (PDIP) | NS (SO) | PW (TSSOP) | RGY (VQFN) | BQA (WQFN) | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 96 | 127 | 80 | 76 | 147.7 | 87.1 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953. All packages 14 pins.
```

## Electrical characteristics

```table id=ec-25 caption="Electrical characteristics, T~A~ = 25 °C" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Typ | Max | Unit |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 2 V | 1.9 | 2 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.58 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.94 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V | | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA [1] | 3 V | | | 0.36 | V |
| ^ | ^ | I~OL~ = 8 mA [1] | 4.5 V | | | 0.36 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | pF |

[1] Source error: printed as I~OH~ = 4 mA and I~OH~ = 8 mA in the V~OL~ rows of all three electrical tables; the rows give V~OL~, and the recommended operating conditions give I~OL~ = 4 mA and 8 mA, so the condition is I~OL~.
```

```table id=ec-54 caption="Electrical characteristics, SN54AHC08, T~A~ = –55 °C to 125 °C" parts=SNJ54*,5962* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | Min | Max | Unit |
|---|---|---|---|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 2 V | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.48 | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.8 | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | 0.5 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | ±1 [1] | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | 20 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
```

```table id=ec-74 caption="Electrical characteristics, SN74AHC08" parts=SN74* conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = –40 °C to 125 °C | < | T~A~ = –40 °C to 85 °C | Unit |
| ^ | ^ | ^ | ^ | Min | Max | Max | ^ |
|---|---|---|---|--:|--:|--:|---|
| V~OH~ | High-level output voltage | I~OH~ = -50 µA | 2 V | 1.9 | | | V |
| ^ | ^ | ^ | 3 V | 2.9 | | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | | | V |
| ^ | ^ | I~OH~ = -4 mA | 3 V | 2.48 | | | V |
| ^ | ^ | I~OH~ = -8 mA | 4.5 V | 3.8 | | | V |
| V~OL~ | Low-level output voltage | I~OL~ = 50 µA | 2 V | | 0.1 | | V |
| ^ | ^ | ^ | 3 V | | 0.1 | | V |
| ^ | ^ | ^ | 4.5 V | | 0.1 | | V |
| ^ | ^ | I~OL~ = 4 mA [1] | 3 V | | 0.5 | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA [1] | 4.5 V | | 0.5 | 0.44 | V |
| I~I~ | Input current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | ±1 | | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | 20 | | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | | 10 | pF |

[1] The source labels the –40 °C to 125 °C limit of these rows "Recommended".
```

```table id=noise caption="Noise characteristics, SN74AHC08 [1]" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25 °C"
| Symbol | Parameter | Min | Max | Unit |
|---|---|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.4 | | V |
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
| Symbol | From (input) | To (output) | Load capacitance | T~A~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~, t~PHL~ | A or B | Y | C~L~ = 15 pF | 25 °C [1] |  | 6.2 | 8.8 | ns |
| ^ | ^ | ^ | ^ | -55 °C to 125 °C, SN54AHC08 [1] | 1 |  | 10.5 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 85 °C, SN74AHC08 | 1 |  | 10.5 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 125 °C Recommended, SN74AHC08 | 1 |  | 10.5 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | 25 °C |  | 8.7 | 12.3 | ns |
| ^ | ^ | ^ | ^ | -55 °C to 125 °C, SN54AHC08 | 1 |  | 14 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 85 °C, SN74AHC08 | 1 |  | 14 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 125 °C Recommended, SN74AHC08 | 1 |  | 14 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ | Min | Typ | Max | Unit |
|---|---|---|---|---|--:|--:|--:|---|
| t~PLH~, t~PHL~ | A or B | Y | C~L~ = 15 pF | 25 °C [1] |  | 4.3 | 5.9 | ns |
| ^ | ^ | ^ | ^ | -55 °C to 125 °C, SN54AHC08 [1] | 1 |  | 7 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 85 °C, SN74AHC08 | 1 |  | 7 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 125 °C Recommended, SN74AHC08 | 1 |  | 7 | ns |
| ^ | ^ | ^ | C~L~ = 50 pF | 25 °C |  | 5.8 | 7.9 | ns |
| ^ | ^ | ^ | ^ | -55 °C to 125 °C, SN54AHC08 | 1 |  | 9 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 85 °C, SN74AHC08 | 1 |  | 9 | ns |
| ^ | ^ | ^ | ^ | -40 °C to 125 °C Recommended, SN74AHC08 | 1 |  | 9 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

```curve id=voh caption="AHC family V~OH~ vs I~OH~, T~A~ = 25 °C, V~CC~ = 5.5 V, all outputs switching [1]"
{"x": {"label": "I_OH", "unit": "mA", "range": [-55, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 5.5]},
 "series": [
  {"label": "AHC", "points": [[-55, 1.79], [-50, 2.62], [-45, 3.13], [-40, 3.49], [-35, 3.83], [-30, 4.1], [-25, 4.34], [-20, 4.57], [-15, 4.8], [-10, 5.02], [-5, 5.24], [0, 5.5]]},
  {"label": "AHCT", "points": [[-55, 0], [-50, 0], [-45, 0.61], [-40, 1.27], [-35, 1.79], [-30, 2.27], [-25, 2.75], [-20, 3.21], [-15, 3.62], [-10, 4.06], [-5, 4.5], [0, 5.02]]}
 ]}
```

```curve id=vol caption="AHC family V~OL~ vs I~OL~, T~A~ = 25 °C, V~CC~ = 5.5 V, all outputs switching [1]"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 40]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 3]},
 "series": [
  {"label": "AHC", "points": [[0, 0], [10, 0.31], [20, 0.66], [30, 1.07], [35, 1.33], [40, 1.65]]},
  {"label": "AHCT", "points": [[0, 0], [10, 0.21], [20, 0.44], [30, 0.72], [40, 1.0]]}
 ]}
```

Source conflict: Figure 5-1 is captioned "AHC Family V~OL~ vs I~OL~" but plots V~OH~ against I~OH~; Figure 8-2 is captioned "AHC Family V~OH~ vs I~OH~" but plots V~OL~ against I~OL~. The captions above follow the plotted axes.

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
VM: 50 % V~CC~
```

The source figure also shows the 3-state and open-drain load circuit,
pulse-duration, setup-and-hold and enable/disable waveforms; all
parameters and waveforms are not applicable to all devices.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness.

A common application for AND gates is the use in power sequencing. Using
the SN74AHC08 to verify that the processor has turned on can protect it
from harmful signals. Typical application (Figure 8-1): input A is tied
to V~CC~ = 5 V, the MCU (MSP43x) drives input B, Y drives the EN input of
a temperature sensor, and the sensor's V~O~ returns to the MCU.

Design requirements: this device uses CMOS technology and has balanced
output drive. Take care to avoid bus contention because it can drive
currents that would exceed maximum limits. The high drive will also
create fast edges into light loads, so routing and load conditions must
be considered to prevent ringing.

- Rise time and fall time: see Δt/Δv in the
  [recommended operating conditions](#recommended-operating-conditions).
- High and low levels: see V~IH~ and V~IL~ in the same table.
- Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at
  any valid V~CC~.
- Load currents should not exceed 25 mA per output and 50 mA total for
  the part.
- Outputs should not be pulled above V~CC~.

Power supply: any voltage between the minimum and maximum supply voltage
rating. Each V~CC~ pin should have a good bypass capacitor to prevent
power disturbance. For devices with a single supply, 0.1 µF is
recommended. If there are multiple V~CC~ pins, 0.01 µF or 0.022 µF is
recommended for each power pin. A 0.1 µF and 1 µF are commonly used in
parallel. The bypass capacitor should be installed as close to the power
pin as possible for best results.

Layout: when using multiple bit logic devices inputs should not ever
float. All unused inputs of digital logic devices must be connected to a
high or low bias to prevent them from floating; generally they will be
tied to GND or V~CC~. It is generally acceptable to float outputs unless
the part is a transceiver. The layout example recommends a GND flood
fill, a 0.1 µF bypass capacitor placed close to the device, unused inputs
tied to V~CC~, unused outputs left floating, and no 90° corners on signal
lines.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
SN74AHC08BQAR | Reel of 3000 | -40 to 125 | AHC08
SN74AHC08DBR | Reel of 2000 | -40 to 125 | HA08
SN74AHC08DGVR | Reel of 2000 | -40 to 125 | HA08
SN74AHC08DR | Reel of 2500 | -40 to 125 | AHC08
SN74AHC08DR1G4 | Reel of 2500 | -40 to 125 | AHC08
SN74AHC08N | Tube of 25 | -40 to 125 | SN74AHC08N
SN74AHC08NSR | Reel of 2000 | -40 to 125 | AHC08
SN74AHC08PWR | Reel of 2000 | -40 to 125 | HA08
SN74AHC08PWRG4 | Reel of 2000 | -40 to 125 | HA08
SN74AHC08RGYR | Reel of 3000 | -40 to 125 | HA08
SN74AHC08RGYRG4 | Reel of 3000 | -40 to 125 | HA08
5962-9682001Q2A | Tube of 55 | -55 to 125 | 5962-9682001Q2A SNJ54AHC08FK
SNJ54AHC08FK | Tube of 55 | -55 to 125 | 5962-9682001Q2A SNJ54AHC08FK
```

The package option addendum lists SN74AHC08D as obsolete. The pin
configuration also names a W package, for which the source lists no
orderable part.

## Conversion notes

- Source pages 1–15 converted; revision history, support pages, the
  package option addendum, tape and reel and mechanical pages dropped,
  replaced by the generated Package and Ordering sections.
- Dropped: features and applications lists, device information table,
  simplified schematic and functional block diagram (one AND gate), the
  layout example drawing (summarized in one sentence).
- The ESD ratings table is merged into Absolute maximum ratings.
- Curves digitized from Figures 5-1 and 8-2 to about 2 % of full scale.
- The typical application circuit (Figure 8-1) is described in text.
- The SN54AHC08 table prints no C~i~ value; the row is kept empty.
- From images: MCU (MSP43x), EN, V~O~ (Figure 8-1, p.13).
- Source issues:
  - Source error: the Description prints the second form of the Boolean
    function as Y = !{A + B} (a NOR); the function table (Table 7-1) gives
    Y = H only when A and B are both H. Kept out; only Y = A · B is given.
  - Source error: V~OL~ rows printed with I~OH~ = 4 mA / 8 mA in all three
    electrical tables; corrected to I~OL~ (table note).
  - Unconfirmed: I~OH~/I~OL~ at V~CC~ = 2 V printed as ±50 mA (ROC table
    note).
  - Source conflict: Overview "low drive ... slow rise and fall times" vs
    Design Requirements "high drive ... fast edges" (Functional
    description note).
  - Source conflict: captions of Figures 5-1 and 8-2 swapped against
    their plotted axes.
