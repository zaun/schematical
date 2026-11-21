---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahct04",
  "title": "SNx4AHCT04 hex inverters",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahct04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS232R",
    "date": "2024-02",
    "pages": 40
  },
  "devices": ["gate:74_04"],
  "parts": ["SN74AHCT04BQAR", "SN74AHCT04D", "SN74AHCT04DBR", "SN74AHCT04DGVR", "SN74AHCT04DR", "SN74AHCT04N", "SN74AHCT04NSR", "SN74AHCT04PW", "SN74AHCT04PWR", "SN74AHCT04RGYR"],
  "packages": {"p14": "D, DB, DGV, N, NS, PW, RGY, BQA"},
  "converted": "2026-11-21"
}
---

# SNx4AHCT04 hex inverters

## Description

The SNx4AHCT04 devices contain six independent inverters. These devices
perform the Boolean function Y = !{A}.

Inputs are TTL-voltage compatible. Latch-up performance exceeds 250mA
per JESD 17. On products compliant to MIL-PRF-38535, all parameters are
tested unless otherwise noted. On all other products, production
processing does not necessarily include testing of all parameters.

## Pinout

```pinout pinout=p14
```

## Pin functions

```pins
1A: 1A Input
1Y: 1Y Output
2A: 2A Input
2Y: 2Y Output
3A: 3A Input
3Y: 3Y Output
4A: 4A Input
4Y: 4Y Output
5A: 5A Input
5Y: 5Y Output
6A: 6A Input
6Y: 6Y Output
GND: Ground Pin
VCC: Power Pin
```

Source error: the pin table of the source (p.3) prints the type of 5Y as I; its description is "5Y Output" and the pin drawings on the same page have 5Y as the output of the fifth inverter.

## Functional description

The SNx4AHCT04 devices contain six independent inverters. These devices
have TTL input levels that allow up translation from 3.3 V to 5 V.

Conversion note: the functional block diagram shows one inverter, input A, output Y.

### Feature description

- V~CC~ is optimized at 5 V
- Allows up voltage translation from 3.3 V to 5 V
  - Inputs Accept V~IH~ levels of 2 V
- Slow edge rates minimize output ringing
- Inputs are TTL-voltage compatible

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
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, which do not imply functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±2000 | V |
| ^ | ^ | Charged device model (CDM), per ANSI/ESDA/JEDEC JS-002, all pins [2] | ±1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | SN54AHCT04 | < | SN74AHCT04 | < | Unit |
| ^ | ^ | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|---|
| V~CC~ | Supply voltage | 4.5 | 5.5 | 4.5 | 5.5 | V |
| V~IH~ | High-level input voltage | 2 | | 2 | | V |
| V~IL~ | Low-level input voltage | | 0.8 | 0 | 0.8 | V |
| V~I~ | Input voltage | 0 | 5.5 | 0 | 5.5 | V |
| V~O~ | Output voltage | 0 | V~CC~ | 0 | V~CC~ | V |
| I~OH~ | High-level output current | | -8 | | -8 | mA |
| I~OL~ | Low-level output current | | 8 | | 8 | mA |
| Δt/Δv | Input transition rise or fall rate | | 20 | | 20 | ns/V |
| T~A~ | Operating free-air temperature | -55 | 125 | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI Application Report, Implications of Slow or Floating CMOS Inputs, (SCBA004).
```

## Thermal information

```table id=thermal caption="Thermal information"
| Thermal metric [1] | < | SN74AHCT04 | < | < | < | < | < | < | < | Unit |
| ^ | ^ | D 14 pins | DB 14 pins | DGV 14 pins | N 14 pins | NS 14 pins | PW 14 pins | RGY 14 pins | BQA 14 pins | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 124.5 | 113.1 | 138.7 | 61.1 | 120.9 | 147.7 | 63.7 | 88.3 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 78.8 | 65.6 | 60.6 | 48.0 | 78.2 | 77.4 | 77.6 | 90.9 | ^ |
| R~θJB~ | Junction-to-board thermal resistance | 81 | 60.4 | 71.8 | 41.0 | 81.6 | 90.9 | 39.7 | 56.8 | ^ |
| ψ~JT~ | Junction-to-top characterization parameter | 37 | 25.5 | 10.6 | 32.4 | 42.8 | 27.2 | 5.7 | 9.9 | ^ |
| ψ~JB~ | Junction-to-board characterization parameter | 80.6 | 59.9 | 71.1 | 40.9 | 81.1 | 90.2 | 39.9 | 56.7 | ^ |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | N/A | N/A | N/A | N/A | N/A | 19.9 | 33.4 | ^ |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report, (SPRA953).
```

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over recommended operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | SN54AHCT04 | < | SN74AHCT04 | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 4.5 V | 4.4 | 4.5 | | 4.4 | | 4.4 | | V |
| ^ | I~OH~ = –8 mA | ^ | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 4.5 V | | | 0.1 | | 0.1 | | 0.1 | V |
| ^ | I~OL~ = 8 mA | ^ | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 [1] | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 2 | | 20 | | 20 | µA |
| ΔI~CC~ [2] | One input at 3.4 V, Other inputs at V~CC~ or GND | 5.5 V | | | 1.35 | | 1.5 | | 1.5 | mA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 4 | 10 | | | | 10 | pF |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested at V~CC~ = 0 V.
[2] This is the increase in supply current for each input at one of the specified TTL voltage levels, rather than 0 V or V~CC~.
```

