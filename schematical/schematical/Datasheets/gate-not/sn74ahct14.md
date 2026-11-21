---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct14",
  "title": "SNx4AHCT14 hex Schmitt-trigger inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS246T",
    "date": "2023-10",
    "pages": 42
  },
  "devices": ["gate:74_14"],
  "parts": ["SN74AHCT14D", "SN74AHCT14DBR", "SN74AHCT14DGVR", "SN74AHCT14DR", "SN74AHCT14N", "SN74AHCT14NSR", "SN74AHCT14PW", "SN74AHCT14RGYR"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT14 hex Schmitt-trigger inverters

## Description

The SNx4AHCT14 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

Inputs are TTL-voltage compatible. Latch-up performance exceeds 250 mA
per JESD 17. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A, 2A, 3A, 4A, 5A, 6A: Input
1Y, 2Y, 3Y, 4Y, 5Y, 6Y: Output
GND: Ground pin
VCC: Power pin
```

Source error: the DESCRIPTION column of the source's pin table (p.3) prints only "1A1", "1Y1" to "6A1", "6Y1" for the inputs and outputs; its TYPE column gives I = input for the A pins and O = output for the Y pins, as described here.

## Functional description

The SNx4AHCT14 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}. Each circuit functions as an
independent inverter, but because of the Schmitt action, the inverters
have different input threshold levels for positive-going (V~T+~) and for
negative-going (V~T−~) signals.

Conversion note: the functional block diagram shows one Schmitt-trigger inverter, input A, output Y.

### Feature description

- Inputs are TTL-Voltage compatible
- Inputs accept very slow or noisy inputs

## Function tables

```table id=function caption="Function table (each inverter)"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
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
| | Continuous current through V~CC~ or GND | | | ±50 | mA |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=handling caption="Handling ratings"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| T~stg~ | Storage temperature range | | -65 | 150 | °C |
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | 0 | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | 0 | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | SN54AHCT14 | < | SN74AHCT14 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Application Report, Implications of Slow or Floating CMOS Inputs, (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT14 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | D 14 pins | DGV 14 pins | DB 14 pins | N 14 pins | NS 14 pins | PW 14 pins | RGY 14 pins | BQA 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 138.7 | 113.1 | 61.1 | 98.6 | 147.7 | 63.7 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.8 | 60.6 | 65.6 | 48.0 | 54.1 | 77.4 | 77.6 | 90.9 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 81 | 71.8 | 60.4 | 41.0 | 57.4 | 90.9 | 39.7 | 56.8 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 37 | 10.6 | 25.5 | 32.4 | 19.6 | 27.2 | 5.7 | 9.9 | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 80.6 | 71.1 | 59.9 | 40.9 | 57.0 | 90.2 | 39.9 | 56.7 | ^ |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | N/A | 19.9 | 33.4 | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT14 | < | SN74AHCT14 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 4.5 V | 0.9 | | 1.9 | 0.9 | 1.9 | 0.9 | 1.9 | V |
| ^ | ^ | | 5.5 V | 1 | | 2.1 | 1 | 2.1 | 1 | 2.1 | ^ |
| V~T–~ | Negative-going input threshold voltage | | 4.5 V | 0.5 | | 1.5 | 0.5 | 1.5 | 0.5 | 1.5 | V |
| ^ | ^ | | 5.5 V | 0.6 | | 1.7 | 0.6 | 1.7 | 0.6 | 1.7 | ^ |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 4.5 V | 0.4 | | 1.4 | 0.4 | 1.4 | 0.4 | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.4 | | 1.5 | 0.4 | 1.5 | 0.4 | 1.5 | ^ |
| V~OH~ | | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | | I~OH~ = –8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74AHCT14 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.9 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.7 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.3 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2.1 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.5 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHCT14 | < | SN74AHCT14 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4 [1] | 7 [1] | 1 [1] | 8 [1] | 1 | 8 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4 [1] | 7 [1] | 1 [1] | 8 [1] | 1 | 8 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.5 | 8 | 1 | 9 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 8 | 1 | 9 | 1 | 9 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

One common misconception is that the current consumption will be less
when switching a slow signal into a Schmitt trigger. This is partly true
because the Schmitt trigger prevents oscillation which can draw a lot of
current; however, you will see higher I~CC~ current due to the amount of
time the input is not at the rail. This is Delta I~CC~. Delta I~CC~ is
where the inputs are not at the rails and upper or lower drive
transistors are partially on. Figure 6-1 shows I~CC~ across the input
voltage sweep.

```curve id=icc-vi caption="Supply current vs input voltage"
{"x": {"label": "V_I - Input Voltage", "unit": "V", "range": [0, 5]},
 "y": {"label": "I_CC - Supply Current", "unit": "mA", "range": [0, 2.5]},
 "series": [
  {"label": "Trace A", "points": [[0, 0], [0.63, 0], [1, 0.15], [1.3, 0.55], [1.47, 1], [1.5, 0.5], [2, 0.16], [2.5, 0.03], [2.85, 0], [5, 0]]},
  {"label": "Trace B", "points": [[0, 0], [0.63, 0], [1, 0.12], [1.5, 0.59], [2, 1.27], [2.5, 2.15], [2.57, 1.75], [3, 0.84], [3.5, 0.31], [4, 0], [5, 0]]}
 ]}
```

Conversion note: the source does not label the two traces of Figure 6-1 or give the supply voltage; "Trace A" and "Trace B" are names given here.

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices.

Conversion note: in Figure 7-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```

Conversion note: the source figure (Figure 7-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

Schmitt triggers should be used anytime you need to translate a sign
wave into a square wave, or used where a slow or noisy input needs to be
sped up or cleaned up as in the switch de-bouncer circuit.

Conversion note: the typical application (Figure 9-1, Switch De-Bouncer Using Schmitt Trigger Inverter) shows a 10 kΩ, 1/6 W resistor from 5 V to the input of one SN74AHCT14 inverter, and a switch and a 10 nf capacitor from that input to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads; therefore, routing and load conditions should be
considered to prevent ringing.

### Detailed design procedure

1. Recommended input conditions:
   - Specified High and low levels: See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table.
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~
2. Recommended output conditions:
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part
   - Outputs should not be pulled above V~CC~

Source conflict: step 1 (p.10) points to V~IH~ and V~IL~ in the Recommended Operating Conditions table; that table (p.4) has no V~IH~ or V~IL~ row.

Conversion note: the typical application curves (Figure 9-2) are three sketches of an input IN and the squared output OUT, lettered "Schmitt triggers should be used any time you need to 1. Change a sign wave into a square wave. 2. Have noisy signals that need to be cleaned up 3. Have slow edges that need to be converted to fast edges."

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table.

Each VCC pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μf is recommended. If
there are multiple VCC pins, 0.01 μf or 0.022 μf is recommended for each
power pin. It is acceptable to parallel multiple bypass caps to reject
different frequencies of noise. A 0.1 μf and 1 μf are commonly used in
parallel. The bypass capacitor should be installed as close to the power
pin as possible for best results.

### Layout guidelines

When using multiple bit logic devices inputs should not ever float.

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Figure 9-3
shows the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or VCC; whichever makes more
sense or is more convenient. It is generally acceptable to float outputs
unless the part is a transceiver. If the transceiver has an output
enable pin, it will disable the outputs section of the part when
asserted. This will not disable the input section of the IO’s so they
cannot float when disabled.

Conversion note: the layout diagram (Figure 9-3) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=PDIP-14
```

```package footprint=SOIC-14
```

```package footprint=SOP-14
```

```package footprint=SSOP-14
```

```package footprint=TSSOP-14
```

```package footprint=TVSOP-14
```

```package footprint=VQFN-14
```

## Ordering information

```ordering
SN74AHCT14D | | -40 to 125 | AHCT14 | Obsolete
SN74AHCT14DBR | Reel of 2000 | -40 to 125 | HB14
SN74AHCT14DGVR | Reel of 2000 | -40 to 85 | HB14 | NRND
SN74AHCT14DR | Reel of 2500 | -40 to 125 | AHCT14
SN74AHCT14N | Tube of 25 | -40 to 125 | SN74AHCT14N
SN74AHCT14NSR | Reel of 2000 | -40 to 125 | AHCT14 | Last Time Buy
SN74AHCT14PW | | -40 to 125 | HB14 | Obsolete
SN74AHCT14RGYR | Reel of 3000 | -40 to 125 | HB14 | NRND
```

## Conversion notes

- Source pages 1–12 converted; p.2 holds only the table of contents and
  revision history, which are dropped. The ordering lines are from the
  package option addendum (pp.14–17). The support page (13), tape and
  reel and mechanical pages (18–42) are left out; the generated Package
  section replaces them.
- Dropped: applications list and the ESD feature lines (the other
  features are kept as sentences in the Description), device information
  table, the simplified schematic (p.1, one Schmitt-trigger inverter);
  the functional block diagram (p.9), the typical application
  (Figure 9-1, p.10), the typical application curves (Figure 9-2, p.11)
  and the layout diagram (Figure 9-3, p.12) are described in text.
- The source also covers the SN54AHCT14 (J, W and FK packages); its
  parts in the addendum (5962-9680101Q2A, 5962-9680101QCA,
  5962-9680101QDA, 5962-9680101VCA, 5962-9680101VDA, SNJ54AHCT14FK,
  SNJ54AHCT14J, SNJ54AHCT14W) are not claimed by this file, so the FK
  pinout is left out; the SN54AHCT14 columns of the tables are kept.
  The addendum also lists SN74AHCT14BQAR, SN74AHCT14DGVRE4 and
  SN74AHCT14PWR, which are not catalog parts of this datasheet.
- The pin table's note "I = input, O = output" is on its TYPE column,
  which the generated pin table draws from the catalog.
- The absolute maximum ratings of the source print the conditions in an
  unnamed column; they are put in the Conditions cells. The source's
  Handling Ratings table (storage temperature and ESD, with MIN and MAX
  columns) is kept as one table.
- The electrical characteristics table of the source prints the name of
  V~T+~, V~T–~ and ΔV~T~ under the symbol in its PARAMETER column and
  only the symbol of the other rows, whose Parameter cells are empty.
- Note B of Figure 7-1 belongs to the enable and disable waveforms,
  which are not drawn, and is left out.
- The curve is digitized from Figure 6-1 (p.7), a picture, to about 2 %
  of full scale.
- From images: overbars of Y = !{A} (p.1 and Overview, p.9); 0 V, 3 V,
  1.5 V and 50% V~CC~ (Figure 7-1, p.8); the axis labels and the curve
  points 0.63, 0.15, 1.3, 0.55, 1.47, 0.16, 0.03, 2.85, 0.12, 0.59,
  1.27, 2.15, 2.57, 1.75, 0.84, 3.5, 0.31 and the others (Figure 6-1,
  p.7); the drawing of Figure 9-1 (p.10); the lettering "Schmitt
  triggers should be used any time you need to 1. Change a sign wave
  into a square wave. 2. Have noisy signals that need to be cleaned up
  3. Have slow edges that need to be converted to fast edges."
  (Figure 9-2, p.11).
- Source issues:
  - Source error: pin descriptions printed "1A1", "1Y1" and so on (Pin
    functions note).
  - Source conflict: V~IH~ and V~IL~ named in the detailed design
    procedure but absent from the Recommended Operating Conditions
    (Application information note).
  - Source error: "μf" and "nf" are printed for μF and nF (p.10 and
    p.11), and "sign wave" for sine wave (p.10); kept as printed.
  - Unconfirmed: Figure 6-1 (p.7) shows supply current peaks near input
    voltages of 1.5 V and 2.5 V, the second above the V~T+~ maximum of
    2.1 V in the electrical characteristics.
