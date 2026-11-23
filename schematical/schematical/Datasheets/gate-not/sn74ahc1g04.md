---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g04",
  "title": "SN74AHC1G04 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc1g04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS318V",
    "date": "2024-02",
    "pages": 26
  },
  "devices": ["gate:1G04"],
  "parts": ["SN74AHC1G04DBVR", "SN74AHC1G04DBVT", "SN74AHC1G04DCK3", "SN74AHC1G04DCKR", "SN74AHC1G04DCKT", "SN74AHC1G04DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G04 single inverter gate

## Description

The SN74AHC1G04 contains one inverter gate. The device performs the
Boolean function Y = !{A}. [[8::1]]

Operating range 2 V to 5.5 V. Max t~pd~ of 6.5 ns at 5 V. Low power
consumption, 10-μA max I~CC~. ±8-mA output drive at 5 V. Schmitt-trigger
action at all inputs makes the circuit tolerant for slower input rise
and fall time. Latch-up performance exceeds 250 mA per JESD 17. [[1::1]]

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

The SN74AHC1G04 device contains one inverter gate. The device performs
the Boolean function Y = !{A}. This single gate inverter has
Schmitt-Trigger action on its input, allowing for slower rise and fall
times and some noise rejection. This is not a true Schmitt-Trigger, so
there is a limit on rise and fall times. [[8::1]] [[8::2]]

Conversion note: the logic diagram (Figure 7-1) shows one inverter, input A, output Y.

### Feature description

- Wide operating voltage range [[8::3]]
  - Operates from 2 V to 5.5 V [[8::3]]
- Allows down-voltage translation [[8::3]]
  - Inputs accept voltages to 5.5 V [[8::3]]
- Lower drive [[8::3]]
  - This will produce slower edges and help prevent ringing on outputs [[8::3]]

## Function tables

```table id=function caption="Function table"
| Input [1] | Output [2] |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |

[1] H = High Voltage Level, L = Low Voltage Level, X = Don’t Care
[2] H = Driving High, L = Driving Low, Z = High Impedance State
```
[[8::Table 7-1]]

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
| T~J~ | Junction temperature | | | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under [Section 5.3](#recommended-operating-conditions) is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±3500 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

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
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G04"
| Symbol | Thermal metric [1] | DBV 5 pins | DCK 5 pins | DRL 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 328.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | 105.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | 150.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | 6.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | 148.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the IC Package Thermal Metrics application report (SPRA953).
```
[[5::table #1]]

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
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |

[1] Source error: printed as I~OH~ = 50 µA; the rows give V~OL~, and the recommended operating conditions give 50 µA as I~OL~, so the condition is I~OL~.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 12 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5 | 7.1 | 1 | 8.5 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 7.1 | 1 | 8.5 | 1 | 9.5 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.5 | 10.6 | 1 | 12 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.5 | 10.6 | 1 | 12 | 1 | 13 | ns |
```
[[6::table #2]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit And Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to 85°C | < | –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.8 | 5.5 | 1 | 6.5 | 1 | 7 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.8 | 5.5 | 1 | 6.5 | 1 | 7 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF [1] | | 5.3 | 7.5 | 1 | 6.5 | 1 | 7 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5.3 | 7.5 | 1 | 6.5 | 1 | 7 | ns |

[1] Unconfirmed: the source prints maximums of 6.5 ns for –40°C to 85°C and 7 ns for –40°C to 125°C at C~L~ = 50 pF, below the 7.5 ns maximum it prints for 25°C on the same rows.
```
[[6::table #3]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature at 5 V"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.3], [25, 2.8], [85, 3.2], [125, 3.6]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "V_CC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 5]},
 "series": [
  {"label": "TPD in ns", "points": [[2.5, 4.5], [3.3, 3.6], [5, 2.9]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. All parameters and
waveforms are not applicable to all devices. [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 6-1]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[7::Figure 6-1]]

Conversion note: the source figure (Figure 6-1) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes, as well as validating and testing
their design implementation to confirm system functionality. [[9::1]]

SN74AHC1G04 is a low-drive CMOS device that can be used for a multitude
of inverting buffer type functions. It can produce 8 mA of drive current
at 5 V, making it Ideal for driving multiple outputs and good for
low-noise applications. The inputs are 5.5-V tolerant, allowing it to
translate down to V~CC~. [[9::2]]

Conversion note: the typical application schematic (Figure 8-1) shows the inverter between a block labelled 5-V accessory and a second block, supplied from a 3.3-V or 5-V regulated rail with a 0.1 µF capacitor to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Care
should be taken to avoid bus contention because it can drive currents
that would exceed maximum limits. The high drive will also create fast
edges into light loads, so routing and load conditions should be
considered to prevent ringing. [[9::3]]

Source conflict: Application Information (§ 8.1) and Feature Description (§ 7.3) say low drive and slower edges, Design Requirements (§ 8.2.1) says "the high drive will also create fast edges"; both are kept as printed.

### Detailed design procedure

1. Recommended Input Conditions [[9::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[9::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[9::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[9::4]]
2. Recommend Output Conditions [[9::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part. [[9::4]]
   - Outputs should not be pulled above V~CC~. [[9::4]]

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the
[Recommended Operating Conditions](#recommended-operating-conditions)
table. [[10::1]]

Each V~CC~ pin should have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1 μF is recommended. If
there are multiple V~CC~ pins, 0.01 μF or 0.022 μF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1 μF and 1 μF are commonly
used in parallel. The bypass capacitor should be installed as close to
the power pin as possible for best results. [[10::2]]

### Layout guidelines

When using multiple bit logic devices, inputs should not float. In many
cases, functions or parts of functions of digital logic devices are
unused. Some examples are when only two inputs of a triple-input AND
gate are used, or when only 3 of the 4-buffer gates are used. Such input
pins should not be left unconnected because the undefined voltages at
the outside connections result in undefined operational states. [[10::3]]

Specified in Figure 8-3 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or
V~CC~, whichever makes more sense or is more convenient. It is
acceptable to float outputs unless the part is a transceiver. If the
transceiver has an output enable pin, it will disable the outputs
section of the part when asserted. This will not disable the input
section of the I/Os so they also cannot float when disabled. [[10::4]]

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
SN74AHC1G04DBVR | Reel of 3000 | -40 to 125 | 38CH, 3BUF, A043, A04G, A04J, A04L, A04S
SN74AHC1G04DBVT | | -40 to 125 | A043, A04G, A04J, A04S | Obsolete
SN74AHC1G04DCK3 | | -40 to 85 | ACY | Obsolete
SN74AHC1G04DCKR | Reel of 3000 | -40 to 125 | 1R6, AC3, ACG, ACJ, ACL, ACS
SN74AHC1G04DCKT | | -40 to 125 | AC3, ACG, ACJ, ACS | Obsolete
SN74AHC1G04DRLR | Reel of 4000 | -40 to 125 | ACB, ACS
```
[[13::table #1]]

## Conversion notes

- Source pages 1–10 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.13–14). The support and revision history page (11), the
  mechanical information page (12), tape and reel and mechanical pages
  (15–26) are left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the
  simplified schematic and the logic diagram (one inverter); the typical
  application schematic (Figure 8-1) and the layout diagram (Figure 8-3)
  are described in text.
- Dropped: Figure 8-2 (typical application curve, an oscilloscope-style
  plot of the AC04, HC04 and AHC1G04 outputs at V~CC~ = 5 V into a
  50 Ω/50 pF load); it is not a characteristic of this part alone.
- Section 5.3, named in absolute maximum ratings note 1, the detailed
  design procedure and the power supply recommendations, is the source's
  Recommended Operating Conditions.
- The addendum also lists SN74AHC1G04DBVRE4, SN74AHC1G04DBVRG4,
  SN74AHC1G04DBVTG4, SN74AHC1G04DCKRE4, SN74AHC1G04DCKRG4,
  SN74AHC1G04DCKTG4 and SN74AHC1G04DRLRG4, which the catalog doesn't
  carry.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale; the source states no load for either figure.
- Figure 6-1 (p.7): the load circuit and the propagation delay waveforms
  were compared with the page image.
- From images: the overbar of Y = !{A} (p.1 and Overview, p.8); the
  curve points -55, 2.3, 2.8, 85, 3.2, 3.6 (Figure 5-1, p.6) and 2.5,
  4.5, 3.3, 3.6, 2.9 (Figure 5-2, p.6).
- Source issues:
  - Source error: the Input voltage row of the recommended operating
    conditions is printed with the symbol V~IH~ for V~I~; kept as
    printed (ROC table note 2).
  - Source error: the first V~OL~ rows are printed with I~OH~ = 50 µA;
    corrected to I~OL~ (electrical characteristics note 1).
  - Source error: the C~i~ row is printed "Input Capacitance V", with a
    stray V after the parameter name; the V is left out.
  - Source error: "Recommend Output Conditions" (§ 8.2.2) and "making it
    Ideal" (§ 8.1); kept as printed.
  - Source conflict: low drive and slower edges (§ 7.3, § 8.1) vs "high
    drive ... fast edges" (§ 8.2.1) (Application information note).
  - Unconfirmed: t~PLH~ and t~PHL~ at V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF
    printed with maximums of 6.5 ns (–40°C to 85°C) and 7 ns (–40°C to
    125°C), below the 7.5 ns maximum at 25°C (switching characteristics
    note 1).
