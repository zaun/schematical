---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1gu04",
  "title": "SN74AHC1GU04 single inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc1gu04.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS343S",
    "date": "2016-10",
    "pages": 26
  },
  "devices": ["gate:1GU04"],
  "parts": ["SN74AHC1GU04DBVR", "SN74AHC1GU04DBVT", "SN74AHC1GU04DCK3", "SN74AHC1GU04DCKR", "SN74AHC1GU04DCKT", "SN74AHC1GU04DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1GU04 single inverter gate

## Description

The SN74AHC1GU04 device contains a single inverter gate. The device
performs the Boolean function Y = !{A}. [[8::1]]

Operating Range of 2-V to 5.5-V V~CC~. Unbuffered Output. ±8-mA Output
Drive at 5 V. Latch-Up Performance Exceeds 250 mA Per JESD 17. [[1::1]]

## Pinout

```pinout pinout=p5
```

NC – No internal connection [[3::1]]

## Pin functions

```pins
NC: No connection
A: Input A
GND: Ground pin
Y: Output Y
VCC: Power pin
```
[[3::table #1]]

## Functional description

The SN74AHC1GU04 device contains a single inverter gate. The device
performs the Boolean function Y = !{A}. Internal circuitry consists of a
single-stage inverter that can be used in analog applications, such as
crystal oscillators. [[8::1]]

Conversion note: the functional block diagram (Figure 4) is the logic diagram of one inverter, input A, output Y.

### Feature description

- Wide operating voltage range [[8::2]]
  - Operates from 2 V to 5.5 V [[8::2]]
- Allows down-voltage translation [[8::2]]
  - Inputs accept voltages to 5.5 V [[8::2]]
- The unbuffered output is ideal for use in oscillator circuits [[8::2]]

### Device functional modes

Table 1 lists the functional modes of SN74AHC1GU04. [[8::3]]

## Function tables

```table id=function caption="Function table"
| Input | Output |
| A | Y |
|:-:|:-:|
| H | L |
| L | H |
```
[[8::Table 1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through each V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001 [1] | 2000 | V |
| ^ | ^ | Charged device model (CDM), per JEDEC specification JESD22-C101 [2] | 1000 | ^ |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2 V | 1.7 | | V |
| ^ | ^ | V~CC~ = 3 V | 2.4 | | ^ |
| ^ | ^ | V~CC~ = 5.5 V | 4.4 | | ^ |
| V~IL~ | Low-level input voltage | V~CC~ = 2 V | | 0.3 | V |
| ^ | ^ | V~CC~ = 3 V | | 0.6 | ^ |
| ^ | ^ | V~CC~ = 5.5 V | | 1.1 | ^ |
| V~IH~ [2] | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | ^ |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | ^ |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. See Implications of Slow or Floating CMOS Inputs (SCBA004).
[2] Source error: the source prints the symbol V~IH~ on this Input voltage row (p.4), kept as printed; V~IH~ is the High-level input voltage of the rows above, and the absolute maximum ratings name the input voltage V~I~.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1GU04"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT) 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 231.3 | 287.6 | 328.7 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 119.9 | 97.7 | 105.1 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 60.6 | 65 [2] | 150.3 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 17.8 | 2.0 | 6.9 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 60.1 | 64.2 | 148.4 | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
[2] Unconfirmed: printed "65." with nothing after the decimal point.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Test conditions | V~CC~ | T~A~ = 25°C | < | < | –40°C to +85°C | < | –40°C to +125°C | < | Unit |
| ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| V~OH~ | I~OH~ = –50 µA | 2 V | 1.8 | 2 | | 1.8 | | 1.9 | | V |
| ^ | ^ | 3 V | 2.7 | 3 | | 2.7 | | 2.7 | | ^ |
| ^ | ^ | 4.5 V | 4 | 4.5 | | 4 | | 4.4 | | ^ |
| ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | 2.48 | | ^ |
| ^ | I~OH~ = –8 mA | 4.5 V | 3.94 | | | 3.8 | | 3.8 | | ^ |
| V~OL~ | I~OL~ = 50 µA | 2 V | | | 0.2 | | 0.2 | | 0.1 | V |
| ^ | ^ | 3 V | | | 0.3 | | 0.3 | | 0.1 | ^ |
| ^ | ^ | 4.5 V | | | 0.5 | | 0.5 | | 0.1 | ^ |
| ^ | I~OL~ = 4 mA | 3 V | | | 0.36 | | 0.44 | | 0.44 | ^ |
| ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | 0.44 | | 0.44 | ^ |
| I~I~ | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | ±1 | | ±1 | µA |
| I~CC~ | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | 10 | | 10 | µA |
| C~i~ | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | 10 | | 10 | pF |
```
[[5::table #2]]

Unconfirmed: at I~OH~ = –50 µA and I~OL~ = 50 µA the –40°C to +125°C column prints tighter limits (1.9 V and 4.4 V minimum, 0.1 V maximum) than the 25°C and –40°C to +85°C columns.

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 7.3 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to +85°C | < | –40°C to +125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 5 | 7.1 | 1 | 8.5 | 1 | 9.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 7.1 | 1 | 8.5 | 1 | 9.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 7.5 | 10.6 | 1 | 12 | 1 | 13 | ns |
| t~PHL~ | ^ | ^ | ^ | | 7.5 | 10.6 | 1 | 12 | 1 | 13 | ^ |
```
[[5::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Figure 3)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | < | –40°C to +85°C | < | –40°C to +125°C | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | | 3.5 | 5.5 | 1 | 6 | 1 | 6.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 3.5 | 5.5 | 1 | 6 | 1 | 6.5 | ^ |
| t~PLH~ | A | Y | C~L~ = 50 pF | | 5 | 7 | 1 | 8 | 1 | 8.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 5 | 7 | 1 | 8 | 1 | 8.5 | ^ |
```
[[5::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 4.5]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 2.93], [-40, 2.97], [25, 3.6], [85, 4.0], [125, 4.27]]}
 ]}
```
[[6::Figure 1]]

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "VCC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 7]},
 "series": [
  {"label": "TPD in ns", "points": [[2, 6], [3, 4.55], [4.5, 3.8], [5, 3.6], [5.5, 3.2]]}
 ]}
```
[[6::Figure 2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time with one input transition per measurement. [[7::1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: see table
```
[[7::Figure 3]]

```figure ref=waveforms-tpd caption="Voltage waveforms, propagation delay times, inverting and noninverting outputs"
VM: 50% VCC
```
[[7::Figure 3]]

Conversion note: the source figure (Figure 3) also shows the load
circuit for 3-state and open-drain outputs with its S1 table, and the
pulse-duration, setup-and-hold and enable/disable waveforms, which are
not drawn here.

## Application information

Information in the following applications sections is not part of the TI
component specification, and TI does not warrant its accuracy or
completeness. TI’s customers are responsible for determining suitability
of components for their purposes. Customers should validate and test
their design implementation to confirm system functionality. [[9::1]]

A CMOS inverter is used as a linear amplifier in oscillator
applications. Similar to a conventional amplifier, their open-loop gain
is a critical characteristic. The bandwidth of an inverter decreases as
the operating voltage decreases. The open-loop gain of the AHC1GU04
device is shown in Figure 6. [[9::2]]

Conversion note: the typical application schematic (Figure 5) shows the CMOS inverter with a feedback resistor R~F~ and a crystal from input to output, and capacitors C~1~ and C~2~ from the input and the output to ground.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. The high drive will also create fast edges into
light loads, so routing and load conditions should be considered to
prevent ringing. [[9::3]]

### Detailed design procedure

1. Recommended Input Conditions [[9::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     Recommended Operating Conditions table. [[9::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     Recommended Operating Conditions table. [[9::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5 V at any valid V~CC~. [[9::4]]
2. Recommended Output Conditions [[9::4]]
   - Load currents should not exceed 25 mA per output and 50 mA total
     for the part. [[9::4]]
   - Outputs should not be pulled above V~CC~. [[9::4]]

Source conflict: step 1 (p.9) points to Δt/ΔV in the Recommended Operating Conditions table; that table (p.4) has no Δt/ΔV row.

### Application curve

```curve id=open-loop-gain caption="Open-loop gain"
{"x": {"label": "Frequency", "unit": "MHz", "range": [0.1, 100], "log": true},
 "y": {"label": "Gain", "unit": "dBV", "range": [-10, 30]},
 "series": [
  {"label": "VCC = 5 V", "points": [[0.1, 18.3], [1, 18.3], [10, 16.5], [100, 8.7]]},
  {"label": "VCC = 3.3 V", "points": [[0.1, 17.7], [1, 17.7], [10, 15.3], [100, 6]]},
  {"label": "VCC = 2.7 V", "points": [[0.1, 16.2], [1, 16.2], [10, 13.5], [100, 3.1]]},
  {"label": "VCC = 2 V", "points": [[0.1, 8.6], [1, 8.6], [10, 5.3], [100, -5.5]]}
 ]}
```
[[10::Figure 6]]

### Power supply recommendations

The power supply can be any voltage between the MIN and MAX supply
voltage rating located in the Recommended Operating Conditions table. [[10::1]]

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
the outside connections result in undefined operational states. [[11::1]]

Specified in Figure 7 are rules that must be observed under all
circumstances. All unused inputs of digital logic devices must be
connected to a high or low bias to prevent them from floating. The logic
level that should be applied to any particular unused input depends on
the function of the device. Generally they will be tied to GND or V~CC~,
whichever makes more sense or is more convenient. It is acceptable to
float outputs unless the part is a transceiver. If the transceiver has
an output enable pin, it will disable the outputs section of the part
when asserted. This will not disable the input section of the I/Os so
they also cannot float when disabled. [[11::2]]

Conversion note: the layout diagram (Figure 7) shows two gates, one with its unused input tied to V~CC~ and one with its unused input tied to ground.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1GU04DBVR | Reel of 3000 | -40 to 125 | AU43, AU4G, AU4J, AU4L, AU4S
SN74AHC1GU04DBVT | | -40 to 125 | AU43, AU4G, AU4J, AU4L, AU4S | Obsolete
SN74AHC1GU04DCK3 | Reel of 3000 | -40 to 85 | ADY | Last Time Buy
SN74AHC1GU04DCKR | Reel of 3000 | -40 to 125 | AD3, ADG, ADJ, ADL, ADS
SN74AHC1GU04DCKT | | -40 to 125 | AD3, ADG, ADJ, ADL, ADS | Obsolete
SN74AHC1GU04DRLR | Reel of 4000 | -40 to 125 | ADS
```
[[13::table #1]]

## Conversion notes

- Source pages 1–11 converted; p.2 holds only the table of contents and
  revision history, which are dropped. The ordering lines are from the
  package option addendum (pp.13–14). The support page (12), tape and
  reel and mechanical pages (15–26) are left out; the generated Package
  section replaces them.
- Dropped: applications list and the ESD feature lines (the other
  features are kept as sentences in the Description), device information
  table, the logic diagram (one inverter); the typical application
  schematic (Figure 5) and the layout diagram (Figure 7) are described
  in text.
- The absolute maximum ratings and recommended operating conditions of
  the source print the conditions in an unnamed column; they are put in
  the Conditions cells.
- The electrical characteristics table of the source gives symbols only,
  so it has no Parameter column.
- The 5 V switching characteristics table of the source runs over
  pp.5–6, repeating its heading on the second page; it is one table
  here.
- The source's Power Supply Recommendations (10) and Layout (11) are
  chapters of their own, after Application and Implementation; they are
  under Application information here.
- The marking "ADL" is printed broken over two lines ("AD" / "L") in the
  addendum (p.13).
- The addendum also lists 74AHC1GU04DBVRG4, 74AHC1GU04DBVTG4 and
  74AHC1GU04DCKTG4, which the catalog doesn't carry.
- Curves digitized from Figures 1 and 2 (p.6) and Figure 6 (p.10) to
  about 2 % of full scale; Figure 6 is a picture with a logarithmic
  frequency axis and is read at its labelled decades.
- Figure 3 (p.7) is drawn as text; the load circuit and waveforms were
  compared with the page image.
- From images: overbars of Y = !{A} (p.1 and Overview, p.8); R~F~,
  C~1~ and C~2~ (Figure 5, p.9); the curve points -55, 2.93, 2.97, 3.6,
  4.0, 4.27 (Figure 1, p.6), 4.55, 3.8, 3.2 (Figure 2, p.6) and 18.3,
  16.5, 8.7, 17.7, 15.3, 16.2, 13.5, 3.1, 8.6, 5.3, -5.5 with the axis
  labels 0.1, 1, 10, 100, 30, -10 and V~CC~ = 5 V, 3.3 V, 2.7 V, 2 V
  (Figure 6, p.10).
- Source issues:
  - Source error: Input voltage row printed with the symbol V~IH~ for
    V~I~; kept as printed (Recommended operating conditions note).
  - Unconfirmed: DCK R~θJB~ printed "65." (Thermal information note).
  - Unconfirmed: –40°C to +125°C limits of V~OH~ and V~OL~ at 50 µA
    tighter than at 25°C (Electrical characteristics).
  - Source conflict: Δt/ΔV named in the detailed design procedure but
    absent from the Recommended Operating Conditions (Application
    information).
  - Source error: "Specified inFigure 7" is printed without a space
    (p.11); written with one.
