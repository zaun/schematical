---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc04-q1",
  "title": "SN74AHC04-Q1 automotive hex inverter",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc04-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS536D",
    "date": "2023-06",
    "pages": 25
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74AHC04QDRG4Q1", "SN74AHC04QDRQ1", "SN74AHC04QPWRG4Q1"],
  "packages": {"p14": "D, PW"},
  "converted": "2026-11-21"
}
---

# SN74AHC04-Q1 automotive hex inverter

## Description

The SN74AHC04-Q1 contains six independent inverters. This device
performs the Boolean function Y = !{A}.

Qualified for automotive applications. EPIC™ (Enhanced-Performance
Implanted CMOS) Process. Operating range of 2-V to 5.5-V V~CC~.

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

## Functional description

Conversion note: the functional block diagram (Figure 8-1) is the logic
diagram of one inverter, input A, output Y.

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ [1] [2] | Input voltage range | | -0.5 | 7 | V |
| V~O~ [1] [2] | Output voltage range | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[2] Source error: the source prints the marker 1 on the line over the table and the marker 2 on V~I~ and V~O~ (p.4), and only one note, (1), which is on the input and output voltage ratings; the note is given to V~I~ and V~O~ here.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM) [1] | ±1500 | V |

[1] AEC Q100-002 indicate that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over recommended operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.1 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 3.85 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.9 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.65 | ^ |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| Δt/Δv | Input transition rise or fall rate | V~CC~ = 3.3 V ± 0.3 V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 20 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, literature number SCBA004.
```

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC04-Q1"
| Symbol | Thermal metric [1] | D (SOIC) 14 pins | PW (TSSOP) 14 pins | BQA (WQFN) 14 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 86 | 147.7 | 88.3 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = −50 μA | 2 V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | ^ |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | ^ |
| ^ | ^ | I~OH~ = −4 mA | 3 V | 2.58 | | | 2.48 | | ^ |
| ^ | ^ | I~OH~ = −8 mA | 4.5 V | 3.94 | | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 μA | 2 V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | 0.1 | ^ |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | 0.1 | ^ |
| ^ | ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.5 | ^ |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.5 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | μA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | pF |
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.8 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 3.5 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 1.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3 V ± 0.3 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5 | 8.9 | 1 | 10.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 8.9 | 1 | 10.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.5 | 11.4 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.5 | 11.4 | 1 | 13 | ^ |
```

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.8 | 5.5 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.8 | 5.5 | 1 | 6.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.3 | 7.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.3 | 7.5 | 1 | 8.5 | ^ |
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied
by generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement.

```figure ref=load-cmos-50pf caption="Figure 7-1. Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Figure 7-1. Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% V~CC~
```

Conversion note: the source figure (Figure 7-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms with their
note B, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

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

When using multiple bit logic devices inputs should not ever float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Specified in
Layout Diagram are the rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~; whichever makes more sense or is more convenient. It is generally
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the IOs, so they cannot float when disabled.

Conversion note: the layout diagram (Figure 9-1) shows a gate with its
unused input tied to V~CC~ and a gate with its unused input tied to
ground.

## Package

```package footprint=SOIC-14
```

```package footprint=TSSOP-14
```

## Ordering information

```ordering
SN74AHC04QDRG4Q1 | Reel of 2500 | -40 to 125 | AHC04Q1 | NRND
SN74AHC04QDRQ1 | Reel of 2500 | -40 to 125 | AHC04Q1
SN74AHC04QPWRG4Q1 | Reel of 2000 | -40 to 125 | AHC04Q1
```

## Conversion notes

- Source pages 1–9 converted; p.2 holds only the table of contents and
  the revision history, which are dropped. The ordering lines are from
  the package option addendum (pp.11–12). The support page (10), tape
  and reel and mechanical pages (13–25) are left out; the generated
  Package section replaces them.
- Dropped: applications list, package information table, the logic
  diagram (one inverter), the layout diagram.
- The feature list is written as sentences in the Description.
- The source's description (p.1) names the device "SN74AHC04"; the data
  sheet's title is SN74AHC04-Q1, and the name is written with the suffix
  here.
- The pin drawing (p.3) is for the D or PW package; the source has no
  drawing of the BQA package. The pin table's Thermal Pad row ("BQA
  Package only. The thermal pad can be connected to GND or left
  floating. Do not connect to any other signal or supply") is left out:
  the catalog does not carry the BQA part, SN74AHC04QWBQARQ1.
- The absolute maximum ratings of the source print the conditions in
  parentheses; they are put in the Conditions cells.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. The electrical and switching tables
  print no heading over their last MIN and MAX columns.
- Figure 7-1 (p.7) is a picture; only its notes and S1 table are in the
  text layer.
- From images: 50% V~CC~ (V~M~, Figure 7-1, p.7); overbar of Y = !{A}
  (Description, p.1).
- Source issues:
  - Source error: marker 1 printed on the line over the absolute maximum
    ratings and marker 2 on V~I~ and V~O~, with one note, (1), under the
    table (table note 2).
