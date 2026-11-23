---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "sn74ahc1g86-q1",
  "title": "SN74AHC1G86-Q1 automotive single 2-input exclusive-OR gate",
  "class": "logic",
  "source": {
    "pdf": "gate-xor/sn74ahc1g86-q1.pdf",
    "vendor": "Texas Instruments",
    "document": "SCLS723B",
    "date": "2024-02",
    "pages": 22
  },
  "devices": ["gate:1G86"],
  "parts": ["SN74AHC1G86QDBVRQ1"],
  "packages": {"p5": "DBV"},
  "converted": "2026-11-21"
}
---

# SN74AHC1G86-Q1 automotive single 2-input exclusive-OR gate

## Description

The SN74AHC1G86-Q1 is a single 2-input exclusive-OR gate. The device
performs the Boolean function Y = A ⊕ B or Y = !{A}B + A!{B} in positive
logic. [[1::1]]

A common application is as a true/complement element. If one of the
inputs is low, the other input is reproduced in true form at the
output. If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[1::4]]

Qualified for automotive applications. Operating range of 2V to 5.5V.
Maximum t~pd~ of 10ns at 5V. Low power consumption, 10μA maximum I~CC~.
±8mA output drive at 5V. Schmitt-trigger action at all inputs makes the
circuit tolerant for slower input rise and fall time. [[1::2]]

## Pinout

```pinout pinout=p5
```

## Pin functions

```pins
A: Input A
B: Input B
GND: Ground
Y: Output Y
VCC: Positive Supply
```
[[3::Table 4-1]]

## Functional description

The SN74AHC1G86-Q1 is an automotive qualified device that performs the
Boolean function Y = !{A}B + A!{B} in positive logic. This single
2-input exclusive-OR gate is designed for 2V to 5.5V V~CC~ operation. [[8::1]]

A common application is as a true or complementary element. If one of
the inputs is low, the other input is reproduced in true form at the
output. If one of the inputs is high, the signal on the other input is
reproduced inverted at the output. [[8::2]]

These are five equivalent exclusive-OR symbols valid for an
SN74AHC1G86-Q1 gate in positive logic; negation may be shown at any two
ports. Conversion note: the five symbols of the functional block
diagram are not drawn here. [[8::3]]

### Balanced CMOS push-pull outputs

A balanced output allows the device to sink and source similar
currents. The drive capability of this device may create fast edges
into light loads so routing and load conditions should be considered to
prevent ringing. Additionally, the outputs of this device are capable
of driving larger currents than the device can sustain without being
damaged. It is important for the output power of the device to be
limited to avoid damage due to over-current. The electrical and thermal
limits must be followed at all times. [[8::4]]

### Standard CMOS inputs

Standard CMOS inputs are high impedance and are typically modeled as a
resistor in parallel with the input capacitance. The worst case
resistance is calculated with the maximum input voltage and the maximum
input leakage current, using ohm's law (R = V ÷ I). [[8::5]]

Signals applied to the inputs need to have fast edge rates, as defined
by Δt/Δv to avoid excessive current consumption and oscillations. If a
slow or noisy input signal is required, a device with a Schmitt-trigger
input should be used to condition the input signal prior to the
standard CMOS input. [[8::6]]

Source conflict: the Features (p.1) say "Schmitt-trigger action at all inputs", § 7.3.2 (p.8) describes standard CMOS inputs and sends slow or noisy signals to a device with a Schmitt-trigger input; both are kept as printed.

### Clamping diodes

The inputs have negative clamping diodes, and the outputs have positive
and negative clamping diodes as depicted in Figure 7-1. [[8::7]]

Voltages beyond the values specified can cause damage to the device.
The input negative-voltage and output voltage ratings may be exceeded
if the input and output clamp-current ratings are observed. [[8::8]]

Conversion note: Figure 7-1 shows one clamping diode from GND to the input (-I~IK~), one from GND to the output (-I~OK~) and one from the output to V~CC~ (+I~OK~).

### Over-voltage tolerant inputs

Input signals to this device can be driven above the supply voltage so
long as they remain below the maximum input voltage value. [[9::2]]

## Function tables

```table id=function caption="Function table"
| Inputs | < | Output |
| A | B | Y |
|:-:|:-:|:-:|
| L | L | L |
| L | H | H |
| H | L | H |
| H | H | L |
```
[[9::Table 7-1]]

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage range | | -0.5 | 7 | V |
| V~I~ | Input voltage range [2] | | -0.5 | 7 | V |
| V~O~ | Output voltage range applied in the high- or low-state [2] | | -0.5 | V~CC~ + 0.5 | V |
| I~IK~ | Input clamp current | (V~I~ < 0V) | | -20 | V [3] |
| I~OK~ | Output clamp current | (V~O~ < 0V or V~O~ > V~CC~) | | ±20 | mA |
| I~O~ | Continuous output current | (V~O~ = 0V to V~CC~) | | ±25 | mA |
| | Continuous current through V~CC~ or GND | | | ±50 | mA |
| T~J~ | Junction temperature | | | 150 | °C |
| T~stg~ | Storage temperature range | | -65 | 150 | °C |

