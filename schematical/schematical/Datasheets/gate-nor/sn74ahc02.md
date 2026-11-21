---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc02",
  "title": "SNx4AHC02 quadruple 2-input positive-NOR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-nor/sn74ahc02.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS254N",
    "date": "2024-02",
    "pages": 40
  },
  "devices": ["gate:74_02"],
  "parts": ["5962-9752801QCA", "5962-9752801QDA", "SN74AHC02BQAR", "SN74AHC02D", "SN74AHC02DBR", "SN74AHC02DGVR", "SN74AHC02DR", "SN74AHC02N", "SN74AHC02NSR", "SN74AHC02PW", "SN74AHC02PWR", "SN74AHC02RGYR", "SNJ54AHC02J", "SNJ54AHC02W", "5962-9752801Q2A", "SNJ54AHC02FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHC02 quadruple 2-input positive-NOR gates

## Description

The ’AHC02 devices contain four independent 2-input NOR gates that
perform the Boolean function Y = !{A} ● !{B} or Y = !{A + B} in positive
logic.

Operating range 2V to 5.5V. Latch-up performance exceeds 250mA per
JESD 17.

## Pinout

```pinout pinout=p14
```

```pinout pinout=lccc20
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
NC: No Connection
VCC: Power Pin
```

## Functional description

Conversion note: the functional block diagram shows the four 2-input NOR gates with the pin numbers of the 14-pin packages.

## Function tables

```table id=function caption="Function table"
| Inputs [1] | < | Output Y |
| A | B | ^ |
|:-:|:-:|:-:|
| H | X | L |
| X | H | L |
| L | L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don't Care
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under “absolute maximum ratings” may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under “recommended operating conditions” is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM) [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM) [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions"
| Symbol | Parameter | Conditions | SN54AHC02 | < | SN74AHC02 | < | Unit |
| ^ | ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | 1.5 | | V |
| ^ | ^ | V~CC~ = 3V | 2.1 | | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | 3.85 | | V |
| V~IL~ | Low-level Input voltage | V~CC~ = 2 V | | 0.5 | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V [1] | | -50 | | -50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V [1] | | 50 | | 50 | mA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | | 8 | mA |
| Δt/Δv | Input Transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -55 | 125 | -40 | 125 | °C |

[1] Unconfirmed: the source prints the unit mA for the V~CC~ = 2 V rows; the absolute maximum continuous output current is ±25 mA and the electrical characteristics test at I~OH~ = –50 µA and I~OL~ = 50 µA.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC02: thermal metric [1]"
| Symbol | Parameter | D 14 pins | DB 14 pins | DGV 14 pins | N 14 pins | NS 14 pins | PW 14 pins | RGY 14 pins | BQA 14 pins | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.6 | 96 | 127 | 80 | 76 | 147.7 | 47 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, SPRA953.
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –55°C to 125°C, SN54AHC02 | < | T~A~ = –40°C to 85°C, SN74AHC02 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC02 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | 1.9 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | 2.9 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | 2.48 | | V |
| ^ | ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 2 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA [2] | 3 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | V |
| ^ | ^ | I~OL~ = 8 mA [2] | 4.5 V | | | 0.36 | | 0.5 | | 0.44 | | 0.5 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | | 20 | µA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | 10 | | | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at VCC = 0 V.
[2] Source error: printed as I~OH~ = 4 mA and I~OH~ = 8 mA; the rows give V~OL~, and the recommended operating conditions give I~OL~ = 4 mA and 8 mA, so the condition is I~OL~.
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74AHC02 | < | Unit |
| ^ | ^ | Min | Max | ^ |
|---|---|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | 4.9 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 15 | pF |
```

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC02 | < | T~A~ = –40°C to 85°C, SN74AHC02 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC02 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 5.6 [1] | 7.9 [1] | 1 [1] | 9.5 [1] | 1 | 9.5 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.6 [1] | 7.9 [1] | 1 [1] | 9.5 [1] | 1 | 9.5 | 1 | 9.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 8.1 | 11.4 | 1 | 13 | 1 | 13 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | 8.1 | 11.4 | 1 | 13 | 1 | 13 | 1 | 13 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | T~A~ = –55°C to 125°C, SN54AHC02 | < | T~A~ = –40°C to 85°C, SN74AHC02 | < | T~A~ = –40°C to 125°C Recommended, SN74AHC02 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 3.6 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 3.6 [1] | 5.5 [1] | 1 [1] | 6.5 [1] | 1 | 6.5 | 1 | 6.5 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 5.1 | | 1 | 8.5 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.1 | | 1 | 8.5 | 1 | 8.5 | 1 | 8.5 | ns |

[1] Source conflict: the 5-V table (p.6) marks these values with (1) and prints no note under the table; the 3.3-V table (p.5) prints note (1): On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```

Conversion note: the source figure (Figure 5-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX
supply-voltage rating located in Section 4.3. The power supply can be
any voltage between the minimum and maximum supply voltage rating
located in the Recommended Operating Conditions. Each V~CC~ terminal
should have a good bypass capacitor to prevent power disturbance. A
0.1-μF capacitor is recommended for this device. It is acceptable to
parallel multiple bypass capacitors to reject different frequencies of
noise. The 0.1-μF and 1-μF capacitors are commonly used in parallel. The
bypass capacitor should be installed as close to the power terminal as
possible for best results, as shown in Layout Example.

### Layout guidelines

When using multiple-input and multiple-channel logic devices, inputs
must never be left floating. In many cases, functions or parts of
functions of digital logic devices are unused (for example, when only
two inputs of a triple-input AND gate are used or only 3 of the 4 buffer
gates are used). Such unused input pins must not be left unconnected
because the undefined voltages at the outside connections result in
undefined operational states. All unused inputs of digital logic devices
must be connected to a logic high or logic low voltage, as defined by
the input voltage specifications, to prevent them from floating. The
logic level that must be applied to any particular unused input depends
on the function of the device. Generally, the inputs are tied to GND or
V~CC~, whichever makes more sense for the logic function or is more
convenient.

Conversion note: the example layout (Figure 7-1) recommends a GND flood fill, a bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and no 90° corners for signal lines.

## Package

```package footprint=SOIC-14
```

```package footprint=SSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=PDIP-14
```

```package footprint=SOP-14
```

```package footprint=TSSOP-14
```

```package footprint=VQFN-14
```

```package footprint=WQFN-14
```

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9752801Q2A | Tube of 55 | -55 to 125 | 5962-9752801Q2A SNJ54AHC02FK
5962-9752801QCA | Tube of 25 | -55 to 125 | 5962-9752801QCA SNJ54AHC02J
5962-9752801QDA | Tube of 25 | -55 to 125 | 5962-9752801QDA SNJ54AHC02W
SN74AHC02BQAR | Reel of 3000 | -40 to 125 | AHC02
SN74AHC02D | | -40 to 125 | AHC02 | Obsolete
SN74AHC02DBR | Reel of 2000 | -40 to 125 | HA02
SN74AHC02DGVR | Reel of 2000 | -40 to 125 | HA02
SN74AHC02DR | Reel of 2500 | -40 to 125 | AHC02
SN74AHC02N | Tube of 25 | -40 to 125 | SN74AHC02N
SN74AHC02NSR | Reel of 2000 | -40 to 125 | AHC02 | NRND
SN74AHC02PW | | -40 to 125 | HA02 | Obsolete
SN74AHC02PWR | Reel of 2000 | -40 to 125 | HA02
SN74AHC02RGYR | Reel of 3000 | -40 to 125 | HA02 | Last Time Buy
SNJ54AHC02FK | Tube of 55 | -55 to 125 | 5962-9752801Q2A SNJ54AHC02FK
SNJ54AHC02J | Tube of 25 | -55 to 125 | 5962-9752801QCA SNJ54AHC02J
SNJ54AHC02W | Tube of 25 | -55 to 125 | 5962-9752801QDA SNJ54AHC02W
```

## Conversion notes

- Source pages 1–9 converted. The ordering lines are from the package
  option addendum (pp.12–14). The support and revision history pages
  (10–11), tape and reel and mechanical pages (15–40) are left out; the
  generated Package section replaces them.
- Dropped: features list (the features are kept as sentences in the
  Description), device information table, the logic diagram and
  functional block diagram (four NOR gates); the example layout
  (Figure 7-1) is described in text.
- The absolute maximum ratings of the source print the parameter name
  and condition of each line in one cell ("Input clamp current
  (V~I~ < 0)"); they are put in separate cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The 5-V switching table (p.6) prints no 25°C maximum for
  C~L~ = 50 pF; the cells are empty as in the source.
- Figure 5-1 (p.7) is a picture; only its notes are in the text layer.
- From images: 50% V~CC~ (V~M~, Figure 5-1, p.7); overbars of
  Y = !{A} ● !{B} and Y = !{A + B} (Description, p.1).
- Source issues:
  - Source error: the pin table (Table 3-1, p.3) prints pin 20 for 3Y in
    the J, W column; the pin drawing (Figure 3-1) shows 3Y on pin 10.
  - Source error: V~OL~ rows printed with I~OH~ = 4 mA / 8 mA; corrected
    to I~OL~ (electrical characteristics note 2).
  - Source conflict: the 5-V switching table marks values with (1) and
    prints no note; the 3.3-V table prints it (switching table note).
  - Unconfirmed: I~OH~/I~OL~ at V~CC~ = 2 V printed as ±50 mA (ROC table
    note).