```table id=noise caption="Noise characteristics" conditions="V~CC~ = 5 V, C~L~ = 50 pF, T~A~ = 25°C [1]"
| Symbol | Parameter | SN74AHCT04 | < | < | Unit |
| ^ | ^ | Min | Typ | Max | ^ |
|---|---|--:|--:|--:|---|
| V~OL(P)~ | Quiet output, maximum dynamic V~OL~ | | 0.8 | | V |
| V~OL(V)~ | Quiet output, minimum dynamic V~OL~ | | -0.8 | | V |
| V~OH(V)~ | Quiet output, minimum dynamic V~OH~ | | 4.7 | | V |
| V~IH(D)~ | High-level dynamic input voltage | 2 | | | V |
| V~IL(D)~ | Low-level dynamic input voltage | | | 0.8 | V |

[1] Characteristics are for surface-mount packages only.
```

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 14 | pF |
```

## Switching characteristics

```table id=sw caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 6-1)"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | SN54AHCT04 | < | SN74AHCT04 | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 4.7 [1] | 6.7 [1] | 1 [1] | 7.5 [1] | 1 | 7.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 4.7 [1] | 6.7 [1] | 1 [1] | 7.5 [1] | 1 | 7.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5.5 | 7.7 | 1 | 8.5 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.5 | 7.7 | 1 | 8.5 | 1 | 8.5 | ^ |

[1] On products compliant to MIL-PRF-38535, this parameter is not production tested.
```

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 4.4], [-40, 4.6], [25, 4.9], [85, 5.4], [125, 5.6]]}
 ]}
```

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices.