[1] Stresses beyond those listed under absolute maximum ratings may cause permanent damage to the device. These are stress ratings only, and functional operation of the device at these or any other conditions beyond those indicated under Recommended Operating Conditions is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.
[2] The input and output voltage ratings may be exceeded if the input and output current ratings are observed.
[3] Source error: the source prints the unit V for this current; the I~OK~ and I~O~ rows of the same table are in mA. Kept as printed.
```
[[4::table #1]]

```table id=esd caption="ESD ratings"
| Symbol | Parameter | Conditions | Value | Unit |
|---|---|---|--:|---|
| V~(ESD)~ | Electrostatic discharge | Human-body model (HBM), per AEC Q100-002 [1] | ±4000 | V |
| ^ | ^ | Charged-device model (CDM), per AEC Q100-011 | ±1000 | V |

[1] AEC Q100-002 indicates that HBM stressing shall be in accordance with the ANSI/ESDA/JEDEC JS-001 specification.
```
[[4::table #2]]

## Recommended operating conditions

```table id=roc caption="Recommended operating conditions [1]" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | | 2 | 5.5 | V |
| V~IH~ | High-level input voltage | V~CC~ = 2V | 1.5 | | V |
| ^ | ^ | V~CC~ = 3V | 2.1 | | V |
| ^ | ^ | V~CC~ = 5.5V | 3.85 | | V |
| V~IL~ | Low-level input voltage | V~CC~ = 2V | | 0.5 | V |
| ^ | ^ | V~CC~ = 3V | | 0.9 | V |
| ^ | ^ | V~CC~ = 5.5V | | 1.65 | V |
| V~I~ | Input voltage | | 0 | 5.5 | V |
| V~O~ | Output voltage | | 0 | V~CC~ | V |
| I~OH~ | High-level output current | V~CC~ = 2V | | -50 | μA |
| ^ | ^ | V~CC~ = 3.3V ±0.3V | | -4 | mA |
| ^ | ^ | V~CC~ = 5V ±0.5V | | -8 | mA |
| I~OL~ | Low-level output current | V~CC~ = 2V | | 50 | μA |
| ^ | ^ | V~CC~ = 3.3V ±0.3V | | 4 | mA |
| ^ | ^ | V~CC~ = 5V ±0.5V | | 8 | mA |
| Δt/ΔV | Input transition rise or fall rate | V~CC~ = 3.3V ±0.3V | | 100 | ns/V |
| ^ | ^ | V~CC~ = 5V ±0.5V | | 20 | ns/V |
| T~A~ | Operating free-air temperature | | -40 | 125 | °C |

[1] All unused inputs of the device must be held at V~CC~ or GND to ensure proper device operation.
```
[[4::table #3]]

## Thermal information

```table id=thermal caption="Thermal information, SN74AHC1G86-Q1: thermal metric [1]"
| Symbol | Parameter | DBV (SOT-23) 5 pins | Unit |
|---|---|--:|---|
| R~θJA~ | Junction-to-ambient thermal resistance | 278 | °C/W |
| R~θJC(top)~ | Junction-to-case (top) thermal resistance | 180.5 | °C/W |
| R~θJB~ | Junction-to-board thermal resistance | 184.4 | °C/W |
| ψ~JT~ | Junction-to-top characterization parameter | 115.4 | °C/W |
| ψ~JB~ | Junction-to-board characterization parameter | 183.4 | °C/W |
| R~θJC(bot)~ | Junction-to-case (bottom) thermal resistance | N/A | °C/W |

