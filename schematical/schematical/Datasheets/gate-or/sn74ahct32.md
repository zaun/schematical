---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct32",
  "title": "SNx4AHCT32 quadruple 2-input positive-OR gates",
  "class": "logic",
  "source": {
    "pdf": "gate-or/sn74ahct32.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS248O",
    "date": "2023-07",
    "pages": 39
  },
  "devices": ["gate:74_32"],
  "parts": ["5962-9682601QCA", "5962-9682601QDA", "SN74AHCT32BQAR", "SN74AHCT32D", "SN74AHCT32DBR", "SN74AHCT32DGVR", "SN74AHCT32DR", "SN74AHCT32N", "SN74AHCT32NSR", "SN74AHCT32PW", "SN74AHCT32PWR", "SN74AHCT32RGYR", "SNJ54AHCT32J", "SNJ54AHCT32W", "5962-9682601Q2A", "SNJ54AHCT32FK"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA, J, W", "lccc20": "FK"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT32 quadruple 2-input positive-OR gates

## Description

The SNx4AHCT32 devices are quadruple 2-input positive-OR gates. These
devices perform the Boolean function Y = A + B in positive logic. [[1::1]]

Source error: the source prints "Y = !{A × B} or Y = A + B" (one bar
over A × B); the function table (Table 8-1) gives Y = L only when A and
B are both L, so !{A × B} (a NAND) is kept out.

Inputs are TTL-voltage compatible. Latch-up performance exceeds 250 mA
per JESD 17. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters. [[1::2]]

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
3Y: 3Y Output
3A: 3A Input
3B: 3B Input
4Y: 4Y Output
4A: 4A Input
4B: 4B Input
GND: Ground Pin
NC: No Connection
VCC: Power Pin
```
[[3::Table 5-1]]

Conversion note: the RGY and BQA packages also have a thermal pad.

## Functional description

The SNx4AHCT32 is a quadruple 2-input positive-OR gate with low drive
that will produce slow rise and fall times. This slow transition
reduces ringing on the output signal. The device has TTL inputs that
allow up translation from 3.3 V to 5 V. The inputs are high impedance
when V~CC~ = 0 V. [[8::1]]

Conversion note: the functional block diagram shows four OR gates, each with inputs A and B and output Y.

- Slow rise and fall time on outputs allows for low-noise outputs [[8::2]]
- TTL inputs allow up translation from 3.3 V to 5 V [[8::1]]

## Function tables

```table id=function caption="Function table (each gate)"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| H | X | H |
| X | H | H |
| L | L | L |
```
[[8::Table 8-1]]

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
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM) | ±1000 | V |
| ^ | ^ | Charged device model (CDM) | ±750 | V |
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | SN54AHCT32 | < | SN74AHCT32 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs (SCBA004).
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHCT32, 14 pins"
| Symbol | Thermal metric [1] | D | DB | DGV | N | NS | PW | RGY | BQA | Unit |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 97.5 | 109.5 | 133.3 | 59.7 | 92.2 | 147.7 | 59.0 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 58.7 | 62.1 | 55.6 | 47.3 | 49.8 | 77.4 | 72.5 | 90.9 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 51.8 | 56.9 | 66.3 | 39.5 | 51.0 | 90.9 | 35.0 | 56.8 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 22.6 | 22.6 | 7.8 | 32.4 | 15.7 | 27.2 | 3.9 | 9.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 51.6 | 56.3 | 56.6 | 39.4 | 50.6 | 90.2 | 35.1 | 56.7 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | | | | | | | 15.4 | 33.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT32 | < | SN74AHCT32 | < | –40°C to 125°C, SN74AHCT32 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | | ±1 | µA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | | 1.5 | mA |
| C~i~ | | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```
[[5::table #2]]

```table id=noise caption="Noise characteristics, SN74AHCT32" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.4 | 0.8 | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.4 | -0.8 | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.5 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```
[[6::table #1]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 11.5 | pF |
```
[[6::table #2]]

## Switching characteristics

```table id=sw caption="Switching characteristics" conditions="Over recommended operating free-air temperature range, V~CC~ = 5 V ± 0.5 V (unless otherwise noted) (see Figure 7-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | SN54AHCT32 | < | SN74AHCT32 | < | –40°C to 125°C, SN74AHCT32 | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 15 pF | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | 1 | 9 | ns |
| t~PHL~ | ^ | ^ | ^ | 5 [1] | 6.9 [1] | 1 [1] | 8 [1] | 1 | 8 | 1 | 9 | ns |
| t~PLH~ | A or B | Y | C~L~ = 50 pF | 5.5 | 7.9 | 1 | 9 | 1 | 9 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 | 7.9 | 1 | 9 | 1 | 9 | 1 | 10 | ns |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```
[[5::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.37], [-40, 4.42], [25, 5], [85, 5.45], [125, 5.7]]}
 ]}
```
[[6::Figure 6-1]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::1]]

Conversion note: C~L~ is 15 pF or 50 pF, as given in the [switching characteristics](#sw); in the figure the inputs swing 0 V to 3 V and are measured at 1.5 V, and the outputs are measured at 50% V~CC~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 15 pF or 50 pF
```
[[7::Figure 7-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```
[[7::Figure 7-1]]

Conversion note: the source figure (Figure 7-1) also shows the load circuit for 3-state and open-drain outputs with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

The SNx4AHCT32 is a low-drive CMOS device that can be used for a
multitude of bus-interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The TTL inputs can accept voltages down to
3.3 V and can translate up to 5 V. [[9::2]]

Conversion note: in the typical application diagram for a single gate (Figure 9-1) a 3.3- or 5-V Bus Driver drives the gate inputs, the gate runs from a 5-V Regulated V~CC~ bypassed with 0.1 µF, and its output drives a 5-V Accessory.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[9::4]]

### Detailed design procedure

1. Recommended input conditions: [[9::5]]
   - For rise time and fall time specifications, see Δt/ΔV in the Recommended Operating Conditions table. [[9::5]]
   - For specified high and low levels, see V~IH~ and V~IL~ in the Recommended Operating Conditions table. [[9::5]]
   - Inputs are overvoltage tolerant allowing them to go as high as 5.5 V at any valid V~CC~. [[9::5]]
2. Recommend output conditions: [[9::5]]
   - Load currents should not exceed 25 mA per output and 50 mA total for the part. [[9::5]]
   - Outputs should not be pulled above V~CC~. [[9::5]]

Conversion note: the application curve (Figure 9-2, Switching Characteristics Comparison, a picture) shows an input edge V~I~ and the falling output edges of ACT32, HCT32 and AHCT32 gates at V~CC~ = 5 V, Load = 50 Ω/50 pF, and is not drawn here.

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX
supply-voltage rating located in Recommended Operating Conditions. [[10::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended.
If there are multiple V~CC~ pins then a 0.01 μF or a 0.022 μF is
recommended for each power pin. It is acceptable to parallel multiple
bypass caps to reject different frequencies of noise. A 0.1 μF and a
1 μF are commonly used in parallel. The bypass capacitor should be
installed as close to the power pin as possible for best results. [[10::2]]

### Layout guidelines

When using multiple bit logic devices, inputs should never float. [[10::3]]

In many cases, functions or parts of functions of digital logic devices
are unused, for example, when only two inputs of a triple-input AND gate
are used or only 3 of the 4 buffer gates are used. Such input pins
should not be left unconnected because the undefined voltages at the
outside connections result in undefined operational states. Figure 9-3
shows the rules that must be observed under all circumstances. All
unused inputs of digital logic devices must be connected to a high or
low bias to prevent them from floating. The logic level that should be
applied to any particular unused input depends on the function of the
device. Generally they will be tied to GND or V~CC~, whichever makes
more sense or is more convenient. It is generally acceptable to float
outputs, unless the part is a transceiver. If the transceiver has an
output enable pin it will disable the outputs section of the part when
asserted. This will not disable the input section of the I/Os so they
also cannot float when disabled. [[10::4]]

Conversion note: Figure 9-3, the layout diagram, shows one gate with its unused input tied to V~CC~ and one with its unused input tied to ground.

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

```package footprint=CDIP-14
```

```package footprint=CFP-14
```

```package footprint=LCCC-20
```

## Ordering information

```ordering
5962-9682601Q2A | Tube of 55 | -55 to 125 | 5962-9682601Q2A SNJ54AHCT32FK
5962-9682601QCA | Tube of 25 | -55 to 125 | 5962-9682601QCA SNJ54AHCT32J
5962-9682601QDA | Tube of 25 | -55 to 125 | 5962-9682601QDA SNJ54AHCT32W
SN74AHCT32BQAR | Reel of 3000 | -40 to 125 | AHCT32
SN74AHCT32D | | -40 to 85 | AHCT32 | Obsolete
SN74AHCT32DBR | Reel of 2000 | -40 to 125 | HB32
SN74AHCT32DGVR | Reel of 2000 | -40 to 125 | HB32
SN74AHCT32DR | Reel of 2500 | -40 to 125 | AHCT32
SN74AHCT32N | Tube of 25 | -40 to 125 | SN74AHCT32N
SN74AHCT32NSR | Reel of 2000 | -40 to 125 | AHCT32
SN74AHCT32PW | | -40 to 125 | HB32 | Obsolete
SN74AHCT32PWR | Reel of 2000 | -40 to 125 | HB32
SN74AHCT32RGYR | Reel of 3000 | -40 to 125 | HB32
SNJ54AHCT32FK | Tube of 55 | -55 to 125 | 5962-9682601Q2A SNJ54AHCT32FK
SNJ54AHCT32J | Tube of 25 | -55 to 125 | 5962-9682601QCA SNJ54AHCT32J
SNJ54AHCT32W | Tube of 25 | -55 to 125 | 5962-9682601QDA SNJ54AHCT32W
```
[[12::table #1]]

## Conversion notes

- Source pages 1–10 converted. The ordering lines are from the package
  option addendum (pp.12–13). The support page (11), the addendum's
  last page (14), tape and reel and mechanical pages (15–39) are left out; the generated Package section
  replaces them.
- Dropped: features and applications lists (features kept as sentences
  in the Description), package information table, revision history
  (p.2); the simplified schematic and functional block diagram (pp.1
  and 8), the typical application diagram (Figure 9-1), the switching
  characteristics comparison (Figure 9-2) and the layout diagram
  (Figure 9-3) are described in text.
- The pin table's note "I = input, O = output" is on its TYPE column,
  which the generated pin table draws from the catalog.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty.
- The thermal table prints a dash for R~θJC(bot)~ of the D, DB, DGV, N,
  NS and PW packages; those cells are empty.
- The curve is digitized from Figure 6-1 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for it.
- From images: the overbar of !{A × B} (Description, p.1); the curve
  points -55, -40, 25, 85, 125 and 4.37, 4.42, 5, 5.45, 5.7 (Figure 6-1,
  p.6); V~I~ and "Load = 50 Ω/50 pF" (Figure 9-2, p.10, a picture).
- Source issues:
  - Source error: Description formula "Y = !{A × B} or Y = A + B"
    (Description note).
  - Source conflict: the Features (p.1) say ESD protection exceeds
    JESD 22 with a 2000-V Human-Body Model; the ESD ratings (p.4) give
    ±1000 V for the human body model.
  - Source conflict: the Application Information (p.9) says low drive
    and slow edge rates, the Design Requirements (p.9) say balanced
    output drive whose high drive will create fast edges into light
    loads; both are kept.
