---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g14",
  "title": "SN74AHC1G14 single Schmitt-trigger inverter gate",
  "class": "logic",
  "source": {
    "pdf": "gate-not/sn74ahc1g14.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS321S",
    "date": "2024-02",
    "pages": 26
  },
  "devices": ["gate:1G14"],
  "parts": ["SN74AHC1G14DBVR", "SN74AHC1G14DCK3", "SN74AHC1G14DCKR", "SN74AHC1G14DCKRG4", "SN74AHC1G14DCKT", "SN74AHC1G14DRLR"],
  "packages": {"p5": "DBV, DCK, DRL"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G14 single Schmitt-trigger inverter gate

## Description

The SN74AHC1G14 device is a single inverter gate. The device performs
the Boolean function Y = !{A}. [[1::1]]

Operating range 2 V to 5.5 V. Maximum t~pd~ of 10 ns at 5 V. Low power
consumption, 10-μA max I~CC~. ±8-mA output drive at 5 V. Latch-up
performance exceeds 250 mA per JESD 17. [[1::2]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
NC: No connect
A: Data Input
GND: Ground
Y: Data Output
VCC: Power
```
[[3::Table 4-1]]

## Functional description

The SN74AHC1G14 device is a single inverter gate. The device performs
the Boolean function Y = !{A}. [[1::1]]

The device functions as an independent inverter gate, but because of the
Schmitt action, gates may have different input threshold levels for
positive- (V~T+~) and negative-going (V~T−~) signals. [[8::2]]

Conversion note: the logic diagram (Figure 7-1) shows one Schmitt-trigger inverter, input A (pin 2), output Y (pin 4).

### Feature description

The SN74AHC1G14 device has a wide operating V~CC~ range of 2 V to 5.5 V,
which allows it to be used in a broad range of systems. The low
propagation delay allows fast switching and higher speeds of operation.
In addition, the low-power consumption makes this device a good choice
for portable and battery power-sensitive applications. [[8::3]]

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
| V~CC~ | Supply voltage | | -0.5 | 7 | V |
| V~I~ | Input voltage [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | V~I~ < 0 | | -20 | mA |
| I~OK~ | Output clamp current | V~O~ < 0 or V~O~ > V~CC~ | | ±20 | mA |
| I~O~ | Continuous output current | V~O~ = 0 to V~CC~ | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~j~ | Maximum junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature | | -65 | 150 | °C |

[1] Stresses beyond those listed under Absolute Maximum Ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human body model (HBM), per ANSI/ESDA/JEDEC JS-001, all pins [1] | ±1500 | V |
| ^ | ^ | Charged-device model (CDM), per JEDEC specification JESD22-C101, all pins [2] | ±1000 | V |

[1] JEDEC document JEP155 states that 500-V HBM allows safe manufacturing with a standard ESD control process.
[2] JEDEC document JEP157 states that 250-V CDM allows safe manufacturing with a standard ESD control process.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions" conditions="Over operating free-air temperature range (unless otherwise noted) [1]"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2 V | | -50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | -4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2 V | | 50 | µA |
| ^ | ^ | V~CC~ = 3.3 V ± 0.3 V | | 4 | mA |
| ^ | ^ | V~CC~ = 5 V ± 0.5 V | | 8 | mA |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation. Refer to the TI application report, Implications of Slow or Floating CMOS Inputs, SCBA004.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G14"
| Symbol | Thermal metric [1] | DBV (SOT-23) 5 pins | DCK (SC70) 5 pins | DRL (SOT) 5 pins | Unit |
|---|---|--:|--:|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | 289.2 | 271.8 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | 205.8 | 116.6 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | 176.2 | 89.9 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | 117.6 | 17.3 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | 175.1 | 89.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bot) thermal resistance | N/A | N/A | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report, SPRA953.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | T~A~ = –40°C to 85°C | < | < | Recommended T~A~ = –40°C to 125°C | < | < | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | Min | Typ | Max | Min | Typ | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| V~T+~ | Positive-going input threshold voltage | | 3 V | 1.2 | | 2.2 | 1.2 | | 2.2 | 1.2 | | 2.2 | V |
| ^ | ^ | | 4.5 V | 1.75 | | 3.15 | 1.75 | | 3.15 | 1.75 | | 3.15 | V |
| ^ | ^ | | 5.5 V [1] | 2.15 | | 3.85 | 2.15 | | 2.85 | 2.15 | | 3.85 | V |
| V~T–~ | Negative-going input threshold voltage | | 3 V | 0.9 | | 1.9 | 0.9 | | 1.9 | 0.9 | | 1.9 | V |
| ^ | ^ | | 4.5 V | 1.35 | | 2.75 | 1.35 | | 2.75 | 1.35 | | 2.75 | V |
| ^ | ^ | | 5.5 V | 1.65 | | 3.35 | 1.65 | | 3.35 | 1.65 | | 3.35 | V |
| ΔV~T~ | Hysteresis (V~T+~ – V~T–~) | | 3 V | 0.3 | | 1.2 | 0.3 | | 1.2 | 0.25 | | 1.2 | V |
| ^ | ^ | | 4.5 V | 0.4 | | 1.4 | 0.4 | | 1.4 | 0.35 | | 1.4 | V |
| ^ | ^ | | 5.5 V | 0.5 | | 1.6 | 0.5 | | 1.6 | 0.45 | | 1.6 | V |
| V~OH~ | High level output voltage | I~OH~ = –50 µA | 2 V | 1.9 | 2 | | 1.9 | | | 1.9 | | | V |
| ^ | ^ | ^ | 3 V | 2.9 | 3 | | 2.9 | | | 2.9 | | | V |
| ^ | ^ | ^ | 4.5 V | 4.4 | 4.5 | | 4.4 | | | 4.4 | | | V |
| ^ | ^ | I~OH~ = –4 mA | 3 V | 2.58 | | | 2.48 | | | 2.4 | | | V |
| ^ | ^ | I~OH~ = –8 mA [2] | 4.5 V | 3.94 | | | 3.8 | | | 3.7 | | | V |
| V~OL~ | Low level output voltage | I~OL~ = 50 µA [3] | 2 V | | | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | 3 V | | | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | ^ | 4.5 V | | | 0.1 | | | 0.1 | | | 0.1 | V |
| ^ | ^ | I~OL~ = 4 mA [3] | 3 V | | | 0.36 | | | 0.44 | | | 0.55 | V |
| ^ | ^ | I~OL~ = 8 mA | 4.5 V | | | 0.36 | | | 0.44 | | | 0.55 | V |
| I~I~ | Input leakage current | V~I~ = 5.5 V or GND | 0 V to 5.5 V | | | ±0.1 | | | ±1 | | | ±1 | µA |
| I~CC~ | Supply current | V~I~ = V~CC~ or GND, I~O~ = 0 | 5.5 V | | | 1 | | | 10 | | | 10 | µA |
| C~i~ | Input Capacitance | V~I~ = V~CC~ or GND | 5 V | | 2 | 10 | | | 10 | | | 10 | pF |

[1] Unconfirmed: the source prints a V~T+~ maximum of 2.85 V at V~CC~ = 5.5 V for –40°C to 85°C, below the 3.85 V it prints for 25°C and for –40°C to 125°C on the same row.
[2] Source error: printed as I~OL~ = –8 mA; the row gives V~OH~, and the recommended operating conditions give –8 mA as I~OH~, so the condition is I~OH~.
[3] Source error: printed as I~OH~ = 50 µA and I~OH~ = 4 mA; the rows give V~OL~, and the recommended operating conditions give 50 µA and 4 mA as I~OL~, so the condition is I~OL~.
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5 V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1 MHz | 9 | pF |
```
[[6::table #1]]

## Switching characteristics

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3 V ± 0.3 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | T~A~ = –40°C to 85°C | < | Recommended T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A | Y | C~L~ = 15 pF | 8.3 | 12.8 | 1 | 15 | 1 | 16 | ns |
| t~PHL~ | ^ | ^ | ^ | 8.3 | 12.8 | 1 | 15 | 1 | 16 | ns |
| t~PLH~ | A | Y | C~L~ = 50 pF | 10.8 | 16.3 | 1 | 18.5 | 1 | 19.5 | ns |
| t~PHL~ | ^ | ^ | ^ | 10.8 | 16.3 | 1 | 18.5 | 1 | 19.5 | ns |
```
[[6::table #2]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5 V ± 0.5 V" conditions="Over recommended operating free-air temperature range (unless otherwise noted) (see Load Circuit and Voltage Waveforms)"
| Symbol | From (input) | To (output) | Output capacitance | T~A~ = 25°C | < | T~A~ = –40°C to 85°C | < | Recommended T~A~ = –40°C to 125°C | < | Unit |
| ^ | ^ | ^ | ^ | Typ | Max | Min | Max | Min | Max | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B [1] | Y | C~L~ = 15 pF | 5.5 | 8.6 | 1 | 10 | 1 | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | 5.5 | 8.6 | 1 | 10 | 1 | 11 | ns |
| t~PLH~ | A or B [1] | Y | C~L~ = 50 pF [2] | 7 | 10.6 | 1 | 12 | 1 | 11 | ns |
| t~PHL~ | ^ | ^ | ^ | 7 | 10.6 | 1 | 12 | 1 | 11 | ns |

[1] Source error: the source prints "A or B" as the input in this table, kept as printed; the device has one input, A (pin functions, p.3), and the V~CC~ = 3.3 V table prints A.
[2] Unconfirmed: the source prints a maximum of 11 ns for –40°C to 125°C at C~L~ = 50 pF, below the 12 ns maximum it prints for –40°C to 85°C on the same rows.
```
[[6::table #3]]

## Typical characteristics

Conversion note: Figure 5-1 (Response Time vs Output Voltage, T~A~ = 25°C, V~A~ = 5 V) is an oscilloscope-style plot of signal voltage against time, 0 to 50 ns, with one falling trace and one rising trace that cross between 25 and 30 ns.

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
[[6::Figure 5-1]]

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

Physically interactive interface elements like push buttons or rotary
knobs offer simple and easy ways to interact with an electronic system.
Many of these physical interface elements often have issues with
bouncing, or where the physical conductive contact can connect and
disconnect multiple times during a button push or release. This bouncing
can cause one or more faulty transient signals to be passed during this
transitional period. These faulty signals can be observed in many common
applications, for example, a television remote with bouncing error can
adjust the TV channel multiple times despite the button being pushed
only once. To mitigate these faulty signals, we can use a
Schmitt-trigger, or a device with hysteresis, to remove these faulty
signals. Hysteresis allows a device to remember its history, and in this
case, the SN74AHC1G14 uses this memory to debounce the signal of the
physical element, or filter the faulty transient signals and pass only
the valid signal each time the element is used. In this example, we show
a push-button signal passed through an SN74AHC1G14 that is debounced and
inverted to the microprocessor for push detection. [[9::2]]

Conversion note: the switch debouncer (Figure 8-1) shows a physical push button to ground with a resistor to V~CC~, driving the SN74AHC1G14, whose output goes to a microprocessor.

### Design requirements

The SN74AHC1G14 device uses CMOS technology and has balanced output
drive. Take care to avoid bus contention because it can drive currents
that would exceed maximum limits. The SN74AHC1G14 allows for performing
logical Boolean functions with hysteresis using digital signals. All
input signals must remain as close as possible to either 0 V or V~CC~
for optimal operation. [[9::3]]

### Detailed design procedure

1. Recommended input conditions: [[10::1]]
   - For rise time and fall time specifications, see Δt/Δv in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[10::1]]
   - For specified high and low levels, see V~IH~ and V~IL~ in the
     [Recommended Operating Conditions](#recommended-operating-conditions)
     table. [[10::1]]
   - Inputs and outputs are overvoltage tolerant and can therefore go as
     high as 5.5 V at any valid V~CC~. [[10::1]]
2. Recommended output conditions: [[10::1]]
   - Load currents must not exceed ±50 mA. [[10::1]]
3. Frequency selection criterion: [[10::1]]
   - The effects of frequency upon the power consumption of the device
     can be studied in CMOS Power Consumption and CPD Calculation,
     SCAA035. [[10::1]]
   - Added trace resistance and capacitance can reduce maximum frequency
     capability; follow the layout practices listed in the
     [Layout Guidelines](#layout-guidelines) section. [[10::1]]

Source conflict: the Recommended Operating Conditions table (p.4) has no Δt/Δv, V~IH~ or V~IL~ row; the input thresholds are V~T+~ and V~T–~ in the Electrical Characteristics (p.5).

Source conflict: "inputs and outputs are overvoltage tolerant" to 5.5 V (p.10) vs the absolute maximum output voltage of V~CC~ + 0.5 V and the recommended output voltage maximum of V~CC~ (p.4); both are kept as printed.

Source conflict: "load currents must not exceed ±50 mA" (p.10) vs the absolute maximum continuous output current of ±25 mA (p.4); both are kept as printed.

### Application curves

```curve id=voh-ioh caption="AHC Family V~OH~ vs I~OH~, T~A~ = 25°C, V~CC~ = 5.5 V, All Outputs Switching"
{"x": {"label": "I_OH", "unit": "mA", "range": [-55, 0]},
 "y": {"label": "V_OH", "unit": "V", "range": [0, 5.5]},
 "series": [
  {"label": "AHC", "points": [[-55, 1.8], [-45, 3.1], [-35, 3.8], [-25, 4.35], [-15, 4.8], [-5, 5.25], [0, 5.5]]}
 ]}
```
[[10::Figure 8-2]]

```curve id=vol-iol caption="AHC Family V~OL~ vs I~OL~, T~A~ = 25°C, V~CC~ = 5.5 V, All Outputs Switching"
{"x": {"label": "I_OL", "unit": "mA", "range": [0, 40]},
 "y": {"label": "V_OL", "unit": "V", "range": [0, 3]},
 "series": [
  {"label": "AHC", "points": [[0, 0], [10, 0.3], [20, 0.65], [30, 1.05], [40, 1.65]]}
 ]}
```
[[10::Figure 8-2]]

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating listed in the
[Recommended Operating Conditions](#recommended-operating-conditions)
table. [[10::2]]

Each V~CC~ terminal must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, a 0.1-μF bypass capacitor
is recommended. If multiple pins are labeled V~CC~, then a 0.01-μF or
0.022-μF capacitor is recommended for each V~CC~ because the V~CC~ pins
are tied together internally. For devices with dual-supply pins
operating at different voltages, for example V~CC~ and V~DD~, a 0.1-µF
bypass capacitor is recommended for each supply pin. To reject different
frequencies of noise, use multiple bypass capacitors in parallel.
Capacitors with values of 0.1 μF and 1 μF are commonly used in parallel.
The bypass capacitor must be installed as close as possible to the power
terminal for best results. [[10::3]]

### Layout guidelines

Reflections and matching are closely related to the loop antenna theory
but are different enough to be discussed separately from the theory.
When a PCB trace turns a corner at a 90° angle, a reflection can occur.
A reflection occurs primarily because of the change of width of the
trace. At the apex of the turn, the trace width increases to 1.414 times
the width. This increase upsets the transmission-line characteristics,
especially the distributed capacitance and self-inductance of the trace,
which results in the reflection. Not all PCB traces can be straight;
therefore some traces must turn corners. Figure 8-4 shows progressively
better techniques of rounding corners. Only the last example (BEST)
maintains constant trace width and minimizes reflections. [[10::4]] [[11::1]]

Conversion note: Figure 8-4 (trace example: WORST, BETTER, BEST corners) is not drawn.

## Package

```package footprint=SOT23-5
```

```package footprint=SC70-5
```

```package footprint=SOT5X3-5
```

## Ordering information

```ordering
SN74AHC1G14DBVR | Reel of 3000 | -40 to 125 | 38SH, 3C2F, A143, A14G, A14J, A14L, A14S
SN74AHC1G14DCK3 | | -40 to 125 | AFY | Obsolete
SN74AHC1G14DCKR | Reel of 3000 | -40 to 125 | 1R9, AF3, AFG, AFJ, AFL, AFS
SN74AHC1G14DCKRG4 | Reel of 3000 | -40 to 125 | AF3
SN74AHC1G14DCKT | | -40 to 125 | AF3, AFG, AFJ, AFS | Obsolete
SN74AHC1G14DRLR | Reel of 4000 | -40 to 125 | AFS
```
[[13::table #1]]

## Conversion notes

- Source pages 1–11 converted; p.2 holds only the table of contents,
  which is dropped. The ordering lines are from the package option
  addendum (pp.13–14). The support, revision history and mechanical
  information page (12), tape and reel and mechanical pages (15–26) are
  left out; the generated Package section replaces them.
- Dropped: features and applications lists (the features are kept as
  sentences in the Description), package information table, the logic
  diagram (one Schmitt-trigger inverter); Figure 5-1 (p.6), the switch
  debouncer (Figure 8-1) and the trace example (Figure 8-4) are
  described in text.
- Section 5.3, named in the detailed design procedure and the power
  supply recommendations, is the source's Recommended Operating
  Conditions; Section 8.4.1 is its Layout Guidelines.
- The addendum also lists SN74AHC1G14DBVRE4, SN74AHC1G14DBVRG4,
  SN74AHC1G14DBVTG4.A, SN74AHC1G14DCKTE4, SN74AHC1G14DCKTG4 and
  SN74AHC1G14DRLRG4, which the catalog doesn't carry.
- Curves digitized from Figures 8-2 and 8-3 (p.10) to about 2 % of full
  scale; only the AHC trace of each is kept, the AHCT trace is left out.
  The conditions in the captions are those printed inside the figures.
- Figure 6-1 (p.7): the load circuit and the propagation delay waveforms
  were compared with the page image.
- From images: the overbar of Y = !{A} (p.1 and Overview, p.8); the
  conditions T~A~ = 25°C, V~CC~ = 5.5 V, All Outputs Switching, the axis
  ranges -55, 40 and the curve points 1.8, -45, 3.1, -35, 3.8, -25,
  4.35, -15, 4.8, -5, 5.25 (Figure 8-2, p.10) and 0.3, 0.65, 1.05, 1.65
  (Figure 8-3, p.10).
- Source issues:
  - Unconfirmed: V~T+~ maximum at V~CC~ = 5.5 V, –40°C to 85°C printed
    as 2.85 V (electrical characteristics note 1).
  - Source error: the 4.5 V V~OH~ row is printed with I~OL~ = –8 mA;
    corrected to I~OH~ (electrical characteristics note 2).
  - Source error: V~OL~ rows are printed with I~OH~ = 50 µA and
    I~OH~ = 4 mA; corrected to I~OL~ (electrical characteristics
    note 3).
  - Source error: the V~CC~ = 5 V switching table prints the input as
    "A or B"; kept as printed (switching characteristics note 1).
  - Unconfirmed: t~PLH~ and t~PHL~ at V~CC~ = 5 V ± 0.5 V, C~L~ = 50 pF,
    –40°C to 125°C printed with a maximum of 11 ns, below the 12 ns at
    –40°C to 85°C (switching characteristics note 2).
  - Source conflict: Δt/Δv, V~IH~ and V~IL~ referenced in the
    Recommended Operating Conditions but not in that table (Detailed
    design procedure note).
  - Source conflict: "inputs and outputs are overvoltage tolerant" to
    5.5 V vs V~O~ maximum of V~CC~ + 0.5 V (Detailed design procedure
    note).
  - Source conflict: "load currents must not exceed ±50 mA" vs I~O~
    ±25 mA (Detailed design procedure note).
  - Unconfirmed: the Description and the feature list call the device an
    inverter gate; the title and logic diagram show a Schmitt-trigger
    inverter, and "Sever Motherboard" in the applications list is
    printed so.