[1] For more information about traditional and new thermal metrics, see the Semiconductor and IC Package Thermal Metrics application report.
```
[[5::table #1]]

## Electrical characteristics

```table id=ec caption="Electrical characteristics" conditions="Over operating free-air temperature range (unless otherwise noted)"
| Symbol | Parameter | Test conditions | V~CC~ | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| V~OH~ | | I~OH~ = -50μA | 2V | 1.9 | 2 | | 1.9 | | V |
| ^ | ^ | ^ | 3V | 2.9 | 3 | | 2.9 | | V |
| ^ | ^ | ^ | 4.5V | 4.4 | 4.5 | | 4.4 | | V |
| ^ | ^ | I~OH~ = -4mA | 3V | 2.58 | | | 2.48 | | V |
| ^ | ^ | I~OH~ = -8mA | 4.5V | 3.94 | | | 3.8 | | V |
| V~OL~ | | I~OL~ = 50μA | 2V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 3V | | | 0.1 | | 0.1 | V |
| ^ | ^ | ^ | 4.5V | | | 0.1 | | 0.1 | V |
| ^ | ^ | I~OL~ = 4mA | 3V | | | 0.36 | | 0.44 | V |
| ^ | ^ | I~OL~ = 8mA | 4.5V | | | 0.36 | | 0.44 | V |
| I~I~ | | V~I~ = 5.5V or GND | 0V to 5.5V | | | ±0.1 | | ±1 | μA |
| I~CC~ | | V~I~ = V~CC~ or GND, I~O~ = 0 A | 5.5V | | | 1 | | 10 | μA |
| C~I~ | | V~I~ = V~CC~ or GND | 5V | | 4 | 10 | | 10 | pF |
```
[[5::table #2]]

```table id=operating caption="Operating characteristics" conditions="V~CC~ = 5V, T~A~ = 25°C"
| Symbol | Parameter | Test conditions | Typ | Unit |
|---|---|---|--:|---|
| C~pd~ | Power dissipation capacitance | No load, f = 1MHz | 18 | pF |
```
[[6::table #1]]

## Switching characteristics

See [parameter measurement information](#parameter-measurement).

```table id=sw-3v3 caption="Switching characteristics, V~CC~ = 3.3V ±0.3V" conditions="Over recommended operating free-air temperature range, V~CC~ = 3.3V ±0.3V, T~A~ = –40°C to 125°C, see Load Circuit and Voltage Waveforms"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 50pF | | 9.5 | 14.5 | 1 | 16.5 | ns |
| t~PHL~ | ^ | ^ | ^ | | 9.5 | 14.5 | 1 | 16.5 | ns |
```
[[5::table #3]]

```table id=sw-5v caption="Switching characteristics, V~CC~ = 5V ±0.5 V" conditions="Over recommended operating free-air temperature range, V~CC~ = 5V ±0.5V, T~A~ = –40°C to 125°C, see Load Circuit and Voltage Waveforms"
| Symbol | From (input) | To (output) | Load capacitance | T~A~ = 25°C | < | < | Min | Max | Unit |
| ^ | ^ | ^ | ^ | Min | Typ | Max | ^ | ^ | ^ |
|---|---|---|---|--:|--:|--:|--:|--:|---|
| t~PLH~ | A or B | Y | C~L~ = 50pF | | 6.3 | 8.8 | 1 | 10 | ns |
| t~PHL~ | ^ | ^ | ^ | | 6.3 | 8.8 | 1 | 10 | ns |
```
[[5::table #4]]

## Typical characteristics

```curve id=tpd-temp caption="TPD vs Temperature"
{"x": {"label": "Temperature", "unit": "°C", "range": [-100, 150]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 6]},
 "series": [
  {"label": "TPD in ns", "points": [[-55, 3.7], [-40, 3.9], [25, 4.5], [85, 4.9], [125, 5.2]]}
 ]}
```
[[6::Figure 5-1]]

```curve id=tpd-vcc caption="TPD vs V~CC~ at 25°C"
{"x": {"label": "V_CC", "unit": "V", "range": [0, 6]},
 "y": {"label": "TPD", "unit": "ns", "range": [0, 8]},
 "series": [
  {"label": "TPD in ns", "points": [[2, 6.9], [3, 5], [4.5, 4.8], [5.5, 4.2]]}
 ]}
```
[[6::Figure 5-2]]

## Parameter measurement information

C~L~ includes probe and jig capacitance. All input pulses are supplied by
generators having the following characteristics: PRR ≤ 1 MHz,
Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns. The outputs are measured one at a
time, with one input transition per measurement. Conversion note: C~L~
is 50pF, as given in the
[switching characteristics](#switching-characteristics). [[7::Figure 6-1]]

```figure ref=load-cmos-50pf caption="Load circuit for totem-pole outputs"
CL: 50pF
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
their design implementation to confirm system functionality. [[10::1]]

The SN74AHC1G86-Q1 is a low-drive CMOS device that can be used for a
multitude of bus interface type applications where output ringing is a
concern. The low drive and slow edge rates will minimize overshoot and
undershoot on the outputs. The inputs can accept voltages to 5.5V at
any valid V~CC~ making it ideal for down translation. [[10::2]]

Conversion note: the typical application schematic (Figure 8-1) shows the gate, supplied from a 3.3-V or 5-V regulated rail with a 0.1 µF capacitor to ground, between 5-V system logic on a 5-V accessory and a µC or system logic.

### Design requirements

This device uses CMOS technology and has balanced output drive. Take
care to avoid bus contention because it can drive currents that would
exceed maximum limits. [[10::3]]

### Detailed design procedure