Conversion note: in Figure 6-1 the inputs swing 0 V to 3 V and are measured at 1.5 V, the outputs are measured at 50% V~CC~, and its table sets S1 to Open for t~PLH~/t~PHL~.

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 1.5 V (input), 50% VCC (output)
```

Conversion note: the source figure (Figure 6-1) also shows the load circuit for 3-state and open-drain outputs (R~L~ = 1 kΩ) with its S1 table, and the pulse-duration, setup-and-hold and enable/disable waveforms, which are not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality.

The SNx4AHCT04 is a low-drive CMOS device that can be used for a
multitude of inverting type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The input switching levels have been lowered
to accommodate TTL inputs of 0.8-V V~IL~ and 2-V V~IH~. This feature
makes it ideal for translating up from 3.3 V to 5 V. Figure 8-1 and
Figure 8-2 show this type of translation.

Conversion note: Figure 8-1 (Driving Relays) shows two inverters in parallel driving a relay coil that has a diode across it.

Conversion note: Figure 8-2 (Driving LEDs) shows two inverters in parallel driving an LED through a resistor R, a) to ground, where V~IL~ at the Input turns on the LED, and b) from V~CC~, where V~IH~ at the Input turns on the LED.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing.

Source conflict: the Application Information (p.9) and the Feature Description (p.8) speak of low drive and slow edge rates; the Design Requirements (p.9) speak of high drive and fast edges. Both are kept.

### Detailed design procedure

1. Recommended input conditions
   - Rise time and fall time specs: See (Δt/ΔV) in the Recommended
     Operating Conditions table.
   - Specified High and low levels: See (V~IH~ and V~IL~) in the
     Recommended Operating Conditions table.
2. Recommend output conditions
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part
   - Outputs should not be pulled above V~CC~

Conversion note: the application curve (Figure 8-3, Up Translation) is an oscilloscope picture of a falling input edge and the rising output edge, lettered "Translating from 3.3V to 5V"; it is not drawn here.

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
outside connections result in undefined operational states. […] shows the
rules that must be observed under all circumstances. All unused
inputs of digital logic devices must be connected to a high or low bias
to prevent them from floating. The logic level that should be applied to
any particular unused input depends on the function of the device.
Generally they will be tied to GND or V~CC~; whichever makes more sense
or is more convenient. It is generally acceptable to float outputs
unless the part is a transceiver. If the transceiver has an output
enable pin, it will disable the outputs section of the part when
asserted. This will not disable the input section of the IOs, so they
cannot float when disabled.

Source error: where this text has "[…]" the source prints "#none#" (p.10), a cross-reference left unresolved; kept out.

Conversion note: the example layout (Figure 8-4) recommends a GND flood fill for improved signal isolation, noise reduction, and thermal dissipation, a 0.1 µF bypass capacitor placed close to the device, unused inputs tied to V~CC~, an unused output left floating, and avoiding 90° corners for signal lines.

Source conflict: Figure 8-4 (p.10), the example layout for the SN74AHCT04, letters the pins 1A, 1B, 1Y to 4B, 4A, 4Y, as on a quadruple 2-input gate; the pin table (p.3) has six inverters, 1A, 1Y to 6A, 6Y.

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

```package footprint=WQFN-14
```

## Ordering information

```ordering
SN74AHCT04BQAR | Reel of 3000 | -40 to 125 | AHCT04
SN74AHCT04D | | -40 to 125 | AHCT04 | Obsolete
SN74AHCT04DBR | Reel of 2000 | -40 to 125 | HB04
SN74AHCT04DGVR | Reel of 2000 | -40 to 125 | HB04
SN74AHCT04DR | Reel of 2500 | -40 to 125 | AHCT04
SN74AHCT04N | Tube of 25 | -40 to 125 | SN74AHCT04N | NRND
SN74AHCT04NSR | Reel of 2000 | -40 to 125 | AHCT04
SN74AHCT04PW | | -40 to 125 | HB04 | Obsolete
SN74AHCT04PWR | Reel of 2000 | -40 to 125 | HB04
SN74AHCT04RGYR | Reel of 3000 | -40 to 125 | HB04 | Last Time Buy
```

## Conversion notes

- Source pages 1–10 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.13–15). The support and revision history page (11), the
  mechanical information page (12), tape and reel and mechanical pages
  (16–40) are left out; the generated Package section replaces them.
- Dropped: applications list (features kept as sentences in the
  Description), package information table, the simplified schematic
  (p.1, one inverter); the functional block diagram (p.8), Figures 8-1
  and 8-2 (p.9), Figure 8-3 and the example layout, Figure 8-4 (p.10),
  are described in text.
- The source also covers the SN54AHCT04 (J, W and FK packages), whose
  parts (5962-9680401Q2A, 5962-9680401QCA, 5962-9680401QDA,
  SNJ54AHCT04FK, SNJ54AHCT04J, SNJ54AHCT04W in the addendum) the catalog
  doesn't carry under this datasheet; the FK pinout and the SN54AHCT04
  pin columns are left out, the SN54AHCT04 columns of the tables are
  kept. The addendum also lists SN74AHCT04RGYRG4, which the catalog
  doesn't carry.
- The pin table lists a Thermal Pad (PAD) for the RGY and BQA packages,
  with no more than its name; the catalog pinout has no pad pin. The pin
  table's note "I = input, O = output" is on its TYPE column, which the
  generated pin table draws from the catalog.
- The absolute maximum ratings of the source print the conditions in an
  unnamed column; they are put in the Conditions cells.
- The electrical characteristics table of the source gives symbols only,
  so the file's table has no Parameter column.
- Note B of Figure 6-1 belongs to the enable and disable waveforms,
  which are not drawn, and is left out.
- The TPD curve is digitized from Figure 5-1 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for it.
- From images: overbar of Y = !{A} (p.1); 0 V, 3 V, 1.5 V and 50% V~CC~
  (Figure 6-1, p.7); the curve points -55, 4.4, 4.6, 25, 4.9, 85, 5.4,
  5.6 (Figure 5-1, p.6); the drawings of Figures 8-1 and 8-2 (p.9);
  "Translating from 3.3V to 5V" (Figure 8-3, p.10); 0.1 µF and the pin
  letters 1B, 2B, 3B, 4B (Figure 8-4, p.10; the text layer prints
  "0.1 F").
- Source issues:
  - Source error: 5Y printed with type I in the pin table (Pin functions
    note).
  - Source conflict: low drive and slow edge rates against high drive
    and fast edges (Design requirements note).
  - Source error: "#none#" printed for a figure reference; kept out
    (Layout guidelines note).
  - Source conflict: the example layout letters the pins of a quadruple
    2-input gate (Layout guidelines note).
  - Unconfirmed: the Recommended Operating Conditions (p.4) print a
    minimum of 0 for V~IL~ in the SN74AHCT04 column only.
