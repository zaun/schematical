---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g02",
  "title": "SN74AHC1G02 single 2-input positive-NOR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahc1g02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS342O",
    "date": "2024-02",
    "pages": 26
  },
  "devices": ["gate:1G02"],
  "parts": ["SN74AHC1G02DBVR", "SN74AHC1G02DBVRE4", "SN74AHC1G02DBVRG4", "SN74AHC1G02DBVT", "SN74AHC1G02DCKR", "SN74AHC1G02DCKRG4", "SN74AHC1G02DCKT", "SN74AHC1G02DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G02 single 2-input positive-NOR gate

## Description

This device contains a single 2-input NOR gate that performs the Boolean
function Y = !{A} × !{B} or Y = !{A + B} in positive logic.

Operating Range of 2 V to 5.5 V. Max t~pd~ of 6.5 ns at 5 V. Low Power
Consumption, 10-µA Max I~CC~. ±8-mA Output Drive at 5 V. Schmitt Trigger
Action at All Inputs Makes the Circuit Tolerant for Slower Input Rise
and Fall Time.

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground Pin
Y: Output Y
VCC: Power Pin
```

## Functional description

This device contains a single 2-input NOR gate that performs the Boolean
function Y = !{A} × !{B} or Y = !{A + B} in positive logic.

Conversion note: the functional block diagram (Figure 7-1) shows one 2-input NOR gate with inputs A and B and output Y.

### Feature description

- Wide operating voltage range
  - Operates from 2 V to 5.5 V
- Allows down-voltage translation
  - Inputs accept voltages to 5.5 V
- The low drive and slow edge rates will minimize overshoot and
  undershoot on the outputs

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output [2] |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through each V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| T~J~ | Junction Temperature | | | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Section 5.3 is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | V |
| V~IH~ [2] | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
[2] Source error: the source prints the symbol V~IH~ on this Input voltage row (p.4), kept as printed; V~IH~ is the High-level input voltage of the rows above, and the absolute maximum ratings name the input voltage V~I~.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G02: thermal metric [1]"
| Symbol | Parameter | DBV 5 pins | DCK 5 pins | DRL 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 328.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | 105.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | 150.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | 6.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | 148.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | High level output voltage | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA [1] | 2 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | | 0.44 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| C~i~ | Input capacitance | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | 10 | | 10 | pF |

[1] Source error: printed as I~OH~ = 50 µA; the rows give V~OL~, and the recommended operating conditions give 50 µA as I~OL~, so the condition is I~OL~.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 15 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5.6 | 7.9 | 1 | 9.5 | 1 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.6 | 7.9 | 1 | 9.5 | 1 | 10.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 8.1 | 11.4 | 1 | 13 | 1 | 14 | ns |
| t~PHL~ | ^ | ^ | ^ | | 8.1 | 11.4 | 1 | 13 | 1 | 14 | ns |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.6 | 5.5 | 1 | 6.5 | 1 | 7 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.6 | 5.5 | 1 | 6.5 | 1 | 7 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.1 | 7.5 | 1 | 8.5 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.1 | 7.5 | 1 | 8.5 | 1 | 9 | ns |
```

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 5]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.8], [-40, 3], [25, 3.6], [125, 4.3]]}
 ]}
```

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "Vcc", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[2, 6], [3, 4.1], [4.5, 3.9], [5.5, 3.3]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. All parameters and
waveforms are not applicable to all devices.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

SN74AHC1G02 is a low-drive CMOS device that can be used for a multitude
of bus interface type applications where output ringing is a concern.
The low drive and slow edge rates will minimize overshoot and undershoot
on the outputs. The inputs can accept voltages to 5.5 V at any valid
V~CC~ making it Ideal for down translation.

Conversion note: the typical application schematic (Figure 8-1) shows the gate, supplied from a 3.3-V or 5-V regulated rail with a 0.1 µF capacitor to ground, between 5-V System Logic on a 5-V accessory and a µC or System Logic.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing.

Source conflict: Application Information (§ 8.1) and Feature Description (§ 7.3) say low drive and slow edge rates, Design Requirements (§ 8.2.1) says "the high drive will also create fast edges"; both are kept as printed.

### Detailed design procedure

1. Recommended Input Conditions
   - For rise time and fall time specifications, see Δt/ΔV in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table.
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~.
2. Recommend Output Conditions
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part.
   - Outputs should not be pulled above V~CC~.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table.

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended. If
there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states.

Specified in Figure 8-3 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or V~CC~,
whichever makes more sense or is more convenient. It is acceptable to
float outputs unless the part is a transceiver. If the transceiver has
an output enable pin, it will disable the outputs section of the part
when asserted. This will not disable the input section of the I/Os so
they also cannot float when disabled.

Conversion note: the layout diagram (Figure 8-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1G02DBVR | Reel of 3000 | -40 to 125 | 38AH, 3BTF, A023, A02G, A02J, A02S
SN74AHC1G02DBVRE4 | Reel of 3000 | -40 to 125 | A02G
SN74AHC1G02DBVRG4 | Reel of 3000 | -40 to 125 | A02G
SN74AHC1G02DBVT | | -40 to 125 | A023, A02G, A02J, A02S | Obsolete
SN74AHC1G02DCKR | Reel of 3000 | -40 to 125 | 1R5, AB3, ABG, ABJ, ABL, ABS
SN74AHC1G02DCKRG4 | Reel of 3000 | -40 to 125 | AB3
SN74AHC1G02DCKT | | -40 to 125 | AB3, ABG, ABJ, ABS | Obsolete
SN74AHC1G02DRLR | Reel of 4000 | -40 to 125 | ABB, ABS
```

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.13–14). The support and revision history page
  (11), the mechanical information page (12), tape and reel and
  mechanical pages (15–26) are left out; the generated Package section
  replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the
  simplified schematic and functional block diagram (one NOR gate); the
  typical application schematic (Figure 8-1) and the layout diagram
  (Figure 8-3) are described in text.
- Dropped: Figure 8-2 (application curves, printed without a title: an
  oscilloscope-style plot of V~I~ and the outputs of three logic
  families); it is not a characteristic of this part alone.
- Section 5.3, named in absolute maximum ratings note 1, is the
  source's Recommended Operating Conditions.
- The recommended operating conditions table of the source runs over
  pp.4–5 and repeats its heading and note marker on p.5; it is one table
  here.
- The addendum also lists SN74AHC1G02DBVTG4.A, which the catalog doesn't
  carry.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for Figure 5-1 and no load
  for Figure 5-2, whose vertical axis is printed "Y Axis Title (Unit)"
  and whose horizontal axis "Vcc" has no unit.
- From images: the overbars of Y = !{A} × !{B} and Y = !{A + B} (p.1 and
  p.8: a bar over A, a bar over B, and one bar over the whole sum); the
  curve points -55, 2.8, 3.6, 4.3 (Figure 5-1, p.6) and 4.1, 4.5, 3.9,
  3.3 (Figure 5-2, p.6).
- Source issues:
  - Source error: the Input voltage row of the recommended operating
    conditions is printed with the symbol V~IH~ for V~I~; kept as
    printed (ROC table note 2).
  - Source error: the V~OL~ rows are printed with I~OH~ = 50 µA;
    corrected to I~OL~ (electrical characteristics note 1).
  - Source conflict: low drive and slow edge rates (§ 7.3, § 8.1) vs
    "high drive ... fast edges" (§ 8.2.1) (Application information
    note).