1. Recommended Input Conditions [[10::4]]
   - For rise time and fall time specifications, see Δt/ΔV in the
     table. [[10::4]]
   - For specified High and low levels, see V~IH~ and V~IL~ in the
     table. [[10::4]]
   - Inputs are overvoltage tolerant allowing them to go as high as
     5.5V at any valid V~CC~. [[10::4]]
2. Recommended Output Conditions [[10::4]]
   - Load currents should not exceed 8mA per output. [[10::4]]
   - Outputs should not be pulled above V~CC~. [[10::4]]

Conversion note: the table meant is the [recommended operating conditions](#recommended-operating-conditions).

### Power supply recommendations

The power supply can be any voltage between the minimum and maximum
supply voltage rating. [[11::1]]

Each V~CC~ pin must have a good bypass capacitor to prevent power
disturbance. For devices with a single supply, 0.1µF is recommended. If
there are multiple V~CC~ pins, 0.01µF or 0.022µF is recommended for
each power pin. It is acceptable to parallel multiple bypass caps to
reject different frequencies of noise. A 0.1µF and 1µF are commonly
used in parallel. The bypass capacitor must be installed as close to
the power pin as possible for best results. [[11::2]]

### Layout guidelines

Even low data rate digital signals can have high frequency signal
components due to fast edge rates. When a PCB trace turns a corner at a
90° angle, a reflection can occur. A reflection occurs primarily
because of the change of width of the trace. At the apex of the turn,
the trace width increases to 1.414 times the width. This increase
upsets the transmission-line characteristics, especially the
distributed capacitance and self–inductance of the trace which results
in the reflection. Not all PCB traces can be straight and therefore
some traces must turn corners. Only the last example (BEST) maintains
constant trace width and minimizes reflections. Conversion note:
Figure 8-3 (trace example: WORST, BETTER, BEST corners) is not drawn
here. [[11::3]]

## Package

```package footprint=SOT23-5
```

## Ordering information

```ordering
SN74AHC1G86QDBVRQ1 | Reel of 3000 | -40 to 125 | 39KH, ACYU
```
[[15::table #1]]

## Conversion notes

- Source pages 1–12 converted. The ordering line is from the package
  option addendum (pp.15–16). The support and revision history page
  (13), the mechanical information page (14), tape and reel and
  mechanical pages (17–22) are left out; the generated Package section
  replaces them.
- Dropped: features and applications lists (features kept as sentences
  in the Description; the AEC-Q100 ESD levels are in the ESD ratings
  table), package information table, the functional block diagram (five
  equivalent exclusive-OR symbols, p.1 and p.8); the clamping-diode
  figure (Figure 7-1), the typical application schematic (Figure 8-1)
  and the trace example (Figure 8-3) are described in text.
- Dropped: Figure 8-2 (switching characteristics comparison, an
  oscilloscope-style plot of V~I~ and the AC08, HC08 and AHC1G86 outputs
  at V~CC~ = 5 V, load 50 Ω/50 pF); it is not a characteristic of this
  part alone.
- The source's cross-references to its tables print as nothing in
  §§ 7.3, 8.2.2, 8.3 and 8.4.1 ("given in the .", "defined the in the
  must be followed", "located in the table", ". shows progressively
  better techniques"); here the words left hanging are deleted, and the
  sentence that starts "shows progressively better techniques of
  rounding corners" is left out.
- The electrical characteristics table of the source gives symbols only,
  so its Parameter cells are empty. Its last Min and Max columns, and
  those of the switching tables, have no group header in the source;
  the switching tables' condition lines give T~A~ = –40°C to 125°C.
- Curves digitized from Figures 5-1 and 5-2 (p.6) to about 2 % of full
  scale; the source states no load or V~CC~ for Figure 5-1 and no load
  for Figure 5-2.
- Figure 6-1 (p.7) is a picture; its notes and values are not in the
  text layer.
- From images: the overbars of Y = !{A}B + A!{B} (p.1 and p.8); the
  curve points -55, 3.7, 3.9, 85, 4.9, 5.2 (Figure 5-1, p.6) and 6.9,
  4.2 (Figure 5-2, p.6); the notes of Figure 6-1 (p.7): "C~L~ includes
  probe and jig capacitance.", "All input pulses are supplied by
  generators having the following characteristics: PRR ≤ 1 MHz,
  Z~O~ = 50 Ω, t~r~ ≤ 3 ns, t~f~ ≤ 3 ns.", "The outputs are measured
  one at a time, with one input transition per measurement."; 50% V~CC~
  (V~M~, Figure 6-1, p.7); AC08, HC08, 50 Ω, 50 pF (Figure 8-2, p.11).
- Source issues:
  - Source error: the I~IK~ row of the absolute maximum ratings is
    printed with the unit V, not mA; kept as printed (absolute maximum
    ratings note 3).
  - Source conflict: "Schmitt-trigger action at all inputs" (Features)
    vs standard CMOS inputs (§ 7.3.2) (Functional description note).
